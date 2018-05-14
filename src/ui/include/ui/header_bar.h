#ifndef SPRING_PLAYER_HEADER_BAR_H
#define SPRING_PLAYER_HEADER_BAR_H

#include <memory>
#include <string>

#include <libspring_global.h>

#include "utility/forward_declarations.h"
#include "utility/g_object_guard.h"
#include "utility/signals.h"

namespace spring
{
    namespace player
    {
        class PlaybackList;

        class HeaderBar
        {
        public:
            HeaderBar(std::shared_ptr<PlaybackList> playback_list) noexcept;
            ~HeaderBar() noexcept;

        public:
            void toggle_sidebar() noexcept;
            void toggle_search() noexcept;
            void hide_controls() noexcept;
            void show_controls() noexcept;

        public:
            signal(search_toggled, bool);
            signal(playlist_toggled, bool);
            signal(back_requested);

        public:
            GtkWidget *operator()() noexcept;

        private:
            static void on_search_button_toggled(GtkToggleButton *search_button,
                                                 HeaderBar *self) noexcept;
            static void on_sidebar_toggled(GtkToggleButton *playlist_button,
                                           HeaderBar *self) noexcept;

            static void on_back_button_clicked(GtkButton *button, HeaderBar *self) noexcept;

            static void on_play_pause_button_clicked(GtkButton *button, HeaderBar *self) noexcept;
            static void on_next_button_clicked(GtkButton *button, HeaderBar *self) noexcept;
            static void on_previous_button_clicked(GtkButton *button, HeaderBar *self) noexcept;

            static bool on_drag_started(GtkWidget *, GdkEvent *, HeaderBar *self) noexcept;
            static bool on_drag_ended(GtkWidget *, GdkEvent *, HeaderBar *self) noexcept;
            static bool on_seek_requested(GtkScale *,
                                          GtkScrollType,
                                          double value,
                                          HeaderBar *self) noexcept;

            static void on_playback_state_changed(std::int32_t state, HeaderBar *self) noexcept;
            static void on_playback_position_changed(std::int64_t position,
                                                     HeaderBar *self) noexcept;

        private:
            utility::GObjectGuard<GtkHeaderBar> header_bar_{ nullptr };

            GtkBox *tool_buttons_{ nullptr };
            GtkButton *back_button_{ nullptr };
            GtkToggleButton *toggle_sidebar_button_{ nullptr };

            GtkBox *playback_progress_layout_{ nullptr };
            GtkButtonBox *playback_controls_{ nullptr };
            GtkButton *previous_button_{ nullptr };
            GtkButton *play_pause_button_{ nullptr };
            GtkImage *play_pause_button_icon_{ nullptr };
            GtkButton *next_button_{ nullptr };

            GtkBox *title_and_progress_{ nullptr };
            GtkLabel *window_title_{ nullptr };
            GtkLabel *current_time_{ nullptr };
            GtkLabel *duration_{ nullptr };
            GtkScale *playback_progress_bar_{ nullptr };
            GtkAdjustment *playback_progress_adjustment_{ nullptr };

            GtkToggleButton *search_button_{ nullptr };

            bool dragging_{ false };
            double seek_value_{ 0.0 };

        private:
            std::weak_ptr<PlaybackList> playback_list_{};

        private:
            DISABLE_COPY(HeaderBar)
            DISABLE_MOVE(HeaderBar)
        };
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_HEADER_BAR_H
