#ifndef SPRING_PLAYER_ALBUMS_PAGE_H
#define SPRING_PLAYER_ALBUMS_PAGE_H

#include <memory>
#include <vector>

#include <gtk/gtk.h>

#include <libspring_music_library.h>

#include "album_widget.h"

namespace spring
{
    namespace player
    {
        class AlbumsPage
        {
        public:
            AlbumsPage(GtkBuilder *builder,
                       const MusicLibrary &music_library) noexcept;
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

            const MusicLibrary &music_library_;
            std::vector<std::unique_ptr<AlbumWidget>> albums_{};

        private:
            DISABLE_COPY(AlbumsPage)
            DISABLE_MOVE(AlbumsPage)
        };
    }
}

#endif // !SPRING_PLAYER_ALBUMS_PAGE_H
