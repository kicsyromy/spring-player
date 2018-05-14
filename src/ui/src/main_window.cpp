#include <granite.h>
#include <gtk/gtk.h>

#include <fmt/format.h>

#include <libspring_logger.h>

#include "main_window.h"
#include "spring_player.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    void load_css_styling(GtkCssProvider *&css_provider) noexcept
    {
        LOG_INFO("MainWindow: Internal: Loading CSS styling...");

        css_provider = gtk_css_provider_new();

        GError *error{ nullptr };
        gtk_css_provider_load_from_resource(css_provider,
                                            APPLICATION_PREFIX "/application_style.css");
        if (error != nullptr)
        {
            LOG_ERROR("MainWindow: Internal: Failed to apply CSS styling: {}", error->message);
            g_error_free(error);
        }

        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                                  gtk_cast<GtkStyleProvider>(css_provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
} // namespace

MainWindow::MainWindow(SpringPlayer &application,
                       std::shared_ptr<PlaybackList> playback_list) noexcept
  : pms_{}
  , header_{ playback_list }
  , playlist_sidebar_{ playback_list }
  , page_stack_{ page_stack_switcher_, playback_list }
  , playback_list_{ playback_list }
{
    LOG_INFO("MainWindow({}): Creating...", void_p(this));

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/main_window.ui");

    get_widget_from_builder_simple(main_window);
    g_object_set(main_window_, "application", &application, nullptr);

    get_widget_from_builder_simple(paned);
    get_widget_from_builder_simple(sidebar_placeholder);
    get_widget_from_builder_simple(main_content);

    get_widget_from_builder_simple(search_revealer);
    get_widget_from_builder_simple(search_entry);

    g_object_unref(builder);

    gtk_window_set_titlebar(gtk_cast<GtkWindow>(main_window_), header_());

    gtk_widget_destroy(sidebar_placeholder_);
    gtk_paned_pack1(paned_, playlist_sidebar_(), true, false);

    server_setup_dialog_.set_parent_window((*this)());

    auto sessions = PlexSession::sessions();
    if (sessions.empty())
    {
        show_welcome_page();
    }
    else
    {
        if (switch_server(sessions, "GLaDOS"))
        {
            show_server_content();
        }
    }

    connect_g_signal(search_entry_, "search-changed", &on_search_changed, this);
    connect_g_signal(search_entry_, "stop-search", &on_search_finished, this);

    header_.on_playlist_toggled(this, &toggle_playlist);
    header_.on_search_toggled(this, &on_search_toggled);
    header_.on_back_requested(this, &on_back_requested);
    playback_list->on_track_queued(this, &on_track_queued);
    welcome_page_.on_new_connection_requested(this, &on_new_connection_requested);
    server_setup_dialog_.on_server_added(this, &on_server_added);
    load_css_styling(css_provider_);
}

MainWindow::~MainWindow() noexcept
{
    LOG_INFO("MainWindow({}): Destroying...", void_p(this));

    auto playlist = playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->on_track_queued(this, &on_track_queued);
    }
    header_.disconnect_playlist_toggled(this);

    g_object_unref(css_provider_);
}

void MainWindow::show() noexcept
{
    LOG_INFO("MainWindow({}): Showing...", void_p(this));
    gtk_window_present(gtk_cast<GtkWindow>(main_window_));
}

void MainWindow::hide() noexcept
{
    LOG_INFO("MainWindow({}): Hiding...", void_p(this));
    gtk_widget_set_visible(gtk_cast<GtkWidget>(main_window_), false);
}

GtkWindow *MainWindow::operator()() noexcept
{
    return gtk_cast<GtkWindow>(main_window_);
}

void MainWindow::on_search_toggled(bool toggled, MainWindow *self) noexcept
{
    LOG_INFO("MainWindow({}): Search toggled", void_p(self));

    gtk_revealer_set_reveal_child(self->search_revealer_, toggled);

    if (!toggled)
    {
        gtk_entry_set_text(gtk_cast<GtkEntry>(self->search_entry_), "");
    }
    else
    {
        gtk_widget_grab_focus(gtk_cast<GtkWidget>(self->search_entry_));
    }
}

void MainWindow::on_search_changed(GtkEntry *entry, MainWindow *self) noexcept
{
    LOG_INFO("MainWindow({}), Search string changed, passing to current page", void_p(self));

    self->page_stack_.filter_current_page(gtk_entry_get_text(entry));
}

void MainWindow::on_search_finished(GtkSearchEntry *, MainWindow *self) noexcept
{
    self->page_stack_.filter_current_page("");
    self->header_.toggle_search();
}

void MainWindow::on_server_added(PlexSession session,
                                 PlexMediaServer server,
                                 MainWindow *self) noexcept
{
    LOG_INFO("MainWindow({}): New Plex Media Server added: {}", void_p(self), session.name());
    session.save();

    self->pms_ = std::move(server);
    self->show_server_content();
}

void MainWindow::on_back_requested(MainWindow *self) noexcept
{
    self->page_stack_.go_back();
}

void MainWindow::toggle_playlist(bool toggled, MainWindow *self) noexcept
{
    toggled ? self->playlist_sidebar_.show() : self->playlist_sidebar_.hide();
}

void MainWindow::on_track_queued(std::shared_ptr<music::Track> &, MainWindow *self) noexcept
{
    if (!gtk_widget_get_visible(self->playlist_sidebar_()))
    {
        self->header_.toggle_sidebar();
    }
}

void MainWindow::on_new_connection_requested(MainWindow *self) noexcept
{
    self->server_setup_dialog_.show();
}

bool MainWindow::switch_server(const std::vector<PlexSession> &sessions,
                               const string_view server_name) noexcept
{
    auto result{ false };

    auto it = std::find_if(sessions.begin(), sessions.end(),
                           [server_name](const PlexSession &s) { return s.name() == server_name; });

    if (it != sessions.end())
    {
        pms_.connect(it->hostname().c_str(), it->port(), it->token().c_str(),
                     PlexMediaServer::SSLErrorHandling::Acknowledge);
        result = true;
    }

    return result;
}

void MainWindow::show_welcome_page() noexcept
{
    header_.hide_controls();
    gtk_box_pack_end(main_content_, welcome_page_(), true, true, 0);
}

void MainWindow::show_server_content() noexcept
{
    page_stack_.set_music_library(
        std::move(static_cast<MusicLibrary &>(pms_.sections().at(2).content())));

    header_.show_controls();
    gtk_widget_hide(welcome_page_());

    auto switcher = page_stack_switcher_();
    gtk_box_pack_start(main_content_, switcher, false, false, 0);
    gtk_container_child_set(gtk_cast<GtkContainer>(main_content_), switcher, "position", 0,
                            nullptr);

    gtk_box_pack_end(main_content_, page_stack_(), true, true, 0);
}
