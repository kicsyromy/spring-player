#include "main_window.h"

#include <fmt/format.h>

#include "now_playing_list.h"
#include "spring_player.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    void load_css_styling(GtkCssProvider *&css_provider) noexcept
    {
        css_provider = gtk_css_provider_new();

        GError *error{ nullptr };
        gtk_css_provider_load_from_resource(css_provider, APPLICATION_PREFIX
                                            "/application_style.css");
        if (error != nullptr)
        {
            g_warning("Error: %s", error->message);
        }

        gtk_style_context_add_provider_for_screen(
            gdk_screen_get_default(), gtk_cast<GtkStyleProvider>(css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

MainWindow::MainWindow(SpringPlayer &application) noexcept
  : pms_{ spring_player_pms() }
{
    auto builder =
        gtk_builder_new_from_resource(APPLICATION_PREFIX "/main_window.ui");

    get_widget_from_builder_simple(main_window);
    g_object_set(main_window_, "application", &application, nullptr);

    get_widget_from_builder_simple(search_revealer);
    get_widget_from_builder_simple(search_entry);
    get_widget_from_builder_simple(search_button);
    get_widget_from_builder_simple(window_title);

    connect_g_signal(search_button_, "toggled", &on_search_toggled, this);

    now_playing_sidebar_ = std::make_unique<NowPlayingSidebar>(builder);
    now_playing_sidebar_->show();

    playback_footer_ = std::make_unique<PlaybackHeader>(builder);

    auto library = std::move(pms_.sections().at(2).content());
    page_stack_ = std::make_unique<PageStack>(
        builder, std::move(static_cast<spring::MusicLibrary &>(library)));

    g_object_unref(builder);

    NowPlayingList::instance().on_playback_state_changed(
        this,
        [](auto state, void *instance) {
            auto self = static_cast<MainWindow *>(instance);

            if (state == NowPlayingList::PlaybackState::Playing)
            {
                const auto &track = NowPlayingList::instance().current_track();
                gtk_label_set_text(self->window_title_,
                                   fmt::format("{} - {} - {}", track->artist(),
                                               track->album(), track->title())
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
    g_warning("Deleting main window");

    NowPlayingList::instance().disconnect_playback_state_changed(this);

    g_object_unref(css_provider_);
}

void MainWindow::show() noexcept
{
    gtk_window_present(gtk_cast<GtkWindow>(main_window_));
}

void MainWindow::hide() noexcept
{
    gtk_widget_set_visible(gtk_cast<GtkWidget>(main_window_), false);
}

void MainWindow::on_search_toggled(GtkToggleButton *toggle_button,
                                   MainWindow *self) noexcept
{
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
