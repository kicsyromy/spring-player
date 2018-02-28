#include "playlist_sidebar.h"

#include <fmt/format.h>

#include <libspring_logger.h>
#include <libspring_music_track.h>

#include "playback_list.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

PlaylistSidebar::PlaylistSidebar(
    GtkBuilder *builder, std::shared_ptr<PlaybackList> playback_list) noexcept
  : playback_list_{ playback_list }
{
    LOG_INFO("PlaylistSidebar({}): Creating...", void_p(this));

    get_widget_from_builder_simple(playlist_sidebar);
    get_widget_from_builder_simple(current_track_cover);
    get_widget_from_builder_simple(playback_list_box);
    get_widget_from_builder_simple(toggle_sidebar_button);

    connect_g_signal(playback_list_box_, "row-activated", &on_track_activated,
                     this);
    connect_g_signal(toggle_sidebar_button_, "toggled", &toggled, this);

    playback_list->on_track_queued(
        this,
        [](const music::Track &track, void *instance) {
            auto self = static_cast<PlaylistSidebar *>(instance);

            LOG_INFO("PlaylistSidebar({}): Track {} queued", instance,
                     track.title());

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

            gtk_container_add(gtk_cast<GtkContainer>(self->playback_list_box_),
                              gtk_cast<GtkWidget>(track_list_entry));

            gtk_widget_set_visible(gtk_cast<GtkWidget>(track_list_entry), true);

            g_object_unref(builder);
        },
        this);

    playback_list->on_playback_state_changed(
        this,
        [](auto state, void *instance) {
            auto self = static_cast<PlaylistSidebar *>(instance);

            LOG_INFO("PlaylistSidebar({}): Playback state changed {}", instance,
                     GStreamerPipeline::playback_state_to_string(state));

            if (state == PlaybackList::PlaybackState::Pending ||
                state == PlaybackList::PlaybackState::Playing)
            {
                auto playlist = self->playback_list_.lock();
                if (playlist != nullptr)
                {
                    const auto &artwork =
                        playlist->current_track().second->artwork();
                    auto pixbuf =
                        load_pixbuf_from_data_scaled<200, 200>(artwork);
                    gtk_image_set_from_pixbuf(self->current_track_cover_,
                                              pixbuf);
                }
                else
                {
                    LOG_ERROR("PlaylistSidebar({}): PlaybackList object "
                              "destroyed, the program will not function "
                              "correctly.",
                              instance);
                }
            }
        },
        this);
}

PlaylistSidebar::~PlaylistSidebar() noexcept
{
    LOG_INFO("PlaylistSidebar({}): Destroying...", void_p(this));

    auto playlist = playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->disconnect_playback_state_changed(this);
        playlist->disconnect_track_queued(this);
    }
}

void PlaylistSidebar::show() noexcept
{
    LOG_INFO("PlaylistSidebar({}): Toggle show", void_p(this));
    gtk_toggle_button_set_active(toggle_sidebar_button_, true);
}

void PlaylistSidebar::hide() noexcept
{
    LOG_INFO("PlaylistSidebar({}): Toggle hide", void_p(this));
    gtk_toggle_button_set_active(toggle_sidebar_button_, false);
}

void PlaylistSidebar::toggled(GtkToggleButton *toggle_button,
                              PlaylistSidebar *self) noexcept
{
    LOG_INFO("PlaylistSidebar({}): Toggle button toggled", void_p(self));
    auto visible = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_visible(gtk_cast<GtkWidget>(self->playlist_sidebar_),
                           visible);
}

void PlaylistSidebar::on_track_activated(GtkListBox *,
                                         GtkListBoxRow *element,
                                         PlaylistSidebar *self) noexcept
{
    auto index = static_cast<std::size_t>(gtk_list_box_row_get_index(element));
    LOG_INFO("PlaylistSidebar({}): Playlist element at {} activated",
             void_p(self), index);

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->play(index);
    }
}
