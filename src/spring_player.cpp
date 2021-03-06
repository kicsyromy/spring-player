#include <memory>

#include <gtk/gtk.h>

#include <fmt/format.h>

#include "spring_player.h"

#include "ui/main_window.h"

#include "playback/gstreamer_pipeline.h"
#include "playback/playlist.h"

#include "utility/async_queue.h"
#include "utility/settings.h"

struct _SpringPlayer
{
    GtkApplication parent;

    std::shared_ptr<spring::player::playback::Playlist> playback_list;
    std::unique_ptr<spring::player::ui::MainWindow> main_window;
};

G_DEFINE_TYPE(SpringPlayer, spring_player, GTK_TYPE_APPLICATION)

static void preferences_activated(GSimpleAction *action, GVariant *parameter, gpointer app)
{
}

static void quit_activated(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    g_application_quit(G_APPLICATION(app));
}

static GActionEntry app_entries[] = { { "preferences", &preferences_activated, nullptr, nullptr,
                                        nullptr },
                                      { "quit", &quit_activated, nullptr, nullptr, nullptr } };

static void spring_player_startup(GApplication *app)
{
    const gchar *quit_accels[2]{ "<Ctrl>Q", nullptr };

    G_APPLICATION_CLASS(spring_player_parent_class)->startup(app);

    g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app), "app.quit", quit_accels);
}

static void spring_player_init(SpringPlayer *self)
{
    self->playback_list.reset();
    self->main_window.release();
}

static void spring_player_activate(GApplication *app)
{
    auto self = reinterpret_cast<SpringPlayer *>(app);
    if (self->main_window == nullptr)
    {
        spring::player::utility::async_queue::start_processing();

        self->playback_list = std::make_shared<spring::player::playback::Playlist>();

        self->main_window =
            std::make_unique<spring::player::ui::MainWindow>(*self, self->playback_list);
        self->main_window->show();
    }
}

static void spring_player_shutdown(GApplication *app)
{
    spring::player::utility::async_queue::stop_processing();

    auto self = reinterpret_cast<SpringPlayer *>(app);
    self->main_window.reset();
    self->playback_list.reset();

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
    return static_cast<SpringPlayer *>(g_object_new(SPRING_PLAYER_TYPE, "application-id",
                                                    APPLICATION_ID "", "flags",
                                                    G_APPLICATION_HANDLES_OPEN, nullptr));
}
