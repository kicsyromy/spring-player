#ifndef SPRING_PLAYER_ALBUMS_PAGE_H
#define SPRING_PLAYER_ALBUMS_PAGE_H

#include <memory>
#include <vector>

#include <gtk/gtk.h>

#include <libspring_music_library.h>

#include "album_widget.h"
#include "now_playing_list.h"

namespace spring
{
    namespace player
    {
        class AlbumsPage
        {
        public:
            AlbumsPage(GtkBuilder *builder,
                       std::weak_ptr<MusicLibrary> music_library,
                       std::weak_ptr<PlaybackList> playback_list) noexcept;
            ~AlbumsPage() noexcept = default;

        public:
            void activated() noexcept;

        private:
            static gboolean filter(GtkFlowBoxChild *child, void *self) noexcept;
            static void on_search_changed(GtkSearchEntry *element,
                                          AlbumsPage *self) noexcept;

        public:
            operator GtkWidget *() noexcept;

        private:
            static void on_child_activated(GtkFlowBox *,
                                           GtkFlowBoxChild *element,
                                           AlbumsPage *self) noexcept;

        private:
            GtkScrolledWindow *albums_page_{ nullptr };
            GtkFlowBox *albums_content_{ nullptr };
            GtkSpinner *albums_loading_spinner_{ nullptr };
            GtkSearchEntry *search_entry_{ nullptr };

            std::weak_ptr<MusicLibrary> music_library_{};
            std::vector<std::unique_ptr<AlbumWidget>> albums_{};

            std::weak_ptr<PlaybackList> playback_list_{};

        private:
            DISABLE_COPY(AlbumsPage)
            DISABLE_MOVE(AlbumsPage)
        };
    }
}

#endif // !SPRING_PLAYER_ALBUMS_PAGE_H
