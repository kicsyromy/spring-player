#ifndef SPRING_PLAYER_ARTIST_WIDGET_H
#define SPRING_PLAYER_ARTIST_WIDGET_H

#include <memory>
#include <utility>

#include <gtk/gtk.h>

#include <libspring_global.h>
#include <libspring_music_artist.h>

#include "playback_list.h"
#include "utility.h"

namespace spring
{
    namespace player
    {
        class ArtistWidget
        {
        public:
            ArtistWidget(music::Artist &&artist,
                         std::weak_ptr<PlaybackList> playback_list) noexcept;
            ~ArtistWidget() noexcept;

        public:
            void activated() noexcept;

        private:
            std::pair<std::vector<music::Track> *, std::vector<utility::GtkRefGuard<GtkBox>> *>
            load_tracks() const noexcept;

            void on_tracks_loaded(
                std::vector<music::Track> *tracks,
                std::vector<utility::GtkRefGuard<GtkBox>> *track_widgets) noexcept;

        private:
            static void on_track_activated(GtkListBox *list_box,
                                           GtkListBoxRow *element,
                                           ArtistWidget *self) noexcept;
            static void on_popover_closed(GtkPopover *popover, ArtistWidget *self) noexcept;

        private:
            utility::GtkRefGuard<GtkBox> artist_widget_{ nullptr };
            GtkImage *thumbnail_{ nullptr };
            GtkLabel *name_{ nullptr };
            GtkLabel *album_count_{ nullptr };

            utility::GtkRefGuard<GtkPopover> track_list_popover_{ nullptr };
            GtkListBox *track_list_{ nullptr };
            GtkSpinner *tracks_loading_spinner_{ nullptr };

            music::Artist artist_{ nullptr };
            std::vector<std::shared_ptr<music::Track>> tracks_{};

            std::weak_ptr<PlaybackList> playback_list_{};

        public:
            GtkWidget *container() noexcept;

        private:
            DISABLE_COPY(ArtistWidget)
            DISABLE_MOVE(ArtistWidget)
        };
    }
}

#endif // !SPRING_PLAYER_ARTIST_WIDGET_H
