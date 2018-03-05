#include "thumbnail.h"

#include <cstring>
#include <memory>
#include <unordered_map>

#include <cairo.h>
#include <gtk/gtk.h>
#include <vips/vips.h>

#include <libspring_logger.h>

#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    struct hash_pixel
    {
        std::size_t operator()(const Thumbnail::pixel &pixel) const noexcept
        {
            auto hash = std::hash<std::int32_t>{};
            return hash(static_cast<std::int32_t>(pixel.red) +
                        static_cast<std::int32_t>(pixel.green) +
                        static_cast<std::int32_t>(pixel.blue));
        }
    };

    struct eq_pixel
    {
        bool operator()(const Thumbnail::pixel &first, const Thumbnail::pixel &second) const
            noexcept
        {
            return (first.red == second.red) && (first.green == second.green) &&
                   (first.blue == second.blue);
        }
    };

    inline auto vips_image_to_argb(const VipsImage *image,
                                   Thumbnail::size new_size = { { -1, -1 } },
                                   Thumbnail::point image_offset = { { 0, 0 } }) noexcept
    {
        auto image_width = vips_image_get_width(image);
        auto image_height = vips_image_get_height(image);

        if (new_size.width == -1)
        {
            new_size.width = image_width;
            new_size.height = image_height;
        }

        const auto argb_buffer_size =
            static_cast<std::size_t>(new_size.width * new_size.height * 4);
        auto argb_buffer = std::unique_ptr<std::uint8_t[]>{ new std::uint8_t[argb_buffer_size] };
        memset(argb_buffer.get(), 0, argb_buffer_size);

        for (auto y = 0; y < new_size.height; ++y)
        {
            const bool y_has_image_data = y >= image_offset.y && y < image_offset.y + image_height;
            if (y_has_image_data)
            {
                const std::uint8_t *image_pixel = VIPS_IMAGE_ADDR(image, 0, y - image_offset.y);
                std::uint8_t *argb_pixel = argb_buffer.get() + new_size.width * 4 * y;

                for (auto x = 0; x < new_size.width; ++x)
                {
                    const bool x_has_image_data =
                        x >= image_offset.x && x < image_offset.x + image_width;
                    if (x_has_image_data)
                    {
                        argb_pixel[0] = image_pixel[2];
                        argb_pixel[1] = image_pixel[1];
                        argb_pixel[2] = image_pixel[0];
                        argb_pixel[3] = 255;
                        image_pixel += 3;
                    }

                    argb_pixel += 4;
                }
            }
        }

        return argb_buffer;
    }
}

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
    g_object_unref(background_);
    g_object_unref(image_);
    gtk_widget_destroy(container_);
}

void Thumbnail::set_image(const std::string &data,
                          Thumbnail::BackgroundType background,
                          Thumbnail::size size) noexcept
{
    if (background_ != nullptr)
    {
        g_object_unref(background_);
    }

    if (image_ != nullptr)
    {
        g_object_unref(image_);
    }

    image_ = vips_image_new_from_buffer(data.data(), data.size(), "", nullptr);

    if (image_ == nullptr)
    {
        return;
    }

    auto image_width = vips_image_get_width(image_);
    auto image_height = vips_image_get_height(image_);

    auto width_scale = static_cast<double>(size.width) / image_width;
    auto height_scale = static_cast<double>(size.height) / image_height;

    auto scaled_image = vips_image_new();
    vips_resize(image_, &scaled_image, width_scale, "vscale", height_scale, nullptr);

    g_object_unref(image_);

    image_ = scaled_image;
    vips_image_wio_input(image_);

    if (background == BackgroundType::FromImage)
    {
        background_ = vips_image_new();
        ;
        vips_gaussblur(image_, &background_, 3, nullptr);
    }

    image_width = vips_image_get_width(image_);
    image_height = vips_image_get_height(image_);

    gtk_widget_set_size_request(container_, image_width, image_height);
    gtk_widget_queue_draw(container_);
}

Thumbnail::pixel Thumbnail::dominant_color() const noexcept
{
    Thumbnail::pixel result{ 255, 255, 255 };

    if (image_ != nullptr)
    {
        std::unordered_map<Thumbnail::pixel, std::int32_t, hash_pixel, eq_pixel> colors;

        std::int32_t count{ 0 };

        constexpr std::size_t sample_offset{ 5 };
        constexpr std::uint8_t top_threshold{ 245 };
        constexpr std::uint8_t bottom_threshold{ 20 };

        const auto length = static_cast<std::size_t>(vips_image_get_width(image_)) *
                            static_cast<std::size_t>(vips_image_get_height(image_)) * 3;
        for (std::size_t y = 0; y < length; y += 3 * sample_offset)
        {
            auto pixel = *reinterpret_cast<Thumbnail::pixel *>(image_->data + y);

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
    if (self->image_ != nullptr)
    {
        auto container_width = gtk_widget_get_allocated_width(self->container_);
        auto container_height = gtk_widget_get_allocated_height(self->container_);

        std::unique_ptr<std::uint8_t[]> cairo_buffer{ nullptr };
        cairo_surface_t *drawing_surface{ nullptr };

        point artwork_offset{ { (container_width / 2) - (vips_image_get_width(self->image_) / 2),
                                (container_height / 2) -
                                    (vips_image_get_height(self->image_) / 2) } };

        VipsImage *thumbnail{ nullptr };
        VipsImage *background{ nullptr };
        std::int32_t error{ 0 };

        if (self->background_ != nullptr)
        {
            auto width_scale =
                static_cast<double>(container_width) / vips_image_get_width(self->background_);
            auto height_scale =
                static_cast<double>(container_height) / vips_image_get_height(self->background_);

            error = vips_resize(self->background_, &background, width_scale, "vscale", height_scale,
                                nullptr);

            if (!error)
            {
                error = vips_insert(background, self->image_, &thumbnail, artwork_offset.x,
                                    artwork_offset.y, nullptr);

                artwork_offset = { { 0, 0 } };
            }
        }
        else
        {
            thumbnail = self->image_;
        }

        if (!error)
        {
            /* libvips is RGB, cairo is ARGB, we have to repack the data. */
            vips_image_wio_input(thumbnail);
            cairo_buffer = vips_image_to_argb(thumbnail, { { container_width, container_height } },
                                              artwork_offset);

            g_object_unref(background);
            g_object_unref(thumbnail);

            drawing_surface = cairo_image_surface_create_for_data(
                cairo_buffer.get(), CAIRO_FORMAT_ARGB32, container_width, container_height,
                container_width * 4);

            cairo_set_source_surface(cairo_context, drawing_surface, 0, 0);
            cairo_paint(cairo_context);
            cairo_surface_destroy(drawing_surface);
        }
    }

    return false;
}
