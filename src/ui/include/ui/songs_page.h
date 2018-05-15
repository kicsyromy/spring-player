#ifndef SPRING_PLAYER_SONGS_PAGE_H
#define SPRING_PLAYER_SONGS_PAGE_H

#include <libspring_music_library.h>

#include "utility/forward_declarations.h"

namespace spring
{
    namespace player
    {
        namespace ui
        {
            class SongsPage
            {
            public:
                SongsPage(GtkBuilder *builder,
                          /* weak_ptr instead of ref */
                          const MusicLibrary &music_library) noexcept;
                ~SongsPage() noexcept;

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
        } // namespace ui
    }     // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_SONGS_PAGE_H
