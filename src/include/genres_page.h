#ifndef SPRING_PLAYER_GENRES_PAGE_H
#define SPRING_PLAYER_GENRES_PAGE_H

#include <libspring_music_library.h>

#include <utility/forward_declarations.h>

namespace spring
{
    namespace player
    {
        class GenresPage
        {
        public:
            GenresPage(GtkBuilder *builder,
                       /* weak_ptr instead of ref */
                       const MusicLibrary &music_library) noexcept;
            ~GenresPage() noexcept;

        public:
            void activated() noexcept;

        public:
            operator GtkWidget *() noexcept;

        private:
            GtkScrolledWindow *genres_page_{ nullptr };
            GtkFlowBox *genres_content_{ nullptr };

            const MusicLibrary &music_library_;

        private:
            DISABLE_COPY(GenresPage)
            DISABLE_MOVE(GenresPage)
        };
    }
}

#endif // !SPRING_PLAYER_GENRES_PAGE_H
