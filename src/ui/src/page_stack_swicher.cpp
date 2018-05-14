#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "page_stack_swicher.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    bool on_toggle_button_press_event(GtkToggleButton *toggle_button, GdkEventButton *, gpointer)
    {
        if (gtk_toggle_button_get_active(toggle_button))
        {
            return true;
        }
        return false;
    }
} // namespace

PageStackSwitcher::PageStackSwitcher() noexcept
  : toggled_button_(settings::get_current_page())
{
    LOG_INFO("PageStackSwitcher({}): Creating...", void_p(this));

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/page_stack_switcher.ui");
    get_guarded_widget_from_builder(container);
    toggle_buttons_[static_cast<std::size_t>(ToggleButton::Albums)] =
        utility::gtk_cast<GtkToggleButton>(gtk_builder_get_object(builder, "albums_toggle_button"));
    toggle_buttons_[static_cast<std::size_t>(ToggleButton::Artists)] =
        utility::gtk_cast<GtkToggleButton>(
            gtk_builder_get_object(builder, "artists_toggle_button"));
    toggle_buttons_[static_cast<std::size_t>(ToggleButton::Songs)] =
        utility::gtk_cast<GtkToggleButton>(gtk_builder_get_object(builder, "songs_toggle_button"));
    g_object_unref(builder);

    gtk_toggle_button_set_active(toggle_buttons_[static_cast<std::size_t>(toggled_button_)], true);

    for (auto &toggle_button : toggle_buttons_)
    {
        connect_g_signal(toggle_button, "button-press-event", &on_toggle_button_press_event,
                         static_cast<void *>(nullptr));
    }

    connect_g_signal(toggle_buttons_[static_cast<std::size_t>(ToggleButton::Albums)], "toggled",
                     &button_toggled, this);
    connect_g_signal(toggle_buttons_[static_cast<std::size_t>(ToggleButton::Artists)], "toggled",
                     &button_toggled, this);
    connect_g_signal(toggle_buttons_[static_cast<std::size_t>(ToggleButton::Songs)], "toggled",
                     &button_toggled, this);
}

PageStackSwitcher::~PageStackSwitcher() noexcept
{
    LOG_INFO("PageStackSwitcher({}): Destroying...", void_p(this));
}

PageStackSwitcher::ToggleButton PageStackSwitcher::toggled_button() const noexcept
{
    return toggled_button_;
}

GtkWidget *PageStackSwitcher::operator()() noexcept
{
    return gtk_cast<GtkWidget>(container_);
}

void PageStackSwitcher::button_toggled(GtkToggleButton *toggle_button,
                                       PageStackSwitcher *self) noexcept
{
    if (!gtk_toggle_button_get_active(toggle_button))
    {
        return;
    }

    gtk_toggle_button_set_active(
        self->toggle_buttons_[static_cast<std::size_t>(self->toggled_button_)], false);

    for (std::size_t it = 0; it < self->toggle_buttons_.size(); ++it)
    {
        if (self->toggle_buttons_[it] == toggle_button)
        {
            self->toggled_button_ = static_cast<ToggleButton>(it);
        }
    }

    settings::set_current_page(self->toggled_button_);

    self->emit_page_requested(self->toggled_button());
}
