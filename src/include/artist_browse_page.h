#ifndef SPRING_PLAYER_ARTIST_BROWSE_PAGE_H
#define SPRING_PLAYER_ARTIST_BROWSE_PAGE_H

#include <libspring_global.h>

#include "utility/forward_declarations.h"
#include "utility/gtk_helpers.h"
#include "utility/signals.h"

namespace spring
{
    namespace player
    {
        class ArtistBrowsePage
        {
        public:
            ArtistBrowsePage() noexcept;
            ~ArtistBrowsePage() noexcept;

        public:
            GtkWidget *operator ()() noexcept;

        private:
            utility::GObjectGuard<GtkBox> root_container_{ nullptr };
            GtkImage *artist_thumbnail_{ nullptr };
            GtkBox *right_pane_container_{ nullptr };
            GtkLabel *artist_name_{ nullptr };
            GtkListBox *popular_tracks_listbox_{ nullptr };
            GtkFlowBox *album_list_content_{ nullptr };
            GtkSpinner *album_list_loading_spinner_{ nullptr };

        private:
            DISABLE_COPY(ArtistBrowsePage)
            DISABLE_MOVE(ArtistBrowsePage)
        };
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_ARTIST_BROWSE_PAGE_H
