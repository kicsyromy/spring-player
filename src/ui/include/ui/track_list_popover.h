#ifndef SPRING_PLAYER_TRACK_LIST_POPOVER_H
#define SPRING_PLAYER_TRACK_LIST_POPOVER_H

#include <memory>
#include <vector>

#include <libspring_global.h>
#include <libspring_music_album.h>
#include <libspring_music_track.h>

#include "utility/forward_declarations.h"
#include "utility/g_object_guard.h"
#include "utility/signals.h"

namespace spring
{
    namespace player
    {
        class PlaybackList;

        class TrackListPopover
        {
        public:
            TrackListPopover(std::weak_ptr<PlaybackList> playback_list) noexcept;
            ~TrackListPopover() noexcept;

        public:
            void show(const music::Album &album, GtkWidget *relative_to) noexcept;

        public:
            signal(closed, TrackListPopover *);

        public:
            GtkWidget *operator()() noexcept;

        private:
            std::pair<std::vector<music::Track> *, std::vector<utility::GObjectGuard<GtkBox>> *>
            load_tracks(const music::Album &album) const noexcept;

            void on_tracks_loaded(
                std::vector<music::Track> *tracks,
                std::vector<utility::GObjectGuard<GtkBox>> *track_widgets) noexcept;

        private:
            static void on_track_activated(GtkListBox *list_box,
                                           GtkListBoxRow *element,
                                           TrackListPopover *self) noexcept;
            static void on_popover_closed(GtkPopover *popover, TrackListPopover *self) noexcept;
            static void on_enqueue_requested(GtkButton *enqueue_button,
                                             TrackListPopover *self) noexcept;

        private:
            utility::GObjectGuard<GtkPopover> listbox_popover_{ nullptr };
            GtkButton *enqueue_button_{ nullptr };
            GtkListBox *listbox_{ nullptr };
            GtkSpinner *loading_spinner_{ nullptr };

            std::vector<std::shared_ptr<music::Track>> tracks_{};

            std::weak_ptr<PlaybackList> playback_list_{};

        private:
            DISABLE_COPY(TrackListPopover)
            DISABLE_MOVE(TrackListPopover)
        };
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_TRACK_LIST_POPOVER_H
