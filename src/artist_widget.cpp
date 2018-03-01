#include "artist_widget.h"

#include <libspring_logger.h>

#include "async_queue.h"
#include "resource_cache.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

ArtistWidget::ArtistWidget(music::Artist &&artist) noexcept
  : artist_(std::move(artist))
{
    LOG_INFO("ArtistWidget({}): Creating artist widget for {}", void_p(this),
             artist_.name());

    auto builder =
        gtk_builder_new_from_resource(APPLICATION_PREFIX "/artist_widget.ui");
    get_guarded_widget_from_builder(artist_widget);
    get_widget_from_builder_simple(thumbnail);
    get_widget_from_builder_simple(name);

    g_object_unref(builder);

    gtk_label_set_text(name_, artist_.name().c_str());

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
                    pixbuf = load_pixbuf_from_data_scaled<200, 200>(
                        artist_.thumbnail());
                    rc.to_cache("artist_thumbnails", artist_.id(), pixbuf);
                }
                else /* File is cached and read, create a pixbuf out of it */
                {
                    pixbuf = load_pixbuf_from_data(result.first);
                }

                async_queue::post_response(new async_queue::Response{
                    "artist_thumbnail_ready", [this, pixbuf] {
                        gtk_image_set_from_pixbuf(thumbnail_, pixbuf);
                        g_object_unref(pixbuf);
                    } });
            }
            else
            {
                LOG_ERROR("AlbumWidget({}): Failed to grab artwork for {}",
                          void_p(this), artist_.name());
            }
        } });
}

ArtistWidget::~ArtistWidget() noexcept
{
}

GtkWidget *ArtistWidget::container() noexcept
{
    return gtk_cast<GtkWidget>(artist_widget_);
}
