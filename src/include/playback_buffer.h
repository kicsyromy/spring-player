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
                signal(buffering_finished);
                signal(buffer_updated, std::uint8_t *, std::size_t);

            public:
                void start_buffering(const music::Track &track) noexcept;
                void stop_buffering() noexcept;
                std::string_view buffer_range(std::size_t index,
                                              std::size_t count) const noexcept;
                std::string &take() noexcept;

            private:
                std::thread thread_{};
                std::atomic_bool keep_buffering_{ false };
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
            signal(minimum_available_buffer_reached);
            signal(minimum_available_buffer_exceeded);
            signal(caching_finished);
            signal(cache_updated, std::size_t);

        private:
            Producer buffer_producer_{};
            std::string buffer_{};
            std::size_t consumed_{ 0 };
            std::size_t file_size_{};

        private:
            DISABLE_COPY(PlaybackBuffer)
            DISABLE_MOVE(PlaybackBuffer)
        };
    }
}

#endif // !SPRING_PLAYER_PLAYBACK_BUFFER_H
