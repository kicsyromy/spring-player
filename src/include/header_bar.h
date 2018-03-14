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
            enum class Text
            {
                Plain,
                Markup
            };

        public:
            HeaderBar(std::shared_ptr<PlaybackList> playback_list) noexcept;
            ~HeaderBar() noexcept;

        public:
            void set_title(const std::string &text, Text text_type) noexcept;

        public:
            signal(search_toggled, bool);
            signal(playlist_toggled, bool);

        public:
            GtkWidget *operator()() noexcept;

        private:
            static void on_search_toggled(GtkToggleButton *search_button, HeaderBar *self) noexcept;
            static void on_sidebar_toggled(GtkToggleButton *playlist_button,
                                           HeaderBar *self) noexcept;

            static void on_play_pause_button_clicked(GtkButton *button, HeaderBar *self) noexcept;
            static void on_next_button_clicked(GtkButton *button, HeaderBar *self) noexcept;
            static void on_previous_button_clicked(GtkButton *button, HeaderBar *self) noexcept;

        private:
            utility::GObjectGuard<GtkHeaderBar> header_bar_{ nullptr };

            GtkBox *tool_buttons_{ nullptr };
            GtkToggleButton *toggle_sidebar_button_{ nullptr };

            GtkButtonBox *playback_controls_{ nullptr };
            GtkButton *previous_button_{ nullptr };
            GtkButton *play_pause_button_{ nullptr };
            GtkButton *next_button_{ nullptr };

            GtkBox *title_and_progress_{ nullptr };
            GtkLabel *window_title_{ nullptr };
            GtkLabel *current_time_{ nullptr };
            GtkLabel *duration_{ nullptr };
            GtkScale *playback_progress_bar_{ nullptr };
            GtkAdjustment *playback_progress_adjustment_{ nullptr };

            GtkToggleButton *search_button_{ nullptr };

        private:
            std::weak_ptr<PlaybackList> playback_list_{};

        private:
            DISABLE_COPY(HeaderBar)
            DISABLE_MOVE(HeaderBar)
        };
    }
}

#endif // !SPRING_PLAYER_HEADER_BAR_H
