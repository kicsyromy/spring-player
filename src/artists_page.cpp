#include "artists_page.h"

#include <thread>

#include <libspring_logger.h>

#include "async_queue.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

ArtistsPage::ArtistsPage(
    GtkBuilder *builder,
    std::weak_ptr<const MusicLibrary> music_library) noexcept
  : music_library_(music_library)
{
    LOG_INFO("ArtistsPage({}): Creating...", void_p(this));

    get_widget_from_builder_simple(artists_page);
    get_widget_from_builder_simple(artists_content);
    get_widget_from_builder_simple(artists_loading_spinner);
}

void ArtistsPage::activated() noexcept
{
    LOG_INFO("ArtistsPage({}): Activated", void_p(this));

    if (artists_.empty())
    {
        gtk_spinner_start(artists_loading_spinner_);

        async_queue::push_front_request(new async_queue::Request{
            "load_albums", [this]() {
                /* TODOD: There is a crash at start-up if we start fetching   */
                /*        artist data immediately so we add an artificial delay*/
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                LOG_INFO("AlbumsPage({}): Loading content...", void_p(this));

                auto music_library = music_library_.lock();

                if (music_library != nullptr)
                {
                    auto artist_widgets =
                        new std::vector<std::unique_ptr<ArtistWidget>>{};
                    auto artists = music_library->artists();

                    for (auto &artist : artists)
                    {
                        artist_widgets->push_back(
                            std::make_unique<ArtistWidget>(std::move(artist)));
                    }

                    async_queue::post_response(new async_queue::Response{
                        "albums_ready", [this, artist_widgets]() {
                            LOG_INFO(
                                "AlbumsPage({}): Content ready, populating "
                                "GtkFlowBox",
                                void_p(this));

                            artists_ = std::move(*artist_widgets);
                            delete artist_widgets;

                            for (auto &artist : artists_)
                            {
                                gtk_flow_box_insert(artists_content_,
                                                    artist->container(), -1);
                            }

                            gtk_spinner_stop(artists_loading_spinner_);
                        } });
                }
            } });
    }
}

ArtistsPage::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(artists_page_);
}
