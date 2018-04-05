#ifndef SPRING_PLAYER_THUMBNAIL_PAGE_H
#define SPRING_PLAYER_THUMBNAIL_PAGE_H

#include <memory>
#include <vector>

#include <gtk/gtk.h>

#include <libspring_logger.h>
#include <libspring_music_library.h>

#include "playback_list.h"
#include "thumbnail_widget.h"

#include "utility/fuzzy_search.h"
#include "utility/g_object_guard.h"
#include "utility/global.h"
#include "utility/gtk_helpers.h"

namespace spring
{
    namespace player
    {
        class PlaybackList;
        class HeaderBar;

        template <typename ContentProvider> class ThumbnailPage
        {
        public:
            ThumbnailPage(std::weak_ptr<MusicLibrary> music_library,
                          std::weak_ptr<PlaybackList> playback_list) noexcept;

        public:
            template <typename FetchFunction> void activated(FetchFunction &&f) noexcept;
            void filter(std::string &&text) noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            static std::int32_t filter(GtkFlowBoxChild *child, void *self) noexcept;
            static void on_child_activated(GtkFlowBox *,
                                           GtkFlowBoxChild *element,
                                           ThumbnailPage *self) noexcept;

        public:
            utility::GObjectGuard<GtkScrolledWindow> page_{ nullptr };
            GtkFlowBox *content_{ nullptr };
            GtkSpinner *loading_spinner_{ nullptr };

            std::weak_ptr<MusicLibrary> music_library_{};
            std::vector<std::unique_ptr<ThumbnailWidget<ContentProvider>>> children_{};

            std::weak_ptr<PlaybackList> playback_list_{};

            std::string search_string_{};
        };

#include "thumbnail_page.tpp"
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_THUMBNAIL_PAGE_H
