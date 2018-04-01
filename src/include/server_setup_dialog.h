#ifndef SPRING_PLAYER_SERVER_SETUP_DIALOG_H
#define SPRING_PLAYER_SERVER_SETUP_DIALOG_H

#include <libspring_global.h>

#include "plex_session.h"

#include "utility/forward_declarations.h"
#include "utility/g_object_guard.h"
#include "utility/signals.h"

namespace spring
{
    namespace player
    {
        class ServerSetupDialog
        {
        public:
            ServerSetupDialog() noexcept;
            ~ServerSetupDialog() noexcept;

        public:
            void set_parent_window(GtkWindow *window) noexcept;
            void show() noexcept;

        public:
            signal(server_added, PlexSession);

        public:
            GtkWidget *operator()() noexcept;

        private:
            static void on_connection_requested(GtkButton *, ServerSetupDialog *self) noexcept;
            static void on_setup_canceled(GtkButton *, ServerSetupDialog *self) noexcept;
            static bool on_destroy_event(GtkButton *, GdkEvent *, ServerSetupDialog *self) noexcept;

        private:
            void set_connecting_state(bool connecting) noexcept;

        private:
            utility::GObjectGuard<GraniteMessageDialog> dialog_;
            GtkWidget *content_{ nullptr };
            GtkRevealer *status_revealer_{ nullptr };
            GtkSpinner *connecting_spinner_{ nullptr };
            GtkEntry *server_url_entry_{ nullptr };
            GtkEntry *username_entry_{ nullptr };
            GtkEntry *password_entry_{ nullptr };
            GtkButton *connect_button_{ nullptr };
            GtkCheckButton *ssl_validation_checkbutton_{ nullptr };

        private:
            DISABLE_COPY(ServerSetupDialog)
            DISABLE_MOVE(ServerSetupDialog)
        };
    }
}

#endif // !SPRING_PLAYER_SERVER_SETUP_DIALOG_H
