#ifndef SPRING_PLAYER_THUMBNAIL_WIDGET_H
#include "thumbnail_widget.h"
#endif

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

template <typename ContentProvider>
ThumbnailWidget<ContentProvider>::ThumbnailWidget(
    ContentProvider &&content_provider,
    string_view main_text,
    string_view secondary_text,
    string_view cache_prefix,
    std::weak_ptr<PlaybackList> playback_list) noexcept
  : content_provider_(std::move(content_provider))
  , playback_list_(playback_list)
{
    LOG_INFO("ThumbnailWidget({}): Creating...", void_p(this));

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/thumbnail_widget.ui");
    thumbnail_widget_ = gtk_cast<GtkBox>(gtk_builder_get_object(builder, "thumbnail_widget"));
    image_ = gtk_cast<GtkImage>(gtk_builder_get_object(builder, "image"));
    main_title_ = gtk_cast<GtkLabel>(gtk_builder_get_object(builder, "main_title"));
    secondary_title_ = gtk_cast<GtkLabel>(gtk_builder_get_object(builder, "secondary_title"));
    g_object_unref(builder);

    builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/track_list_popover.ui");
    listbox_popover_ = gtk_cast<GtkPopover>(gtk_builder_get_object(builder, "listbox_popover"));
    listbox_ = gtk_cast<GtkListBox>(gtk_builder_get_object(builder, "listbox"));
    loading_spinner_ = gtk_cast<GtkSpinner>(gtk_builder_get_object(builder, "loading_spinner"));
    g_object_unref(builder);

    connect_g_signal(listbox_, "row-activated", &on_track_activated, this);
    connect_g_signal(listbox_popover_, "closed", &on_popover_closed, this);

    gtk_label_set_text(main_title_, main_text.data());
    gtk_label_set_text(secondary_title_, secondary_text.data());

    std::string text{ secondary_text };

    async_queue::push_back_request(async_queue::Request{
        "load_artwork", [this, cache_prefix, text] {

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

            auto result = rc.from_cache(cache_prefix, content_provider_.id());
            if (result.second)
            {
                GdkPixbuf *pixbuf{ nullptr };

                /* File is not cached, load it from the server and cache it */
                if (!result.first)
                {
                    pixbuf = load_pixbuf_from_data_scaled<200, 200>(content_provider_.artwork());

                    auto header = reinterpret_cast<header_t *>(result.first.header.data());
                    header->alpha = gdk_pixbuf_get_has_alpha(pixbuf);
                    header->bits_per_sample = gdk_pixbuf_get_bits_per_sample(pixbuf);
                    header->width = gdk_pixbuf_get_width(pixbuf);
                    header->height = gdk_pixbuf_get_height(pixbuf);
                    header->rowstride = gdk_pixbuf_get_rowstride(pixbuf);
                    guint size{ 0 };
                    result.first.buffer.data = gdk_pixbuf_get_pixels_with_length(pixbuf, &size);
                    result.first.buffer.size = size;

                    rc.to_cache(cache_prefix, content_provider_.id(), result.first);
                }
                else /* File is cached and read, create a pixbuf out of it */
                {
                    auto header = reinterpret_cast<header_t *>(result.first.header.data());
                    pixbuf = gdk_pixbuf_new_from_data(
                        result.first.buffer.data, GDK_COLORSPACE_RGB, header->alpha,
                        header->bits_per_sample, header->width, header->height, header->rowstride,
                        [](guchar *data, void *) { delete data; }, nullptr);
                }

                async_queue::post_response(async_queue::Response{ "artwork_ready", [this, pixbuf] {
                                                                     gtk_image_set_from_pixbuf(
                                                                         image_, pixbuf);
                                                                     g_object_unref(pixbuf);
                                                                 } });
            }
            else
            {
                LOG_ERROR("ThumbnailWidget({}): Failed to grab artwork for {}", void_p(this),
                          content_provider_.title());
            }
        } });
}

template <typename ContentProvider>
string_view ThumbnailWidget<ContentProvider>::main_title() const noexcept
{
    return gtk_label_get_text(main_title_);
}

template <typename ContentProvider>
string_view ThumbnailWidget<ContentProvider>::secondary_title() const noexcept
{
    return gtk_label_get_text(secondary_title_);
}

template <typename ContentProvider> void ThumbnailWidget<ContentProvider>::activated() noexcept
{
    LOG_INFO("ThumbnailWidget({}): Activated", void_p(this));

    gtk_spinner_start(loading_spinner_);

    async_queue::push_front_request(async_queue::Request{
        "load_tracks_for_album", [this] {
            auto tracks = load_tracks();
            auto callback = std::bind(&ThumbnailWidget<ContentProvider>::on_tracks_loaded, this,
                                      tracks.first, tracks.second);
            async_queue::post_response(async_queue::Response{ "tracks_ready", callback });
        } });

    if (gtk_popover_get_relative_to(listbox_popover_) == nullptr)
    {
        gtk_popover_set_relative_to(listbox_popover_, gtk_cast<GtkWidget>(image_));
    }

#if GTK_MINOR_VERSION >= 22
    gtk_popover_popup(listbox_popover_);
#else
    gtk_popover_set_transitions_enabled(listbox_popover_, true);
    gtk_widget_show(gtk_cast<GtkWidget>(listbox_popover_));
#endif
}

template <typename ContentProvider>
GtkWidget *ThumbnailWidget<ContentProvider>::operator()() noexcept
{
    return gtk_cast<GtkWidget>(thumbnail_widget_);
}

template <typename ContentProvider>
std::pair<std::vector<music::Track> *, std::vector<utility::GObjectGuard<GtkBox>> *> spring::
    player::ThumbnailWidget<ContentProvider>::load_tracks() const noexcept
{
    LOG_INFO("ThumbnailWidget({}): Loading tracks", void_p(this));

    auto tracks = new std::vector<music::Track>();
    *tracks = content_provider_.tracks();
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

template <typename ContentProvider>
void ThumbnailWidget<ContentProvider>::on_tracks_loaded(
    std::vector<music::Track> *tracks,
    std::vector<utility::GObjectGuard<GtkBox>> *track_widgets) noexcept
{
    LOG_INFO("ThumbnailWidget({}): Tracks ready", void_p(this));

    if (gtk_widget_get_visible(gtk_cast<GtkWidget>(listbox_popover_)))
    {
        on_popover_closed(listbox_popover_, this);

        std::unique_ptr<std::vector<music::Track>> track_list{ tracks };
        std::unique_ptr<std::vector<GObjectGuard<GtkBox>>> track_widget_list{ track_widgets };

        tracks_.clear();
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

template <typename ContentProvider>
void ThumbnailWidget<ContentProvider>::on_track_activated(GtkListBox *,
                                                          GtkListBoxRow *element,
                                                          ThumbnailWidget *self) noexcept
{
    std::size_t element_index = static_cast<std::size_t>(gtk_list_box_row_get_index(element));

    LOG_INFO("ThumbnailWidget({}): Track {} activated from {}", void_p(self),
             self->tracks_.at(element_index)->title(), self->content_provider_.title());

    auto playlist = self->playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->enqueue(self->tracks_.at(element_index));
    }
}

template <typename ContentProvider>
void ThumbnailWidget<ContentProvider>::on_popover_closed(GtkPopover *,
                                                         ThumbnailWidget *self) noexcept
{
    gtk_container_foreach(gtk_cast<GtkContainer>(self->listbox_),
                          [](GtkWidget *widget, gpointer) { gtk_widget_destroy(widget); }, nullptr);
}
