#include <chrono>
#include <cstring>
#include <memory>
#include <unordered_map>

#include <cairo.h>
#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "thumbnail.h"

#include "utility/exponential_blur.h"
#include "utility/global.h"
#include "utility/gtk_helpers.h"
#include "utility/pixbuf_loader.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    struct hash_pixel
    {
        std::size_t operator()(const Thumbnail::pixel_t &pixel) const noexcept
        {
            auto hash = std::hash<std::int32_t>{};
            return hash(255 - static_cast<std::int32_t>(pixel.red) +
                        static_cast<std::int32_t>(pixel.green) +
                        static_cast<std::int32_t>(pixel.blue) * 2);
        }
    };

    struct eq_pixel
    {
        bool operator()(const Thumbnail::pixel_t &first, const Thumbnail::pixel_t &second) const
            noexcept
        {
            return (first.red == second.red) && (first.green == second.green) &&
                   (first.blue == second.blue);
        }
    };
} // namespace

Thumbnail::Thumbnail() noexcept
  : container_(gtk_drawing_area_new())
{
    g_object_ref(container_);
    connect_g_signal(container_, "size-allocate", &on_size_allocated, this);
    connect_g_signal(container_, "draw", &on_draw_requested, this);
    gtk_widget_set_visible(container_, true);
}

Thumbnail::~Thumbnail() noexcept
{
    if (background_ != nullptr)
    {
        g_object_unref(background_);
    }

    if (image_ != nullptr)
    {
        g_object_unref(image_);
    }

    gtk_widget_destroy(container_);
}

void Thumbnail::set_image(const std::string &data,
                          Thumbnail::BackgroundType background,
                          size_t size) noexcept
{
    if (background_ != nullptr)
    {
        g_object_unref(background_);
    }

    if (image_ != nullptr)
    {
        g_object_unref(image_);
    }

    const auto start = std::chrono::high_resolution_clock::now();
    auto thumbnail = utility::load_pixbuf_from_data(data);
    image_ = gdk_pixbuf_scale_simple(thumbnail, size.width, size.height, GDK_INTERP_TILES);

    if (background == Thumbnail::BackgroundType::FromImage)
    {
        background_ = gdk_pixbuf_copy(image_);

        auto channels = gdk_pixbuf_get_n_channels(background_);
        auto pixels = gdk_pixbuf_get_pixels(background_);
        const size_t size{ { gdk_pixbuf_get_width(background_),
                             gdk_pixbuf_get_height(background_) } };
        const auto radius{ 5 };

        if (channels == 3)
        {
            exponential_blur<3>(pixels, size.width, size.height, radius);
        }
        else
        {
            exponential_blur<4>(pixels, size.width, size.height, radius);
        }
    }

    g_object_unref(thumbnail);

    const auto end = std::chrono::high_resolution_clock::now();
    LOG_INFO("Loading image took: {}ms",
             std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

    gtk_widget_set_size_request(container_, size.width, size.height);
    gtk_widget_queue_draw(container_);
}

Thumbnail::pixel_t Thumbnail::dominant_color() const noexcept
{
    pixel_t result{ 255, 255, 255 };

    if (image_ != nullptr)
    {
        std::unordered_map<pixel_t, std::int32_t, hash_pixel, eq_pixel> colors;

        std::int32_t count{ 0 };
        auto channel_count = static_cast<std::size_t>(gdk_pixbuf_get_n_channels(image_));

        constexpr std::size_t sample_offset{ 5 };
        constexpr std::uint8_t top_threshold{ 245 };
        constexpr std::uint8_t bottom_threshold{ 20 };

        const auto length = static_cast<std::size_t>(gdk_pixbuf_get_width(image_)) *
                            static_cast<std::size_t>(gdk_pixbuf_get_height(image_)) * channel_count;
        for (std::size_t y = 0; y < length; y += channel_count * sample_offset)
        {
            auto pixel = *reinterpret_cast<pixel_t *>(gdk_pixbuf_get_pixels(image_) + y);

            if ((pixel.red > top_threshold && pixel.green > top_threshold) ||
                (pixel.red > top_threshold && pixel.blue > top_threshold) ||
                (pixel.blue > top_threshold && pixel.green > top_threshold))
            {
                continue;
            }

            if ((pixel.red < bottom_threshold && pixel.green < bottom_threshold) ||
                (pixel.red < bottom_threshold && pixel.blue < bottom_threshold) ||
                (pixel.blue < bottom_threshold && pixel.green < bottom_threshold))
            {
                continue;
            }

            pixel.red /= 10;
            pixel.red *= 10;

            pixel.green /= 10;
            pixel.green *= 10;

            pixel.blue /= 10;
            pixel.blue *= 10;

            auto it = colors.find(pixel);
            if (it != colors.end())
            {
                ++(it->second);
                if (it->second > count)
                {
                    count = it->second;
                    result = it->first;
                }
            }
            else
            {
                colors.insert({ pixel, 1 });
            }
        }
    }

    return result;
}

GtkWidget *Thumbnail::operator()() noexcept
{
    return container_;
}

void Thumbnail::on_size_allocated(void *, void *, Thumbnail *self) noexcept
{
    gtk_widget_queue_draw(self->container_);
}

std::int32_t Thumbnail::on_draw_requested(GtkWidget *,
                                          cairo_t *cairo_context,
                                          Thumbnail *self) noexcept
{
    const auto start = std::chrono::high_resolution_clock::now();
    if (self->image_ != nullptr)
    {
        const size_t container_size{ { gtk_widget_get_allocated_width(self->container_),
                                       gtk_widget_get_allocated_height(self->container_) } };

        cairo_surface_t *background = cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32, container_size.width, container_size.height);
        cairo_t *background_context = cairo_create(background);

        size_t background_size{ { gdk_pixbuf_get_width(self->background_),
                                  gdk_pixbuf_get_height(self->background_) } };

        const auto x_scale = static_cast<double>(container_size.width) / background_size.width;
        const auto y_scale = static_cast<double>(container_size.height) / background_size.height;

        double background_scale{ 0.0 };
        if (x_scale > y_scale)
        {
            background_scale = x_scale;
        }
        else
        {
            background_scale = y_scale;
        }

        cairo_scale(background_context, background_scale, background_scale);

        gdk_cairo_set_source_pixbuf(background_context, self->background_, 0, 0);
        cairo_paint(background_context);

        cairo_surface_flush(background);
        cairo_destroy(background_context);

        cairo_set_source_surface(cairo_context, background, 0, 0);
        cairo_paint(cairo_context);
        cairo_surface_destroy(background);

        const point_t artwork_offset{
            { (container_size.width >> 1) - (gdk_pixbuf_get_width(self->image_) >> 1),
              (container_size.height >> 1) - (gdk_pixbuf_get_height(self->image_) >> 1) }
        };

        gdk_cairo_set_source_pixbuf(cairo_context, self->image_, artwork_offset.x,
                                    artwork_offset.y);
        cairo_paint(cairo_context);
    }
    const auto end = std::chrono::high_resolution_clock::now();
    LOG_INFO("Rendering took: {}ms",
             std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

    return false;
}
