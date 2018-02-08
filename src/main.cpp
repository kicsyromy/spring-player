#include <cstdio>

#include <string>

#include <gst/gst.h>
#include <gtk/gtk.h>

#include "spring_player.h"

GRESOURCE_UI_DECLARE_RESOURCE();

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    auto ui = GRESOURCE_UI_INIT_RESOURCE();

    auto application = spring_player_new();
    auto result = g_application_run(G_APPLICATION(application), argc, argv);

    g_resource_unref(ui);

    return result;
}
