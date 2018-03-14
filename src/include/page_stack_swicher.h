#ifndef SPRING_PLAYER_PAGE_STACK_SWITCHER_H
#define SPRING_PLAYER_PAGE_STACK_SWITCHER_H

#include <array>
#include <cstdint>
#include <functional>

#include <libspring_global.h>

#include "application_settings.h"

#include "utility/forward_declarations.h"

namespace spring
{
    namespace player
    {
        class PageStackSwitcher
        {
        public:
            using ToggleButton = settings::Page;

        public:
            PageStackSwitcher(GtkBuilder *builder,
                              std::function<void(ToggleButton)> &&toggled_handler) noexcept;
            ~PageStackSwitcher() noexcept;

        public:
            ToggleButton toggled_button() const noexcept;

        private:
            static void button_toggled(GtkToggleButton *toggle_button,
                                       PageStackSwitcher *self) noexcept;

        private:
            std::array<GtkToggleButton *, 4> toggle_buttons_{};
            std::function<void(ToggleButton)> toggled_handler_{};
            ToggleButton toggled_button_{ ToggleButton::Albums };

        private:
            DISABLE_COPY(PageStackSwitcher)
            DISABLE_MOVE(PageStackSwitcher)
        };
    }
}

#endif // !SPRING_PLAYER_PAGE_STACK_SWITCHER_H
