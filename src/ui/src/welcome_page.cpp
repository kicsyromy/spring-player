#include <gtk/gtk.h>

#include <granite.h>

#include <libspring_logger.h>

#include "main_window.h"
#include "server_setup_dialog.h"
#include "welcome_page.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    const auto WELCOME_TITLE{ _("Welcome to Spring Player") };
    const auto WELCOME_SUBTITLE{ _("No Plex Media Server is configured for use.") };

    enum Action
    {
        NewConnection
    };
} // namespace

WelcomePage::WelcomePage() noexcept
  : welcome_(granite_widgets_welcome_new(WELCOME_TITLE, WELCOME_SUBTITLE))
{
    LOG_INFO("WelcomePage({}): Creating...", void_p(this));

    granite_widgets_welcome_append(welcome_, "preferences-system-network", _("Set-Up"),
                                   _("Connect to an existing Plex Media Server."));

    gtk_widget_show_all(gtk_cast<GtkWidget>(welcome_));

    connect_g_signal(welcome_, "activated", &on_action_activated, this);
}

WelcomePage::~WelcomePage() noexcept
{
    LOG_INFO("WelcomePage({}): Destroying...", void_p(this));
}

GtkWidget *WelcomePage::operator()() noexcept
{
    return gtk_cast<GtkWidget>(welcome_);
}

void WelcomePage::on_action_activated(GraniteWidgetsWelcome *,
                                      int32_t index,
                                      WelcomePage *self) noexcept
{
    LOG_INFO("WelcomePage({}): Activated item at index {}", void_p(self), index);

    switch (index)
    {
        case NewConnection:
            self->emit_new_connection_requested();
    }
}
