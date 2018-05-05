#ifndef SPRING_PLAYER_ARTIST_BROWSE_PAGE_H
#define SPRING_PLAYER_ARTIST_BROWSE_PAGE_H

#include <memory>
#include <vector>

#include <libspring_global.h>
#include <libspring_music_artist.h>

#include "thumbnail_widget.h"

#include "utility/compatibility.h"
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
            void set_artist(const music::Artist &artist) noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            void clear_all() noexcept;

        private:
            std::pair<std::vector<music::Track> *, std::vector<utility::GObjectGuard<GtkBox>> *>
            load_popular_tracks(const music::Artist &artist) const noexcept;

            void on_tracks_loaded(
                std::vector<music::Track> *tracks,
                std::vector<utility::GObjectGuard<GtkBox>> *track_widgets) noexcept;

            std::vector<ThumbnailWidget<music::Album>> *load_albums(
                const music::Artist &artist) const noexcept;

            void on_albums_loaded(
                std::vector<ThumbnailWidget<music::Album>> *album_widgets) noexcept;

        private:
            utility::GObjectGuard<GtkBox> root_container_{ nullptr };
            GtkImage *artist_thumbnail_{ nullptr };
            GtkBox *right_pane_container_{ nullptr };
            GtkLabel *artist_name_{ nullptr };
            GtkSpinner *popular_tracks_loading_spinner_{ nullptr };
            GtkListBox *popular_tracks_listbox_{ nullptr };
            GtkFlowBox *album_list_content_{ nullptr };
            GtkSpinner *album_list_loading_spinner_{ nullptr };

            std::vector<std::shared_ptr<music::Track>> popular_tracks_{};
            std::vector<ThumbnailWidget<music::Album>> album_thumbnails_{};

        private:
            DISABLE_COPY(ArtistBrowsePage)
            DISABLE_MOVE(ArtistBrowsePage)
        };
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_ARTIST_BROWSE_PAGE_H
