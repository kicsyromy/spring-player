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

void NowPlayingList::play() noexcept
{
    if (!content_.empty())
    {
        if (pipeline_.current_track() == nullptr)
        {
            pipeline_.play(content_.front());
        }
        else
        {
            pipeline_.pause_resume();
        }
    }
}

void NowPlayingList::pause() noexcept
{
    pipeline_.pause_resume();
}

void NowPlayingList::stop() noexcept
{
    pipeline_.stop();
}

void NowPlayingList::shuffle() noexcept
{
}

void NowPlayingList::clear() noexcept
{
    pipeline_.stop();
    content_.clear();
}

void NowPlayingList::restart_current_track() noexcept
{
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
