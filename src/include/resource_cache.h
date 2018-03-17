#ifndef SPRING_PLAYER_RESOURCE_CACHE_H
#define SPRING_PLAYER_RESOURCE_CACHE_H

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <gdk/gdk.h>
#include <glib-object.h>

#include <fmt/format.h>

#include <libspring_global.h>
#include <libspring_logger.h>

#include "application_settings.h"

#include "utility/compatibility.h"
#include "utility/forward_declarations.h"

namespace spring
{
    namespace player
    {
        template <std::size_t header_length> class ResourceCache
        {
            struct Resource
            {
                std::array<std::uint8_t, header_length> header{};
                struct
                {
                    std::uint8_t *data{};
                    std::size_t size{ 0 };
                } buffer{};

                operator bool() const noexcept { return buffer.size > 0; }
            };

        public:
            ResourceCache() noexcept = default;
            ~ResourceCache() noexcept = default;

        public:
            void to_cache(const utility::string_view &prefix,
                          const utility::string_view &resource_id,
                          const Resource &resource) noexcept;

            std::pair<Resource, bool> from_cache(const utility::string_view &prefix,
                                                 const utility::string_view &resource_id) noexcept;

        private:
            struct posix_fd_t
            {
            public:
                inline posix_fd_t(utility::string_view path, std::int32_t open_flags) noexcept
                  : handle_(open(path.data(), open_flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))
                {
                    error_ = errno;
                }
                inline ~posix_fd_t() noexcept
                {
                    if (*this)
                    {
                        close(handle_);
                    }
                }
                inline operator bool() const noexcept { return handle_ != -1; }
                inline std::int32_t operator()() noexcept { return handle_; }
                inline std::int32_t error() const noexcept { return error_; }

            private:
                std::int32_t error_;
                std::int32_t handle_;
            };
        };

#include "resource_cache.tpp"
    }
}

#endif // !SPRING_PLAYER_RESOURCE_CACHE_H
