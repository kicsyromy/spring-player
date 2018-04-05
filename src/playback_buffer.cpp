#include <cstring>

#include <libspring_logger.h>

#include "playback_buffer.h"

#include "utility/global.h"

using namespace spring;
using namespace spring::player;

namespace
{
    constexpr const std::size_t MINIMUM_UNCONSUMED_BUFFER{ 128 * 1024 };

    constexpr music::Track::Seconds milliseconds_to_seconds(music::Track::Milliseconds value)
    {
        return std::chrono::duration_cast<std::chrono::seconds>(value);
    };
} // namespace

PlaybackBuffer::Producer::Producer() noexcept
{
    LOG_INFO("PlaybackBuffer::Producer: Creating...");
}

PlaybackBuffer::Producer::~Producer() noexcept
{
    LOG_INFO("PlaybackBuffer::Producer: Destroying...");
    stop_buffering();
}

void PlaybackBuffer::Producer::start_buffering(std::weak_ptr<const music::Track> target_track,
                                               std::chrono::seconds offset) noexcept
{
    auto track = target_track.lock();
    if (track == nullptr)
    {
        LOG_WARN("PlaybackBuffer::Producer({}): Attempt to buffer null track", void_p(this));
        return;
    }

    LOG_INFO("PlaybackBuffer::Producer({}): New buffering session for track {}", void_p(this),
             track->title());

    thread_ = std::thread{ [this, track, offset] {
        keep_buffering_ = true;
        LOG_INFO("PlaybackBuffer::Producer({}): Start buffering for track {}", void_p(this),
                 track->title());

        track->trackData(
            [](std::uint8_t *data, std::size_t size, void *instance) {
                auto self = static_cast<PlaybackBuffer::Producer *>(instance);

                std::size_t result{ 0 };
                if (self->keep_buffering_)
                {
                    auto buffer = new std::uint8_t[size];
                    memcpy(buffer, data, size);
                    result = size;
                    self->emit_queued_buffer_updated(std::move(buffer), std::move(size));
                }
                else
                {
                    LOG_INFO("PlaybackBuffer::Producer: Buffering interrupted...");
                }
                return result;
            },
            offset, this);

        if (keep_buffering_)
        {
            LOG_INFO("PlaybackBuffer::Producer({}): Buffering finished for track {}", void_p(this),
                     track->title());
            emit_queued_buffering_finished();
        }
        else
        {
            LOG_INFO("PlaybackBuffer::Producer({}): Buffering interrupted for "
                     "track {}",
                     void_p(this), track->title());
        }
        keep_buffering_ = false;
    } };
}

void PlaybackBuffer::Producer::stop_buffering() noexcept
{
    LOG_INFO("PlaybackBuffer::Producer({}): Attempting to stop buffering...", void_p(this));

    keep_buffering_ = false;

    if (thread_.joinable())
    {
        thread_.join();
    }
}

PlaybackBuffer::PlaybackBuffer() noexcept
{
    LOG_INFO("PlaybackBuffer({}): Creating...", void_p(this));

    buffer_producer_.on_buffering_finished(this,
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

            std::unique_ptr<const char[]> buffer{ reinterpret_cast<const char *>(data) };

            self->buffer_.append(buffer.get(), size);
            auto new_size = self->buffer_.size();
            self->emit_cache_updated(std::move(new_size));

            if (self->buffer_.size() - self->consumed_ >= MINIMUM_UNCONSUMED_BUFFER)
            {
                self->emit_minimum_available_buffer_reached();
            }
        },
        this);
}

PlaybackBuffer::~PlaybackBuffer() noexcept
{
    LOG_INFO("PlaybackBuffer({}): Destroying...", void_p(this));

    buffer_producer_.disconnect_buffer_updated(this);
    buffer_producer_.disconnect_buffering_finished(this);
}

void PlaybackBuffer::cache(std::shared_ptr<const music::Track> track) noexcept
{
    LOG_INFO("PlaybackBuffer({}): Caching track {}", void_p(this), track->title());

    current_track_ = track;
    start_caching();
}

const utility::string_view PlaybackBuffer::consume(std::size_t count) noexcept
{
    const auto range_begin = consumed_;
    const auto range_size =
        consumed_ + count >= buffer_.size() ? buffer_.size() - consumed_ : count;

    consumed_ += range_size;
    if (!buffering_finished_ && buffer_.size() - consumed_ < MINIMUM_UNCONSUMED_BUFFER)
    {
        emit_minimum_available_buffer_exceeded();
    }
    return { buffer_.data() + range_begin, range_size };
}

void PlaybackBuffer::seek(music::Track::Milliseconds offset) noexcept
{
    start_caching(milliseconds_to_seconds(offset));
}

void PlaybackBuffer::start_caching(music::Track::Seconds offset) noexcept
{
    buffer_producer_.stop_buffering();

    consumed_ = 0;
    buffering_finished_ = false;
    buffer_.clear();
    buffer_producer_.start_buffering(current_track_, offset);
}
