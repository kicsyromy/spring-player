#ifndef SPRING_PLAYER_ARTISTS_PAGE_H
#define SPRING_PLAYER_ARTISTS_PAGE_H

#include <memory>

#include <gtk/gtk.h>

#include <libspring_music_library.h>

#include "artist_widget.h"

namespace spring
{
    namespace player
    {
        class ArtistsPage
        {
        public:
            ArtistsPage(
                GtkBuilder *builder,
                std::weak_ptr<const MusicLibrary> music_library) noexcept;
            ~ArtistsPage() noexcept = default;

        public:
            void activated() noexcept;

        public:
            operator GtkWidget *() noexcept;

        private:
            GtkScrolledWindow *artists_page_{ nullptr };
            GtkFlowBox *artists_content_{ nullptr };
            GtkSpinner *artists_loading_spinner_{ nullptr };

            std::weak_ptr<const MusicLibrary> music_library_{};
            std::vector<std::unique_ptr<ArtistWidget>> artists_{};

        private:
            DISABLE_COPY(ArtistsPage)
            DISABLE_MOVE(ArtistsPage)
        };
    }
}

#endif // !SPRING_PLAYER_ARTISTS_PAGE_H
