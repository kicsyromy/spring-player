#ifndef SPRING_PLAYER_MAIN_WINDOW_H
#define SPRING_PLAYER_MAIN_WINDOW_H

#include <memory>

#include <gtk/gtk.h>

#include <libspring_global.h>
#include <libspring_plex_media_server.h>

#include "now_playing_sidebar.h"
#include "page_stack.h"
#include "playback_header.h"
#include "utility.h"

using SpringPlayer = struct _SpringPlayer;

namespace spring
{
    namespace player
    {
        class MainWindow
        {
        public:
            MainWindow(SpringPlayer &application) noexcept;
            ~MainWindow() noexcept;

        public:
            void show() noexcept;
            void hide() noexcept;

        private:
            static void on_search_toggled(GtkToggleButton *toggle_button,
                                          MainWindow *self) noexcept;

        private:
            GtkApplicationWindow *main_window_{ nullptr };
            GtkRevealer *search_revealer_{ nullptr };
            GtkSearchEntry *search_entry_{ nullptr };
            GtkToggleButton *search_button_{ nullptr };
            GtkLabel *window_title_{ nullptr };
            std::unique_ptr<PageStack> page_stack_{ nullptr };
            std::unique_ptr<NowPlayingSidebar> now_playing_sidebar_{ nullptr };
            std::unique_ptr<PlaybackHeader> playback_footer_{ nullptr };

            PlexMediaServer pms_;

            GtkCssProvider *css_provider_{ nullptr };

        private:
            DISABLE_COPY(MainWindow)
            DISABLE_MOVE(MainWindow)
        };
    }
}

#endif // !SPRING_PLAYER_MAIN_WINDOW_H
