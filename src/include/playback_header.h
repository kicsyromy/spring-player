#ifndef SPRING_PLAYER_PLAYBACK_FOOTER_H
#define SPRING_PLAYER_PLAYBACK_FOOTER_H

#include <gtk/gtk.h>

#include <libspring_global.h>

namespace spring
{
    namespace player
    {
        class PlaybackHeader
        {
        public:
            PlaybackHeader(GtkBuilder *builder) noexcept;
            ~PlaybackHeader() noexcept;

        private:
            GtkBox *playback_progress_layout_{ nullptr };
            GtkScale *playback_progress_bar_{ nullptr };
            GtkAdjustment *playback_progress_adjustment_{ nullptr };
            GtkLabel *current_time_{ nullptr };
            GtkLabel *total_time_{ nullptr };
            GtkButton *previous_button_{ nullptr };
            GtkButton *next_button_{ nullptr };
            GtkButton *play_pause_button_{ nullptr };
            GtkImage *play_pause_button_icon_{ nullptr };

        private:
            static void on_play_pause_button_clicked(
                GtkButton *button, PlaybackHeader *self) noexcept;
            static void on_next_button_clicked(GtkButton *button,
                                               PlaybackHeader *self) noexcept;
            static void on_previous_button_clicked(
                GtkButton *button, PlaybackHeader *self) noexcept;

        private:
            DISABLE_COPY(PlaybackHeader)
            DISABLE_MOVE(PlaybackHeader)
        };
    }
}

#endif // !SPRING_PLAYER_PLAYBACK_FOOTER_H
