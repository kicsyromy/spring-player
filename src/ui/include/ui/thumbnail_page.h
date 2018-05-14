#ifndef SPRING_PLAYER_THUMBNAIL_PAGE_H
#define SPRING_PLAYER_THUMBNAIL_PAGE_H

#include <memory>
#include <vector>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <libspring_logger.h>
#include <libspring_music_library.h>

#include "playback/playlist.h"
#include "ui/thumbnail_widget.h"

#include "utility/fuzzy_search.h"
#include "utility/g_object_guard.h"
#include "utility/global.h"
#include "utility/gtk_helpers.h"
#include "utility/signals.h"

namespace spring
{
    namespace player
    {
        namespace playback
        {
            class Playlist;
        }

        namespace ui
        {
            class HeaderBar;

            template <typename ContentProvider> class ThumbnailPage
            {
            private:
                static constexpr auto SECONDARY_PAGE_TITLE{ "SECONDARY_PAGE" };

            public:
                ThumbnailPage(std::weak_ptr<MusicLibrary> music_library,
                              std::weak_ptr<playback::Playlist> playback_list) noexcept;

            public:
                template <typename FetchFunction> void activated(FetchFunction &&f) noexcept;
                void filter(std::string &&text) noexcept;

            public:
                void set_secondary_content_widget(GtkWidget *widget) noexcept;
                void switch_to_primary_page() noexcept;
                void switch_to_secondary_page() noexcept;

            public:
                signal(thumbnail_activated, ThumbnailWidget<ContentProvider> *);

            public:
                GtkWidget *operator()() noexcept;

            private:
                static std::int32_t filter(GtkFlowBoxChild *child, void *self) noexcept;
                static void on_child_activated(GtkFlowBox *,
                                               GtkFlowBoxChild *element,
                                               ThumbnailPage *self) noexcept;

            public:
                utility::GObjectGuard<GtkScrolledWindow> page_{ nullptr };
                GtkStack *content_stack_{ nullptr };
                GtkWidget *main_content_page_{ nullptr };
                GtkFlowBox *content_{ nullptr };
                GtkSpinner *loading_spinner_{ nullptr };
                utility::GObjectGuard<GtkWidget> secondary_content_page_{ nullptr };

                std::weak_ptr<MusicLibrary> music_library_{};
                std::vector<std::unique_ptr<ThumbnailWidget<ContentProvider>>> children_{};

                std::weak_ptr<playback::Playlist> playback_list_{};

                std::string search_string_{};
            };

#include "thumbnail_page.tpp"
        } // namespace ui
    }     // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_THUMBNAIL_PAGE_H
