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

    gtk_header_bar_pack_end(header_bar_, gtk_cast<GtkWidget>(search_button_));

    g_object_unref(builder);

    connect_g_signal(previous_button_, "clicked", &on_previous_button_clicked, this);
    connect_g_signal(play_pause_button_, "clicked", &on_play_pause_button_clicked, this);
    connect_g_signal(next_button_, "clicked", &on_next_button_clicked, this);
    connect_g_signal(toggle_sidebar_button_, "toggled", &on_sidebar_toggled, this);
}

HeaderBar::~HeaderBar() noexcept
{
    LOG_INFO("HeaderBar({}): Destroying...", void_p(this));
}

void HeaderBar::set_title(const std::string &text, Text text_type) noexcept
{
    if (text_type == Text::Markup)
    {
        gtk_label_set_markup(window_title_, clean_markup(text).c_str());
    }
    else
    {
        gtk_label_set_text(window_title_, text.c_str());
    }
}

GtkWidget *HeaderBar::operator()() noexcept
{
    return gtk_cast<GtkWidget>(header_bar_);
}

void HeaderBar::on_search_toggled(GtkToggleButton *toggle_button, HeaderBar *self) noexcept
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
