#ifndef SPRING_PLAYER_ALBUM_WIDGET_H
#define SPRING_PLAYER_ALBUM_WIDGET_H

#include <gtk/gtk.h>

#include <libspring_global.h>
#include <libspring_music_album.h>
#include <libspring_music_track.h>

#include "utility.h"

struct TrackListUpdateRequest;
struct TrackListUpdateResponse;

namespace spring
{
    namespace player
    {
        class AlbumWidget
        {
        public:
            AlbumWidget(music::Album &&album) noexcept;
            ~AlbumWidget() noexcept = default;

        public:
            const std::string &title() const noexcept;
            const std::string &artist() const noexcept;
            void activated() noexcept;

        public:
            operator GtkWidget *() noexcept;

        private:
            static void on_track_activated(GtkListBox *list_box,
                                           GtkListBoxRow *element,
                                           AlbumWidget *self) noexcept;
            static void on_popover_closed(GtkPopover *popover,
                                          AlbumWidget *self) noexcept;

        private:
            utility::GtkRefGuard<GtkBox> album_widget_{ nullptr };
            GtkImage *cover_{ nullptr };
            GtkLabel *artist_{ nullptr };
            GtkLabel *title_{ nullptr };
            utility::GtkRefGuard<GtkPopover> track_list_popover_{ nullptr };
            GtkListBox *track_list_{ nullptr };
            GtkSpinner *tracks_loading_spinner_{ nullptr };

            music::Album album_{ nullptr };
            std::vector<music::Track> tracks_{};

        private:
            DISABLE_COPY(AlbumWidget)
            DISABLE_MOVE(AlbumWidget)

        private:
            friend struct ::TrackListUpdateRequest;
            friend struct ::TrackListUpdateResponse;
        };
    }
}

#endif // !SPRING_PLAYER_ALBUM_WIDGET_H
