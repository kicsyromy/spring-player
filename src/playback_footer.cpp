#include "playback_footer.h"

#include <fmt/format.h>

#include "now_playing_list.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;

PlaybackFooter::PlaybackFooter(GtkBuilder *builder) noexcept
{
    get_widget_from_builder_simple(playback_progress_layout);
    get_widget_from_builder_simple(playback_progress_bar);
    get_widget_from_builder_simple(playback_progress_adjustment);
    get_widget_from_builder_simple(current_time);
    get_widget_from_builder_simple(total_time);
    get_widget_from_builder_simple(previous_button);
    get_widget_from_builder_simple(next_button);
    get_widget_from_builder_simple(play_pause_button);

    NowPlayingList::instance().on_state_changed([this](auto state) {
        if (state == NowPlayingList::PlaybackState::Playing)
        {
            const auto duration_seconds =
                NowPlayingList::instance().current_track()->duration().count() /
                1000;
            gtk_adjustment_set_upper(playback_progress_adjustment_,
                                     duration_seconds);
            gtk_adjustment_set_value(playback_progress_adjustment_, 0);

            const auto minutes = duration_seconds / 60;
            const auto seconds = duration_seconds % 60;
            gtk_label_set_text(current_time_, "0:00");
            gtk_label_set_text(
                total_time_,
                seconds < 10 ? fmt::format("{}:0{}", minutes, seconds).c_str() :
                               fmt::format("{}:{}", minutes, seconds).c_str());
            gtk_widget_set_visible(
                gtk_cast<GtkWidget>(playback_progress_layout_), true);
        }
    });

    NowPlayingList::instance().on_playback_position_changed(
        [this](auto position_seconds) {
            gtk_adjustment_set_value(playback_progress_adjustment_,
                                     position_seconds);
            const auto elapsed_minutes = position_seconds / 60;
            const auto elapsed_seconds = position_seconds % 60;
            gtk_label_set_text(
                current_time_,
                elapsed_seconds < 10 ?
                    fmt::format("{}:0{}", elapsed_minutes, elapsed_seconds)
                        .c_str() :
                    fmt::format("{}:{}", elapsed_minutes, elapsed_seconds)
                        .c_str());
        });
}
