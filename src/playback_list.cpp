#include <gtk/gtk.h>

#include <memory>
#include <random>

#include <libspring_logger.h>

#include "playback_list.h"

#include "utility/global.h"

using namespace spring;
using namespace spring::player;

PlaybackList::PlaybackList() noexcept
{
    LOG_INFO("PlaybackList({}): Creating...", void_p(this));

    void (*playback_state_changed_handler)(PlaybackState, PlaybackList *) noexcept =
        &on_playback_state_changed;
    pipeline_.on_playback_state_changed(this, playback_state_changed_handler);

    pipeline_.on_playback_position_changed(this,
                                           [](Milliseconds milliseconds, void *instance) {
                                               auto self = static_cast<PlaybackList *>(instance);
                                               self->emit_playback_position_changed(
                                                   milliseconds.count());
                                           },
                                           this);

    pipeline_.on_track_cache_updated(this,
                                     [](std::size_t new_size, void *instance) {
                                         auto self = static_cast<PlaybackList *>(instance);
                                         self->emit_track_cache_updated(std::move(new_size));
                                     },
                                     this);

    pipeline_.on_track_cached(
        this,
        [](void *instance) {
            auto self = static_cast<PlaybackList *>(instance);
            LOG_INFO("PlaybackList({}): Finished caching track {}", instance,
                     self->current_track().second ? self->current_track().second->title() : "");
            self->emit_track_cached();
        },
        this);
};

PlaybackList::~PlaybackList() noexcept
{
    LOG_INFO("PlaybackList({}): Destroying...", void_p(this));

    pipeline_.disconnect_track_cached(this);
    pipeline_.disconnect_track_cache_updated(this);
    pipeline_.disconnect_playback_position_changed(this);
    pipeline_.disconnect_playback_state_changed(this);
}

std::pair<int32_t, const music::Track *> PlaybackList::current_track() const noexcept
{
    music::Track *result{ nullptr };
    if (current_index_ >= 0)
    {
        result = content_.at(static_cast<std::size_t>(current_index_)).get();
    }

    return { current_index_, result };
}

std::size_t PlaybackList::track_count() const noexcept
{
    return content_.size();
}

PlaybackList::PlaybackState PlaybackList::playback_state() const noexcept
{
    return pipeline_.playback_state();
}

void PlaybackList::set_repeat_all_active(bool value) noexcept
{
    if (repeat_all_active_ != value)
    {
        LOG_INFO("PlaybackList({}): Repeat all {}", void_p(this), value ? "ON" : "OFF");
        repeat_all_active_ = value;
    }
}

void PlaybackList::set_repeat_one_active(bool value) noexcept
{
    if (repeat_one_active_ != value)
    {
        LOG_INFO("PlaybackList({}): Repeat one {}", void_p(this), value ? "ON" : "OFF");
        repeat_one_active_ = value;
    }
}

void PlaybackList::set_shuffle_active(bool value) noexcept
{
    if (shuffle_active_ != value)
    {
        LOG_INFO("PlaybackList({}): Suffle {}", void_p(this), value ? "ON" : "OFF");
        shuffle_active_ = value;
    }
}

void PlaybackList::play(std::size_t index) noexcept
{
    LOG_INFO("PlaybackList({}): Playing track at index {}", void_p(this), index);

    if (index < content_.size())
    {
        current_index_ = static_cast<std::int32_t>(index);
        pipeline_.play(content_.at(index));
    }
    else
    {
        LOG_WARN("PlaybackList({}): Index {} is out of playlist bounds. "
                 "Current playlist size is {}",
                 void_p(this), index, content_.size());
    }
}

void PlaybackList::play_pause() noexcept
{
    LOG_INFO("PlaybackList({}): Play/Pause", void_p(this));

    if (current_index_ == -1)
    {
        play(0);
    }
    else
    {
        pipeline_.pause_resume();
    }
}

void PlaybackList::seek_current_track(PlaybackList::Milliseconds value) noexcept
{
    LOG_INFO("PlaybackList({}): Seek to {}", void_p(this), value.count());
    pipeline_.seek(value);
}

void PlaybackList::stop() noexcept
{
    LOG_INFO("PlaybackList({}): Stop playback", void_p(this));

    pipeline_.stop();
    current_index_ = -1;
}

void PlaybackList::next() noexcept
{
    LOG_INFO("PlaybackList({}): Skip forward", void_p(this));

    std::size_t new_index = current_index_ > -1 ? static_cast<std::size_t>(current_index_ + 1) :
                                                  content_.size() > 0 ? content_.size() - 1 : 0;
    play(new_index);
}

void PlaybackList::previous() noexcept
{
    LOG_INFO("PlaybackList({}): Skip backward", void_p(this));

    std::size_t new_index = current_index_ > 0 ? static_cast<std::size_t>(current_index_ - 1) : 0;
    play(new_index);
}

void PlaybackList::clear() noexcept
{
    LOG_INFO("PlaybackList({}): Clear", void_p(this));
    pipeline_.stop();
    content_.clear();

    emit_list_cleared();
}

void PlaybackList::enqueue(std::shared_ptr<music::Track> track) noexcept
{
    LOG_INFO("PlaybackList({}): Enqueue {}", void_p(this), track->title());

    content_.push_back(track);
    auto &t = content_.back();

    emit_track_queued(t);
}

void PlaybackList::on_playback_state_changed(PlaybackState new_state, PlaybackList *self) noexcept
{
    LOG_INFO("PlaybackList({}): Playback state changed to {} for {}", void_p(self),
             GStreamerPipeline::playback_state_to_string(new_state),
             self->current_track().second ? self->current_track().second->title() : "");

    if (new_state == PlaybackState::Stopped)
    {
        if (self->current_index_ > -1)
        {
            if (self->repeat_one_active_)
            {
                self->play(static_cast<std::size_t>(self->current_index_));
            }
            else if (self->shuffle_active_)
            {
                std::random_device entropy{};
                std::mt19937 generator{ entropy() };
                std::uniform_int_distribution<std::size_t> distribution{ 0, self->content_.size() -
                                                                                1 };
                self->play(distribution(generator));
            }
            else if (self->current_index_ < static_cast<std::int32_t>(self->content_.size()) - 1)
            {
                self->next();
            }
            else
            {
                self->stop();
            }
        }
    }

    self->emit_playback_state_changed(std::move(new_state));
}
