#ifndef SPRING_PLAYER_MAIN_WINDOW_H
#define SPRING_PLAYER_MAIN_WINDOW_H

#include <memory>

#include <libspring_global.h>
#include <libspring_plex_media_server.h>

#include "header_bar.h"
#include "playlist_sidebar.h"

#include "page_stack.h"
#include "page_stack_swicher.h"
#include "playback_list.h"
#include "plex_session.h"
#include "server_setup_dialog.h"
#include "welcome_page.h"

#include "utility/forward_declarations.h"

using SpringPlayer = struct _SpringPlayer;

namespace spring
{
    namespace player
    {
        class MainWindow
        {
        public:
            MainWindow(SpringPlayer &application,
                       std::shared_ptr<PlaybackList> playback_list) noexcept;
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
            static void on_server_added(PlexSession session, PlexMediaServer server, MainWindow *self) noexcept;

        private:
            static void toggle_playlist(bool toggled, MainWindow *self) noexcept;
            static void on_track_queued(std::shared_ptr<music::Track> &, MainWindow *self) noexcept;
            static void on_new_connection_requested(MainWindow *self) noexcept;

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

            std::weak_ptr<PlaybackList> playback_list_{};

            GtkCssProvider *css_provider_{ nullptr };

        private:
            DISABLE_COPY(MainWindow)
            DISABLE_MOVE(MainWindow)
        };
    }
}

#endif // !SPRING_PLAYER_MAIN_WINDOW_H
