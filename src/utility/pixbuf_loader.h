#ifndef SPRING_PLAYER_UTILITY_PIXBUF_LOADER_H
#define SPRING_PLAYER_UTILITY_PIXBUF_LOADER_H

#include <string>

#include "forward_declarations.h"

namespace spring
{
    namespace player
    {
        namespace utility
        {
            inline GdkPixbuf *load_pixbuf_from_data(const std::string &data) noexcept
            {
                auto loader = gdk_pixbuf_loader_new();

                gdk_pixbuf_loader_write(loader, reinterpret_cast<const std::uint8_t *>(data.data()),
                                        data.size(), nullptr);

                auto pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

                gdk_pixbuf_loader_close(loader, nullptr);

                return pixbuf;
            }

            template <int width, int height>
            inline GdkPixbuf *load_pixbuf_from_data_scaled(const std::string &data,
                                                           int w = width,
                                                           int h = height) noexcept
            {
                auto pixbuf = load_pixbuf_from_data(data);
                auto scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, w, h, GDK_INTERP_TILES);

                g_object_unref(pixbuf);

                return scaled_pixbuf;
            }
        }
    }
}

#endif // !SPRING_PLAYER_UTILITY_PIXBUF_LOADER_H
