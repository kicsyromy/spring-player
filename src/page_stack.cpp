#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "page_stack.h"
#include "page_stack_swicher.h"

#include "artist_browse_page.h"
#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;

PageStack::PageStack(PageStackSwitcher &stack_switcher,
                     std::weak_ptr<PlaybackList> playback_list) noexcept
  : page_stack_{ gtk_cast<GtkStack>(gtk_stack_new()) }
  , albums_page_{ music_library_, playback_list }
  , artists_page_{ music_library_, playback_list }
  , playback_list_(playback_list)
  , track_list_popover_{ playback_list_ }
{
    LOG_INFO("PageStack({}): Creating...", void_p(this));

    gtk_widget_set_visible(gtk_cast<GtkWidget>(page_stack_), true);
    gtk_stack_set_transition_type(page_stack_, GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

    gtk_stack_add_named(page_stack_, albums_page_(), "albums_page");
    gtk_stack_add_named(page_stack_, artists_page_(), "artists_page");

    /* weak_ptr instead of ref */
    //    genres_page_ = std::make_unique<GenresPage>(builder, *music_library_);
    //    songs_page_ = std::make_unique<SongsPage>(builder, *music_library_);

    stack_switcher.on_page_requested(this, &on_page_requested);
    albums_page_.on_thumbnail_activated(this, &on_album_activated);
    artists_page_.on_thumbnail_activated(this, &on_artist_activated);
    artists_page_.set_secondary_content_widget(artist_browse_page_());
}

PageStack::~PageStack() noexcept
{
    LOG_INFO("PageStack({}): Destroying...", void_p(this));
}

void PageStack::set_music_library(MusicLibrary &&library) noexcept
{
    music_library_ = std::make_shared<MusicLibrary>(std::move(library));
    on_page_requested(settings::get_current_page(), this);
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

void PageStack::go_back() noexcept
{
    /* TODO: Check to see that Artists are curently in view */
    artists_page_.switch_to_primary_page();
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

                auto album_widgets = new std::vector<std::unique_ptr<AlbumWidget>>{};

                if (self->music_library_ != nullptr)
                {
                    auto albums = self->music_library_->albums();
                    album_widgets->reserve(albums.size());

                    std::string main_text;
                    std::string secondary_text;

                    for (auto &album : albums)
                    {
                        main_text = album.artist();
                        secondary_text = album.title();

                        album_widgets->push_back(std::make_unique<AlbumWidget>(
                            std::move(album), main_text, secondary_text, "album_artwork",
                            playback_list));
                    }
                }
                else
                {
                    LOG_WARN("PageStack({}): Music library is null, failed to load content",
                             void_p(self));
                }

                return album_widgets;
            });

            break;
        case Page::Artists:
            gtk_stack_set_visible_child(self->page_stack_, artists_page());
            self->artists_page_.activated([self, playback_list]() {
                using ArtistWidget = ThumbnailWidget<music::Artist>;

                auto artists = self->music_library_->artists();
                auto artist_widgets = new std::vector<std::unique_ptr<ArtistWidget>>{};
                artist_widgets->reserve(artists.size());

                std::string main_text;
                std::string secondary_text;

                for (auto &artist : artists)
                {
                    main_text = artist.name();
//                    auto albums = artist.albums();
//                    auto album_count = albums.size();
//                    secondary_text =
//                        fmt::format("{} {}", album_count, album_count > 1 ? "albums" : "album");

                    artist_widgets->push_back(
                        std::make_unique<ArtistWidget>(std::move(artist), main_text, secondary_text,
                                                       "artist_artwork", playback_list));
                }

                return artist_widgets;
            });
            break;
        case Page::Songs:
            //                gtk_stack_set_visible_child(page_stack_, *songs_page_.get());
            //                songs_page_->activated();
            break;
        default:
            break;
    }
}

void PageStack::on_album_activated(ThumbnailWidget<music::Album> *thumbnail,
                                   PageStack *self) noexcept
{
    self->track_list_popover_.show(thumbnail->content_provider(), (*thumbnail)());
}

void PageStack::on_artist_activated(ThumbnailWidget<music::Artist> *thumbnail,
                                    PageStack *self) noexcept
{
    self->artist_browse_page_.set_artist(thumbnail->content_provider());
    self->artists_page_.switch_to_secondary_page();
}
