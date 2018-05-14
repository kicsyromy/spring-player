#ifndef SPRING_PLAYER_GSTREAMER_PIPELINE_H
#define SPRING_PLAYER_GSTREAMER_PIPELINE_H

#include <atomic>
#include <memory>

#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

#include <libspring_global.h>
#include <libspring_music_track.h>

#include "playback_buffer.h"

#include "utility/signals.h"

namespace spring
{
    namespace player
    {
        class PlaybackList;

        class GStreamerPipeline
        {
        public:
            enum class PlaybackState
            {
                Invalid = -1,
                Pending,
                Playing,
                Paused,
                Stopped,
                Count
            };

            using Milliseconds = music::Track::Milliseconds;

        public:
            GStreamerPipeline(const PlaybackList &playback_list) noexcept;
            ~GStreamerPipeline() noexcept;

        public:
            void play(const std::shared_ptr<const music::Track> &track) noexcept;
            void pause_resume() noexcept;
            void stop() noexcept;
            void seek(music::Track::Milliseconds target) noexcept;
            PlaybackState playback_state() const noexcept;

        public:
            static const char *playback_state_to_string(PlaybackState state) noexcept;

        public:
            signal(playback_state_changed, PlaybackState);
            signal(playback_position_changed, Milliseconds);
            signal(track_cache_updated, std::size_t);
            signal(track_cached);

        private:
            static void gst_playback_finished(GstBus *bus,
                                              GstMessage *message,
                                              GStreamerPipeline *self) noexcept;

            static void gst_playback_state_changed(GstBus *bus,
                                                   GstMessage *message,
                                                   GStreamerPipeline *self) noexcept;

            static PlaybackState on_gst_state_change_void_pending(GStreamerPipeline &self) noexcept;
            static PlaybackState on_gst_state_change_null(GStreamerPipeline &self) noexcept;
            static PlaybackState on_gst_state_change_ready(GStreamerPipeline &self) noexcept;
            static PlaybackState on_gst_state_change_paused(GStreamerPipeline &self) noexcept;
            static PlaybackState on_gst_state_change_playing(GStreamerPipeline &self) noexcept;

            static void gst_playback_error(GstBus *bus,
                                           GstMessage *message,
                                           GStreamerPipeline *self) noexcept;

            static std::int32_t update_playback_position(GStreamerPipeline *self) noexcept;

            static void gst_appsrc_setup(GstElement *pipeline,
                                         GstElement *source,
                                         GStreamerPipeline *self) noexcept;

            static void gst_appsrc_need_data(GstAppSrc *src,
                                             guint,
                                             GStreamerPipeline *self) noexcept;

            static std::int32_t push_data_to_appsrc(GStreamerPipeline *self) noexcept;

            static void gst_appsrc_enough_data(GstAppSrc *src, GStreamerPipeline *self) noexcept;

            static gboolean gst_appsrc_seek_data(GstAppSrc *src,
                                                 guint64 offset,
                                                 GStreamerPipeline *self) noexcept;

        private:
            GstElement *playbin_{ nullptr };
            GstAppSrc *appsrc_{ nullptr };
            GstBus *bus_{ nullptr };
            PlaybackState current_state_{ PlaybackState::Stopped };
            GstState gst_state_{ GST_STATE_VOID_PENDING };

            PlaybackBuffer playback_buffer_{};
            const PlaybackList &playback_list_;

            std::uint32_t progress_update_source_id_{ 0 };
            std::atomic<std::uint32_t> push_data_source_id_{ 0 };

        private:
            using gst_state_change_handler_t = PlaybackState (*)(GStreamerPipeline &);
            gst_state_change_handler_t gst_state_change_handlers_[5]{
                &GStreamerPipeline::on_gst_state_change_void_pending,
                &GStreamerPipeline::on_gst_state_change_null,
                &GStreamerPipeline::on_gst_state_change_ready,
                &GStreamerPipeline::on_gst_state_change_paused,
                &GStreamerPipeline::on_gst_state_change_playing,
            };

        private:
            DISABLE_COPY(GStreamerPipeline)
            DISABLE_MOVE(GStreamerPipeline)
        };
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_GSTREAMER_PIPELINE_H
