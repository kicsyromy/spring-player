#ifndef SPRING_PLAYER_GSTREAMER_PIPELINE_H
#define SPRING_PLAYER_GSTREAMER_PIPELINE_H

#include <mutex>
#include <thread>

#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

#include <libspring_global.h>
#include <libspring_music_track.h>

#include "utility.h"

namespace spring
{
    namespace player
    {
        class GStreamerPipeline
        {
        public:
            static inline void initialize() noexcept
            {
                gst_init(nullptr, nullptr);
            }

        public:
            enum class PlaybackState
            {
                Invalid = -1,
                Playing,
                Paused,
                Stopped
            };

            using Milliseconds = music::Track::Milliseconds;

        public:
            GStreamerPipeline() noexcept;
            ~GStreamerPipeline() noexcept;

        public:
            void play(const music::Track &track) noexcept;
            void pause_resume() noexcept;
            void stop() noexcept;
            void seek(music::Track::Milliseconds target) noexcept;

        public:
            const music::Track *current_track() const noexcept;

        public:
            signal(playback_state_changed, PlaybackState);
            signal(playback_position_changed, Milliseconds);

        private:
            static void gst_playback_finished(GstBus *bus,
                                              GstMessage *message,
                                              GStreamerPipeline *self) noexcept;

            static void gst_playback_state_changed(
                GstBus *bus,
                GstMessage *message,
                GStreamerPipeline *self) noexcept;

            static void gst_playback_error(GstBus *bus,
                                           GstMessage *message,
                                           GStreamerPipeline *self) noexcept;

            static gboolean update_playback_position(
                GStreamerPipeline *self) noexcept;

            static void gst_appsrc_setup(GstElement *,
                                         GstElement *,
                                         GStreamerPipeline *self) noexcept;

            static void gst_appsrc_need_data(GstAppSrc *src,
                                             guint length,
                                             void *instance) noexcept;

            static void gst_appsrc_enough_data(GstAppSrc *src,
                                               void *instance) noexcept;

            static gboolean gst_appsrc_seek_data(GstAppSrc *src,
                                                 guint64 offset,
                                                 void *instance) noexcept;

        private:
            GstElement *playbin_{ nullptr };
            GstAppSrc *appsrc_{ nullptr };
            GstBus *bus_{ nullptr };
            std::uint32_t position_update_callback_tag_{ 0 };
            PlaybackState current_state_{ PlaybackState::Stopped };

            std::string playback_buffer_{};
            Semaphore playback_buffer_fill_count_{ 0 };
            std::size_t playback_buffer_bytes_read_{ std::string::npos };
            std::thread playback_buffer_producer_{};

            GstAppSrcCallbacks gst_appsrc_callbacks_{
                &GStreamerPipeline::gst_appsrc_need_data,
                &GStreamerPipeline::gst_appsrc_enough_data,
                &GStreamerPipeline::gst_appsrc_seek_data,
                {}
            };

            const music::Track *current_track_{ nullptr };

        private:
            DISABLE_COPY(GStreamerPipeline)
            DISABLE_MOVE(GStreamerPipeline)
        };
    }
}

#endif // !SPRING_PLAYER_GSTREAMER_PIPELINE_H
