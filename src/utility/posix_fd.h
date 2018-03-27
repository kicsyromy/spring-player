#ifndef SPRING_PLAYER_UTILITY_POSIX_FD_H
#define SPRING_PLAYER_UTILITY_POSIX_FD_H

#include <fcntl.h>
#include <unistd.h>

#include "compatibility.h"

namespace spring
{
    namespace player
    {
        namespace utility
        {
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
        }
    }
}

#endif // !SPRING_PLAYER_UTILITY_POSIX_FD_H
