#ifndef SPRING_PLAYER_PLAYLIST_SIDEBAR_H
#define SPRING_PLAYER_PLAYLIST_SIDEBAR_H

#include <memory>
#include <vector>

#include <gtk/gtk.h>

#include <libspring_global.h>
#include <libspring_music_track.h>

#include "playback_list.h"
#include "thumbnail.h"

namespace spring
{
    namespace player
    {
        class Thumbnail;

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

            static std::int32_t on_list_bow_draw_requested(GtkWidget *,
                                                           cairo_t *cairo_context,
                                                           PlaylistSidebar *self) noexcept;

        private:
            GtkBox *playlist_sidebar_{ nullptr };
            GtkContainer *artwork_container_{ nullptr };
            GtkListBox *playback_list_box_{ nullptr };
            GtkToggleButton *toggle_sidebar_button_{ nullptr };

            Thumbnail artwork_{};

            std::weak_ptr<PlaybackList> playback_list_{};
            std::vector<const music::Track *> playlist_{};

        private:
            DISABLE_COPY(PlaylistSidebar)
            DISABLE_MOVE(PlaylistSidebar)
        };
    }
}

#endif // !SPRING_PLAYER_PLAYLIST_SIDEBAR_H
