#include "playback_header.h"

#include <fmt/format.h>

#include "playback_list.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

PlaybackHeader::PlaybackHeader(
    GtkBuilder *builder, std::shared_ptr<PlaybackList> playback_list) noexcept
  : playback_list_(playback_list)
{
    get_widget_from_builder_simple(playback_progress_layout);
    get_widget_from_builder_simple(playback_progress_bar);
    get_widget_from_builder_simple(playback_progress_adjustment);
    get_widget_from_builder_simple(current_time);
    get_widget_from_builder_simple(total_time);
    get_widget_from_builder_simple(previous_button);
    get_widget_from_builder_simple(next_button);
    get_widget_from_builder_simple(play_pause_button);
    get_widget_from_builder_simple(play_pause_button_icon);

    connect_g_signal(play_pause_button_, "clicked",
                     &on_play_pause_button_clicked, this);
    connect_g_signal(next_button_, "clicked", &on_next_button_clicked, this);
    connect_g_signal(previous_button_, "clicked", &on_previous_button_clicked,
                     this);

    playback_list->on_playback_state_changed(
        this,
        [](auto state, void *instance) {
            auto self = static_cast<PlaybackHeader *>(instance);

            auto playlist = self->playback_list_.lock();
            if (playlist != nullptr)
            {
                auto track = playlist->current_track();
                if (track != nullptr)
                {
                    auto duration = track->duration().count();

                    if (state == PlaybackList::PlaybackState::Pending ||
                        state == PlaybackList::PlaybackState::Playing)
                    {
                        gtk_adjustment_set_upper(
                            self->playback_progress_adjustment_, duration);
                        gtk_adjustment_set_value(
                            self->playback_progress_adjustment_, 0);

                        auto duration_seconds_total = duration / 1000;
                        const auto minutes = duration_seconds_total / 60;
                        const auto seconds = duration_seconds_total % 60;
                        gtk_label_set_text(self->current_time_, "0:00");
                        gtk_label_set_text(
                            self->total_time_,
                            seconds < 10 ?
                                fmt::format("{}:0{}", minutes, seconds)
                                    .c_str() :
                                fmt::format("{}:{}", minutes, seconds).c_str());
                        gtk_widget_set_visible(
                            gtk_cast<GtkWidget>(
                                self->playback_progress_layout_),
                            true);

                        gtk_image_set_from_icon_name(
                            self->play_pause_button_icon_,
                            "media-playback-pause-symbolic", GTK_ICON_SIZE_DND);
                    }
                    else
                    {
                        gtk_image_set_from_icon_name(
                            self->play_pause_button_icon_,
                            "media-playback-start-symbolic", GTK_ICON_SIZE_DND);
                    }
                }
            }
        },
        this);

    playback_list->on_playback_position_changed(
        this,
        [](auto position, void *instance) {
            auto self = static_cast<PlaybackHeader *>(instance);

            gtk_adjustment_set_value(self->playback_progress_adjustment_,
                                     position);
            auto elapsed_seconds_total = position / 1000;
            const auto elapsed_minutes = elapsed_seconds_total / 60;
            const auto elapsed_seconds = elapsed_seconds_total % 60;
            gtk_label_set_text(
                self->current_time_,
                elapsed_seconds < 10 ?
                    fmt::format("{}:0{}", elapsed_minutes, elapsed_seconds)
                        .c_str() :
                    fmt::format("{}:{}", elapsed_minutes, elapsed_seconds)
                        .c_str());
        },
        this);

    playback_list->on_track_cache_updated(
        this,
        [](std::size_t new_size, void *instance) {
            auto self = static_cast<PlaybackHeader *>(instance);

            auto playlist = self->playback_list_.lock();
            if (playlist != nullptr)
            {
                auto &track = *playlist->current_track();
                auto file_size = track.fileSize();
                auto duration =
                    static_cast<std::size_t>(track.duration().count());

                gtk_range_set_fill_level(
                    gtk_cast<GtkRange>(self->playback_progress_bar_),
                    new_size * duration / file_size);
            }
        },
        this);

    playback_list->on_track_cached(
        this,
        [](void *instance) {
            auto self = static_cast<PlaybackHeader *>(instance);

            auto playlist = self->playback_list_.lock();
            if (playlist != nullptr)
            {
                auto &track = *playlist->current_track();
                auto duration =
                    static_cast<std::size_t>(track.duration().count());

                gtk_range_set_fill_level(
                    gtk_cast<GtkRange>(self->playback_progress_bar_), duration);
            }
        },
        this);
}

PlaybackHeader::~PlaybackHeader() noexcept
{
    auto playlist = playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->disconnect_track_cached(this);
        playlist->disconnect_playback_position_changed(this);
        playlist->disconnect_playback_state_changed(this);
    }
}

void PlaybackHeader::on_play_pause_button_clicked(GtkButton *,
                                                  PlaybackHeader *self) noexcept
{
    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->play_pause();
    }
}

void PlaybackHeader::on_next_button_clicked(GtkButton *,
                                            PlaybackHeader *self) noexcept
{
    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->next();
    }
}

void PlaybackHeader::on_previous_button_clicked(GtkButton *,
                                                PlaybackHeader *self) noexcept
{
    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->previous();
    }
}
