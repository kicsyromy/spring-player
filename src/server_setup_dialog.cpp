#include <gtk/gtk.h>

#include <granite.h>

#include <glib/gi18n.h>

#include <libspring_logger.h>

#include "async_queue.h"
#include "server_setup_dialog.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

ServerSetupDialog::ServerSetupDialog() noexcept
  : dialog_(granite_message_dialog_new_with_image_from_icon_name(
        "Connection Set-Up",
        "Connect to an existing Plex Media Server",
        "text-x-generic",
        GTK_BUTTONS_NONE))
{
    LOG_INFO("ServerSetupDialog({}): Creating...", void_p(this));

    connect_g_signal(dialog_, "delete-event", &on_destroy_event, this);

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/server_setup_dialog.ui");
    get_widget_from_builder_simple(content);
    get_widget_from_builder_simple(status_revealer);
    get_widget_from_builder_simple(connecting_spinner);
    get_widget_from_builder_simple(status_error_icon);
    get_widget_from_builder_simple(status_text);
    get_widget_from_builder_simple(server_url_entry);
    get_widget_from_builder_simple(username_entry);
    get_widget_from_builder_simple(password_entry);
    get_widget_from_builder_simple(ssl_validation_checkbutton);

    cancel_button_ = gtk_cast<GtkButton>(gtk_button_new_with_label(_("Cancel")));
    auto cancel_button = cancel_button_;
    gtk_widget_set_visible(gtk_cast<GtkWidget>(cancel_button), true);
    gtk_dialog_add_action_widget(gtk_cast<GtkDialog>(dialog_), gtk_cast<GtkWidget>(cancel_button),
                                 GTK_RESPONSE_CANCEL);

    connect_button_ = gtk_cast<GtkButton>(gtk_button_new_with_label("Connect"));
    auto connect_button = gtk_cast<GtkWidget>(connect_button_);
    gtk_widget_set_visible(connect_button, true);
    auto style_context = gtk_widget_get_style_context(connect_button);
    gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_SUGGESTED_ACTION);
    gtk_dialog_add_action_widget(gtk_cast<GtkDialog>(dialog_), connect_button, GTK_RESPONSE_ACCEPT);

    GtkBin *content_bin = nullptr;
    g_object_get(dialog_, "custom_bin", &content_bin, nullptr);
    gtk_container_add(gtk_cast<GtkContainer>(content_bin), gtk_cast<GtkWidget>(content_));

    g_object_unref(builder);

    connect_g_signal(cancel_button, "clicked", &on_setup_canceled, this);
    connect_g_signal(connect_button, "clicked", &on_connection_requested, this);

    gtk_window_set_modal(gtk_cast<GtkWindow>(dialog_), true);
}

ServerSetupDialog::~ServerSetupDialog() noexcept
{
    LOG_INFO("ServerSetupDialog({}): Destroying...", void_p(this));
}

void ServerSetupDialog::set_parent_window(GtkWindow *window) noexcept
{
    gtk_window_set_transient_for(gtk_cast<GtkWindow>(dialog_), window);
}

void ServerSetupDialog::show() noexcept
{
    gtk_widget_set_visible(gtk_cast<GtkWidget>(dialog_), true);
}

GtkWidget *ServerSetupDialog::operator()() noexcept
{
    return gtk_cast<GtkWidget>(dialog_);
}

void ServerSetupDialog::on_connection_requested(GtkButton *, ServerSetupDialog *self) noexcept
{
    LOG_INFO("ServerSetupDialog({}): Setting up server...", void_p(self));
    self->set_connecting_state(true);

    std::string server_address{ gtk_entry_get_text(self->server_url_entry_) };
    std::int32_t port = -1;
    auto index = server_address.find_last_of(':');
    if (index != std::string::npos && index != 4 && index != 5)
    {
        std::string port_str{ server_address.data() + index + 1 };
        std::size_t offset{ 0 };
        port = std::stoi(port_str, &offset, 10);
        if (offset < port_str.size())
        {
            LOG_WARN(
                "ServerSetupDialog({}): Port not given as a proper number, connection might fail",
                void_p(self));
        }
        server_address.erase(index, std::string::npos);
    }

    auto username = gtk_entry_get_text(self->username_entry_);
    auto password = gtk_entry_get_text(self->password_entry_);
    auto enable_ssl_validation = static_cast<PlexMediaServer::SSLErrorHandling>(
        gtk_toggle_button_get_active(gtk_cast<GtkToggleButton>(self->ssl_validation_checkbutton_)));

    LOG_INFO("ServerSetupDialog({}): Connecting to {} on port {} with username {} and password {}",
             void_p(self), server_address, port, username, password);

    async_queue::push_front_request(async_queue::Request{
        "Connect to server",
        [self, server_address, port, username, password, enable_ssl_validation] {
            PlexMediaServer server;

            auto result = server.connect(server_address.c_str(), port, username, password,
                                         enable_ssl_validation);
            if (result.error)
            {
                LOG_ERROR("ServerSetupDialog({}): Failed to connect to server {}", void_p(self),
                          result.error.message());

                auto error = new Error;
                *error = std::move(result.error);
                async_queue::post_response(
                    async_queue::Response{ "Connection failed", [self, error]() mutable {
                                              auto e = std::unique_ptr<Error>(error);
                                              on_connection_failed(std::move(*e), self);
                                          } });
            }
            else
            {
                LOG_INFO("ServerSetupDialog({}): Connection successful", void_p(self));
                async_queue::post_response(async_queue::Response{
                    "Connection successful", [self]() { on_connection_successful(self); } });
                self->emit_queued_server_added(
                    PlexSession{ server.name().c_str(), server_address.data(), port, result.value },
                    std::move(server));
            }
        } });
}

void ServerSetupDialog::on_connection_failed(Error error, ServerSetupDialog *self) noexcept
{
    self->set_connecting_state(false);
    gtk_revealer_set_reveal_child(self->status_revealer_, true);
    gtk_label_set_text(self->status_text_,
                       fmt::format("Connection failed: {}", error.message()).c_str());
    gtk_widget_set_visible(gtk_cast<GtkWidget>(self->status_error_icon_), true);
}

void ServerSetupDialog::on_connection_successful(ServerSetupDialog *self) noexcept
{
    self->close();
}

void ServerSetupDialog::on_setup_canceled(GtkButton *, ServerSetupDialog *self) noexcept
{
    LOG_INFO("ServerSetupDialog({}): Setup canceled...", void_p(self));
    self->close();
}

bool ServerSetupDialog::on_destroy_event(GtkButton *, GdkEvent *, ServerSetupDialog *self) noexcept
{
    self->close();
    return true;
}

void ServerSetupDialog::set_connecting_state(bool connecting) noexcept
{
    gtk_widget_set_sensitive(gtk_cast<GtkWidget>(server_url_entry_), !connecting);
    gtk_widget_set_sensitive(gtk_cast<GtkWidget>(username_entry_), !connecting);
    gtk_widget_set_sensitive(gtk_cast<GtkWidget>(password_entry_), !connecting);
    gtk_widget_set_sensitive(gtk_cast<GtkWidget>(ssl_validation_checkbutton_), !connecting);

    gtk_widget_set_sensitive(gtk_cast<GtkWidget>(connect_button_), !connecting);
    gtk_widget_set_sensitive(gtk_cast<GtkWidget>(cancel_button_), !connecting);

    gtk_revealer_set_reveal_child(status_revealer_, connecting);

    gtk_widget_set_visible(gtk_cast<GtkWidget>(connecting_spinner_), connecting);

    gtk_label_set_text(status_text_, connecting ? _("Connecting...") : "");
    gtk_widget_set_visible(gtk_cast<GtkWidget>(status_error_icon_), false);
}

void ServerSetupDialog::close() noexcept
{
    set_connecting_state(false);

    gtk_entry_set_text(server_url_entry_, "");
    gtk_entry_set_text(username_entry_, "");
    gtk_entry_set_text(password_entry_, "");

    gtk_widget_set_visible(gtk_cast<GtkWidget>(dialog_), false);
}
