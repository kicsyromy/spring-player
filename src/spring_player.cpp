#include "spring_player.h"

#include <memory>

#include "application_settings.h"
#include "async_queue.h"
#include "gstreamer_pipeline.h"
#include "main_window.h"
#include "utility.h"

struct _SpringPlayer
{
    GtkApplication parent;

    std::unique_ptr<spring::player::MainWindow> main_window;
};

G_DEFINE_TYPE(SpringPlayer, spring_player, GTK_TYPE_APPLICATION)

static void preferences_activated(GSimpleAction *action,
                                  GVariant *parameter,
                                  gpointer app)
{
}

static void quit_activated(GSimpleAction *action,
                           GVariant *parameter,
                           gpointer app)
{
    g_application_quit(G_APPLICATION(app));
}

static GActionEntry app_entries[] = {
    { "preferences", &preferences_activated, nullptr, nullptr, nullptr },
    { "quit", &quit_activated, nullptr, nullptr, nullptr }
};

static void spring_player_startup(GApplication *app)
{
    const gchar *quit_accels[2]{ "<Ctrl>Q", nullptr };

    G_APPLICATION_CLASS(spring_player_parent_class)->startup(app);

    g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries,
                                    G_N_ELEMENTS(app_entries), app);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app), "app.quit",
                                          quit_accels);

    auto builder =
        gtk_builder_new_from_resource(APPLICATION_PREFIX "/app_menu.ui");
    auto app_menu = G_MENU_MODEL(gtk_builder_get_object(builder, "appmenu"));
    gtk_application_set_app_menu(GTK_APPLICATION(app), app_menu);
    g_object_unref(builder);
}

static void spring_player_init(SpringPlayer *self)
{
    self->main_window.release();
    spring::player::GStreamerPipeline::initialize();
}

static void spring_player_activate(GApplication *app)
{
    auto self = reinterpret_cast<SpringPlayer *>(app);
    if (self->main_window == nullptr)
    {
        spring::player::async_queue::start_processing();

        self->main_window = std::make_unique<spring::player::MainWindow>(*self);
        self->main_window->show();
    }
}

static void spring_player_shutdown(GApplication *app)
{

    auto self = reinterpret_cast<SpringPlayer *>(app);
    self->main_window.reset(nullptr);

    spring::player::async_queue::stop_processing();

    G_APPLICATION_CLASS(spring_player_parent_class)->shutdown(app);
}

static void spring_player_class_init(SpringPlayerClass *klass)
{
    G_APPLICATION_CLASS(klass)->startup = &spring_player_startup;
    G_APPLICATION_CLASS(klass)->activate = &spring_player_activate;
    G_APPLICATION_CLASS(klass)->shutdown = &spring_player_shutdown;
}

SpringPlayer *spring_player_new()
{
    g_warning("****** home: %s",
              spring::player::settings::home_directory().c_str());
    g_warning("****** data: %s",
              spring::player::settings::data_directory().c_str());
    g_warning("****** config: %s",
              spring::player::settings::config_directory().c_str());
    g_warning("****** cache: %s",
              spring::player::settings::cache_directory().c_str());

    return static_cast<SpringPlayer *>(
        g_object_new(SPRING_PLAYER_TYPE, "application-id", APPLICATION_ID,
                     "flags", G_APPLICATION_HANDLES_OPEN, nullptr));
}

spring::PlexMediaServer spring_player_pms()
{
    char host[100];
    FILE *f = fopen("/home/kicsyromy/workspace/host.txt", "r");
    fscanf(f, "%s", host);
    fclose(f);
    return { host, -1, "username", "password" };
}
