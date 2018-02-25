#include "now_playing_sidebar.h"

#include <fmt/format.h>

#include <libspring_music_track.h>

#include "now_playing_list.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

NowPlayingSidebar::NowPlayingSidebar(GtkBuilder *builder) noexcept
{
    get_widget_from_builder_simple(now_playing_sidebar);
    get_widget_from_builder_simple(now_playing_cover);
    get_widget_from_builder_simple(now_playing_list);
    get_widget_from_builder_simple(toggle_sidebar_button);

    connect_g_signal(now_playing_list_, "row-activated", &on_track_activated,
                     this);
    connect_g_signal(toggle_sidebar_button_, "toggled", &toggled, this);

    NowPlayingList::instance().on_track_queued(
        this,
        [](const music::Track &track, void *instance) {
            auto self = static_cast<NowPlayingSidebar *>(instance);

            self->playlist_.push_back(&track);
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

            gtk_container_add(gtk_cast<GtkContainer>(self->now_playing_list_),
                              gtk_cast<GtkWidget>(track_list_entry));

            gtk_widget_set_visible(gtk_cast<GtkWidget>(track_list_entry), true);

            g_object_unref(builder);
        },
        this);

    NowPlayingList::instance().on_playback_state_changed(
        this,
        [](auto playback_state, void *instance) {
            auto self = static_cast<NowPlayingSidebar *>(instance);
            if (playback_state == NowPlayingList::PlaybackState::Pending ||
                playback_state == NowPlayingList::PlaybackState::Playing)
            {
                const auto &artwork =
                    NowPlayingList::instance().current_track()->artwork();
                load_image_from_data_scaled<200, 200>(artwork,
                                                      self->now_playing_cover_);
            }
        },
        this);
}

NowPlayingSidebar::~NowPlayingSidebar() noexcept
{
    NowPlayingList::instance().disconnect_playback_state_changed(this);
    NowPlayingList::instance().disconnect_track_queued(this);
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

void NowPlayingSidebar::on_track_activated(GtkListBox *,
                                           GtkListBoxRow *element,
                                           NowPlayingSidebar *self) noexcept
{
    g_warning("****** activated");
    auto index = static_cast<std::size_t>(gtk_list_box_row_get_index(element));

    const auto &track = *self->playlist_.at(index);
    NowPlayingList::instance().restart_current_track();
}
