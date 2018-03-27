#include <cstdio>
#include <string>

#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "plex_session.h"
#include "spring_player.h"

GRESOURCE_UI_DECLARE_RESOURCE();

int main(int argc, char *argv[])
{
    auto ui = GRESOURCE_UI_INIT_RESOURCE();

    LOG_INFO("SpringPlayer: Starting up...");

    auto sessions = spring::player::PlexSession::sessions();

    auto application = spring_player_new();
    auto result = g_application_run(G_APPLICATION(application), argc, argv);

    g_resource_unref(ui);

    LOG_INFO("SpringPlayer: Shutting down...");

    return result;
}
