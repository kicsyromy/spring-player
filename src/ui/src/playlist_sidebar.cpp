#include <gtk/gtk.h>

#include <fmt/format.h>

#include <libspring_logger.h>
#include <libspring_music_track.h>

#include "ui/playlist_sidebar.h"

#include "playback/playlist.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::ui;
using namespace spring::player::playback;
using namespace spring::player::utility;

namespace
{
    inline auto determine_text_color(const Thumbnail::pixel_t &background_color) noexcept
    {
        std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> text_color{
            std::numeric_limits<std::uint16_t>::max(), std::numeric_limits<std::uint16_t>::max(),
            std::numeric_limits<std::uint16_t>::max()
        };

        if ((background_color.red > 120 && background_color.green > 120) ||
            (background_color.red > 120 && background_color.blue > 120) ||
            (background_color.blue > 120 && background_color.green > 120))
        {
            text_color = std::make_tuple(0, 0, 0);
        }

        return text_color;
    }
} // namespace

PlaylistSidebar::PlaylistSidebar(std::shared_ptr<Playlist> playback_list) noexcept
  : playback_list_{ playback_list }
{
    LOG_INFO("PlaylistSidebar({}): Creating...", void_p(this));

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/playlist_sidebar.ui");

    get_guarded_widget_from_builder(playlist_sidebar);
    get_widget_from_builder_simple(artwork_container);
    get_widget_from_builder_simple(shuffle_button);
    get_widget_from_builder_simple(repeat_button);
    get_widget_from_builder_simple(track_list_container);

    gtk_container_add(artwork_container_, artwork_());

    connect_g_signal(shuffle_button_, "toggled", &on_shuffle_toggled, this);
    connect_g_signal(repeat_button_, "toggled", &on_repeat_toggled, this);
    connect_g_signal(track_list_container_, "row-activated", &on_track_activated, this);
    connect_g_signal(track_list_container_, "draw", &on_list_box_draw_requested, this);

    playback_list->on_track_queued(
        this,
        [](std::shared_ptr<music::Track> &track, void *instance) {
            auto self = static_cast<PlaylistSidebar *>(instance);

            LOG_INFO("PlaylistSidebar({}): Track {} queued", instance, track->title());

            auto new_entry = std::unique_ptr<PlaylistItem>{ new PlaylistItem(track) };

            auto result = self->playlist_.emplace((*new_entry)(), std::move(new_entry));

            if (result.second)
            {
                auto *widget = result.first->first;
                gtk_container_add(gtk_cast<GtkContainer>(self->track_list_container_), widget);
                gtk_widget_set_visible(widget, true);
                result.first->second->set_text_color(
                    determine_text_color(self->artwork_.dominant_color()));
            }
        },
        this);

    playback_list->on_list_cleared(
        this,
        [](void *instance) {
            auto self = static_cast<PlaylistSidebar *>(instance);

            gtk_container_foreach(gtk_cast<GtkContainer>(self->track_list_container_),
                                  [](GtkWidget *widget, gpointer) { gtk_widget_destroy(widget); },
                                  nullptr);

            self->playlist_.clear();
        },
        this);

    playback_list->on_playback_state_changed(this, &on_playback_state_changed);
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
    gtk_widget_set_visible(gtk_cast<GtkWidget>(playlist_sidebar_), true);
}

void PlaylistSidebar::hide() noexcept
{
    LOG_INFO("PlaylistSidebar({}): Toggle hide", void_p(this));
    gtk_widget_set_visible(gtk_cast<GtkWidget>(playlist_sidebar_), false);
}

GtkWidget *PlaylistSidebar::operator()() noexcept
{
    return gtk_cast<GtkWidget>(playlist_sidebar_);
}

void PlaylistSidebar::on_shuffle_toggled(GtkToggleButton *button, PlaylistSidebar *self) noexcept
{
    LOG_INFO("PlaylistSidebar({}): Shuffle toggled", void_p(self));

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->set_shuffle_active(gtk_toggle_button_get_active(button));
    }
}

void PlaylistSidebar::on_repeat_toggled(GtkToggleButton *button, PlaylistSidebar *self) noexcept
{
    LOG_INFO("PlaylistSidebar({}): Repeat toggled", void_p(self));

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->set_repeat_one_active(gtk_toggle_button_get_active(button));
    }
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

void PlaylistSidebar::on_playback_state_changed(Playlist::PlaybackState new_state,
                                                PlaylistSidebar *self) noexcept
{
    LOG_INFO("PlaylistSidebar({}): Playback state changed {}", void_p(self),
             GStreamerPipeline::playback_state_to_string(new_state));

    if (new_state == Playlist::PlaybackState::Pending ||
        new_state == Playlist::PlaybackState::Playing)
    {
        auto playlist = self->playback_list_.lock();
        if (playlist != nullptr)
        {
            auto current_track = playlist->current_track();
            if (current_track.second != nullptr)
            {
                auto element =
                    gtk_list_box_get_row_at_index(self->track_list_container_, current_track.first);
                auto it = self->playlist_.find(gtk_bin_get_child(gtk_cast<GtkBin>(element)));
                if (it != self->playlist_.end())
                {
                    if (self->current_item_ != nullptr)
                    {
                        self->current_item_->set_playing(false);
                    }
                    self->current_item_ = it->second.get();
                    it->second->set_playing(true);
                }

                const auto &artwork = current_track.second->artwork();
                self->artwork_.set_image(artwork, Thumbnail::BackgroundType::FromImage);
                gtk_widget_queue_draw(gtk_cast<GtkWidget>(self->track_list_container_));

                auto background_color = self->artwork_.dominant_color();

                for (auto &track_item : self->playlist_)
                {
                    track_item.second->set_text_color(determine_text_color(background_color));
                }

                GdkRGBA gdk_color{ static_cast<gdouble>(background_color.red) / 255,
                                   static_cast<gdouble>(background_color.green) / 255,
                                   static_cast<gdouble>(background_color.blue) / 255, 0.8 };
                gtk_widget_override_background_color(
                    gtk_cast<GtkWidget>(self->track_list_container_), GTK_STATE_FLAG_NORMAL,
                    &gdk_color);
            }
        }
        else
        {
            LOG_ERROR("PlaylistSidebar({}): Playlist object destroyed, the program will not "
                      "function correctly.",
                      void_p(self));
        }
    }
}

std::int32_t PlaylistSidebar::on_list_box_draw_requested(GtkWidget *,
                                                         cairo_t * /*cairo_context*/,
                                                         PlaylistSidebar * /*self*/) noexcept
{
    return false;
}

PlaylistSidebar::PlaylistItem::PlaylistItem(std::shared_ptr<music::Track> &track) noexcept
  : track_(track)
{
    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/playlist_item.ui");

    get_guarded_widget_from_builder(playlist_item);
    get_widget_from_builder_simple(title);
    get_widget_from_builder_simple(duration);
    get_widget_from_builder_simple(playing_icon);

    auto title = fmt::format("{} by {}", track->title(), track->artist());
    gtk_label_set_text(title_, title.c_str());

    auto duration_seconds = track->duration().count() / 1000;
    auto minutes = duration_seconds / 60;
    auto seconds = duration_seconds % 60;
    gtk_label_set_text(duration_, seconds < 10 ? fmt::format("{}:0{}", minutes, seconds).c_str() :
                                                 fmt::format("{}:{}", minutes, seconds).c_str());

    g_object_unref(builder);
}

void PlaylistSidebar::PlaylistItem::set_playing(bool value) noexcept
{
    gtk_widget_set_opacity(gtk_cast<GtkWidget>(playing_icon_), value ? 1.0 : 0.0);
}

void PlaylistSidebar::PlaylistItem::set_text_color(
    const std::tuple<uint16_t, uint16_t, uint16_t> &color) noexcept
{
    auto attributes = pango_attr_list_new();

    auto foreground_color =
        pango_attr_foreground_new(std::get<0>(color), std::get<1>(color), std::get<2>(color));
    pango_attr_list_insert(attributes, foreground_color);

    gtk_label_set_attributes(title_, attributes);
    gtk_label_set_attributes(duration_, attributes);

    pango_attr_list_unref(attributes);
}

GtkWidget *PlaylistSidebar::PlaylistItem::operator()() noexcept
{
    return gtk_cast<GtkWidget>(playlist_item_);
}
