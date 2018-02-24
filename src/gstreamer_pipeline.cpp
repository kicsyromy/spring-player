#include "gstreamer_pipeline.h"

#include <cstring>
#include <thread>

#include <gst/audio/audio.h>

#include "utility.h"

using namespace spring;
using namespace spring::player;

namespace
{
    constexpr const char MP3_CAPS_STRING[]{
        "audio/mpeg, mpegversion=(int)1, layer=(int)3"
    };
}

GStreamerPipeline::GStreamerPipeline() noexcept
  : playbin_(gst_element_factory_make("playbin", "play"))
  , bus_(gst_pipeline_get_bus(gtk_cast<GstPipeline>(playbin_)))
{
    gst_bus_add_signal_watch(bus_);

    g_object_set(gtk_cast<GObject>(playbin_), "uri", "appsrc://", nullptr);
    connect_g_signal(playbin_, "source-setup", &gst_appsrc_setup, this);

    connect_g_signal(bus_, "message::eos", &gst_playback_finished, this);
    connect_g_signal(bus_, "message::state-changed",
                     &gst_playback_state_changed, this);
    connect_g_signal(bus_, "message::error", &gst_playback_error, this);

    playback_buffer_.on_precaching_finished(
        [](void *instance) {
            auto self = static_cast<GStreamerPipeline *>(instance);
            gst_element_set_state(self->playbin_, GST_STATE_PLAYING);
        },
        this);
}

GStreamerPipeline::~GStreamerPipeline() noexcept
{
    g_warning("******************* destroying gstreamer pipeline");
    g_signal_handlers_disconnect_by_data(bus_, this);
    g_signal_handlers_disconnect_by_data(playbin_, this);
    gst_object_unref(bus_);
    gst_element_set_state(playbin_, GST_STATE_NULL);
    gst_object_unref(playbin_);
}

void GStreamerPipeline::play(const music::Track &track) noexcept
{
    current_track_ = &track;
    playback_buffer_.cache(track);
    gst_element_set_state(playbin_, GST_STATE_NULL);
}

void GStreamerPipeline::pause_resume() noexcept
{
    if (current_state_ == PlaybackState::Playing)
    {
        gst_element_set_state(playbin_, GST_STATE_PAUSED);
    }
    else if (current_state_ == PlaybackState::Paused &&
             current_track_ != nullptr)
    {
        gst_element_set_state(playbin_, GST_STATE_PLAYING);
    }
    else
    {
        g_warning(
            "GStreamerPipeline: Cannot resume playback on an empty track");
    }
}

void GStreamerPipeline::stop() noexcept
{
    gst_element_set_state(playbin_, GST_STATE_NULL);
    current_track_ = nullptr;
}

void GStreamerPipeline::seek(music::Track::Milliseconds) noexcept
{
}

const music::Track *GStreamerPipeline::current_track() const noexcept
{
    return current_track_;
}

void GStreamerPipeline::gst_playback_finished(GstBus *,
                                              GstMessage *,
                                              GStreamerPipeline *self) noexcept
{
    g_warning("GStreamerPipeline: Playback finished");
    gst_element_set_state(self->playbin_, GST_STATE_NULL);
    self->current_track_ = nullptr;
}

void GStreamerPipeline::gst_playback_state_changed(
    GstBus *, GstMessage *message, GStreamerPipeline *self) noexcept
{
    auto message_type = GST_MESSAGE_TYPE(message);

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
                    1,
                    reinterpret_cast<GSourceFunc>(
                        &GStreamerPipeline::update_playback_position),
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
            self->emit_playback_state_changed(new_state);
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
    g_warning("GStreamerPipeline: Error: %s", error_message);
    free(error_message);
}

gboolean GStreamerPipeline::update_playback_position(
    GStreamerPipeline *self) noexcept
{
    static constexpr const auto nanoseconds_to_milliseconds =
        [](gint64 nanosecs) { return Milliseconds{ nanosecs / 1000000 }; };

    gint64 position_nanoseconds;
    gst_element_query_position(self->playbin_, GST_FORMAT_TIME,
                               &position_nanoseconds);

    self->emit_playback_position_changed(
        nanoseconds_to_milliseconds(position_nanoseconds));

    return true;
}

void GStreamerPipeline::gst_appsrc_setup(GstElement *,
                                         GstElement *source,
                                         GStreamerPipeline *self) noexcept
{
    self->appsrc_ = gtk_cast<GstAppSrc>(source);

    auto caps = gst_caps_from_string(MP3_CAPS_STRING);
    gst_app_src_set_caps(self->appsrc_, caps);
    gst_caps_unref(caps);

    gst_app_src_set_stream_type(self->appsrc_, GST_APP_STREAM_TYPE_SEEKABLE);
    gst_app_src_set_size(self->appsrc_,
                         static_cast<gint64>(self->current_track_->fileSize()));
    gst_app_src_set_callbacks(self->appsrc_, &self->gst_appsrc_callbacks_, self,
                              [](auto) {});
    g_object_set(self->appsrc_, "format", GST_FORMAT_TIME, nullptr);
}

void GStreamerPipeline::gst_appsrc_need_data(GstAppSrc *src,
                                             guint length,
                                             void *instance) noexcept
{
    g_warning("need more data");
    static constexpr std::size_t BUFFER_SIZE{ 16 * 1024 };
    auto self = static_cast<GStreamerPipeline *>(instance);

    auto range = self->playback_buffer_.consume(BUFFER_SIZE);

    auto gst_buffer = gst_buffer_new_allocate(nullptr, range.size(), nullptr);
    GstMapInfo buffer_info;
    gst_buffer_map(gst_buffer, &buffer_info, GST_MAP_WRITE);

    memcpy(buffer_info.data, range.data(), range.size());

    gst_buffer_unmap(gst_buffer, &buffer_info);

    gst_app_src_push_buffer(self->appsrc_, gst_buffer);

    if (range.size() < BUFFER_SIZE)
    {
        gst_app_src_end_of_stream(self->appsrc_);
    }
}

void GStreamerPipeline::gst_appsrc_enough_data(GstAppSrc *src,
                                               void *instance) noexcept
{
    auto self = static_cast<GStreamerPipeline *>(instance);
    g_warning("enough data");
}

gboolean GStreamerPipeline::gst_appsrc_seek_data(GstAppSrc *src,
                                                 guint64 offset,
                                                 void *instance) noexcept
{
    auto self = static_cast<GStreamerPipeline *>(instance);
    g_warning("seek data");
    return true;
}
