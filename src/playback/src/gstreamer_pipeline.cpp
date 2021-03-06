#include <cstring>
#include <thread>

#include <gst/audio/audio.h>

#include <libspring_logger.h>

#include <gtk/gtk.h>

#include "playback/gstreamer_pipeline.h"
#include "playback/playlist.h"
#include "utility/async_queue.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;
using namespace spring::player::playback;

namespace
{
    constexpr const char MP3_CAPS_STRING[]{ "audio/mpeg, mpegversion=(int)1, layer=(int)3" };

    constexpr GStreamerPipeline::Milliseconds nanoseconds_to_milliseconds(gint64 nanosecs)
    {
        return GStreamerPipeline::Milliseconds{ nanosecs / 1000000 };
    };

    constexpr GStreamerPipeline::Milliseconds nanoseconds_to_milliseconds(guint64 nanosecs)
    {
        return GStreamerPipeline::Milliseconds{ nanosecs / 1000000 };
    };

    constexpr gint64 milliseconds_to_nanoseconds(GStreamerPipeline::Milliseconds value)
    {
        return value.count() * 1000000;
    };

    const std::array<const char *,
                     static_cast<std::size_t>(GStreamerPipeline::PlaybackState::Count)>
        PLAYBACK_STATE{
            "PlaybackState::Pending",
            "PlaybackState::Playing",
            "PlaybackState::Paused",
            "PlaybackState::Stopped",
        };
} // namespace

GStreamerPipeline::GStreamerPipeline(const Playlist &playback_list) noexcept
  : playback_list_(playback_list)
{
    LOG_INFO("GStreamerPipeline({}): Creating...", void_p(this));

    gst_init(nullptr, nullptr);

    playbin_ = gst_element_factory_make("playbin", "spring-player-playbin");
    bus_ = gst_pipeline_get_bus(gtk_cast<GstPipeline>(playbin_));

    gst_bus_add_signal_watch(bus_);

    g_object_set(gtk_cast<GObject>(playbin_), "uri", "appsrc://", nullptr);
    connect_g_signal(playbin_, "source-setup", &gst_appsrc_setup, this);

    connect_g_signal(bus_, "message::eos", &gst_playback_finished, this);
    connect_g_signal(bus_, "message::state-changed", &gst_playback_state_changed, this);
    connect_g_signal(bus_, "message::error", &gst_playback_error, this);

    playback_buffer_.on_minimum_available_buffer_reached(this, [](void *instance) {
        auto self = static_cast<GStreamerPipeline *>(instance);
        LOG_INFO("GStreamerPipeline({}): Minimum buffer reached", void_p(self));
        gst_element_set_state(self->playbin_, GST_STATE_PLAYING);
    });

    playback_buffer_.on_cache_updated(this, [](std::size_t new_size, void *instance) {
        auto self = static_cast<GStreamerPipeline *>(instance);
        self->emit_track_cache_updated(std::move(new_size));
    });

    playback_buffer_.on_caching_finished(this, [](void *instance) {
        auto self = static_cast<GStreamerPipeline *>(instance);
        self->emit_track_cached();
    });
}

GStreamerPipeline::~GStreamerPipeline() noexcept
{
    LOG_INFO("GStreamerPipeline({}): Destroying...", void_p(this));

    gst_element_set_state(playbin_, GST_STATE_NULL);

    playback_buffer_.disconnect_caching_finished(this);
    playback_buffer_.disconnect_cache_updated(this);
    playback_buffer_.disconnect_minimum_available_buffer_reached(this);
    playback_buffer_.disconnect_minimum_available_buffer_exceeded(this);

    gst_object_unref(bus_);
    gst_object_unref(playbin_);
}

void GStreamerPipeline::play(const std::shared_ptr<const music::Track> &track) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Playing {}", void_p(this), track->title());

    gst_element_set_state(playbin_, GST_STATE_READY);

    playback_buffer_.set_track(track);
    gst_element_set_state(playbin_, GST_STATE_PAUSED);
}

void GStreamerPipeline::pause_resume() noexcept
{
    LOG_INFO("GStreamerPipeline({}): Pause/Resume", void_p(this));

    if (current_state_ == PlaybackState::Playing)
    {
        LOG_INFO("GStreamerPipeline({}): Pausing playback", void_p(this));
        gst_element_set_state(playbin_, GST_STATE_PAUSED);
    }
    else
    {
        LOG_INFO("GStreamerPipeline({}): Resuming playback", void_p(this));
        gst_element_set_state(playbin_, GST_STATE_PLAYING);
    }
}

void GStreamerPipeline::stop() noexcept
{
    LOG_INFO("GStreamerPipeline({}): Stop", void_p(this));

    gst_element_set_state(playbin_, GST_STATE_NULL);
    current_state_ = PlaybackState::Stopped;

    emit_playback_state_changed(PlaybackState::Stopped);
    emit_playback_position_changed(Milliseconds{ 0 });
}

void GStreamerPipeline::seek(music::Track::Milliseconds count) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Seek to {}", void_p(this), count.count());
    gst_element_seek_simple(playbin_, GST_FORMAT_TIME,
                            static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                            milliseconds_to_nanoseconds(count));
}

GStreamerPipeline::PlaybackState GStreamerPipeline::playback_state() const noexcept
{
    return current_state_;
}

const char *GStreamerPipeline::playback_state_to_string(
    GStreamerPipeline::PlaybackState state) noexcept
{
    return state > PlaybackState::Invalid ? PLAYBACK_STATE[static_cast<std::size_t>(state)] :
                                            "PlaybackState::Invalid";
}

void GStreamerPipeline::gst_playback_finished(GstBus *,
                                              GstMessage *,
                                              GStreamerPipeline *self) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Internal: Playback finished", void_p(self));
    self->stop();
}

void GStreamerPipeline::gst_playback_state_changed(GstBus *,
                                                   GstMessage *message,
                                                   GStreamerPipeline *self) noexcept
{
    auto message_type = gtk_cast<GstMessage>(message)->type;

    if (message_type == GST_MESSAGE_STATE_CHANGED)
    {
        GstState gst_state;
        gst_message_parse_state_changed(message, nullptr, &gst_state, nullptr);

        if (self->gst_state_ != gst_state)
        {
            PlaybackState new_state = self->gst_state_change_handlers_[gst_state](*self);
            self->gst_state_ = gst_state;

            LOG_INFO("GStreamerPipeline({}): Internal: Playbin state "
                     "changed "
                     "to GST_STATE_{}",
                     void_p(self), gst_element_state_get_name(gst_state));

            if (self->current_state_ != new_state)
            {
                LOG_INFO("GStreamerPipeline({}): Internal: State changed to: {}", void_p(self),
                         playback_state_to_string(new_state));
                self->current_state_ = new_state;
                self->emit_playback_state_changed(std::move(new_state));
            }
        }
    }
}

GStreamerPipeline::PlaybackState GStreamerPipeline::on_gst_state_change_void_pending(
    GStreamerPipeline &) noexcept
{
    return PlaybackState::Invalid;
}

GStreamerPipeline::PlaybackState GStreamerPipeline::on_gst_state_change_null(
    GStreamerPipeline &self) noexcept
{
    on_gst_state_change_void_pending(self);

    auto new_state = PlaybackState::Stopped;
    /* When transitioning from READY to NULL it means that playback is starting  */
    /* and not that playback has actually been stopped. The workaround is not to */
    /* change the state presented to external components                         */
    if (self.gst_state_ == GST_STATE_READY)
    {
        new_state = PlaybackState::Pending;
    }

    return new_state;
}

GStreamerPipeline::PlaybackState GStreamerPipeline::on_gst_state_change_ready(
    GStreamerPipeline &self) noexcept
{
    on_gst_state_change_void_pending(self);

    auto new_state = PlaybackState::Pending;
    /* When first loading a new track GStreamer changes internally from READY to PAUSED    */
    /* several times before transitioning to PLAYING, avoid sending this rapid consecutive */
    /* state change to other components                                                    */
    if (self.gst_state_ == GST_STATE_PAUSED)
    {
        new_state = PlaybackState::Paused;
    }

    return new_state;
}

GStreamerPipeline::PlaybackState GStreamerPipeline::on_gst_state_change_paused(
    GStreamerPipeline &self) noexcept
{
    on_gst_state_change_void_pending(self);
    return PlaybackState::Paused;
}

GStreamerPipeline::PlaybackState GStreamerPipeline::on_gst_state_change_playing(
    GStreamerPipeline &self) noexcept
{
    if (!self.progress_update_source_id_)
    {
        self.progress_update_source_id_ = g_timeout_add_seconds(
            1, reinterpret_cast<GSourceFunc>(&GStreamerPipeline::update_playback_position), &self);
    }

    return PlaybackState::Playing;
}

void GStreamerPipeline::gst_playback_error(GstBus *,
                                           GstMessage *message,
                                           GStreamerPipeline *self) noexcept
{
    gst_element_set_state(self->playbin_, GST_STATE_NULL);
    gchar *error_message;
    gst_message_parse_error(message, nullptr, &error_message);
    LOG_ERROR("GStreamerPipeline({}): Internal: Playbin error: {}", void_p(self), error_message);
    g_free(error_message);
}

std::int32_t GStreamerPipeline::update_playback_position(GStreamerPipeline *self) noexcept
{
    auto result = G_SOURCE_CONTINUE;

    if (self->current_state_ != PlaybackState::Playing)
    {
        result = G_SOURCE_REMOVE;
        self->progress_update_source_id_ = 0;
    }
    else
    {
        gint64 position_nanoseconds{ 0 };
        gst_element_query_position(self->playbin_, GST_FORMAT_TIME, &position_nanoseconds);
        self->emit_playback_position_changed(nanoseconds_to_milliseconds(position_nanoseconds));
    }

    return result;
}

void GStreamerPipeline::gst_appsrc_setup(GstElement *,
                                         GstElement *source,
                                         GStreamerPipeline *self) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Internal: Configuring application source for playbin",
             void_p(self));

    self->appsrc_ = gtk_cast<GstAppSrc>(source);

    connect_g_signal(self->appsrc_, "need-data", &gst_appsrc_need_data, self);
    connect_g_signal(self->appsrc_, "enough-data", &gst_appsrc_enough_data, self);
    connect_g_signal(self->appsrc_, "seek-data", &gst_appsrc_seek_data, self);

    auto caps = gst_caps_from_string(MP3_CAPS_STRING);
    gst_app_src_set_caps(self->appsrc_, caps);
    gst_caps_unref(caps);

    gst_app_src_set_stream_type(self->appsrc_, GST_APP_STREAM_TYPE_SEEKABLE);
    g_object_set(self->appsrc_, "format", GST_FORMAT_TIME, nullptr);

    auto current_track = self->playback_list_.current_track().second;
    if (current_track != nullptr)
    {
        g_object_set(self->appsrc_, "duration",
                     milliseconds_to_nanoseconds(current_track->duration()), nullptr);
    }
    else
    {
        LOG_WARN("GStreamerPipeline({}): Internal: Starting playback on a null "
                 "track, this most likely indicates a logic error",
                 void_p(self));
    }
}

void GStreamerPipeline::gst_appsrc_need_data(GstAppSrc *, guint, GStreamerPipeline *self) noexcept
{
    if (self->push_data_source_id_ == 0)
    {
        LOG_INFO("GStreamerPipeline({}): Internal: Start pushing data to 'appsrc'", void_p(self));
        self->push_data_source_id_ =
            g_idle_add(reinterpret_cast<std::int32_t (*)(void *)>(&push_data_to_appsrc), self);
    }
}

std::int32_t GStreamerPipeline::push_data_to_appsrc(GStreamerPipeline *self) noexcept
{
    constexpr std::size_t buffer_size{ 4096 };

    auto result = G_SOURCE_CONTINUE;

    if (!self->playback_buffer_.minimum_available_buffer_exceeded())
    {
        auto range = self->playback_buffer_.consume(buffer_size);
        auto gst_buffer = gst_buffer_new_allocate(nullptr, range.size(), nullptr);
        GstMapInfo buffer_info;
        gst_buffer_map(gst_buffer, &buffer_info, GST_MAP_WRITE);

        memcpy(buffer_info.data, range.data(), range.size());

        gst_buffer_unmap(gst_buffer, &buffer_info);

        GstFlowReturn gst_result = gst_app_src_push_buffer(self->appsrc_, gst_buffer);
        if (gst_result != GST_FLOW_OK)
        {
            LOG_ERROR("GStreamerPipeline({}): Error when pushing data to 'appsrc': {}",
                      void_p(self), gst_result);
        }

        if (range.size() < buffer_size && !self->playback_buffer_.buffering())
        {
            auto current_track = self->playback_list_.current_track().second;
            if (current_track != nullptr)
            {
                gst_result = gst_app_src_end_of_stream(self->appsrc_);
                if (gst_result != GST_FLOW_OK)
                {
                    LOG_ERROR("GStreamerPipeline({}): Error while announcing end-of-stream: {}",
                              void_p(self), gst_result);
                }
                result = G_SOURCE_REMOVE;
                self->push_data_source_id_ = 0;
            }
            else
            {
                LOG_WARN("GStreamerPipeline({}): Internal: Ending playback prematurely since track "
                         "is null",
                         void_p(self));
                gst_result = gst_app_src_end_of_stream(self->appsrc_);
                if (gst_result != GST_FLOW_OK)
                {
                    LOG_ERROR("GStreamerPipeline({}): Error while announcing end-of-stream: {}",
                              void_p(self), gst_result);
                }
                result = G_SOURCE_REMOVE;
                self->push_data_source_id_ = 0;
            }
        }
    }
    else
    {
        gst_element_set_state(self->playbin_, GST_STATE_PAUSED);
    }

    return result;
}

void GStreamerPipeline::gst_appsrc_enough_data(GstAppSrc *, GStreamerPipeline *self) noexcept
{
    if (self->push_data_source_id_ != 0)
    {
        LOG_INFO("GStreamerPipeline({}): Internal: Stop pushing data to 'appsrc', buffer full",
                 void_p(self));
        g_source_remove(self->push_data_source_id_);
        self->push_data_source_id_ = 0;
    }
}

gboolean GStreamerPipeline::gst_appsrc_seek_data(GstAppSrc *,
                                                 guint64 offset,
                                                 GStreamerPipeline *self) noexcept
{
    LOG_INFO("GStreamerPipeline({}): Internal: Seek request to offset {}", void_p(self), offset);

    self->playback_buffer_.seek(nanoseconds_to_milliseconds(offset));

    return true;
}
