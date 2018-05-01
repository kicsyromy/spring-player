#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "track_list_popover.h"

#include "async_queue.h"
#include "playback_list.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

TrackListPopover::TrackListPopover(std::weak_ptr<PlaybackList> playback_list) noexcept
  : playback_list_{ playback_list }
{
    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/track_list_popover.ui");
    get_guarded_widget_from_builder(listbox_popover);
    get_widget_from_builder_simple(enqueue_button);
    get_widget_from_builder_simple(listbox);
    get_widget_from_builder_simple(loading_spinner);
    g_object_unref(builder);

    connect_g_signal(enqueue_button_, "clicked", &on_enqueue_requested, this);
    connect_g_signal(listbox_, "row-activated", &on_track_activated, this);
    connect_g_signal(listbox_popover_, "closed", &on_popover_closed, this);
}

TrackListPopover::~TrackListPopover() noexcept
{
}

void TrackListPopover::show(const music::Album &album, GtkWidget *relative_to) noexcept
{
    gtk_spinner_start(loading_spinner_);

    async_queue::push_front_request(async_queue::Request{
        "load_tracks_for_album", [this, &album] {
            auto tracks = load_tracks(album);
            auto callback =
                std::bind(&TrackListPopover::on_tracks_loaded, this, tracks.first, tracks.second);
            async_queue::post_response(async_queue::Response{ "tracks_ready", callback });
        } });

    gtk_popover_set_relative_to(listbox_popover_, relative_to);

#if GTK_MINOR_VERSION >= 22
    gtk_popover_popup(listbox_popover_);
#else
    gtk_popover_set_transitions_enabled(listbox_popover_, true);
    gtk_widget_show(gtk_cast<GtkWidget>(listbox_popover_));
#endif
}

GtkWidget *TrackListPopover::operator()() noexcept
{
    return gtk_cast<GtkWidget>(listbox_popover_);
}

std::pair<std::vector<music::Track> *, std::vector<utility::GObjectGuard<GtkBox>> *>
TrackListPopover::load_tracks(const music::Album &album) const noexcept
{
    LOG_INFO("TrackListPopover({}): Loading tracks", void_p(this));

    auto tracks = new std::vector<music::Track>();
    *tracks = album.tracks();
    auto track_list_entries = new std::vector<GObjectGuard<GtkBox>>;
    track_list_entries->reserve(tracks->size());

    for (const auto &track : *tracks)
    {
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

        track_list_entries->emplace_back(track_list_entry);

        g_object_unref(builder);
    }

    return { tracks, track_list_entries };
}

void TrackListPopover::on_tracks_loaded(
    std::vector<music::Track> *tracks,
    std::vector<utility::GObjectGuard<GtkBox>> *track_widgets) noexcept
{
    LOG_INFO("TrackListPopover({}): Tracks ready", void_p(this));

    if (gtk_widget_get_visible(gtk_cast<GtkWidget>(listbox_popover_)))
    {
        on_popover_closed(listbox_popover_, this);

        std::unique_ptr<std::vector<music::Track>> track_list{ tracks };
        std::unique_ptr<std::vector<GObjectGuard<GtkBox>>> track_widget_list{ track_widgets };

        std::size_t index{ 0 };
        for (auto &track_widget : *track_widget_list)
        {
            gtk_container_add(gtk_cast<GtkContainer>(listbox_), gtk_cast<GtkWidget>(track_widget));

            tracks_.push_back(std::make_shared<music::Track>(std::move(track_list->at(index++))));
        }

        gtk_widget_show_all(gtk_cast<GtkWidget>(listbox_));
    }

    gtk_spinner_stop(loading_spinner_);
}

void TrackListPopover::on_track_activated(GtkListBox *,
                                          GtkListBoxRow *element,
                                          TrackListPopover *self) noexcept
{
    std::size_t element_index = static_cast<std::size_t>(gtk_list_box_row_get_index(element));

    LOG_INFO("TrackListPopover({}): Track {} activated", void_p(self),
             self->tracks_.at(element_index)->title());

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->enqueue(self->tracks_.at(element_index));
    }
}

void TrackListPopover::on_popover_closed(GtkPopover *, TrackListPopover *self) noexcept
{
    self->tracks_.clear();
    gtk_container_foreach(gtk_cast<GtkContainer>(self->listbox_),
                          [](GtkWidget *widget, gpointer) { gtk_widget_destroy(widget); }, nullptr);

    self->emit_closed(std::move(self));
}

void TrackListPopover::on_enqueue_requested(GtkButton *, TrackListPopover *self) noexcept
{
    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        for (auto &track : self->tracks_)
        {
            playlist->enqueue(track);
        }
    }
}
