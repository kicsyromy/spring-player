#ifndef SPRING_PLAYER_PAGE_STACK_H
#define SPRING_PLAYER_PAGE_STACK_H

#include <memory>

#include <libspring_global.h>
#include <libspring_music_library.h>

#include "application_settings.h"

#include "thumbnail_page.h"
/* TODO: Forward declare these */
#include "page_stack_swicher.h"
#include "songs_page.h"

#include "utility/forward_declarations.h"
#include "utility/g_object_guard.h"

namespace spring
{
    namespace player
    {
        class PlaybackList;

        class PageStack
        {
        public:
            using Page = settings::Page;

        public:
            PageStack(MusicLibrary &&music_library,
                      std::weak_ptr<PlaybackList> playback_list) noexcept;
            ~PageStack() noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            utility::GObjectGuard<GtkStack> page_stack_{ nullptr };
            std::unique_ptr<PageStackSwitcher> page_stack_switcher_{ nullptr };

            std::shared_ptr<MusicLibrary> music_library_{ new MusicLibrary{ nullptr } };

            std::unique_ptr<ThumbnailPage<music::Album>> albums_page_{ nullptr };
            std::unique_ptr<ThumbnailPage<music::Artist>> artists_page_{ nullptr };

            std::unique_ptr<SongsPage> songs_page_{ nullptr };

        private:
            DISABLE_COPY(PageStack)
            DISABLE_MOVE(PageStack)
        };
    }
}

#endif // !SPRING_PLAYER_PAGE_STACK_H
