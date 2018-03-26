#ifndef SPRING_PLAYER_THUMBNAIL_H
#define SPRING_PLAYER_THUMBNAIL_H

#include <string>

#include <libspring_global.h>

#include "utility/forward_declarations.h"

namespace spring
{
    namespace player
    {
        class Thumbnail
        {
        public:
            union pair_t {
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

            using point_t = pair_t;
            using size_t = pair_t;

            struct pixel_t
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
                           size_t size = { { 200, 200 } }) noexcept;

        public:
            pixel_t dominant_color() const noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            static void on_size_allocated(void *, void *, Thumbnail *self) noexcept;
            static std::int32_t on_draw_requested(GtkWidget *,
                                                  cairo_t *cairo_context,
                                                  Thumbnail *self) noexcept;

        private:
            GtkWidget *container_{ nullptr };
            GdkPixbuf *image_{ nullptr };
            GdkPixbuf *background_{ nullptr };

        private:
            DISABLE_COPY(Thumbnail)
            DISABLE_MOVE(Thumbnail)
        };
    }
}

#endif // !SPRING_PLAYER_THUMBNAIL_H
