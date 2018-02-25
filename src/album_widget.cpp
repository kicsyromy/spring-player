#include "album_widget.h"

#include <fmt/format.h>

#include "async_queue.h"
#include "now_playing_list.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

AlbumWidget::AlbumWidget(music::Album &&album) noexcept
  : album_(std::move(album))
{
    auto builder =
        gtk_builder_new_from_resource(APPLICATION_PREFIX "/album_widget.ui");
    get_guarded_widget_from_builder(album_widget);
    get_widget_from_builder_simple(cover);
    get_widget_from_builder_simple(artist);
    get_widget_from_builder_simple(title);

    get_guarded_widget_from_builder(track_list_popover);
    get_widget_from_builder_simple(track_list);
    get_widget_from_builder_simple(tracks_loading_spinner);

    g_object_unref(builder);

    connect_g_signal(track_list_, "row-activated", &on_track_activated, this);
    connect_g_signal(track_list_popover_, "closed", &on_popover_closed, this);

    gtk_label_set_text(artist_, album_.artist().c_str());
    gtk_label_set_text(title_, album_.title().c_str());

    //    load_image_from_data_scaled<200, 200>(album_.artwork(), cover_);
}

const std::string &AlbumWidget::title() const noexcept
{
    return album_.title();
}

const std::string &AlbumWidget::artist() const noexcept
{
    return album_.artist();
}

void AlbumWidget::activated() noexcept
{
    gtk_spinner_start(tracks_loading_spinner_);

    async_queue::post_request(new async_queue::Request{
        "get_tracks_for_album", [this]() {
        auto tracks = new std::vector<music::Track>();
        *tracks = album_.tracks();
        std::vector<GtkRefGuard<GtkBox>> track_list_entries;
        track_list_entries.reserve(tracks->size());

        for (const auto &track : *tracks)
        {
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

            track_list_entries.emplace_back(track_list_entry);

            g_object_unref(builder);
        }
        /* clang-format off */
            async_queue::post_response(new async_queue::Response{
                "tracks_for_album_ready",
                [
                    this,
                    entries{ std::move(track_list_entries) },
                    tracks
                ]() {
                    if (gtk_widget_get_visible(
                            gtk_cast<GtkWidget>(track_list_popover_)))
                    {
                        on_popover_closed(track_list_popover_, this);

                        auto &track_list_entries =
                            const_cast<std::vector<GtkRefGuard<GtkBox>> &>(entries);
                        std::unique_ptr<std::vector<music::Track>> track_list;
                        track_list.reset(tracks);

                        gint index{ 0 };
                        for (auto &track_entry : track_list_entries)
                        {
                            gtk_container_add(
                                gtk_cast<GtkContainer>(track_list_),
                                gtk_cast<GtkWidget>(track_entry));
                        }
                        tracks_ = std::move(*track_list);
                        gtk_widget_show_all(gtk_cast<GtkWidget>(track_list_));
                        gtk_spinner_stop(tracks_loading_spinner_);
                    }
                } });
        } });

    if (gtk_popover_get_relative_to(track_list_popover_) == nullptr)
    {
        gtk_popover_set_relative_to(track_list_popover_,
                                    gtk_cast<GtkWidget>(cover_));
    }

    gtk_popover_popup(track_list_popover_);
/* clang-format on */
}

void AlbumWidget::on_track_activated(GtkListBox *,
                                     GtkListBoxRow *element,
                                     AlbumWidget *self) noexcept
{
    std::size_t element_index =
        static_cast<std::size_t>(gtk_list_box_row_get_index(element));
    NowPlayingList::instance().enqueue(
        std::move(self->tracks_.at(element_index)));
}

void AlbumWidget::on_popover_closed(GtkPopover *, AlbumWidget *self) noexcept
{
    gtk_container_foreach(
        gtk_cast<GtkContainer>(self->track_list_),
        [](GtkWidget *widget, gpointer) { gtk_widget_destroy(widget); },
        nullptr);
}

AlbumWidget::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(album_widget_);
}
