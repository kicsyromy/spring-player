#include "playlist_sidebar.h"

#include <fmt/format.h>

#include <libspring_logger.h>
#include <libspring_music_track.h>

#include "playback_list.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

PlaylistSidebar::PlaylistSidebar(GtkBuilder *builder,
                                 std::shared_ptr<PlaybackList> playback_list) noexcept
  : playback_list_{ playback_list }
{
    LOG_INFO("PlaylistSidebar({}): Creating...", void_p(this));

    get_widget_from_builder_simple(playlist_sidebar);
    get_widget_from_builder_simple(artwork_container);
    get_widget_from_builder_simple(playback_list_box);
    get_widget_from_builder_simple(toggle_sidebar_button);

    gtk_container_add(artwork_container_, artwork_());

    connect_g_signal(playback_list_box_, "row-activated", &on_track_activated, this);
    connect_g_signal(toggle_sidebar_button_, "toggled", &toggled, this);
    connect_g_signal(playback_list_box_, "draw", &on_list_bow_draw_requested, this);

    playback_list->on_track_queued(
        this,
        [](const music::Track &track, void *instance) {
            auto self = static_cast<PlaylistSidebar *>(instance);

            LOG_INFO("PlaylistSidebar({}): Track {} queued", instance, track.title());

            self->playlist_.push_back(&track);
            auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/track_widget.ui");

            get_widget_from_builder_new(GtkBox, track_list_entry);
            get_widget_from_builder_new(GtkLabel, artist_name);
            get_widget_from_builder_new(GtkLabel, song_title);
            get_widget_from_builder_new(GtkLabel, duration);

            gtk_label_set_text(artist_name, track.artist().c_str());
            gtk_label_set_text(song_title, track.title().c_str());

            auto duration_seconds = track.duration().count() / 1000;
            auto minutes = duration_seconds / 60;
            auto seconds = duration_seconds % 60;
            gtk_label_set_text(duration, seconds < 10 ?
                                             fmt::format("{}:0{}", minutes, seconds).c_str() :
                                             fmt::format("{}:{}", minutes, seconds).c_str());

            gtk_container_add(gtk_cast<GtkContainer>(self->playback_list_box_),
                              gtk_cast<GtkWidget>(track_list_entry));

            gtk_widget_set_visible(gtk_cast<GtkWidget>(track_list_entry), true);

            auto dominant_color = self->artwork_.dominant_color();

            auto attributes = pango_attr_list_new();
            auto foreground_color = pango_attr_foreground_new(
                dominant_color.red * std::numeric_limits<std::int16_t>::max() /
                    std::numeric_limits<std::uint8_t>::max(),
                dominant_color.green * std::numeric_limits<std::int16_t>::max() /
                    std::numeric_limits<std::uint8_t>::max(),
                dominant_color.blue * std::numeric_limits<std::int16_t>::max() /
                    std::numeric_limits<std::uint8_t>::max());
            pango_attr_list_insert(attributes, foreground_color);
            gtk_label_set_attributes(artist_name, attributes);
            gtk_label_set_attributes(song_title, attributes);
            gtk_label_set_attributes(duration, attributes);

            pango_attr_list_unref(attributes);

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
                    const auto &artwork = playlist->current_track().second->artwork();
                    self->artwork_.set_image(artwork, Thumbnail::BackgroundType::FromImage);
                    gtk_widget_queue_draw(gtk_cast<GtkWidget>(self->playback_list_box_));

                    auto background_color = self->artwork_.dominant_color();
                    GdkRGBA gdk_color{ static_cast<gdouble>(background_color.red) / 255,
                                       static_cast<gdouble>(background_color.green) / 255,
                                       static_cast<gdouble>(background_color.blue) / 255, 1.0 };
                    gtk_widget_override_background_color(
                        gtk_cast<GtkWidget>(self->playback_list_box_), GTK_STATE_FLAG_NORMAL,
                        &gdk_color);
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

void PlaylistSidebar::toggled(GtkToggleButton *toggle_button, PlaylistSidebar *self) noexcept
{
    LOG_INFO("PlaylistSidebar({}): Toggle button toggled", void_p(self));
    auto visible = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_visible(gtk_cast<GtkWidget>(self->playlist_sidebar_), visible);
}

void PlaylistSidebar::on_track_activated(GtkListBox *,
                                         GtkListBoxRow *element,
                                         PlaylistSidebar *self) noexcept
{
    auto index = static_cast<std::size_t>(gtk_list_box_row_get_index(element));
    LOG_INFO("PlaylistSidebar({}): Playlist element at {} activated", void_p(self), index);

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->play(index);
    }
}

int32_t PlaylistSidebar::on_list_bow_draw_requested(GtkWidget *,
                                                    cairo_t *cairo_context,
                                                    PlaylistSidebar *self) noexcept
{
    //    auto list_box_width =
    //        gtk_widget_get_allocated_width(gtk_cast<GtkWidget>(self->playback_list_box_));
    //    auto list_box_height =
    //        gtk_widget_get_allocated_height(gtk_cast<GtkWidget>(self->playback_list_box_));

    //    //    auto context = gtk_widget_get_style_context(gtk_cast<GtkWidget>(self->playback_list_box_));
    //    //    gtk_render_background(context, cairo_context, 0, 0, list_box_width, list_box_height);

    //    auto background_color = self->artwork_.dominant_color();
    //    GdkRGBA gdk_color{ static_cast<gdouble>(background_color.red) / 255,
    //                       static_cast<gdouble>(background_color.green) / 255,
    //                       static_cast<gdouble>(background_color.blue) / 255, 1.0 };

    //    gdk_cairo_set_source_rgba(cairo_context, &gdk_color);

    //    cairo_fill(cairo_context);

    return false;
}
