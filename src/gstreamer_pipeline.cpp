#include "gstreamer_pipeline.h"

#include <cstring>
#include <thread>

#include <gst/audio/audio.h>

#include <libspring_logger.h>

#include "async_queue.h"
#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

namespace
{
    constexpr const char MP3_CAPS_STRING[]{
        "audio/mpeg, mpegversion=(int)1, layer=(int)3"
    };

    constexpr GStreamerPipeline::Milliseconds nanoseconds_to_milliseconds(
        gint64 nanosecs)
    {
        return GStreamerPipeline::Milliseconds{ nanosecs / 1000000 };
    };

    const std::array<const char *,
                     static_cast<std::size_t>(
                         GStreamerPipeline::PlaybackState::Count)>
        PLAYBACK_STATE{
            "PlaybackState::Pending",
            "PlaybackState::Playing",
            "PlaybackState::Paused",
            "PlaybackState::Stopped",
        };
}

GStreamerPipeline::GStreamerPipeline() noexcept
  : playbin_(gst_element_factory_make("playbin", "play"))
  , bus_(gst_pipeline_get_bus(gtk_cast<GstPipeline>(playbin_)))
{
    LOG_INFO("GStreamerPipeline({}): Creating...", void_p(this));

    gst_bus_add_signal_watch(bus_);

    g_object_set(gtk_cast<GObject>(playbin_), "uri", "appsrc://", nullptr);
    connect_g_signal(playbin_, "source-setup", &gst_appsrc_setup, this);

    connect_g_signal(bus_, "message::eos", &gst_playback_finished, this);
    connect_g_signal(bus_, "message::state-changed",
                     &gst_playback_state_changed, this);
    connect_g_signal(bus_, "message::error", &gst_playback_error, this);

    playback_buffer_.on_minimum_available_buffer_exceeded(
        this,
        [](void *instance) {
            auto self = static_cast<GStreamerPipeline *>(instance);
            gst_element_set_state(self->playbin_, GST_STATE_PAUSED);
        },
        this);

    playback_buffer_.on_minimum_available_buffer_reached(
        this,
        [](void *instance) {
            auto self = static_cast<GStreamerPipeline *>(instance);
            gst_element_set_state(self->playbin_, GST_STATE_PLAYING);
        },
        this);

    playback_buffer_.on_cache_updated(
        this,
        [](std::size_t new_size, void *instance) {
            auto self = static_cast<GStreamerPipeline *>(instance);
            self->emit_track_cache_updated(std::move(new_size));
        },
        this);

    playback_buffer_.on_caching_finished(
        this,
        [](void *instance) {
            auto self = static_cast<GStreamerPipeline *>(instance);
            self->emit_track_cached();
        },
        this);
}

GStreamerPipeline::~GStreamerPipeline() noexcept
{
    LOG_INFO("GStreamerPipeline({}): Destroying...", void_p(this));

    playback_buffer_.disconnect_caching_finished(this);
    playback_buffer_.disconnect_cache_updated(this);
    playback_buffer_.disconnect_minimum_available_buffer_reached(this);
    playback_buffer_.disconnect_minimum_available_buffer_exceeded(this);

    gst_object_unref(bus_);
    gst_element_set_state(playbin_, GST_STATE_NULL);
    gst_object_unref(playbin_);
}

void GStreamerPipeline::play(const music::Track &track) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Playing {}", void_p(this), track.title());

    stop();
    current_track_ = &track;
    current_state_ = PlaybackState::Pending;
    emit_playback_state_changed(PlaybackState::Pending);

    playback_buffer_.cache(track);
    gst_element_set_state(playbin_, GST_STATE_NULL);
}

void GStreamerPipeline::pause_resume() noexcept
{
    LOG_INFO("GStreamerPipeline({}): Pause/Resume {}", void_p(this),
             (current_track_ ? current_track_->title() : ""));

    if (current_state_ == PlaybackState::Playing)
    {
        LOG_INFO("GStreamerPipeline({}): Pausing playback", void_p(this));
        gst_element_set_state(playbin_, GST_STATE_PAUSED);
    }
    else if (current_track_ != nullptr)
    {
        LOG_INFO("GStreamerPipeline({}): Resuming playback", void_p(this));
        gst_element_set_state(playbin_, GST_STATE_PLAYING);
    }
    else
    {
        LOG_WARN("GStreamerPipeline({}): Current track is null", void_p(this));
    }
}

void GStreamerPipeline::stop() noexcept
{
    LOG_INFO("GStreamerPipeline({}): Stop {}", void_p(this),
             (current_track_ ? current_track_->title() : ""));

    if (position_update_callback_tag_ > 0)
    {
        g_source_remove(position_update_callback_tag_);
    }

    emit_playback_position_changed(Milliseconds{ 0 });

    gst_element_set_state(playbin_, GST_STATE_NULL);
    current_track_ = nullptr;
}

void GStreamerPipeline::seek(music::Track::Milliseconds count) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Seek to {}", void_p(this), count.count());
}

const music::Track *GStreamerPipeline::current_track() const noexcept
{
    return current_track_;
}

const char *GStreamerPipeline::playback_state_to_string(
    GStreamerPipeline::PlaybackState state) noexcept
{
    return state > PlaybackState::Invalid ?
               PLAYBACK_STATE[static_cast<std::size_t>(state)] :
               "PlaybackState::Invalid";
}

void GStreamerPipeline::gst_playback_finished(GstBus *,
                                              GstMessage *,
                                              GStreamerPipeline *self) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Internal: Playback finished {}",
             void_p(self),
             self->current_track_ ? self->current_track_->title() : "");

    gst_element_set_state(self->playbin_, GST_STATE_READY);
    self->current_track_ = nullptr;
}

void GStreamerPipeline::gst_playback_state_changed(
    GstBus *, GstMessage *message, GStreamerPipeline *self) noexcept
{
    auto message_type = gtk_cast<GstMessage>(message)->type;

    LOG_INFO("GStreamerPipeline({}): Internal: Playback state changed",
             void_p(self));

    if (message_type == GST_MESSAGE_STATE_CHANGED)
    {
        GstState gst_state;
        gst_message_parse_state_changed(message, nullptr, &gst_state, nullptr);

        PlaybackState new_state;
        switch (gst_state)
        {
            case GST_STATE_VOID_PENDING:
            {
                new_state = PlaybackState::Invalid;
                LOG_INFO(
                    "GStreamerPipeline({}): Internal: Playbin state changed "
                    "to GST_STATE_VOID_PENDING for {}",
                    void_p(self),
                    self->current_track_ ? self->current_track_->title() : "");
                break;
            }

            case GST_STATE_READY:
            {
                new_state = PlaybackState::Stopped;
                LOG_INFO(
                    "GStreamerPipeline({}): Internal: Playbin state changed "
                    "to GST_STATE_READY for {}",
                    void_p(self),
                    self->current_track_ ? self->current_track_->title() : "");
                break;
            }

            case GST_STATE_NULL:
            {
                new_state = PlaybackState::Stopped;
                LOG_INFO(
                    "GStreamerPipeline({}): Internal: Playbin state changed "
                    "to GST_STATE_NULL for {}",
                    void_p(self),
                    self->current_track_ ? self->current_track_->title() : "");
                break;
            }

            case GST_STATE_PLAYING:
            {
                new_state = PlaybackState::Playing;
                self->position_update_callback_tag_ = g_timeout_add_seconds(
                    1,
                    reinterpret_cast<GSourceFunc>(
                        &GStreamerPipeline::update_playback_position),
                    self);
                LOG_INFO(
                    "GStreamerPipeline({}): Internal: Playbin state changed "
                    "to GST_STATE_PLAYING for {}",
                    void_p(self),
                    self->current_track_ ? self->current_track_->title() : "");
                break;
            }

            case GST_STATE_PAUSED:
            {
                new_state = PlaybackState::Paused;
                LOG_INFO(
                    "GStreamerPipeline({}): Internal: Playbin state changed "
                    "to GST_STATE_PAUSED for {}",
                    void_p(self),
                    self->current_track_ ? self->current_track_->title() : "");
                break;
            }
        }

        if (new_state != PlaybackState::Invalid &&
            new_state != self->current_state_)
        {
            LOG_INFO("GStreamerPipeline({}): Internal: State changed to: {}",
                     void_p(self), playback_state_to_string(new_state));
            self->current_state_ = new_state;
            self->emit_playback_state_changed(std::move(new_state));
        }
    }
}

void GStreamerPipeline::gst_playback_error(GstBus *,
                                           GstMessage *message,
                                           GStreamerPipeline *self) noexcept
{
    gst_element_set_state(self->playbin_, GST_STATE_NULL);
    gchar *error_message;
    gst_message_parse_error(message, nullptr, &error_message);
    LOG_ERROR("GStreamerPipeline({}): Internal: Playbin error: {}",
              void_p(self), error_message);
    g_free(error_message);
}

gboolean GStreamerPipeline::update_playback_position(
    GStreamerPipeline *self) noexcept
{
    gint64 position_nanoseconds;
    gst_element_query_position(self->playbin_, GST_FORMAT_TIME,
                               &position_nanoseconds);

    self->emit_playback_position_changed(
        nanoseconds_to_milliseconds(position_nanoseconds));

    // TODO: Return false if playback state is stopped
    return true;
}

void GStreamerPipeline::gst_appsrc_setup(GstElement *,
                                         GstElement *source,
                                         GStreamerPipeline *self) noexcept
{
    LOG_INFO(
        "GStreamerPipeline({}): Internal: Configuring application source for "
        "playbin",
        void_p(self));

    self->appsrc_ = gtk_cast<GstAppSrc>(source);

    auto caps = gst_caps_from_string(MP3_CAPS_STRING);
    gst_app_src_set_caps(self->appsrc_, caps);
    gst_caps_unref(caps);

    gst_app_src_set_stream_type(self->appsrc_, GST_APP_STREAM_TYPE_SEEKABLE);
    gst_app_src_set_size(self->appsrc_,
                         static_cast<gint64>(self->current_track_->fileSize()));
    gst_app_src_set_callbacks(self->appsrc_, &self->gst_appsrc_callbacks_, self,
                              [](void *) {});

    g_object_set(self->appsrc_, "format", GST_FORMAT_TIME, nullptr);
}

void GStreamerPipeline::gst_appsrc_need_data(GstAppSrc *,
                                             guint length,
                                             void *instance) noexcept
{
    const std::size_t buffer_size{ length };
    auto self = static_cast<GStreamerPipeline *>(instance);

    auto range = self->playback_buffer_.consume(buffer_size);

    auto gst_buffer = gst_buffer_new_allocate(nullptr, range.size(), nullptr);
    GstMapInfo buffer_info;
    gst_buffer_map(gst_buffer, &buffer_info, GST_MAP_WRITE);

    memcpy(buffer_info.data, range.data(), range.size());

    gst_buffer_unmap(gst_buffer, &buffer_info);

    gst_app_src_push_buffer(self->appsrc_, gst_buffer);

    if (range.size() < buffer_size)
    {
        constexpr const Milliseconds MAXIMUM_DELTA{ 300 };
        gint64 position_nanoseconds;
        gst_element_query_position(self->playbin_, GST_FORMAT_TIME,
                                   &position_nanoseconds);

        if (nanoseconds_to_milliseconds(position_nanoseconds) + MAXIMUM_DELTA >=
            self->current_track_->duration())
        {
            gst_app_src_end_of_stream(self->appsrc_);
        }
    }
}

void GStreamerPipeline::gst_appsrc_enough_data(GstAppSrc *,
                                               void *instance) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Internal: Playbin buffer full", instance);

    auto self = static_cast<GStreamerPipeline *>(instance);
    static_cast<void>(self);
}

gboolean GStreamerPipeline::gst_appsrc_seek_data(GstAppSrc *,
                                                 guint64 offset,
                                                 void *instance) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Internal: Seek request to offset {}",
             instance, offset);

    auto self = static_cast<GStreamerPipeline *>(instance);
    static_cast<void>(self);

    return true;
}
