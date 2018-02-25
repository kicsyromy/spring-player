#include "playback_buffer.h"

using namespace spring;
using namespace spring::player;

namespace
{
    constexpr const std::size_t PREBUFFER_SIZE_KB{ 512 * 1024 };
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
    buffer_.reserve(track.fileSize());

    thread_ = std::thread{ [this, &track] {
        buffering_done_ = false;
        keep_buffering_ = true;

        struct WorkerData
        {
            PlaybackBuffer::Producer *instance;
            bool prebuffer_filled;
        } worker_data{ this, false };

        track.trackData(
            [](std::uint8_t *data, std::size_t size, void *wdata) {
                auto worker_data = static_cast<WorkerData *>(wdata);
                auto self = worker_data->instance;
                auto &prebuffer_filled = worker_data->prebuffer_filled;

                std::size_t result{ 0 };

                if (self->keep_buffering_)
                {
                    std::unique_lock<std::mutex> lock(self->mutex_);
                    self->buffer_.append(reinterpret_cast<const char *>(data),
                                         size);
                    auto buffer_size = self->buffer_.size();
                    self->condition_variable_.notify_one();

                    if (!prebuffer_filled &&
                        self->buffer_.size() >= PREBUFFER_SIZE_KB)
                    {
                        self->emit_queued_prebuffer_filled();
                    }

                    result = size;

                    self->emit_queued_buffer_updated(std::move(buffer_size));
                    //                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }

                g_warning("returning size = %d", result);

                return result;
            },
            &worker_data);

        buffering_done_ = true;

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

    buffer_.clear();
}

std::string_view PlaybackBuffer::Producer::buffer_range(std::size_t index,
                                                        std::size_t count) const
    noexcept
{
    std::unique_lock<std::mutex> lock(mutex_);
    condition_variable_.wait(lock, [this, index, count] {
        return (index + count <= buffer_.size());
    });
    return { buffer_.data() + index, count };
}

bool PlaybackBuffer::Producer::buffering_finished() const noexcept
{
    return buffering_done_;
}

std::string &PlaybackBuffer::Producer::take() noexcept
{
    return buffer_;
}

PlaybackBuffer::PlaybackBuffer() noexcept
{
    buffer_producer_.on_prebuffer_filled(
        this,
        [](void *instance) {
            auto self = static_cast<PlaybackBuffer *>(instance);
            self->emit_precaching_finished();
        },
        this);

    buffer_producer_.on_buffering_finished(
        this,
        [](void *instance) {
            auto self = static_cast<PlaybackBuffer *>(instance);

            self->completed_buffer_ = std::move(self->buffer_producer_.take());

            self->emit_caching_finished();
        },
        this);

    buffer_producer_.on_buffer_updated(
        this,
        [](std::size_t new_size, void *instance) {
            auto self = static_cast<PlaybackBuffer *>(instance);

            self->emit_cache_updated(std::move(new_size));
        },
        this);
}

PlaybackBuffer::~PlaybackBuffer() noexcept
{
    buffer_producer_.disconnect_buffer_updated(this);
    buffer_producer_.disconnect_buffering_finished(this);
    buffer_producer_.disconnect_prebuffer_filled(this);
}

void PlaybackBuffer::cache(const music::Track &track) noexcept
{
    consumed_ = 0;
    file_size_ = track.fileSize();
    completed_buffer_.clear();
    buffer_producer_.start_buffering(track);
}

const std::string_view PlaybackBuffer::consume(std::size_t count) noexcept
{
    if (completed_buffer_.empty())
    {
        return buffer_producer_.buffer_range(consumed_, count);
    }
    else
    {
        const auto range_begin = consumed_;
        const auto range_size = consumed_ + count >= completed_buffer_.size() ?
                                    completed_buffer_.size() - consumed_ :
                                    count;

        consumed_ += range_size;
        return { completed_buffer_.data() + range_begin, range_size };
    }
}
