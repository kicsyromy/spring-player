#ifndef SPRING_PLAYER_PAGE_STACK_H
#define SPRING_PLAYER_PAGE_STACK_H

#include <memory>

#include <libspring_global.h>
#include <libspring_music_library.h>

#include "ui/artist_browse_page.h"
#include "ui/thumbnail_page.h"
#include "ui/track_list_popover.h"
/* TODO: Forward declare these */
#include "ui/songs_page.h"

#include "utility/forward_declarations.h"
#include "utility/g_object_guard.h"
#include "utility/settings.h"

namespace spring
{
    namespace player
    {
        namespace playback
        {
            class Playlist;
        }

        namespace ui
        {
            class PageStackSwitcher;

            class PageStack
            {
            public:
                using Page = settings::Page;

            public:
                PageStack(PageStackSwitcher &stack_switcher,
                          std::weak_ptr<playback::Playlist> playback_list) noexcept;
                ~PageStack() noexcept;

            public:
                void set_music_library(MusicLibrary &&library) noexcept;

            public:
                void filter_current_page(std::string &&text) noexcept;
                void go_back() noexcept;

            public:
                GtkWidget *operator()() noexcept;

            private:
                static void on_page_requested(Page page, PageStack *self) noexcept;
                static void on_album_activated(ThumbnailWidget<music::Album> *thumbnail,
                                               PageStack *self) noexcept;
                static void on_artist_activated(ThumbnailWidget<music::Artist> *thumbnail,
                                                PageStack *self) noexcept;

            private:
                utility::GObjectGuard<GtkStack> page_stack_{ nullptr };
                std::unique_ptr<PageStackSwitcher> page_stack_switcher_{ nullptr };

                std::shared_ptr<MusicLibrary> music_library_{};

                ThumbnailPage<music::Album> albums_page_;
                ThumbnailPage<music::Artist> artists_page_;
                std::unique_ptr<SongsPage> songs_page_{ nullptr };

                std::weak_ptr<playback::Playlist> playback_list_{};

                TrackListPopover track_list_popover_{ playback_list_ };
                ArtistBrowsePage artist_browse_page_{ playback_list_ };

            private:
                DISABLE_COPY(PageStack)
                DISABLE_MOVE(PageStack)
            };
        } // namespace ui
    }     // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_PAGE_STACK_H
