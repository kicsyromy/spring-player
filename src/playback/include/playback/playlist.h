#ifndef SPRING_PLAYER_PLAYBACK_LIST_H
#define SPRING_PLAYER_PLAYBACK_LIST_H

#include <functional>
#include <memory>
#include <vector>

#include <libspring_music_track.h>

#include "playback/gstreamer_pipeline.h"
#include "utility/signals.h"

namespace spring
{
    namespace player
    {
        namespace playback
        {
            class Playlist
            {
            public:
                using PlaybackState = GStreamerPipeline::PlaybackState;
                using Milliseconds = GStreamerPipeline::Milliseconds;

            public:
                Playlist() noexcept;
                ~Playlist() noexcept;

            public:
                std::pair<std::int32_t, const music::Track *> current_track() const noexcept;
                std::size_t track_count() const noexcept;
                PlaybackState playback_state() const noexcept;

                void set_repeat_all_active(bool value) noexcept;
                void set_repeat_one_active(bool value) noexcept;
                void set_shuffle_active(bool value) noexcept;

            public:
                void play(std::size_t index = 0) noexcept;
                void play_pause() noexcept;
                void seek_current_track(Milliseconds value) noexcept;
                void stop() noexcept;
                void next() noexcept;
                void previous() noexcept;
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
                                                      Playlist *self) noexcept;

            private:
                GStreamerPipeline pipeline_{ *this };
                std::vector<std::shared_ptr<music::Track>> content_{};
                std::int32_t current_index_{ -1 };
                bool repeat_all_active_{ false };
                bool repeat_one_active_{ false };
                bool shuffle_active_{ false };

            private:
                DISABLE_COPY(Playlist)
                DISABLE_MOVE(Playlist)
            };
        } // namespace playback
    }     // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_PLAYBACK_LIST_H
