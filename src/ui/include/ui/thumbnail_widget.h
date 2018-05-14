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
#include "utility/signals.h"

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
            const ContentProvider &content_provider() const noexcept;

        public:
            GtkWidget *operator()() noexcept;

        private:
            utility::GObjectGuard<GtkBox> thumbnail_widget_{ nullptr };
            GtkImage *image_{ nullptr };
            GtkLabel *main_title_{ nullptr };
            GtkLabel *secondary_title_{ nullptr };

            ContentProvider content_provider_{ nullptr };

            std::weak_ptr<PlaybackList> playback_list_{};
        };

#include "thumbnail_widget.tpp"
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_THUMBNAIL_WIDGET_H
