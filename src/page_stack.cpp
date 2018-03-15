#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "page_stack.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;

PageStack::PageStack(MusicLibrary &&music_library,
                     std::weak_ptr<PlaybackList> playback_list) noexcept
  : page_stack_(gtk_cast<GtkStack>(gtk_stack_new()))
{
    LOG_INFO("PageStack({}): Creating...", void_p(this));

    gtk_widget_set_visible(gtk_cast<GtkWidget>(page_stack_), true);

    *music_library_ = std::move(music_library);

    albums_page_ = std::make_unique<ThumbnailPage<music::Album>>(music_library_, playback_list);
    artists_page_ = std::make_unique<ThumbnailPage<music::Artist>>(music_library_, playback_list);

    auto &albums_page = *albums_page_;
    auto &artists_page = *artists_page_;

    gtk_stack_add_named(page_stack_, albums_page(), "albums_page");
    gtk_stack_add_named(page_stack_, artists_page(), "artists_page");

    auto switch_page = [this, playback_list](Page page) {
        auto &albums_page = *albums_page_;
        auto &artists_page = *artists_page_;

        switch (page)
        {
            case Page::Albums:
                gtk_stack_set_visible_child(page_stack_, albums_page());
                albums_page_->activated([this, playback_list]() {
                    using AlbumWidget = ThumbnailWidget<music::Album>;

                    auto albums = music_library_->albums();
                    auto album_widgets = new std::vector<std::unique_ptr<AlbumWidget>>{};
                    album_widgets->reserve(albums.size());

                    std::string main_text;
                    std::string secondary_text;

                    for (auto &album : albums)
                    {
                        main_text = album.artist();
                        secondary_text = album.title();

                        album_widgets->push_back(std::make_unique<AlbumWidget>(
                            std::move(album), main_text, secondary_text, playback_list));
                    }

                    return album_widgets;
                });

                break;
            case Page::Artists:
                gtk_stack_set_visible_child(page_stack_, artists_page());
                //                artists_page_->activated();
                break;
            case Page::Genres:
            case Page::Songs:
                //                gtk_stack_set_visible_child(page_stack_, *songs_page_.get());
                //                songs_page_->activated();
                break;
            default:
                break;
        }
    };

    /* weak_ptr instead of ref */
    //    genres_page_ = std::make_unique<GenresPage>(builder, *music_library_);
    //    songs_page_ = std::make_unique<SongsPage>(builder, *music_library_);

    //    switch_page(settings::get_current_page());
    switch_page(settings::Page::Albums);

    //    page_stack_switcher_ = std::make_unique<PageStackSwitcher>(builder, switch_page);
}

PageStack::~PageStack() noexcept
{
    LOG_INFO("PageStack({}): Destroying...", void_p(this));
}

GtkWidget *PageStack::operator()() noexcept
{
    return gtk_cast<GtkWidget>(page_stack_);
}
