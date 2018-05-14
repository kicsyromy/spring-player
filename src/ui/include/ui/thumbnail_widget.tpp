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
  : content_provider_{ std::move(content_provider) }
  , playback_list_{ playback_list }
{
    LOG_INFO("ThumbnailWidget({}): Creating...", void_p(this));

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/thumbnail_widget.ui");
    thumbnail_widget_ = gtk_cast<GtkBox>(gtk_builder_get_object(builder, "thumbnail_widget"));
    image_ = gtk_cast<GtkImage>(gtk_builder_get_object(builder, "image"));
    main_title_ = gtk_cast<GtkLabel>(gtk_builder_get_object(builder, "main_title"));
    secondary_title_ = gtk_cast<GtkLabel>(gtk_builder_get_object(builder, "secondary_title"));
    g_object_unref(builder);

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
                        [](guchar *data, void *) { delete[] data; }, nullptr);
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
                          content_provider_.id());
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

template <typename ContentProvider>
const ContentProvider &ThumbnailWidget<ContentProvider>::content_provider() const noexcept
{
    return content_provider_;
}

template <typename ContentProvider>
GtkWidget *ThumbnailWidget<ContentProvider>::operator()() noexcept
{
    return gtk_cast<GtkWidget>(thumbnail_widget_);
}
