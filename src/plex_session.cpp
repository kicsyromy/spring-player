#include <libspring_logger.h>

#include "application_settings.h"
#include "plex_session.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    struct session_table_t
    {
        std::int32_t id;
        std::string name;
        std::string token;
    };

    const auto DATABASE_PATH{ settings::data_directory() + "/sessions.db" };

    inline auto create_storage() noexcept
    {
        using namespace sqlite_orm;

        return make_storage(
            DATABASE_PATH,
            make_table("SESSIONS",
                       make_column("ID", &session_table_t::id, autoincrement(), primary_key()),
                       make_column("NAME", &session_table_t::name),
                       make_column("TOKEN", &session_table_t::token)));
    }
}

PlexSession::PlexSession() noexcept
{
}

std::vector<PlexSession> PlexSession::sessions() noexcept
{
    using namespace sqlite_orm;

    auto storage = create_storage();
    storage.sync_schema();

    //    storage.insert<session_table_t>({ -1, "name1", "token1" });
    //    storage.insert<session_table_t>({ -1, "name2", "token2" });

    auto sessions = storage.select(columns(&session_table_t::token, &session_table_t::name));

    auto plex_sessions = reinterpret_cast<std::vector<PlexSession> *>(&sessions);
    return std::move(*plex_sessions);
}
