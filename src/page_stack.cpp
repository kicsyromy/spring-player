#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "page_stack.h"
#include "page_stack_swicher.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;

PageStack::PageStack(MusicLibrary &&music_library,
                     PageStackSwitcher &stack_switcher,
                     std::weak_ptr<PlaybackList> playback_list) noexcept
  : page_stack_{ gtk_cast<GtkStack>(gtk_stack_new()) }
  , music_library_{ new MusicLibrary{ std::move(music_library) } }
  , albums_page_{ music_library_, playback_list }
  , artists_page_{ music_library_, playback_list }
  , playback_list_(playback_list)
{
    LOG_INFO("PageStack({}): Creating...", void_p(this));

    gtk_widget_set_visible(gtk_cast<GtkWidget>(page_stack_), true);
    gtk_stack_set_transition_type(page_stack_, GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

    gtk_stack_add_named(page_stack_, albums_page_(), "albums_page");
    gtk_stack_add_named(page_stack_, artists_page_(), "artists_page");

    /* weak_ptr instead of ref */
    //    genres_page_ = std::make_unique<GenresPage>(builder, *music_library_);
    //    songs_page_ = std::make_unique<SongsPage>(builder, *music_library_);

    on_page_requested(settings::get_current_page(), this);
    stack_switcher.on_page_requested(this, &on_page_requested);
}

PageStack::~PageStack() noexcept
{
    LOG_INFO("PageStack({}): Destroying...", void_p(this));
}

void PageStack::filter_current_page(std::string &&text) noexcept
{
    switch (settings::get_current_page())
    {
        default:
            break;
        case Page::Albums:
            albums_page_.filter(std::move(text));
            break;
    }
}

GtkWidget *PageStack::operator()() noexcept
{
    return gtk_cast<GtkWidget>(page_stack_);
}

void PageStack::on_page_requested(PageStack::Page page, PageStack *self) noexcept
{
    auto &albums_page = self->albums_page_;
    auto &artists_page = self->artists_page_;
    auto playback_list = self->playback_list_;

    switch (page)
    {
        case Page::Albums:
            gtk_stack_set_visible_child(self->page_stack_, albums_page());
            self->albums_page_.activated([self, playback_list]() {
                using AlbumWidget = ThumbnailWidget<music::Album>;

                auto albums = self->music_library_->albums();
                auto album_widgets = new std::vector<std::unique_ptr<AlbumWidget>>{};
                album_widgets->reserve(albums.size());

                std::string main_text;
                std::string secondary_text;

                for (auto &album : albums)
                {
                    main_text = album.artist();
                    secondary_text = album.title();

                    album_widgets->push_back(
                        std::make_unique<AlbumWidget>(std::move(album), main_text, secondary_text,
                                                      "album_artwork", playback_list));
                }

                return album_widgets;
            });

            break;
        case Page::Artists:
            gtk_stack_set_visible_child(self->page_stack_, artists_page());
            //                artists_page_->activated();
            break;
        case Page::Songs:
            //                gtk_stack_set_visible_child(page_stack_, *songs_page_.get());
            //                songs_page_->activated();
            break;
        default:
            break;
    }
}
