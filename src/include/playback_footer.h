#ifndef SPRING_PLAYER_PLAYBACK_FOOTER_H
#define SPRING_PLAYER_PLAYBACK_FOOTER_H

#include <gtk/gtk.h>

#include <libspring_global.h>

namespace spring
{
    namespace player
    {
        class PlaybackFooter
        {
        public:
            PlaybackFooter(GtkBuilder *builder) noexcept;
            ~PlaybackFooter() noexcept = default;

        private:
            GtkBox *playback_progress_layout_{ nullptr };
            GtkScale *playback_progress_bar_{ nullptr };
            GtkAdjustment *playback_progress_adjustment_{ nullptr };
            GtkLabel *current_time_{ nullptr };
            GtkLabel *total_time_{ nullptr };
            GtkButton *previous_button_{ nullptr };
            GtkButton *next_button_{ nullptr };
            GtkButton *play_pause_button_{ nullptr };

        private:
            DISABLE_COPY(PlaybackFooter)
            DISABLE_MOVE(PlaybackFooter)
        };
    }
}

#endif // !SPRING_PLAYER_PLAYBACK_FOOTER_H