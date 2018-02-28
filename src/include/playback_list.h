#ifndef SPRING_PLAYER_PLAYBACK_LIST_H
#define SPRING_PLAYER_PLAYBACK_LIST_H

#include <functional>
#include <vector>

#include <gst/gst.h>

#include <libspring_music_track.h>

#include "gstreamer_pipeline.h"
#include "utility.h"

namespace spring
{
    namespace player
    {
        class PlaybackList
        {
        public:
            using PlaybackState = GStreamerPipeline::PlaybackState;
            using Milliseconds = GStreamerPipeline::Milliseconds;

        public:
            PlaybackList() noexcept;
            ~PlaybackList() noexcept;

        public:
            const music::Track *current_track() const noexcept;
            std::size_t track_count() const noexcept;

        public:
            void play_from(std::size_t index) noexcept;
            void play_pause() noexcept;
            void stop() noexcept;
            void next() noexcept;
            void previous() noexcept;
            void shuffle() noexcept;
            void clear() noexcept;
            void enqueue(const music::Track &track) noexcept;
            const music::Track &enqueue(music::Track &&track) noexcept;

        public:
            signal(playback_state_changed, PlaybackState);
            signal(playback_position_changed, std::int64_t);
            signal(track_queued, const music::Track &);
            signal(track_cache_updated, std::size_t);
            signal(track_cached);

        private:
            GStreamerPipeline pipeline_{};
            std::vector<music::Track> content_{};
            std::size_t current_index_{ 0 };

        private:
            DISABLE_COPY(PlaybackList)
            DISABLE_MOVE(PlaybackList)
        };
    }
}

#endif // !SPRING_PLAYER_PLAYBACK_LIST_H
