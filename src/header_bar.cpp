#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "header_bar.h"
#include "playback_list.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    inline std::string clean_markup(const std::string &text) noexcept
    {
        std::size_t ampersand_pos{ 0 };
        std::string result = text;

        for (;;)
        {
            ampersand_pos = result.find('&', ampersand_pos + 1);

            if (ampersand_pos != std::string::npos)
            {
                result.replace(ampersand_pos, 1, "&amp;");
            }
            else
            {
                break;
            }
        }

        return result;
    }
}

HeaderBar::HeaderBar(std::shared_ptr<PlaybackList> playback_list) noexcept
  : header_bar_(gtk_cast<GtkHeaderBar>(gtk_header_bar_new()))
  , playback_list_(playback_list)
{
    LOG_INFO("HeaderBar({}): Creating...", void_p(this));

    gtk_header_bar_set_show_close_button(header_bar_, true);
    gtk_widget_set_visible(gtk_cast<GtkWidget>(header_bar_), true);

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/header_bar.ui");

    get_widget_from_builder_simple(tool_buttons);
    get_widget_from_builder_simple(toggle_sidebar_button);

    get_widget_from_builder_simple(playback_controls);
    get_widget_from_builder_simple(previous_button);
    get_widget_from_builder_simple(play_pause_button);
    get_widget_from_builder_simple(play_pause_button_icon);
    get_widget_from_builder_simple(next_button);

    get_widget_from_builder_simple(title_and_progress);
    get_widget_from_builder_simple(window_title);
    get_widget_from_builder_simple(current_time);
    get_widget_from_builder_simple(duration);
    get_widget_from_builder_simple(playback_progress_bar);
    get_widget_from_builder_simple(playback_progress_adjustment);

    get_widget_from_builder_simple(search_button);

    gtk_header_bar_pack_start(header_bar_, gtk_cast<GtkWidget>(tool_buttons_));
    gtk_header_bar_pack_start(header_bar_, gtk_cast<GtkWidget>(playback_controls_));

    gtk_header_bar_set_custom_title(header_bar_, gtk_cast<GtkWidget>(title_and_progress_));
    gtk_label_set_markup(window_title_, "<span weight=\"bold\">Spring Player</span>");

    gtk_header_bar_pack_end(header_bar_, gtk_cast<GtkWidget>(search_button_));

    g_object_unref(builder);

    connect_g_signal(previous_button_, "clicked", &on_previous_button_clicked, this);
    connect_g_signal(play_pause_button_, "clicked", &on_play_pause_button_clicked, this);
    connect_g_signal(next_button_, "clicked", &on_next_button_clicked, this);
    connect_g_signal(toggle_sidebar_button_, "toggled", &on_sidebar_toggled, this);
    connect_g_signal(search_button_, "toggled", &on_search_button_toggled, this);

    playback_list->on_playback_state_changed(this, &on_playback_state_changed);
    playback_list->on_playback_position_changed(this, &on_playback_position_changed);
    playback_list->on_track_cache_updated(this, &on_track_cache_updated);
    playback_list->on_track_cached(this, &on_track_cached);
}

HeaderBar::~HeaderBar() noexcept
{
    LOG_INFO("HeaderBar({}): Destroying...", void_p(this));

    auto playlist = playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->disconnect_track_cached(this);
        playlist->disconnect_track_cache_updated(this);
        playlist->disconnect_playback_position_changed(this);
        playlist->disconnect_playback_state_changed(this);
    }
}

void HeaderBar::toggle_sidebar() noexcept
{
    const bool is_active = gtk_toggle_button_get_active(toggle_sidebar_button_);
    gtk_toggle_button_set_active(toggle_sidebar_button_, !is_active);
}

void HeaderBar::toggle_search() noexcept
{
    const bool is_active = gtk_toggle_button_get_active(search_button_);
    gtk_toggle_button_set_active(search_button_, !is_active);
}

GtkWidget *HeaderBar::operator()() noexcept
{
    return gtk_cast<GtkWidget>(header_bar_);
}

void HeaderBar::on_search_button_toggled(GtkToggleButton *toggle_button, HeaderBar *self) noexcept
{
    self->emit_search_toggled(gtk_toggle_button_get_active(toggle_button));
}

void HeaderBar::on_sidebar_toggled(GtkToggleButton *playlist_button, HeaderBar *self) noexcept
{
    self->emit_playlist_toggled(gtk_toggle_button_get_active(playlist_button));
}

void HeaderBar::on_play_pause_button_clicked(GtkButton *, HeaderBar *self) noexcept
{
    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->play_pause();
    }
}

void HeaderBar::on_next_button_clicked(GtkButton *, HeaderBar *self) noexcept
{
    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->next();
    }
}

void HeaderBar::on_previous_button_clicked(GtkButton *, HeaderBar *self) noexcept
{
    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->previous();
    }
}

void HeaderBar::on_playback_state_changed(std::int32_t new_state, HeaderBar *self) noexcept
{
    using state_t = PlaybackList::PlaybackState;

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        auto track = playlist->current_track().second;
        if (track != nullptr)
        {
            auto state = static_cast<PlaybackList::PlaybackState>(new_state);
            auto duration = track->duration().count();

            switch (state)
            {
                case state_t::Playing:
                {
                    gtk_image_set_from_icon_name(self->play_pause_button_icon_,
                                                 "media-playback-pause-symbolic",
                                                 GTK_ICON_SIZE_LARGE_TOOLBAR);
                    gtk_label_set_markup(
                        self->window_title_,
                        clean_markup(
                            fmt::format(
                                "<span weight=\"bold\">{}</span> by <span "
                                "weight=\"bold\">{}</span> from <span weight=\"bold\">{}</span>",
                                track->title(), track->artist(), track->album()))
                            .c_str());
                    break;
                }
                case state_t::Pending:
                {
                    gtk_adjustment_set_upper(self->playback_progress_adjustment_, duration);
                    gtk_adjustment_set_value(self->playback_progress_adjustment_, 0);

                    auto duration_seconds_total = duration / 1000;
                    const auto minutes = duration_seconds_total / 60;
                    const auto seconds = duration_seconds_total % 60;
                    gtk_label_set_text(self->current_time_, "00:00");
                    gtk_label_set_text(self->duration_,
                                       seconds < 10 ?
                                           fmt::format("{}:0{}", minutes, seconds).c_str() :
                                           fmt::format("{}:{}", minutes, seconds).c_str());
                    break;
                }
                case state_t::Invalid:
                case state_t::Stopped:
                {
                    gtk_adjustment_set_value(self->playback_progress_adjustment_, 0);
                    gtk_range_set_fill_level(gtk_cast<GtkRange>(self->playback_progress_bar_), 0);
                    gtk_label_set_text(self->current_time_, "00:00");
                    gtk_label_set_text(self->duration_, "00:00");

                    gtk_label_set_markup(self->window_title_,
                                         "<span weight=\"bold\">Spring Player</span>");
                    SPRING_FALLTHROUGH;
                }
                case state_t::Paused:
                {
                    gtk_image_set_from_icon_name(self->play_pause_button_icon_,
                                                 "media-playback-start-symbolic",
                                                 GTK_ICON_SIZE_LARGE_TOOLBAR);
                    break;
                }
            }
        }
    }
}

void HeaderBar::on_playback_position_changed(int64_t position, HeaderBar *self) noexcept
{
    gtk_adjustment_set_value(self->playback_progress_adjustment_, position);
    auto elapsed_seconds_total = position / 1000;
    const auto elapsed_minutes = elapsed_seconds_total / 60;
    const auto elapsed_seconds = elapsed_seconds_total % 60;
    gtk_label_set_text(self->current_time_,
                       elapsed_seconds < 10 ?
                           fmt::format("{}:0{}", elapsed_minutes, elapsed_seconds).c_str() :
                           fmt::format("{}:{}", elapsed_minutes, elapsed_seconds).c_str());
}

void HeaderBar::on_track_cache_updated(std::size_t amount, HeaderBar *self) noexcept
{
    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        auto &track = *playlist->current_track().second;
        auto file_size = track.fileSize();
        auto duration = static_cast<std::size_t>(track.duration().count());

        gtk_range_set_fill_level(gtk_cast<GtkRange>(self->playback_progress_bar_),
                                 amount * duration / file_size);
    }
}

void HeaderBar::on_track_cached(HeaderBar *self) noexcept
{
    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        auto &track = *playlist->current_track().second;
        auto duration = static_cast<std::size_t>(track.duration().count());

        gtk_range_set_fill_level(gtk_cast<GtkRange>(self->playback_progress_bar_), duration);
    }
}
