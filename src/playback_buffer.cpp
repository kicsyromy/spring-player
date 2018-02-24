#include "playback_buffer.h"

using namespace spring;
using namespace spring::player;

namespace
{
    constexpr const std::size_t PREBUFFER_SIZE_KB{ 100 * 1024 };
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
        {
            std::lock_guard<std::mutex> lock(mutex_);
            buffering_done_ = false;
            keep_buffering_ = true;
        }

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

                bool keep_buffering{ true };
                {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    keep_buffering = self->keep_buffering_;
                }
                if (keep_buffering)
                {
                    std::unique_lock<std::mutex> lock(self->mutex_);
                    self->buffer_.append(reinterpret_cast<const char *>(data),
                                         size);
                    self->condition_variable_.notify_one();

                    if (!prebuffer_filled &&
                        self->buffer_.size() >= PREBUFFER_SIZE_KB)
                    {
                        self->emit_queued_prebuffer_filled();
                    }

                    result = size;
                }

                return result;
            },
            &worker_data);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            buffering_done_ = true;
            keep_buffering_ = false;
        }

        emit_queued_buffering_finished();
    } };
}

void PlaybackBuffer::Producer::stop_buffering() noexcept
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        keep_buffering_ = false;
    }

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
    std::lock_guard<std::mutex> lock(mutex_);
    return buffering_done_;
}

std::size_t PlaybackBuffer::Producer::capacity() const noexcept
{
    return buffer_.capacity();
}

PlaybackBuffer::PlaybackBuffer() noexcept
{
    buffer_producer_.on_prebuffer_filled(
        [](void *instance) {
            auto self = static_cast<PlaybackBuffer *>(instance);
            self->emit_precaching_finished();
        },
        this);

    buffer_producer_.on_buffering_finished(
        [](void *instance) {
            auto self = static_cast<PlaybackBuffer *>(instance);
            self->emit_caching_finished();
        },
        this);
}

PlaybackBuffer::~PlaybackBuffer() noexcept = default;

void PlaybackBuffer::cache(const music::Track &track) noexcept
{
    consumed_ = 0;
    buffer_producer_.start_buffering(track);
}

const std::string_view PlaybackBuffer::consume(std::size_t count) noexcept
{
    const auto range_begin = consumed_;
    const auto range_end_index = consumed_ + count;
    const auto file_size = buffer_producer_.capacity();
    const auto range_size =
        range_end_index >= file_size ? file_size - consumed_ : count;

    consumed_ += count;

    return buffer_producer_.buffer_range(range_begin, range_size);
}
