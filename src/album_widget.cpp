#include "album_widget.h"

#include <fmt/format.h>

#include "async_queue.h"
#include "playback_list.h"
#include "resource_cache.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

#include <libspring_logger.h>

AlbumWidget::AlbumWidget(music::Album &&album,
                         std::weak_ptr<PlaybackList> playback_list) noexcept
  : album_(std::move(album))
  , playback_list_(playback_list)
{
    LOG_INFO("AlbumWidget({}): Creating album widget for album {}",
             void_p(this), album_.title());

    auto builder =
        gtk_builder_new_from_resource(APPLICATION_PREFIX "/album_widget.ui");
    get_guarded_widget_from_builder(album_widget);
    get_widget_from_builder_simple(cover);
    get_widget_from_builder_simple(artist);
    get_widget_from_builder_simple(title);
    g_object_unref(builder);

    builder = gtk_builder_new_from_resource(APPLICATION_PREFIX
                                            "/track_list_popover.ui");
    get_guarded_widget_from_builder(track_list_popover);
    get_widget_from_builder_simple(track_list);
    get_widget_from_builder_simple(tracks_loading_spinner);
    g_object_unref(builder);

    connect_g_signal(track_list_, "row-activated", &on_track_activated, this);
    connect_g_signal(track_list_popover_, "closed", &on_popover_closed, this);

    gtk_label_set_text(artist_, album_.artist().c_str());
    gtk_label_set_text(title_, album_.title().c_str());

    async_queue::push_back_request(new async_queue::Request{
        "load_album_artwork", [this] {
            ResourceCache rc;
            auto result = rc.from_cache("album_artwork", album_.id());
            if (result.second)
            {
                GdkPixbuf *pixbuf{ nullptr };

                /* File is not cached, load it from the server and cache it */
                if (result.first.empty())
                {
                    pixbuf = load_pixbuf_from_data_scaled<200, 200>(
                        album_.artwork());
                    rc.to_cache("album_artwork", album_.id(), pixbuf);
                }
                else /* File is cached and read, create a pixbuf out of it */
                {
                    pixbuf = load_pixbuf_from_data(result.first);
                }

                async_queue::post_response(new async_queue::Response{
                    "album_artwork_ready", [this, pixbuf] {
                        gtk_image_set_from_pixbuf(cover_, pixbuf);
                        g_object_unref(pixbuf);
                    } });
            }
            else
            {
                LOG_ERROR("AlbumWidget({}): Failed to grab artwork for {}",
                          void_p(this), album_.title());
            }
        } });
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
    LOG_INFO("AlbumWidget({}): Activated album {}", void_p(this),
             album_.title());

    gtk_spinner_start(tracks_loading_spinner_);

    async_queue::push_front_request(new async_queue::Request{
        "get_tracks_for_album", [this]() {

            LOG_INFO("AlbumWidget({}): Loading tracks for {}", void_p(this),
                     album_.title());

            auto tracks = new std::vector<music::Track>();
            *tracks = album_.tracks();
            std::vector<GtkRefGuard<GtkBox>> track_list_entries;
            track_list_entries.reserve(tracks->size());

            for (const auto &track : *tracks)
            {
                auto builder = gtk_builder_new_from_resource(
                    APPLICATION_PREFIX "/track_widget.ui");

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
                    duration,
                    seconds < 10 ?
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
                    LOG_INFO("AlbumWidget({}): Tracks ready for {}",
                             void_p(this),
                             album_.title());

                    if (gtk_widget_get_visible(
                            gtk_cast<GtkWidget>(track_list_popover_)))
                    {
                        on_popover_closed(track_list_popover_, this);

                        auto &track_list_entries =
                            const_cast<std::vector<GtkRefGuard<GtkBox>> &>(entries);
                        std::unique_ptr<std::vector<music::Track>> track_list;
                        track_list.reset(tracks);

                        tracks_.clear();
                        std::size_t index { 0 };
                        for (auto &track_entry : track_list_entries)
                        {
                            gtk_container_add(
                                gtk_cast<GtkContainer>(track_list_),
                                gtk_cast<GtkWidget>(track_entry));

                            tracks_.push_back(std::make_shared<music::Track>(
                                                            std::move(
                                                            track_list->at(
                                                            index++))));
                        }

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

#if GTK_MINOR_VERSION >= 22
    gtk_popover_popup(track_list_popover_);
#else
    gtk_popover_set_transitions_enabled(track_list_popover_, true);
    gtk_widget_show(gtk_cast<GtkWidget>(track_list_popover_));
#endif
/* clang-format on */
}

void AlbumWidget::on_track_activated(GtkListBox *,
                                     GtkListBoxRow *element,
                                     AlbumWidget *self) noexcept
{
    std::size_t element_index =
        static_cast<std::size_t>(gtk_list_box_row_get_index(element));

    LOG_INFO("AlbumWidget({}): Track {} activated from album {}", void_p(self),
             self->tracks_.at(element_index)->title(), self->album_.title());

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->enqueue(self->tracks_.at(element_index));
    }
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
