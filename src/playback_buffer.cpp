#include "playback_buffer.h"

#include <cstring>

using namespace spring;
using namespace spring::player;

namespace
{
    constexpr const std::size_t MINIMUM_UNCONSUMED_BUFFER{ 128 * 1024 };
}

PlaybackBuffer::Producer::Producer() noexcept = default;
PlaybackBuffer::Producer::~Producer() noexcept
{
    stop_buffering();
}

void PlaybackBuffer::Producer::start_buffering(
    const music::Track &track) noexcept
{
    stop_buffering();

    thread_ = std::thread{ [this, &track] {
        keep_buffering_ = true;

        track.trackData(
            [](std::uint8_t *data, std::size_t size, void *instance) {
                auto self = static_cast<PlaybackBuffer::Producer *>(instance);

                std::size_t result{ 0 };
                if (self->keep_buffering_)
                {
                    auto buffer = new std::uint8_t[size];
                    memcpy(buffer, data, size);
                    result = size;
                    self->emit_queued_buffer_updated(std::move(buffer),
                                                     std::move(size));
                }
                return result;
            },
            this);

        if (keep_buffering_)
        {
            emit_queued_buffering_finished();
        }
        keep_buffering_ = false;
    } };
}

void PlaybackBuffer::Producer::stop_buffering() noexcept
{
    keep_buffering_ = false;

    if (thread_.joinable())
    {
        thread_.join();
    }
}

PlaybackBuffer::PlaybackBuffer() noexcept
{
    buffer_producer_.on_buffering_finished(
        this,
        [](void *instance) {
            auto self = static_cast<PlaybackBuffer *>(instance);

            self->buffering_finished_ = true;
            self->emit_caching_finished();
        },
        this);

    buffer_producer_.on_buffer_updated(
        this,
        [](std::uint8_t *data, std::size_t size, void *instance) {
            auto self = static_cast<PlaybackBuffer *>(instance);

            std::unique_ptr<const char[]> buffer{
                reinterpret_cast<const char *>(data)
            };

            self->buffer_.append(buffer.get(), size);
            auto new_size = self->buffer_.size();
            self->emit_cache_updated(std::move(new_size));

            if (self->buffer_.size() - self->consumed_ >=
                MINIMUM_UNCONSUMED_BUFFER)
            {
                self->emit_minimum_available_buffer_reached();
            }
        },
        this);
}

PlaybackBuffer::~PlaybackBuffer() noexcept
{
    buffer_producer_.disconnect_buffer_updated(this);
    buffer_producer_.disconnect_buffering_finished(this);
}

void PlaybackBuffer::cache(const music::Track &track) noexcept
{
    consumed_ = 0;
    buffering_finished_ = false;
    buffer_.clear();
    buffer_producer_.start_buffering(track);
}

const std::string_view PlaybackBuffer::consume(std::size_t count) noexcept
{
    const auto range_begin = consumed_;
    const auto range_size = consumed_ + count >= buffer_.size() ?
                                buffer_.size() - consumed_ :
                                count;

    consumed_ += range_size;
    if (!buffering_finished_ &&
        buffer_.size() - consumed_ < MINIMUM_UNCONSUMED_BUFFER)
    {
        emit_minimum_available_buffer_exceeded();
    }
    return { buffer_.data() + range_begin, range_size };
}
