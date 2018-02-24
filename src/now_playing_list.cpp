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
        [](PlaybackState new_state, void *instance) {
            auto self = static_cast<NowPlayingList *>(instance);
            for (const auto &callback : self->state_changed_callbacks_)
            {
                callback(new_state);
            }
        },
        this);

    pipeline_.on_playback_position_changed(
        [](Milliseconds milliseconds, void *instance) {
            auto self = static_cast<NowPlayingList *>(instance);
            for (const auto &callback :
                 self->playback_position_changed_callbacks_)
            {
                callback(milliseconds.count());
            }
        },
        this);
};
NowPlayingList::~NowPlayingList() noexcept = default;

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

    for (const auto &callback : track_queued_callbacks_)
    {
        callback(t);
    }

    return t;
}
