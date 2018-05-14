#include <cstring>

#include <libspring_logger.h>

#include "playback/buffer.h"

#include "utility/global.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::playback;

namespace
{
    constexpr const std::size_t MINIMUM_UNCONSUMED_BUFFER{ 128 * 1024 };

    constexpr music::Track::Seconds milliseconds_to_seconds(music::Track::Milliseconds value)
    {
        return std::chrono::duration_cast<std::chrono::seconds>(value);
    };
} // namespace

Buffer::Producer::Producer() noexcept
{
    LOG_INFO("Buffer::Producer: Creating...");
}

Buffer::Producer::~Producer() noexcept
{
    LOG_INFO("Buffer::Producer: Destroying...");
    stop_buffering();
}

void Buffer::Producer::start_buffering(std::weak_ptr<const music::Track> target_track,
                                       std::chrono::seconds offset) noexcept
{
    auto track = target_track.lock();
    if (track == nullptr)
    {
        LOG_WARN("Buffer::Producer({}): Attempt to buffer null track", void_p(this));
        return;
    }

    LOG_INFO("Buffer::Producer({}): New buffering session for track {}", void_p(this),
             track->title());

    thread_ = std::thread{ [this, track, offset] {
        keep_buffering_ = true;
        LOG_INFO("Buffer::Producer({}): Start buffering for track {}", void_p(this),
                 track->title());

        track->trackData(
            [](std::uint8_t *data, std::size_t size, void *instance) {
                auto self = static_cast<Buffer::Producer *>(instance);

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
                    LOG_INFO("Buffer::Producer: Buffering interrupted...");
                }
                return result;
            },
            offset, this);

        if (keep_buffering_)
        {
            LOG_INFO("Buffer::Producer({}): Buffering finished for track {}", void_p(this),
                     track->title());
            emit_queued_buffering_finished();
        }
        else
        {
            LOG_INFO("Buffer::Producer({}): Buffering interrupted for "
                     "track {}",
                     void_p(this), track->title());
        }
        keep_buffering_ = false;
    } };
}

void Buffer::Producer::stop_buffering() noexcept
{
    LOG_INFO("Buffer::Producer({}): Attempting to stop buffering...", void_p(this));

    keep_buffering_ = false;

    if (thread_.joinable())
    {
        thread_.join();
    }
}

Buffer::Buffer() noexcept
{
    LOG_INFO("Buffer({}): Creating...", void_p(this));

    buffer_producer_.on_buffering_finished(this,
                                           [](void *instance) {
                                               auto self = static_cast<Buffer *>(instance);

                                               self->buffering_finished_ = true;
                                               self->emit_caching_finished();
                                           },
                                           this);

    buffer_producer_.on_buffer_updated(
        this,
        [](std::uint8_t *data, std::size_t size, void *instance) {
            auto self = static_cast<Buffer *>(instance);

            std::unique_ptr<const char[]> buffer{ reinterpret_cast<const char *>(data) };

            self->buffer_.append(buffer.get(), size);
            auto new_size = self->buffer_.size();
            self->emit_cache_updated(std::move(new_size));

            if (self->buffer_.size() - self->consumed_ >= MINIMUM_UNCONSUMED_BUFFER)
            {
                if (self->minimum_available_buffer_exceeded_)
                {
                    self->emit_minimum_available_buffer_reached();
                    self->minimum_available_buffer_exceeded_ = false;
                }
            }
        },
        this);
}

Buffer::~Buffer() noexcept
{
    LOG_INFO("Buffer({}): Destroying...", void_p(this));

    buffer_producer_.disconnect_buffer_updated(this);
    buffer_producer_.disconnect_buffering_finished(this);
}

bool Buffer::minimum_available_buffer_exceeded() const noexcept
{
    return minimum_available_buffer_exceeded_;
}

bool Buffer::buffering() const noexcept
{
    return !buffering_finished_;
}

void Buffer::set_track(const std::shared_ptr<const music::Track> &track) noexcept
{
    current_track_ = track;
}

void Buffer::start_caching(music::Track::Seconds offset) noexcept
{
    auto track = current_track_.lock();
    if (track != nullptr)
    {
        LOG_INFO("Buffer({}): Caching track {}", void_p(this), track->title());

        buffer_producer_.stop_buffering();

        consumed_ = 0;
        buffering_finished_ = false;
        buffer_.clear();
        buffer_producer_.start_buffering(current_track_, offset);

        minimum_available_buffer_exceeded_ = true;
        emit_minimum_available_buffer_exceeded();
    }
    else
    {
        LOG_INFO("Buffer({}): Failed to start caching NULL track", void_p(this));
    }
}

const utility::string_view Buffer::consume(std::size_t count) noexcept
{
    const auto range_begin = consumed_;
    const auto range_size =
        consumed_ + count >= buffer_.size() ? buffer_.size() - consumed_ : count;
    consumed_ += range_size;

    LOG_INFO("Buffer({}): Consume {} bytes from buffer, with {} bytes remaining", void_p(this),
             count, buffer_.size() - consumed_);

    if (!buffering_finished_ && buffer_.size() - consumed_ < MINIMUM_UNCONSUMED_BUFFER)
    {
        emit_minimum_available_buffer_exceeded();
        minimum_available_buffer_exceeded_ = true;
    }
    return { buffer_.data() + range_begin, range_size };
}

void Buffer::seek(music::Track::Milliseconds offset) noexcept
{
    start_caching(milliseconds_to_seconds(offset));
}
