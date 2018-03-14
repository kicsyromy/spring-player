#ifndef SPRING_PLAYER_RESOURCE_CACHE_H
#define SPRING_PLAYER_RESOURCE_CACHE_H

#include <string>

#include <libspring_global.h>

#include "utility/compatibility.h"
#include "utility/forward_declarations.h"

namespace spring
{
    namespace player
    {
        class ResourceCache
        {
        public:
            ResourceCache() noexcept;
            ~ResourceCache() noexcept;

        public:
            void to_cache(const utility::string_view &prefix,
                          const utility::string_view &resource_id,
                          const char *data,
                          std::size_t size) noexcept;

            void to_cache(const utility::string_view &prefix,
                          const utility::string_view &resource_id,
                          GdkPixbuf *pixbuf) noexcept;

            std::pair<std::string, bool> from_cache(
                const utility::string_view &prefix,
                const utility::string_view &resource_id) noexcept;

        private:
            DISABLE_COPY(ResourceCache)
            DISABLE_MOVE(ResourceCache)
        };
    }
}

#endif // !SPRING_PLAYER_RESOURCE_CACHE_H
