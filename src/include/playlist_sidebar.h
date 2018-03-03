#ifndef SPRING_PLAYER_PLAYLIST_SIDEBAR_H
#define SPRING_PLAYER_PLAYLIST_SIDEBAR_H

#include <memory>
#include <vector>

#include <gtk/gtk.h>

#include <libspring_global.h>
#include <libspring_music_track.h>

#include "playback_list.h"

namespace spring
{
    namespace player
    {
        class PlaylistSidebar
        {
        public:
            PlaylistSidebar(GtkBuilder *builder,
                            std::shared_ptr<PlaybackList> playback_list) noexcept;
            ~PlaylistSidebar() noexcept;

        public:
            void show() noexcept;
            void hide() noexcept;

        private:
            static void toggled(GtkToggleButton *toggle_button, PlaylistSidebar *self) noexcept;

            static void on_track_activated(GtkListBox *,
                                           GtkListBoxRow *element,
                                           PlaylistSidebar *self) noexcept;

        private:
            GtkBox *playlist_sidebar_{ nullptr };
            GtkImage *current_track_cover_{ nullptr };
            GtkListBox *playback_list_box_{ nullptr };
            GtkToggleButton *toggle_sidebar_button_{ nullptr };

            std::weak_ptr<PlaybackList> playback_list_{};
            std::vector<const music::Track *> playlist_{};

        private:
            DISABLE_COPY(PlaylistSidebar)
            DISABLE_MOVE(PlaylistSidebar)
        };
    }
}

#endif // !SPRING_PLAYER_PLAYLIST_SIDEBAR_H
