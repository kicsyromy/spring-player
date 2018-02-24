#ifndef SPRING_PLAYER_GSTREAMER_PIPELINE_H
#define SPRING_PLAYER_GSTREAMER_PIPELINE_H

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

        private:
            GstElement *playbin_{ nullptr };
            GstBus *bus_{ nullptr };
            std::uint32_t position_update_callback_tag_{ 0 };
            PlaybackState current_state_{ PlaybackState::Stopped };

            const music::Track *current_track_{ nullptr };

        private:
            DISABLE_COPY(GStreamerPipeline)
            DISABLE_MOVE(GStreamerPipeline)
        };
    }
}

#endif // !SPRING_PLAYER_GSTREAMER_PIPELINE_H
