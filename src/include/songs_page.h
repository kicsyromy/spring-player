#ifndef SPRING_PLAYER_SONGS_PAGE_H
#define SPRING_PLAYER_SONGS_PAGE_H

#include <gtk/gtk.h>

#include <libspring_music_library.h>

namespace spring
{
    namespace player
    {
        class SongsPage
        {
        public:
            SongsPage(GtkBuilder *builder,
                      const MusicLibrary &music_library) noexcept;
            ~SongsPage() noexcept = default;

        public:
            void activated() noexcept;

        public:
            operator GtkWidget *() noexcept;

        private:
            GtkScrolledWindow *songs_page_{ nullptr };
            GtkFlowBox *songs_content_{ nullptr };

            const MusicLibrary &music_library_;

        private:
            DISABLE_COPY(SongsPage)
            DISABLE_MOVE(SongsPage)
        };
    }
}

#endif // !SPRING_PLAYER_SONGS_PAGE_H
