#include <libspring_logger.h>

#include "plex/session.h"
#include "utility/settings.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;
using namespace spring::player::plex;

namespace
{
    struct session_table_t
    {
        std::int32_t id;
        std::string name;
        std::string hostname;
        std::int32_t port;
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
                       make_column("HOSTNAME", &session_table_t::hostname),
                       make_column("PORT", &session_table_t::port),
                       make_column("TOKEN", &session_table_t::token)));
    }
} // namespace

Session::Session() noexcept
{
}

Session::Session(std::string &&name,
                 std::string &&hostname,
                 int32_t port,
                 std::string &&token) noexcept
  : name_(std::move(name))
  , hostname_(std::move(hostname))
  , port_(port)
  , token_(std::move(token))
{
}

const std::string &Session::name() const noexcept
{
    return name_;
}

const std::string &Session::hostname() const noexcept
{
    return hostname_;
}

int32_t Session::port() const noexcept
{
    return port_;
}

const std::string &Session::token() const noexcept
{
    return token_;
}

void Session::save() const noexcept
{
    auto storage = create_storage();
    storage.sync_schema();

    storage.insert<session_table_t>({ -1, name_, hostname_, port_, token_ });
}

std::vector<Session> Session::sessions() noexcept
{
    using namespace sqlite_orm;

    auto storage = create_storage();
    storage.sync_schema();

    auto sessions = storage.select(columns(&session_table_t::token, &session_table_t::port,
                                           &session_table_t::hostname, &session_table_t::name));

    auto plex_sessions = reinterpret_cast<std::vector<Session> *>(&sessions);
    return std::move(*plex_sessions);
}
