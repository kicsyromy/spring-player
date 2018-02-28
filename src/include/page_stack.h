#ifndef SPRING_PLAYER_PAGE_STACK_H
#define SPRING_PLAYER_PAGE_STACK_H

#include <memory>

#include <gtk/gtk.h>

#include <libspring_global.h>
#include <libspring_music_library.h>

#include "application_settings.h"

#include "page_stack_swicher.h"

#include "albums_page.h"
#include "artists_page.h"
#include "genres_page.h"
#include "playback_list.h"
#include "songs_page.h"

namespace spring
{
    namespace player
    {
        class PageStack
        {
        public:
            using Page = settings::Page;

        public:
            PageStack(GtkBuilder *builder,
                      MusicLibrary &&music_library,
                      std::weak_ptr<PlaybackList> playback_list) noexcept;
            ~PageStack() noexcept;

        private:
            GtkStack *page_stack_{ nullptr };
            std::unique_ptr<PageStackSwitcher> page_stack_switcher_{ nullptr };
            std::unique_ptr<AlbumsPage> albums_page_{ nullptr };
            std::unique_ptr<ArtistsPage> artists_page_{ nullptr };
            std::unique_ptr<GenresPage> genres_page_{ nullptr };
            std::unique_ptr<SongsPage> songs_page_{ nullptr };

            std::shared_ptr<MusicLibrary> music_library_{ new MusicLibrary{
                nullptr } };

        private:
            DISABLE_COPY(PageStack)
            DISABLE_MOVE(PageStack)
        };
    }
}

#endif // !SPRING_PLAYER_PAGE_STACK_H
