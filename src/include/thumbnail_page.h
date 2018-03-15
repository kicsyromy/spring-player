#ifndef SPRING_PLAYER_THUMBNAIL_PAGE_H
#define SPRING_PLAYER_THUMBNAIL_PAGE_H

#include <memory>
#include <vector>

#include <libspring_music_library.h>

#include "utility/forward_declarations.h"
#include "utility/g_object_guard.h"

namespace spring
{
    namespace player
    {
        class PlaybackList;
        class ThumbnailWidget;
        class HeaderBar;

        class ThumbnailPage
        {
        public:
            ThumbnailPage(std::weak_ptr<MusicLibrary> music_library,
                          std::weak_ptr<PlaybackList> playback_list) noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            static std::int32_t filter(GtkFlowBoxChild *child, void *self) noexcept;
            static void on_search_changed(GtkSearchEntry *element, ThumbnailPage *self) noexcept;
            static void on_child_activated(GtkFlowBox *,
                                           GtkFlowBoxChild *element,
                                           ThumbnailPage *self) noexcept;

        public:
            utility::GObjectGuard<GtkScrolledWindow> page_{ nullptr };
            GtkFlowBox *content_{ nullptr };
            GtkSpinner *loading_spinner_{ nullptr };

            std::weak_ptr<MusicLibrary> music_library_{};
            std::vector<std::unique_ptr<ThumbnailWidget>> albums_{};

            std::weak_ptr<PlaybackList> playback_list_{};
        };
    }
}

#endif // !SPRING_PLAYER_THUMBNAIL_PAGE_H
