#ifndef SPRING_PLAYER_NOW_PLAYING_SIDEBAR_H
#define SPRING_PLAYER_NOW_PLAYING_SIDEBAR_H

#include <gtk/gtk.h>

#include <libspring_global.h>

namespace spring
{
    namespace player
    {
        class NowPlayingSidebar
        {
        public:
            NowPlayingSidebar(GtkBuilder *builder) noexcept;
            ~NowPlayingSidebar() noexcept = default;

        public:
            void show() noexcept;
            void hide() noexcept;

        private:
            static void toggled(GtkToggleButton *toggle_button,
                                NowPlayingSidebar *self) noexcept;

        private:
            GtkBox *now_playing_sidebar_{ nullptr };
            GtkImage *now_playing_cover_{ nullptr };
            GtkListBox *now_playing_list_{ nullptr };
            GtkToggleButton *toggle_sidebar_button_{ nullptr };

        private:
            DISABLE_COPY(NowPlayingSidebar)
            DISABLE_MOVE(NowPlayingSidebar)
        };
    }
}

#endif // !SPRING_PLAYER_NOW_PLAYING_SIDEBAR_H
