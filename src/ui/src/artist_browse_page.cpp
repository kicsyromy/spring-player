#include <glib/gi18n.h>
#include <granite.h>
#include <gtk/gtk.h>

#include <fmt/format.h>

#include <libspring_logger.h>

#include "ui/artist_browse_page.h"

#include "utility/async_queue.h"
#include "utility/global.h"
#include "utility/pixbuf_loader.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::ui;
using namespace spring::player::playback;
using namespace spring::player::utility;

ArtistBrowsePage::ArtistBrowsePage(std::weak_ptr<Playlist> list) noexcept
  : playback_list_{ list }
  , track_list_popover_{ playback_list_ }
{
    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/artist_browse_page.ui");
    get_guarded_widget_from_builder(root_container);
    get_widget_from_builder_simple(artist_thumbnail);
    get_widget_from_builder_simple(right_pane_container);
    get_widget_from_builder_simple(artist_name);
    get_widget_from_builder_simple(popular_tracks_loading_spinner);
    get_widget_from_builder_simple(popular_tracks_listbox);
    get_widget_from_builder_simple(album_list_content);
    get_widget_from_builder_simple(album_list_loading_spinner);
    g_object_unref(builder);

    connect_g_signal(album_list_content_, "child-activated", &on_album_activated, this);
    connect_g_signal(popular_tracks_listbox_, "row-activated", &on_track_activated, this);
}

ArtistBrowsePage::~ArtistBrowsePage() noexcept
{
}

void ArtistBrowsePage::set_artist(const music::Artist &artist) noexcept
{
    clear_all();

    /* TODO: Move this logic to a common place since it's used here and ThumbnailWidget */
    async_queue::push_back_request(async_queue::Request{
        "load_artwork", [this, &artist] {
            struct header_t
            {
                std::int32_t alpha;
                std::int32_t bits_per_sample;
                std::int32_t width;
                std::int32_t height;
                std::int32_t rowstride;
            };

            using cache_t = ResourceCache<5 * sizeof(header_t)>;
            cache_t rc;

            auto cache_prefix = "artist_artwork";

            auto result = rc.from_cache(cache_prefix, artist.id());
            if (result.second)
            {
                GdkPixbuf *pixbuf{ nullptr };

                /* File is not cached, load it from the server and cache it */
                if (!result.first)
                {
                    pixbuf = load_pixbuf_from_data_scaled<200, 200>(artist.artwork());

                    auto header = reinterpret_cast<header_t *>(result.first.header.data());
                    header->alpha = gdk_pixbuf_get_has_alpha(pixbuf);
                    header->bits_per_sample = gdk_pixbuf_get_bits_per_sample(pixbuf);
                    header->width = gdk_pixbuf_get_width(pixbuf);
                    header->height = gdk_pixbuf_get_height(pixbuf);
                    header->rowstride = gdk_pixbuf_get_rowstride(pixbuf);
                    guint size{ 0 };
                    result.first.buffer.data = gdk_pixbuf_get_pixels_with_length(pixbuf, &size);
                    result.first.buffer.size = size;

                    rc.to_cache(cache_prefix, artist.id(), result.first);
                }
                else /* File is cached and read, create a pixbuf out of it */
                {
                    auto header = reinterpret_cast<header_t *>(result.first.header.data());
                    pixbuf = gdk_pixbuf_new_from_data(
                        result.first.buffer.data, GDK_COLORSPACE_RGB, header->alpha,
                        header->bits_per_sample, header->width, header->height, header->rowstride,
                        [](guchar *data, void *) { delete[] data; }, nullptr);
                }

                async_queue::post_response(async_queue::Response{ "artwork_ready", [this, pixbuf] {
                                                                     gtk_image_set_from_pixbuf(
                                                                         artist_thumbnail_, pixbuf);
                                                                     g_object_unref(pixbuf);
                                                                 } });
            }
            else
            {
                LOG_ERROR("ArtistBrowsePage({}): Failed to grab artwork for {}", void_p(this),
                          artist.id());
            }
        } });

    gtk_label_set_text(artist_name_, artist.name().c_str());

    gtk_spinner_start(popular_tracks_loading_spinner_);
    async_queue::push_front_request(async_queue::Request{
        "load_popular_tracks_for_artist", [this, &artist] {
            auto tracks = load_popular_tracks(artist);
            auto callback =
                std::bind(&ArtistBrowsePage::on_tracks_loaded, this, tracks.first, tracks.second);
            async_queue::post_response(async_queue::Response{ "tracks_ready", callback });
        } });

    gtk_spinner_start(album_list_loading_spinner_);
    async_queue::push_front_request(async_queue::Request{
        "load_albums_for_artist", [this, &artist] {
            auto albums = load_albums(artist);
            auto callback = std::bind(&ArtistBrowsePage::on_albums_loaded, this, albums);
            async_queue::post_response(async_queue::Response{ "tracks_ready", callback });
        } });
}

GtkWidget *ArtistBrowsePage::operator()() noexcept
{
    return gtk_cast<GtkWidget>(root_container_);
}

void ArtistBrowsePage::clear_all() noexcept
{
    popular_tracks_.clear();
    album_thumbnails_.clear();
    gtk_container_foreach(gtk_cast<GtkContainer>(popular_tracks_listbox_),
                          [](GtkWidget *widget, gpointer) { gtk_widget_destroy(widget); }, nullptr);
    gtk_container_foreach(gtk_cast<GtkContainer>(album_list_content_),
                          [](GtkWidget *widget, gpointer) { g_object_unref(widget); }, nullptr);
}

std::pair<std::vector<music::Track> *, std::vector<utility::GObjectGuard<GtkBox>> *>
ArtistBrowsePage::load_popular_tracks(const music::Artist &artist) const noexcept
{
    LOG_INFO("ArtistBrowsePage({}): Loading popular tracks for artist {}", void_p(this),
             artist.name());

    auto tracks = new std::vector<music::Track>();
    *tracks = artist.popularTracks(5);
    auto track_list_entries = new std::vector<GObjectGuard<GtkBox>>;
    track_list_entries->reserve(tracks->size());

    std::size_t index{ 1 };
    for (const auto &track : *tracks)
    {
        auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/artist_browse_page.ui");
        get_widget_from_builder_new(GtkBox, track_list_entry);
        get_widget_from_builder_new(GtkLabel, track_number);
        get_widget_from_builder_new(GtkLabel, track_title);
        get_widget_from_builder_new(GtkLabel, track_duration);

        gtk_label_set_text(track_number, fmt::format("{}", index++).c_str());
        gtk_label_set_text(track_title, track.title().c_str());

        auto duration_seconds = track.duration().count() / 1000;
        auto minutes = duration_seconds / 60;
        auto seconds = duration_seconds % 60;
        gtk_label_set_text(track_duration, seconds < 10 ?
                                               fmt::format("{}:0{}", minutes, seconds).c_str() :
                                               fmt::format("{}:{}", minutes, seconds).c_str());

        track_list_entries->emplace_back(track_list_entry);

        g_object_unref(builder);
    }

    return { tracks, track_list_entries };
}

void ArtistBrowsePage::on_tracks_loaded(
    std::vector<music::Track> *tracks,
    std::vector<utility::GObjectGuard<GtkBox>> *track_widgets) noexcept
{
    LOG_INFO("ArtistBrowsePage({}): Popular tracks ready for artist {}", void_p(this),
             gtk_label_get_text(artist_name_));

    std::unique_ptr<std::vector<music::Track>> track_list{ tracks };
    std::unique_ptr<std::vector<GObjectGuard<GtkBox>>> track_widget_list{ track_widgets };

    std::size_t index{ 0 };
    for (auto &track_widget : *track_widget_list)
    {
        gtk_container_add(gtk_cast<GtkContainer>(popular_tracks_listbox_),
                          gtk_cast<GtkWidget>(track_widget));
        popular_tracks_.push_back(
            std::make_shared<music::Track>(std::move(track_list->at(index++))));
    }

    gtk_spinner_stop(popular_tracks_loading_spinner_);
}

std::vector<ThumbnailWidget<music::Album>> *ArtistBrowsePage::load_albums(
    const music::Artist &artist) const noexcept
{
    LOG_INFO("ArtistBrowsePage({}): Loading albums for artist {}", void_p(this), artist.name());

    auto albums = artist.albums();
    auto album_widgets = new std::vector<ThumbnailWidget<music::Album>>;
    album_widgets->reserve(albums.size());

    std::string main_text;
    for (auto &album : albums)
    {
        main_text = album.title();
        album_widgets->emplace_back(std::move(album), main_text, "", "album_artwork",
                                    playback_list_);
    }

    return album_widgets;
}

void ArtistBrowsePage::on_albums_loaded(
    std::vector<ThumbnailWidget<music::Album>> *album_widgets) noexcept
{
    LOG_INFO("ArtistBrowsePage({}): Albums ready for artist {}", void_p(this),
             gtk_label_get_text(artist_name_));

    std::unique_ptr<std::vector<ThumbnailWidget<music::Album>>> album_widget_list{ album_widgets };
    album_thumbnails_ = std::move(*album_widget_list);

    for (auto &widget : album_thumbnails_)
    {
        gtk_flow_box_insert(album_list_content_, widget(), -1);
    }

    gtk_spinner_stop(album_list_loading_spinner_);
}

void ArtistBrowsePage::on_track_activated(GtkListBox *,
                                          GtkListBoxRow *element,
                                          ArtistBrowsePage *self) noexcept
{
    auto index = static_cast<std::size_t>(gtk_list_box_row_get_index(element));

    LOG_INFO("ArtistBrowsePage({}): Track {} activated", void_p(self),
             self->popular_tracks_.at(index)->title());

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->enqueue(self->popular_tracks_.at(index));
    }
}

void ArtistBrowsePage::on_album_activated(GtkFlowBox *,
                                          GtkFlowBoxChild *element,
                                          ArtistBrowsePage *self) noexcept
{
    auto index = static_cast<std::size_t>(gtk_flow_box_child_get_index(element));
    auto &activated_album = self->album_thumbnails_.at(index);

    LOG_INFO("ArtistBrowsePage({}): Album {} activated", void_p(self),
             activated_album.content_provider().title());

    self->track_list_popover_.show(activated_album.content_provider(), activated_album());
}
