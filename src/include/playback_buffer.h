#ifndef SPRING_PLAYER_PLAYBACK_BUFFER_H
#define SPRING_PLAYER_PLAYBACK_BUFFER_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <string>
#include <thread>

#include <libspring_global.h>
#include <libspring_music_track.h>

#include "utility.h"

namespace spring
{
    namespace player
    {
        class PlaybackBuffer
        {
        private:
            class Producer
            {
            public:
                Producer() noexcept;
                ~Producer() noexcept;

            public:
                signal(prebuffer_filled);
                signal(buffering_finished);

            public:
                void start_buffering(const music::Track &track) noexcept;
                void stop_buffering() noexcept;
                std::string_view buffer_range(std::size_t index,
                                              std::size_t count) const noexcept;
                bool buffering_finished() const noexcept;
                std::string &take() noexcept;

            private:
                std::string buffer_{};

                std::thread thread_{};
                mutable std::mutex mutex_{};
                mutable std::condition_variable condition_variable_{};

                std::atomic_bool keep_buffering_{ false };
                std::atomic_bool buffering_done_{ true };
            };

        public:
            static constexpr const std::size_t CHUNK_SIZE{ 16 * 1024 };

        public:
            explicit PlaybackBuffer() noexcept;
            ~PlaybackBuffer() noexcept;

        public:
            void cache(const music::Track &track) noexcept;
            const std::string_view consume(std::size_t count) noexcept;

        public:
            signal(precaching_finished);
            signal(caching_finished);

        private:
            Producer buffer_producer_{};
            std::string completed_buffer_{};
            std::size_t consumed_{ 0 };
            std::size_t file_size_{};

        private:
            DISABLE_COPY(PlaybackBuffer)
            DISABLE_MOVE(PlaybackBuffer)
        };
    }
}

#endif // !SPRING_PLAYER_PLAYBACK_BUFFER_H
