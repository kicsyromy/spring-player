#include "gstreamer_pipeline.h"

#include "utility.h"

using namespace spring;
using namespace spring::player;

GStreamerPipeline::GStreamerPipeline() noexcept
  : playbin_(gst_element_factory_make("playbin", "play"))
  , bus_(gst_pipeline_get_bus(gtk_cast<GstPipeline>(playbin_)))
{
    gst_bus_add_signal_watch(bus_);

    connect_g_signal(bus_, "message::eos", &gst_playback_finished, this);
    connect_g_signal(bus_, "message::state-changed",
                     &gst_playback_state_changed, this);
    connect_g_signal(bus_, "message::error", &gst_playback_error, this);
}

GStreamerPipeline::~GStreamerPipeline() noexcept
{
    gst_object_unref(bus_);
    gst_element_set_state(playbin_, GST_STATE_NULL);
    gst_object_unref(playbin_);
}

void GStreamerPipeline::play(const music::Track &track) noexcept
{
    gst_element_set_state(playbin_, GST_STATE_NULL);
    g_object_set(gtk_cast<GObject>(playbin_), "uri", track.url().c_str(),
                 nullptr);
    gst_element_set_state(playbin_, GST_STATE_PLAYING);
    current_track_ = &track;
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
    g_error("GStreamerPipeline: Error: %s", error_message);
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
