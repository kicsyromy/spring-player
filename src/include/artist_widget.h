#ifndef SPRING_PLAYER_ARTIST_WIDGET_H
#define SPRING_PLAYER_ARTIST_WIDGET_H

#include <gtk/gtk.h>

#include <libspring_global.h>
#include <libspring_music_artist.h>

#include "utility.h"

namespace spring
{
    namespace player
    {
        class ArtistWidget
        {
        public:
            ArtistWidget(music::Artist &&artist) noexcept;
            ~ArtistWidget() noexcept;

        private:
            utility::GtkRefGuard<GtkBox> artist_widget_{ nullptr };
            GtkImage *thumbnail_{ nullptr };
            GtkLabel *name_{ nullptr };
            music::Artist artist_{ nullptr };

        public:
            GtkWidget *container() noexcept;

        private:
            DISABLE_COPY(ArtistWidget)
            DISABLE_MOVE(ArtistWidget)
        };
    }
}

#endif // !SPRING_PLAYER_ARTIST_WIDGET_H
