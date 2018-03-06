#include "main_window.h"

#include <fmt/format.h>

#include <libspring_logger.h>

#include "spring_player.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    inline std::string clean_markup(const std::string &text) noexcept
    {
        std::size_t ampersand_pos{ 0 };
        std::string result = text;

        for (;;)
        {
            ampersand_pos = result.find('&', ampersand_pos + 1);

            if (ampersand_pos != std::string::npos)
            {
                result.replace(ampersand_pos, 1, "&amp;");
            }
            else
            {
                break;
            }
        }

        return result;
    }

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
}

MainWindow::MainWindow(SpringPlayer &application,
                       std::shared_ptr<PlaybackList> playback_list) noexcept
  : playback_list_{ playback_list }
  , pms_{ spring_player_pms() }
{
    LOG_INFO("MainWindow({}): Creating...", void_p(this));

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/main_window.ui");

    get_widget_from_builder_simple(main_window);
    g_object_set(main_window_, "application", &application, nullptr);

    get_widget_from_builder_simple(search_revealer);
    get_widget_from_builder_simple(search_entry);
    get_widget_from_builder_simple(search_button);
    get_widget_from_builder_simple(window_title);

    connect_g_signal(search_button_, "toggled", &on_search_toggled, this);

    playlist_sidebar_ = std::make_unique<PlaylistSidebar>(builder, playback_list);
    playlist_sidebar_->show();

    playback_footer_ = std::make_unique<PlaybackHeader>(builder, playback_list);

    auto library = pms_.sections().at(2).content();
    page_stack_ = std::make_unique<PageStack>(
        builder, std::move(static_cast<spring::MusicLibrary &>(library)), playback_list);

    g_object_unref(builder);

    playback_list->on_playback_state_changed(
        this,
        [](auto state, void *instance) {
            auto self = static_cast<MainWindow *>(instance);

            LOG_INFO("MainWindow({}): Playback state changed to {}", instance,
                     GStreamerPipeline::playback_state_to_string(state));

            if (state == PlaybackList::PlaybackState::Playing)
            {
                auto playlist = self->playback_list_.lock();
                const auto &track = playlist->current_track();
                gtk_label_set_markup(
                    self->window_title_,
                    clean_markup(
                        fmt::format(
                            "<span weight=\"bold\">{}</span> by <span "
                            "weight=\"bold\">{}</span> from <span weight=\"bold\">{}</span>",
                            track.second->title(), track.second->artist(), track.second->album()))
                        .c_str());
            }
            else
            {
                gtk_label_set_text(self->window_title_, "Spring Player");
            }
        },
        this);

    load_css_styling(css_provider_);
}

MainWindow::~MainWindow() noexcept
{
    LOG_INFO("MainWindow({}): Destroying...", void_p(this));

    auto playlist = playback_list_.lock();
    if (playlist != nullptr)
    {
        playlist->disconnect_playback_state_changed(this);
    }

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

void MainWindow::on_search_toggled(GtkToggleButton *toggle_button, MainWindow *self) noexcept
{
    LOG_INFO("MainWindow({}): Search toggled", void_p(self));

    auto reveal = gtk_toggle_button_get_active(toggle_button);
    gtk_revealer_set_reveal_child(self->search_revealer_, reveal);

    if (!reveal)
    {
        gtk_entry_set_text(gtk_cast<GtkEntry>(self->search_entry_), "");
    }
    else
    {
        gtk_widget_grab_focus(gtk_cast<GtkWidget>(self->search_entry_));
    }
}
