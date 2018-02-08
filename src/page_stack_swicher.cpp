#include "page_stack_swicher.h"

#include "common.h"

using namespace spring;
using namespace spring::player;

namespace
{
    bool on_toggle_button_press_event(GtkToggleButton *toggle_button,
                                      GdkEventButton *,
                                      gpointer)
    {
        if (gtk_toggle_button_get_active(toggle_button))
        {
            return true;
        }
        return false;
    }
}

PageStackSwitcher::PageStackSwitcher(
    GtkBuilder *builder,
    std::function<void(ToggleButton)> &&toggled_handler) noexcept
  : toggle_buttons_{ get_widget_from_builder(GtkToggleButton,
                                             albums_toggle_button),
                     get_widget_from_builder(GtkToggleButton,
                                             artists_toggle_button),
                     get_widget_from_builder(GtkToggleButton,
                                             genres_toggle_button),
                     get_widget_from_builder(GtkToggleButton,
                                             songs_toggle_button) }
  , toggled_handler_(std::move(toggled_handler))
  , toggled_button_(settings::get_current_page())
{
    gtk_toggle_button_set_active(
        toggle_buttons_[static_cast<std::size_t>(toggled_button_)], true);

    for (auto &toggle_button : toggle_buttons_)
    {
        g_signal_connect(toggle_button, "button-press-event",
                         G_CALLBACK(&on_toggle_button_press_event), nullptr);
    }

    g_signal_connect(
        toggle_buttons_[static_cast<std::size_t>(ToggleButton::Albums)],
        "toggled", G_CALLBACK(&button_toggled), this);
    g_signal_connect(
        toggle_buttons_[static_cast<std::size_t>(ToggleButton::Artists)],
        "toggled", G_CALLBACK(&button_toggled), this);
    g_signal_connect(
        toggle_buttons_[static_cast<std::size_t>(ToggleButton::Genres)],
        "toggled", G_CALLBACK(&button_toggled), this);
    g_signal_connect(
        toggle_buttons_[static_cast<std::size_t>(ToggleButton::Songs)],
        "toggled", G_CALLBACK(&button_toggled), this);
}

PageStackSwitcher::ToggleButton PageStackSwitcher::toggled_button() const
    noexcept
{
    return toggled_button_;
}

void PageStackSwitcher::button_toggled(GtkToggleButton *toggle_button,
                                       PageStackSwitcher *self) noexcept
{
    if (!gtk_toggle_button_get_active(toggle_button))
    {
        return;
    }

    gtk_toggle_button_set_active(
        self->toggle_buttons_[static_cast<std::size_t>(self->toggled_button_)],
        false);

    for (std::size_t it = 0; it < self->toggle_buttons_.size(); ++it)
    {
        if (self->toggle_buttons_[it] == toggle_button)
        {
            self->toggled_button_ = static_cast<ToggleButton>(it);
        }
    }

    settings::set_current_page(self->toggled_button_);

    self->toggled_handler_(self->toggled_button_);
}
