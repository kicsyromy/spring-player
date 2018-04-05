#ifndef SPRING_PLAYER_PLAYBACK_LIST_H
#define SPRING_PLAYER_PLAYBACK_LIST_H

#include <functional>
#include <memory>
#include <vector>

#include <libspring_music_track.h>

#include "gstreamer_pipeline.h"
#include "utility/signals.h"

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
            std::pair<std::int32_t, const music::Track *> current_track() const noexcept;
            std::size_t track_count() const noexcept;
            PlaybackState playback_state() const noexcept;

        public:
            void play(std::size_t index = 0) noexcept;
            void play_pause() noexcept;
            void seek_current_track(Milliseconds value) noexcept;
            void stop() noexcept;
            void next() noexcept;
            void previous() noexcept;
            void shuffle() noexcept;
            void clear() noexcept;
            void enqueue(std::shared_ptr<music::Track> track) noexcept;

        public:
            signal(playback_state_changed, PlaybackState);
            signal(playback_position_changed, std::int64_t);
            signal(track_queued, std::shared_ptr<music::Track> &);
            signal(list_cleared);
            signal(track_cache_updated, std::size_t);
            signal(track_cached);

        private:
            static void on_playback_state_changed(PlaybackState new_state,
                                                  PlaybackList *self) noexcept;

        private:
            GStreamerPipeline pipeline_{ *this };
            std::vector<std::shared_ptr<music::Track>> content_{};
            std::int32_t current_index_{ -1 };

        private:
            DISABLE_COPY(PlaybackList)
            DISABLE_MOVE(PlaybackList)
        };
    } // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_PLAYBACK_LIST_H
