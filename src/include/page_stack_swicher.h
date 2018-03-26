#ifndef SPRING_PLAYER_PAGE_STACK_SWITCHER_H
#define SPRING_PLAYER_PAGE_STACK_SWITCHER_H

#include <array>
#include <cstdint>
#include <functional>

#include <libspring_global.h>

#include "application_settings.h"

#include "utility/forward_declarations.h"
#include "utility/g_object_guard.h"
#include "utility/signals.h"

namespace spring
{
    namespace player
    {
        class PageStackSwitcher
        {
        public:
            using ToggleButton = settings::Page;

        public:
            PageStackSwitcher() noexcept;
            ~PageStackSwitcher() noexcept;

        public:
            ToggleButton toggled_button() const noexcept;

        public:
            signal(page_requested, settings::Page);

        public:
            GtkWidget *operator()() noexcept;

        private:
            static void button_toggled(GtkToggleButton *toggle_button,
                                       PageStackSwitcher *self) noexcept;

        private:
            static constexpr auto TOGGLE_BUTTON_COUNT{ static_cast<std::size_t>(
                ToggleButton::Count) };

            utility::GObjectGuard<GtkButtonBox> container_{ nullptr };
            std::array<GtkToggleButton *, TOGGLE_BUTTON_COUNT> toggle_buttons_{};
            ToggleButton toggled_button_{ ToggleButton::Albums };

        private:
            DISABLE_COPY(PageStackSwitcher)
            DISABLE_MOVE(PageStackSwitcher)
        };
    }
}

#endif // !SPRING_PLAYER_PAGE_STACK_SWITCHER_H
