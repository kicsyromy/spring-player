#ifndef SPRING_PLAYER_THUMBNAIL_WIDGET_H
#define SPRING_PLAYER_THUMBNAIL_WIDGET_H

#include <memory>

#include <gtk/gtk.h>

#include <libspring_logger.h>
#include <libspring_music_track.h>

#include "async_queue.h"
#include "resource_cache.h"

#include "utility/compatibility.h"
#include "utility/global.h"
#include "utility/gtk_helpers.h"
#include "utility/pixbuf_loader.h"

namespace spring
{
    namespace player
    {
        class PlaybackList;

        template <typename ContentProvider> class ThumbnailWidget
        {
        public:
            ThumbnailWidget(ContentProvider &&content_provider,
                            utility::string_view main_text,
                            utility::string_view secondary_text,
                            utility::string_view cache_prefix,
                            std::weak_ptr<PlaybackList> playback_list) noexcept;

        public:
            utility::string_view main_title() const noexcept;
            utility::string_view secondary_title() const noexcept;

        public:
            void activated() noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            std::pair<std::vector<music::Track> *, std::vector<utility::GObjectGuard<GtkBox>> *>
            load_tracks() const noexcept;

            void on_tracks_loaded(
                std::vector<music::Track> *tracks,
                std::vector<utility::GObjectGuard<GtkBox>> *track_widgets) noexcept;

        private:
            static void on_track_activated(GtkListBox *list_box,
                                           GtkListBoxRow *element,
                                           ThumbnailWidget *self) noexcept;
            static void on_popover_closed(GtkPopover *popover, ThumbnailWidget *self) noexcept;

        private:
            utility::GObjectGuard<GtkBox> thumbnail_widget_{ nullptr };
            GtkImage *image_{ nullptr };
            GtkLabel *main_title_{ nullptr };
            GtkLabel *secondary_title_{ nullptr };

            utility::GObjectGuard<GtkPopover> listbox_popover_{ nullptr };
            GtkListBox *listbox_{ nullptr };
            GtkSpinner *loading_spinner_{ nullptr };

            ContentProvider content_provider_{ nullptr };
            std::vector<std::shared_ptr<music::Track>> tracks_{};

            std::weak_ptr<PlaybackList> playback_list_{};
        };

#include "thumbnail_widget.tpp"
    }
}

#endif // !SPRING_PLAYER_THUMBNAIL_WIDGET_H
