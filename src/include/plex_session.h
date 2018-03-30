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

        public:
            const std::string &name() const noexcept;
            const std::string &token() const noexcept;

        public:
            static std::vector<PlexSession> sessions() noexcept;

        private:
            std::string name_{};
            std::string token_{};
        };
    }
}

#endif // !SPRING_PLAYER_PLEX_SESSION_H
