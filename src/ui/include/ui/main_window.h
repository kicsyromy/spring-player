#ifndef SPRING_PLAYER_MAIN_WINDOW_H
#define SPRING_PLAYER_MAIN_WINDOW_H

#include <memory>

#include <libspring_global.h>
#include <libspring_plex_media_server.h>

#include "ui/header_bar.h"
#include "ui/page_stack.h"
#include "ui/page_stack_swicher.h"
#include "ui/playlist_sidebar.h"
#include "ui/server_setup_dialog.h"
#include "ui/welcome_page.h"

#include "playback/playlist.h"

#include "plex/session.h"

#include "utility/forward_declarations.h"

using SpringPlayer = struct _SpringPlayer;

namespace spring
{
    namespace player
    {
        namespace ui
        {
            class MainWindow
            {
            public:
                MainWindow(SpringPlayer &application,
                           std::shared_ptr<playback::Playlist> playback_list) noexcept;
                ~MainWindow() noexcept;

            public:
                void show() noexcept;
                void hide() noexcept;

            public:
                GtkWindow *operator()() noexcept;

            private:
                static void on_search_toggled(bool toggled, MainWindow *self) noexcept;
                static void on_search_changed(GtkEntry *entry, MainWindow *self) noexcept;
                static void on_search_finished(GtkSearchEntry *entry, MainWindow *self) noexcept;
                static void on_server_added(plex::Session session,
                                            PlexMediaServer server,
                                            MainWindow *self) noexcept;
                static void on_back_requested(MainWindow *self) noexcept;

            private:
                static void toggle_playlist(bool toggled, MainWindow *self) noexcept;
                static void on_track_queued(std::shared_ptr<music::Track> &,
                                            MainWindow *self) noexcept;
                static void on_new_connection_requested(MainWindow *self) noexcept;

            private:
                bool switch_server(const std::vector<plex::Session> &sessions,
                                   const utility::string_view server_name) noexcept;
                void show_welcome_page() noexcept;
                void show_server_content() noexcept;

            private:
                GtkApplicationWindow *main_window_{ nullptr };
                GtkPaned *paned_{ nullptr };
                GtkWidget *sidebar_placeholder_{ nullptr };
                GtkBox *main_content_{ nullptr };
                GtkRevealer *search_revealer_{ nullptr };
                GtkSearchEntry *search_entry_{ nullptr };

                PlexMediaServer pms_;

                HeaderBar header_{ nullptr };
                PlaylistSidebar playlist_sidebar_{ nullptr };
                WelcomePage welcome_page_{};
                ServerSetupDialog server_setup_dialog_{};
                PageStackSwitcher page_stack_switcher_{};
                PageStack page_stack_;

                std::weak_ptr<playback::Playlist> playback_list_{};

                GtkCssProvider *css_provider_{ nullptr };

            private:
                DISABLE_COPY(MainWindow)
                DISABLE_MOVE(MainWindow)
            };
        } // namespace ui
    }     // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_MAIN_WINDOW_H
