#ifndef SPRING_PLAYER_NOW_PLAYING_LIST_H
#define SPRING_PLAYER_NOW_PLAYING_LIST_H

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
        class NowPlayingList
        {
        public:
            static NowPlayingList &instance() noexcept;

        public:
            using PlaybackState = GStreamerPipeline::PlaybackState;
            using Milliseconds = GStreamerPipeline::Milliseconds;

        private:
            NowPlayingList() noexcept;
            ~NowPlayingList() noexcept;

        public:
            const music::Track *current_track() const noexcept;

        public:
            void play() noexcept;
            void pause() noexcept;
            void stop() noexcept;
            void shuffle() noexcept;
            void clear() noexcept;
            void restart_current_track() noexcept;
            void enqueue(const music::Track &track) noexcept;
            const music::Track &enqueue(music::Track &&track) noexcept;

        public:
            signal(playback_state_changed, PlaybackState);
            signal(playback_position_changed, std::int64_t);
            signal(track_queued, const music::Track &);

        private:
            GStreamerPipeline pipeline_{};
            std::vector<music::Track> content_{};

        private:
            DISABLE_COPY(NowPlayingList)
            DISABLE_MOVE(NowPlayingList)
        };
    }
}

#endif // !SPRING_PLAYER_NOW_PLAYING_LIST_H
