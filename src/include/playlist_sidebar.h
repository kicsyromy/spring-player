#ifndef SPRING_PLAYER_PLAYLIST_SIDEBAR_H
#define SPRING_PLAYER_PLAYLIST_SIDEBAR_H

#include <memory>
#include <unordered_map>

#include <libspring_global.h>
#include <libspring_music_track.h>

#include "playback_list.h"
#include "thumbnail.h"

#include "utility/forward_declarations.h"
#include "utility/g_object_guard.h"

namespace spring
{
    namespace player
    {
        class Thumbnail;

        class PlaylistSidebar
        {
        public:
            PlaylistSidebar(std::shared_ptr<PlaybackList> playback_list) noexcept;
            ~PlaylistSidebar() noexcept;

        public:
            void show() noexcept;
            void hide() noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            static void toggled(GtkToggleButton *toggle_button, PlaylistSidebar *self) noexcept;

            static void on_track_activated(GtkListBox *,
                                           GtkListBoxRow *element,
                                           PlaylistSidebar *self) noexcept;
            static void on_playback_state_changed(PlaybackList::PlaybackState new_state,
                                                  PlaylistSidebar *self) noexcept;

            static std::int32_t on_list_box_draw_requested(GtkWidget *,
                                                           cairo_t *cairo_context,
                                                           PlaylistSidebar *self) noexcept;

        private:
            class PlaylistItem
            {
            public:
                PlaylistItem(std::shared_ptr<music::Track> &track) noexcept;

            public:
                void set_playing(bool value) noexcept;
                void set_text_color(
                    const std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> &color) noexcept;

            public:
                GtkWidget *operator()() noexcept;

            private:
                utility::GObjectGuard<GtkBox> playlist_item_{ nullptr };
                GtkLabel *title_{ nullptr };
                GtkLabel *duration_{ nullptr };
                GtkImage *playing_icon_{ nullptr };

                std::shared_ptr<music::Track> track_;
            };

        private:
            utility::GObjectGuard<GtkBox> playlist_sidebar_{ nullptr };
            GtkContainer *artwork_container_{ nullptr };
            GtkListBox *track_list_container_{ nullptr };

            Thumbnail artwork_{};

            std::weak_ptr<PlaybackList> playback_list_{};
            std::unordered_map<GtkWidget *, std::unique_ptr<PlaylistItem>> playlist_{};
            PlaylistItem *current_item_{ nullptr };

        private:
            DISABLE_COPY(PlaylistSidebar)
            DISABLE_MOVE(PlaylistSidebar)
        };
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_PLAYLIST_SIDEBAR_H
