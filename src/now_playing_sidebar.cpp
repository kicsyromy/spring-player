#include "now_playing_sidebar.h"

#include <fmt/format.h>

#include <libspring_music_track.h>

#include "common.h"
#include "now_playing_list.h"

using namespace spring;
using namespace spring::player;

NowPlayingSidebar::NowPlayingSidebar(GtkBuilder *builder) noexcept
{
    get_widget_from_builder_simple(now_playing_sidebar);
    get_widget_from_builder_simple(now_playing_cover);
    get_widget_from_builder_simple(now_playing_list);
    get_widget_from_builder_simple(toggle_sidebar_button);

    g_signal_connect(toggle_sidebar_button_, "toggled", G_CALLBACK(&toggled),
                     this);

    NowPlayingList::instance().on_track_queued(
        [this](const music::Track &track) {
            auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX
                                                         "/track_widget.ui");

            get_widget_from_builder_new(GtkBox, track_list_entry);
            get_widget_from_builder_new(GtkLabel, artist_name);
            get_widget_from_builder_new(GtkLabel, song_title);
            get_widget_from_builder_new(GtkLabel, duration);

            gtk_label_set_text(artist_name, track.artist().c_str());
            gtk_label_set_text(song_title, track.title().c_str());

            auto duration_seconds = track.duration().count() / 1000;
            auto minutes = duration_seconds / 60;
            auto seconds = duration_seconds % 60;
            gtk_label_set_text(
                duration, seconds < 10 ?
                              fmt::format("{}:0{}", minutes, seconds).c_str() :
                              fmt::format("{}:{}", minutes, seconds).c_str());

            gtk_container_add(gtk_cast<GtkContainer>(now_playing_list_),
                              gtk_cast<GtkWidget>(track_list_entry));

            gtk_widget_set_visible(gtk_cast<GtkWidget>(track_list_entry), true);

            g_object_unref(builder);
        });

    NowPlayingList::instance().on_state_changed([this](auto playback_state) {
        if (playback_state == NowPlayingList::PlaybackState::Playing)
        {
            const auto &artwork =
                NowPlayingList::instance().current_track()->artwork();
            load_image_from_data_scaled<200, 200>(artwork, now_playing_cover_);
        }
    });
}

void NowPlayingSidebar::show() noexcept
{
    gtk_toggle_button_set_active(toggle_sidebar_button_, true);
}

void NowPlayingSidebar::hide() noexcept
{
    gtk_toggle_button_set_active(toggle_sidebar_button_, false);
}

void NowPlayingSidebar::toggled(GtkToggleButton *toggle_button,
                                NowPlayingSidebar *self) noexcept
{
    auto visible = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_visible(gtk_cast<GtkWidget>(self->now_playing_sidebar_),
                           visible);
}
