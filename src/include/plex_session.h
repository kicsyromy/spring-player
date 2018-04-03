#ifndef SPRING_PLAYER_PLEX_SESSION_H
#define SPRING_PLAYER_PLEX_SESSION_H

#include <string>

#include <sqlite_orm.h>

#include "utility/compatibility.h"

namespace spring
{
    namespace player
    {
        class PlexSession
        {
        public:
            PlexSession() noexcept;
            PlexSession(std::string &&name,
                        std::string &&hostname,
                        std::int32_t port,
                        std::string &&token) noexcept;

        public:
            const std::string &name() const noexcept;
            const std::string &hostname() const noexcept;
            std::int32_t port() const noexcept;
            const std::string &token() const noexcept;

        public:
            void save() const noexcept;

        public:
            static std::vector<PlexSession> sessions() noexcept;

        private:
            std::string name_{};
            std::string hostname_{};
            std::int32_t port_{};
            std::string token_{};
        };
    }
}

#endif // !SPRING_PLAYER_PLEX_SESSION_H
