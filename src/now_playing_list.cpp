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
  : playbin_(gst_element_factory_make("playbin", "play"))
  , bus_(gst_pipeline_get_bus(reinterpret_cast<GstPipeline *>(playbin_)))
{
    gst_bus_add_signal_watch(bus_);

    g_signal_connect(bus_, "message::eos", G_CALLBACK(&on_playback_finished),
                     this);
    g_signal_connect(bus_, "message::state-changed",
                     G_CALLBACK(&on_playback_state_changed), this);
    g_signal_connect(bus_, "message::error", G_CALLBACK(&on_playback_error),
                     this);
}

NowPlayingList::~NowPlayingList()
{
    gst_object_unref(bus_);
    gst_element_set_state(playbin_, GST_STATE_NULL);
    gst_object_unref(playbin_);
}

const music::Track *NowPlayingList::current_track() const noexcept
{
    return current_track_;
}

void NowPlayingList::play() noexcept
{
    if (!content_.empty())
    {
        if (current_track_ == nullptr)
        {
            current_track_ = &content_.front();
            gst_element_set_state(playbin_, GST_STATE_NULL);
            g_object_set(G_OBJECT(playbin_), "uri",
                         current_track_->url().c_str(), nullptr);
            gst_element_set_state(playbin_, GST_STATE_PLAYING);
        }
        else
        {
            gst_element_set_state(playbin_, GST_STATE_PLAYING);
        }
    }
}

void NowPlayingList::pause() noexcept
{
    gst_element_set_state(playbin_, GST_STATE_PAUSED);
}

void NowPlayingList::stop() noexcept
{
    g_source_remove(position_update_callback_tag_);
    current_track_ = nullptr;
    gst_element_set_state(playbin_, GST_STATE_NULL);
}

void NowPlayingList::shuffle() noexcept
{
}

void NowPlayingList::clear() noexcept
{
    g_source_remove(position_update_callback_tag_);
    current_track_ = nullptr;
    content_.clear();
    gst_element_set_state(playbin_, GST_STATE_NULL);
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

void NowPlayingList::on_playback_finished(GstBus *,
                                          GstMessage *message,
                                          NowPlayingList *self) noexcept
{
    g_warning("NowPlayingList: Playback finished");
    gst_element_set_state(self->playbin_, GST_STATE_NULL);
}

void NowPlayingList::on_playback_state_changed(GstBus *,
                                               GstMessage *message,
                                               NowPlayingList *self) noexcept
{
    auto message_type = GST_MESSAGE_TYPE(message);
    g_warning("NowPlayingList: %s", gst_message_type_get_name(message_type));

    if (message_type == GST_MESSAGE_STATE_CHANGED)
    {
        GstState gst_state;
        gst_message_parse_state_changed(message, nullptr, &gst_state, nullptr);

        PlaybackState new_state;
        switch (gst_state)
        {
            default:
            case GST_STATE_VOID_PENDING:
            case GST_STATE_READY:
                new_state = PlaybackState::Invalid;
                break;

            case GST_STATE_NULL:
                new_state = PlaybackState::Stopped;
                break;

            case GST_STATE_PLAYING:
                new_state = PlaybackState::Playing;
                self->position_update_callback_tag_ = g_timeout_add_seconds(
                    1, (GSourceFunc)&NowPlayingList::update_playback_position,
                    self);
                break;

            case GST_STATE_PAUSED:
                new_state = PlaybackState::Paused;
                break;
        }

        if (new_state != PlaybackState::Invalid &&
            new_state != self->current_state_)
        {
            self->current_state_ = new_state;
            for (const auto &callback : self->state_changed_callbacks_)
            {
                callback(new_state);
            }
        }
    }
}

void NowPlayingList::on_playback_error(GstBus *,
                                       GstMessage *message,
                                       NowPlayingList *self) noexcept
{
    gst_element_set_state(self->playbin_, GST_STATE_NULL);
    gchar *error_message;
    gst_message_parse_error(message, nullptr, &error_message);
    g_warning("NowPlayingList: Error: %s", error_message);
    free(error_message);
}

gboolean NowPlayingList::update_playback_position(NowPlayingList *self) noexcept
{
    static constexpr const auto nanoseconds_to_seconds =
        [](gint64 nanosecs) -> std::size_t { return (nanosecs / 1000000000); };

    gint64 position_nanoseconds;
    gst_element_query_position(self->playbin_, GST_FORMAT_TIME,
                               &position_nanoseconds);

    for (const auto &callback : self->playback_position_changed_callbacks_)
    {
        callback(nanoseconds_to_seconds(position_nanoseconds));
    }

    return true;
}
