#ifndef SPRING_PLAYER_ARTISTS_PAGE_H
#define SPRING_PLAYER_ARTISTS_PAGE_H

#include <gtk/gtk.h>

#include <libspring_music_library.h>

namespace spring
{
    namespace player
    {
        class ArtistsPage
        {
        public:
            ArtistsPage(GtkBuilder *builder,
                        /* weak_ptr instead of ref */
                        const MusicLibrary &music_library) noexcept;
            ~ArtistsPage() noexcept = default;

        public:
            void activated() noexcept;

        public:
            operator GtkWidget *() noexcept;

        private:
            GtkScrolledWindow *artists_page_{ nullptr };
            GtkFlowBox *artists_content_{ nullptr };

            const MusicLibrary &music_library_;

        private:
            DISABLE_COPY(ArtistsPage)
            DISABLE_MOVE(ArtistsPage)
        };
    }
}

#endif // !SPRING_PLAYER_ARTISTS_PAGE_H
