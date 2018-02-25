#include "now_playing_list.h"

#include <memory>

using namespace spring;
using namespace spring::player;

NowPlayingList &NowPlayingList::instance() noexcept
{
    static NowPlayingList instance;
    return instance;
}

NowPlayingList::NowPlayingList() noexcept
{
    pipeline_.on_playback_state_changed(
        this,
        [](PlaybackState new_state, void *instance) {
            auto self = static_cast<NowPlayingList *>(instance);

            g_warning("new state %d", new_state);
            if (new_state == PlaybackState::Stopped)
            {
                if (!self->content_.empty() &&
                    self->current_index_ < self->content_.size() - 1)
                {
                    ++self->current_index_;
                    self->play_pause();
                }
            }

            self->emit_playback_state_changed(std::move(new_state));
        },
        this);

    pipeline_.on_playback_position_changed(
        this,
        [](Milliseconds milliseconds, void *instance) {
            auto self = static_cast<NowPlayingList *>(instance);
            self->emit_playback_position_changed(milliseconds.count());
        },
        this);

    pipeline_.on_track_cache_updated(
        this,
        [](std::size_t new_size, void *instance) {
            auto self = static_cast<NowPlayingList *>(instance);
            self->emit_track_cache_updated(std::move(new_size));
        },
        this);

    pipeline_.on_track_cached(this,
                              [](void *instance) {
                                  auto self =
                                      static_cast<NowPlayingList *>(instance);
                                  self->emit_track_cached();
                              },
                              this);
};
NowPlayingList::~NowPlayingList() noexcept
{
    pipeline_.disconnect_track_cached(this);
    pipeline_.disconnect_track_cache_updated(this);
    pipeline_.disconnect_playback_position_changed(this);
    pipeline_.disconnect_playback_state_changed(this);
}

const music::Track *NowPlayingList::current_track() const noexcept
{
    return pipeline_.current_track();
}

std::size_t NowPlayingList::track_count() const noexcept
{
    return content_.size();
}

void NowPlayingList::play_from(std::size_t index) noexcept
{
    pipeline_.stop();
    if (index >= content_.size())
    {
        current_index_ = content_.empty() ? 0 : content_.size() - 1;
    }
    else
    {
        current_index_ = index;
    }
    play_pause();
}

void NowPlayingList::play_pause() noexcept
{
    if (!content_.empty())
    {
        if (pipeline_.current_track() == nullptr)
        {
            pipeline_.play(content_.at(current_index_));
        }
        else
        {
            pipeline_.pause_resume();
        }
    }
}

void NowPlayingList::stop() noexcept
{
    pipeline_.stop();
    current_index_ = 0;
}

void NowPlayingList::next() noexcept
{
    pipeline_.stop();
    ++current_index_;
    if (current_index_ < content_.size())
    {
        play_pause();
    }
    else
    {
        current_index_ = 0;
    }
}

void NowPlayingList::previous() noexcept
{
    pipeline_.stop();
    if (current_index_ > 0)
    {
        --current_index_;
    }

    play_pause();
}

void NowPlayingList::shuffle() noexcept
{
}

void NowPlayingList::clear() noexcept
{
    pipeline_.stop();
    content_.clear();
}

void NowPlayingList::enqueue(const music::Track &track) noexcept
{
}

const music::Track &NowPlayingList::enqueue(music::Track &&track) noexcept
{
    content_.push_back(std::move(track));
    const auto &t = content_.back();

    emit_track_queued(t);

    return t;
}
