#ifndef SPRING_PLAYER_WELCOME_PAGE_H
#define SPRING_PLAYER_WELCOME_PAGE_H

#include <libspring_global.h>

#include "utility/forward_declarations.h"
#include "utility/g_object_guard.h"
#include "utility/signals.h"

namespace spring
{
    namespace player
    {
        class MainWindow;

        class WelcomePage
        {
        public:
            WelcomePage() noexcept;
            ~WelcomePage() noexcept;

        public:
            signal(new_connection_requested);

        public:
            GtkWidget *operator()() noexcept;

        private:
            static void on_action_activated(GraniteWidgetsWelcome *,
                                            std::int32_t index,
                                            WelcomePage *self) noexcept;

        private:
            utility::GObjectGuard<GraniteWidgetsWelcome> welcome_;

        private:
            DISABLE_COPY(WelcomePage)
            DISABLE_MOVE(WelcomePage)
        };
    }
}

#endif // !SPRING_PLAYER_WELCOME_PAGE_H
