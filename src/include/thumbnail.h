#ifndef SPRING_PLAYER_THUMBNAIL_H
#define SPRING_PLAYER_THUMBNAIL_H

#include <string>

#include <libspring_global.h>

using GtkWidget = struct _GtkWidget;
using cairo_t = struct _cairo;
using VipsImage = struct _VipsImage;

namespace spring
{
    namespace player
    {
        class Thumbnail
        {
        public:
            union pair {
                struct
                {
                    std::int32_t x;
                    std::int32_t y;
                };

                struct
                {
                    std::int32_t width;
                    std::int32_t height;
                };
            };

            using point = pair;
            using size = pair;

            struct pixel
            {
                std::uint8_t red;
                std::uint8_t green;
                std::uint8_t blue;
            };

            enum class BackgroundType
            {
                Transparent,
                FromImage
            };

        public:
            Thumbnail() noexcept;
            ~Thumbnail() noexcept;

        public:
            void set_image(const std::string &data,
                           BackgroundType background = BackgroundType::Transparent,
                           size size = { { 200, 200 } }) noexcept;

        public:
            pixel dominant_color() const noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            static void on_size_allocated(void *, void *, Thumbnail *self) noexcept;
            static std::int32_t on_draw_requested(GtkWidget *,
                                                  cairo_t *cairo_context,
                                                  Thumbnail *self) noexcept;

        private:
            GtkWidget *container_{ nullptr };
            VipsImage *image_{ nullptr };
            VipsImage *background_{ nullptr };

        private:
            DISABLE_COPY(Thumbnail)
            DISABLE_MOVE(Thumbnail)
        };
    }
}

#endif // !SPRING_PLAYER_THUMBNAIL_H
