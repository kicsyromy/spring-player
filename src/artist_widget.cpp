#include "artist_widget.h"

#include <libspring_logger.h>

#include "async_queue.h"
#include "resource_cache.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

ArtistWidget::ArtistWidget(music::Artist &&artist,
                           std::weak_ptr<PlaybackList> playback_list) noexcept
  : artist_(std::move(artist))
  , playback_list_(playback_list)
{
    LOG_INFO("ArtistWidget({}): Creating artist widget for {}", void_p(this), artist_.name());

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/artist_widget.ui");
    get_guarded_widget_from_builder(artist_widget);
    get_widget_from_builder_simple(thumbnail);
    get_widget_from_builder_simple(name);
    get_widget_from_builder_simple(album_count);
    g_object_unref(builder);

    builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/track_list_popover.ui");
    get_guarded_widget_from_builder(track_list_popover);
    get_widget_from_builder_simple(track_list);
    get_widget_from_builder_simple(tracks_loading_spinner);
    g_object_unref(builder);

    connect_g_signal(track_list_, "row-activated", &on_track_activated, this);
    connect_g_signal(track_list_popover_, "closed", &on_popover_closed, this);

    auto album_count = artist_.albums().size();
    gtk_label_set_text(name_, artist_.name().c_str());
    gtk_label_set_text(album_count_,
                       fmt::format("{} album{}", album_count, album_count > 1 ? "s" : "").c_str());

    async_queue::push_back_request(new async_queue::Request{
        "load_artist_thumbnail", [this] {
            ResourceCache rc;
            auto result = rc.from_cache("artist_thumbnails", artist_.id());
            if (result.second)
            {
                GdkPixbuf *pixbuf{ nullptr };

                /* File is not cached, load it from the server and cache it */
                if (result.first.empty())
                {
                    pixbuf = load_pixbuf_from_data_scaled<200, 200>(artist_.thumbnail());
                    rc.to_cache("artist_thumbnails", artist_.id(), pixbuf);
                }
                else /* File is cached and read, create a pixbuf out of it */
                {
                    pixbuf = load_pixbuf_from_data(result.first);
                }

                async_queue::post_response(
                    new async_queue::Response{ "artist_thumbnail_ready", [this, pixbuf] {
                                                  gtk_image_set_from_pixbuf(thumbnail_, pixbuf);
                                                  g_object_unref(pixbuf);
                                              } });
            }
            else
            {
                LOG_ERROR("AlbumWidget({}): Failed to grab artwork for {}", void_p(this),
                          artist_.name());
            }
        } });
}

ArtistWidget::~ArtistWidget() noexcept
{
}

void ArtistWidget::activated() noexcept
{
    LOG_INFO("AlbumWidget({}): Activated album {}", void_p(this), artist_.name());

    gtk_spinner_start(tracks_loading_spinner_);

    async_queue::push_front_request(new async_queue::Request{
        "load_tracks_for_artist", [this] {
            auto tracks = load_tracks();
            auto callback =
                std::bind(&ArtistWidget::on_tracks_loaded, this, tracks.first, tracks.second);
            async_queue::post_response(
                new async_queue::Response{ "tracks_for_artist_ready", callback });
        } });

    if (gtk_popover_get_relative_to(track_list_popover_) == nullptr)
    {
        gtk_popover_set_relative_to(track_list_popover_, gtk_cast<GtkWidget>(thumbnail_));
    }

#if GTK_MINOR_VERSION >= 22
    gtk_popover_popup(track_list_popover_);
#else
    gtk_popover_set_transitions_enabled(track_list_popover_, true);
    gtk_widget_show(gtk_cast<GtkWidget>(track_list_popover_));
#endif
}

std::pair<std::vector<music::Track> *, std::vector<GtkRefGuard<GtkBox>> *> ArtistWidget::
    load_tracks() const noexcept
{
    LOG_INFO("ArtistWidget({}): Loading tracks for {}", static_cast<const void *>(this),
             artist_.name());

    auto tracks = new std::vector<music::Track>();
    *tracks = artist_.tracks();
    auto track_list_entries = new std::vector<GtkRefGuard<GtkBox>>;
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

void ArtistWidget::on_tracks_loaded(std::vector<music::Track> *tracks,
                                    std::vector<GtkRefGuard<GtkBox>> *track_widgets) noexcept
{
    LOG_INFO("ArtistWidget({}): Tracks ready for {}", void_p(this), artist_.name());

    if (gtk_widget_get_visible(gtk_cast<GtkWidget>(track_list_popover_)))
    {
        on_popover_closed(track_list_popover_, this);

        std::unique_ptr<std::vector<music::Track>> track_list{ tracks };
        std::unique_ptr<std::vector<GtkRefGuard<GtkBox>>> track_widget_list{ track_widgets };

        tracks_.clear();
        std::size_t index{ 0 };
        for (auto &track_widget : *track_widget_list)
        {
            gtk_container_add(gtk_cast<GtkContainer>(track_list_),
                              gtk_cast<GtkWidget>(track_widget));

            tracks_.push_back(std::make_shared<music::Track>(std::move(track_list->at(index++))));
        }

        gtk_widget_show_all(gtk_cast<GtkWidget>(track_list_));
    }

    gtk_spinner_stop(tracks_loading_spinner_);
}

void ArtistWidget::on_track_activated(GtkListBox *,
                                      GtkListBoxRow *element,
                                      ArtistWidget *self) noexcept
{
    std::size_t element_index = static_cast<std::size_t>(gtk_list_box_row_get_index(element));

    LOG_INFO("ArtistWidget({}): Track {} activated from artist {}", void_p(self),
             self->tracks_.at(element_index)->title(), self->artist_.name());

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->enqueue(self->tracks_.at(element_index));
    }
}

void ArtistWidget::on_popover_closed(GtkPopover *, ArtistWidget *self) noexcept
{
    gtk_container_foreach(gtk_cast<GtkContainer>(self->track_list_),
                          [](GtkWidget *widget, gpointer) { gtk_widget_destroy(widget); }, nullptr);
}

GtkWidget *ArtistWidget::container() noexcept
{
    return gtk_cast<GtkWidget>(artist_widget_);
}
