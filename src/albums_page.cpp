#include "albums_page.h"

#include <thread>

#include <libspring_logger.h>

#include "async_queue.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

AlbumsPage::AlbumsPage(GtkBuilder *builder,
                       std::weak_ptr<MusicLibrary> music_library,
                       std::weak_ptr<PlaybackList> playback_list) noexcept
  : music_library_(music_library)
  , playback_list_(playback_list)
{
    LOG_INFO("AlbumsPage({}): Creating...", void_p(this));

    get_widget_from_builder_simple(albums_page);
    get_widget_from_builder_simple(albums_content);
    get_widget_from_builder_simple(albums_loading_spinner);
    get_widget_from_builder_simple(search_entry);

    gtk_flow_box_set_filter_func(albums_content_, &AlbumsPage::filter,
                                 reinterpret_cast<void *>(this), [](void *) {});

    connect_g_signal(albums_content_, "child-activated", &on_child_activated, this);
    connect_g_signal(search_entry_, "search-changed", &on_search_changed, this);
}

void AlbumsPage::activated() noexcept
{
    LOG_INFO("AlbumsPage({}): Activated", void_p(this));

    if (albums_.empty())
    {
        gtk_spinner_start(albums_loading_spinner_);

        async_queue::push_front_request(new async_queue::Request{
            "load_albums", [this]() {
                /* TODOD: There is a crash at start-up if we start fetching   */
                /*        album data immediately so we add an artificial delay*/
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                LOG_INFO("AlbumsPage({}): Loading content...", void_p(this));

                auto music_library = music_library_.lock();

                if (music_library != nullptr)
                {
                    auto album_widgets = new std::vector<std::unique_ptr<AlbumWidget>>{};
                    auto albums = music_library->albums();

                    for (auto &album : albums)
                    {
                        album_widgets->push_back(
                            std::make_unique<AlbumWidget>(std::move(album), playback_list_));
                    }

                    async_queue::post_response(new async_queue::Response{
                        "albums_ready", [this, album_widgets]() {
                            LOG_INFO("AlbumsPage({}): Content ready, populating "
                                     "GtkFlowBox",
                                     void_p(this));

                            albums_ = std::move(*album_widgets);
                            delete album_widgets;

                            for (auto &album : albums_)
                            {
                                gtk_flow_box_insert(albums_content_, *album, -1);
                            }

                            gtk_spinner_stop(albums_loading_spinner_);
                        } });
                }
            } });
    }
}

gboolean AlbumsPage::filter(GtkFlowBoxChild *element, void *instance) noexcept
{
    auto self = static_cast<AlbumsPage *>(instance);
    const auto searched_text =
        std::string{ gtk_entry_get_text(gtk_cast<GtkEntry>(self->search_entry_)) };

    LOG_INFO("AlbumsPage({}): Applying filter for string {}", void_p(self), searched_text);

    const auto &album_title =
        self->albums_.at(static_cast<std::size_t>(gtk_flow_box_child_get_index(element)))->title();
    const auto &artist_name =
        self->albums_.at(static_cast<std::size_t>(gtk_flow_box_child_get_index(element)))->artist();

    const bool is_near_title_match{ utility::fuzzy_match(searched_text, album_title, 2) };
    const bool is_near_artist_match{ utility::fuzzy_match(searched_text, artist_name, 2) };

    return searched_text.empty() || is_near_title_match || is_near_artist_match;
}

void AlbumsPage::on_search_changed(GtkSearchEntry *, AlbumsPage *self) noexcept
{
    LOG_INFO("AlbumsPage({}): Search string changed, resetting filter", void_p(self));

    if (gtk_widget_get_visible(gtk_cast<GtkWidget>(self->albums_page_)))
    {
        gtk_flow_box_invalidate_filter(self->albums_content_);
    }
}

void AlbumsPage::on_child_activated(GtkFlowBox *,
                                    GtkFlowBoxChild *element,
                                    AlbumsPage *self) noexcept
{
    auto index = static_cast<std::size_t>(gtk_flow_box_child_get_index(element));

    LOG_INFO("AlbumsPage({}): Activated album at index {}", void_p(self), index);

    self->albums_.at(index)->activated();
}

AlbumsPage::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(albums_page_);
}
