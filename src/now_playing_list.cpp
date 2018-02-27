#include "now_playing_list.h"

#include <memory>

#include <libspring_logger.h>

using namespace spring;
using namespace spring::player;

NowPlayingList &NowPlayingList::instance() noexcept
{
    static NowPlayingList instance;
    return instance;
}

NowPlayingList::NowPlayingList() noexcept
{
    LOG_INFO("NowPlayingList({}): Creating...", void_p(this));

    pipeline_.on_playback_state_changed(
        this,
        [](PlaybackState new_state, void *instance) {
            auto self = static_cast<NowPlayingList *>(instance);

            LOG_INFO("NowPlayingList({}): Playback state changed to {} for {}",
                     instance,
                     GStreamerPipeline::playback_state_to_string(new_state),
                     self->current_track() ? self->current_track()->title() :
                                             "");
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

    pipeline_.on_track_cached(
        this,
        [](void *instance) {
            auto self = static_cast<NowPlayingList *>(instance);
            LOG_INFO("NowPlayingList({}): Finished caching track {}", instance,
                     self->current_track() ? self->current_track()->title() :
                                             "");
            self->emit_track_cached();
        },
        this);
};
NowPlayingList::~NowPlayingList() noexcept
{
    LOG_INFO("NowPlayingList({}): Destroying...", void_p(this));

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
    LOG_INFO("NowPlayingList({}): Playing track at index {}", void_p(this),
             index);

    pipeline_.stop();
    if (index >= content_.size())
    {
        current_index_ = content_.empty() ? 0 : content_.size() - 1;
        LOG_WARN("NowPlayingList({}): Index out of bounds, attempt to restart "
                 "playback from index 0",
                 void_p(this));
    }
    else
    {
        current_index_ = index;
    }
    play_pause();
}

void NowPlayingList::play_pause() noexcept
{
    LOG_INFO("NowPlayingList({}): Play/Pause", void_p(this));

    if (!content_.empty())
    {
        if (pipeline_.current_track() == nullptr)
        {
            pipeline_.play(content_.at(current_index_));
            LOG_INFO("NowPlayingList({}): Current track is null, attempting to "
                     "play {} found at index {}",
                     void_p(this),
                     current_track() ? current_track()->title() : "",
                     current_index_);
        }
        else
        {
            pipeline_.pause_resume();
        }
    }
    else
    {
        LOG_WARN("NowPlayingList({}): Playlist is empty, nothing to play",
                 void_p(this));
    }
}

void NowPlayingList::stop() noexcept
{
    LOG_INFO("NowPlayingList({}): Stop playback", void_p(this));

    pipeline_.stop();
    current_index_ = 0;
}

void NowPlayingList::next() noexcept
{
    LOG_INFO("NowPlayingList({}): Skip forward", void_p(this));

    pipeline_.stop();
    ++current_index_;
    if (current_index_ < content_.size())
    {
        LOG_INFO("NowPlayingList({}): Next track in bounds", void_p(this));
        play_pause();
    }
    else
    {
        LOG_INFO(
            "NowPlayingList({}): Next track out of bounds, restarting from "
            "the first track",
            void_p(this));
        current_index_ = 0;
    }
}

void NowPlayingList::previous() noexcept
{
    LOG_INFO("NowPlayingList({}): Skip backward", void_p(this));

    pipeline_.stop();
    if (current_index_ > 0)
    {
        LOG_INFO("NowPlayingList({}): Previous track in bounds", void_p(this));
        --current_index_;
    }
    else
    {
        LOG_INFO(
            "NowPlayingList({}): Previous track out of bounds, starting the "
            "last track",
            void_p(this));
        current_index_ = !content_.empty() ? content_.size() - 1 : 0;
    }

    play_pause();
}

void NowPlayingList::shuffle() noexcept
{
    LOG_INFO("NowPlayingList({}): Shuffle", void_p(this));
}

void NowPlayingList::clear() noexcept
{
    LOG_INFO("NowPlayingList({}): Clear", void_p(this));
    pipeline_.stop();
    content_.clear();
}

void NowPlayingList::enqueue(const music::Track &track) noexcept
{
    LOG_INFO("NowPlayingList({}): Enqueue (non-owning) {}", void_p(this),
             track.title());
}

const music::Track &NowPlayingList::enqueue(music::Track &&track) noexcept
{
    LOG_INFO("NowPlayingList({}): Enqueue (owning) {}", void_p(this),
             track.title());

    content_.push_back(std::move(track));
    const auto &t = content_.back();

    emit_track_queued(t);

    return t;
}
