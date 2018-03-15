#ifndef SPRING_PLAYER_MAIN_WINDOW_H
#define SPRING_PLAYER_MAIN_WINDOW_H

#include <memory>

#include <libspring_global.h>
#include <libspring_plex_media_server.h>

#include "header_bar.h"
#include "playlist_sidebar.h"

#include "page_stack.h"
#include "playback_header.h"
#include "playback_list.h"

#include "utility/forward_declarations.h"

using SpringPlayer = struct _SpringPlayer;

namespace spring
{
    namespace player
    {
        class MainWindow
        {
        public:
            MainWindow(SpringPlayer &application,
                       std::shared_ptr<PlaybackList> playback_list) noexcept;
            ~MainWindow() noexcept;

        public:
            void show() noexcept;
            void hide() noexcept;

        private:
            static void on_search_toggled(GtkToggleButton *toggle_button,
                                          MainWindow *self) noexcept;

        private:
            static void set_tile(PlaybackList::PlaybackState state, MainWindow *self) noexcept;
            static void toggle_playlist(bool toggled, MainWindow *self) noexcept;

        private:
            GtkApplicationWindow *main_window_{ nullptr };
            GtkPaned *paned_{ nullptr };
            GtkWidget *sidebar_placeholder_{ nullptr };
            GtkBox *main_content_{ nullptr };
            GtkWidget *page_stack_placeholder_{ nullptr };
            GtkRevealer *search_revealer_{ nullptr };
            GtkSearchEntry *search_entry_{ nullptr };

            PlexMediaServer pms_;

            HeaderBar header_{ nullptr };
            PlaylistSidebar playlist_sidebar_{ nullptr };
            PageStack page_stack_;

            std::weak_ptr<PlaybackList> playback_list_{};

            GtkCssProvider *css_provider_{ nullptr };

        private:
            DISABLE_COPY(MainWindow)
            DISABLE_MOVE(MainWindow)
        };
    }
}

#endif // !SPRING_PLAYER_MAIN_WINDOW_H
