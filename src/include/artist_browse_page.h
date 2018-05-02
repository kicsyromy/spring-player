#ifndef SPRING_PLAYER_ARTIST_BROWSE_PAGE_H
#define SPRING_PLAYER_ARTIST_BROWSE_PAGE_H

#include <vector>

#include <libspring_global.h>
#include <libspring_music_artist.h>

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
        private:
            class TrackListEntry
            {
            public:
                TrackListEntry(std::size_t index,
                               const utility::string_view &title,
                               music::Track::Milliseconds duration) noexcept;
                ~TrackListEntry() noexcept;

            public:
                GtkWidget *operator()() noexcept;

            private:
                utility::GObjectGuard<GtkBox> track_list_entry_{ nullptr };
                GtkLabel *track_number_{ nullptr };
                GtkLabel *track_title_{ nullptr };
                GtkLabel *track_duration_{ nullptr };

            private:
                DISABLE_COPY(TrackListEntry)
                DISABLE_MOVE(TrackListEntry)
            };

        public:
            ArtistBrowsePage() noexcept;
            ~ArtistBrowsePage() noexcept;

        public:
            void set_artist(const music::Artist &artist) noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            utility::GObjectGuard<GtkBox> root_container_{ nullptr };
            GtkImage *artist_thumbnail_{ nullptr };
            GtkBox *right_pane_container_{ nullptr };
            GtkLabel *artist_name_{ nullptr };
            GtkListBox *popular_tracks_listbox_{ nullptr };
            GtkFlowBox *album_list_content_{ nullptr };
            GtkSpinner *album_list_loading_spinner_{ nullptr };

            std::vector<TrackListEntry> popular_tracks_{};

        private:
            DISABLE_COPY(ArtistBrowsePage)
            DISABLE_MOVE(ArtistBrowsePage)
        };
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_ARTIST_BROWSE_PAGE_H
