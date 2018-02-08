#ifndef SPRING_PLAYER_NOW_PLAYING_LIST_H
#define SPRING_PLAYER_NOW_PLAYING_LIST_H

#include <functional>
#include <vector>

#include <gst/gst.h>

#include <libspring_music_track.h>

namespace spring
{
    namespace player
    {
        class NowPlayingList
        {
        public:
            static NowPlayingList &instance() noexcept;

        public:
            enum class PlaybackState
            {
                Invalid = -1,
                Playing,
                Paused,
                Stopped
            };

        private:
            NowPlayingList() noexcept;
            ~NowPlayingList() noexcept;

        public:
            const music::Track *current_track() const noexcept;

            template <typename Callback>
            inline void on_state_changed(Callback &&callback) noexcept
            {
                state_changed_callbacks_.emplace_back(
                    std::forward<Callback>(callback));
            }

            template <typename Callback>
            inline void on_track_queued(Callback &&callback) noexcept
            {
                track_queued_callbacks_.emplace_back(
                    std::forward<Callback>(callback));
            }

            template <typename Callback>
            inline void on_playback_position_changed(
                Callback &&callback) noexcept
            {
                playback_position_changed_callbacks_.emplace_back(
                    std::forward<Callback>(callback));
            }

        public:
            void play() noexcept;
            void pause() noexcept;
            void stop() noexcept;
            void shuffle() noexcept;
            void clear() noexcept;
            void enqueue(const music::Track &track) noexcept;
            const music::Track &enqueue(music::Track &&track) noexcept;

        private:
            static void on_playback_finished(GstBus *bus,
                                             GstMessage *message,
                                             NowPlayingList *self) noexcept;
            static void on_playback_state_changed(
                GstBus *bus,
                GstMessage *message,
                NowPlayingList *self) noexcept;
            static void on_playback_error(GstBus *bus,
                                          GstMessage *message,
                                          NowPlayingList *self) noexcept;

            static gboolean update_playback_position(
                NowPlayingList *self) noexcept;

        private:
            GstElement *playbin_{ nullptr };
            GstBus *bus_{ nullptr };

            std::vector<music::Track> content_{};
            music::Track *current_track_{ nullptr };
            PlaybackState current_state_{ PlaybackState::Stopped };

            std::vector<std::function<void(PlaybackState)>>
                state_changed_callbacks_{};
            std::vector<std::function<void(const music::Track &)>>
                track_queued_callbacks_{};
            std::vector<std::function<void(std::int32_t)>>
                playback_position_changed_callbacks_{};

            std::uint32_t position_update_callback_tag_{ 0 };

        private:
            DISABLE_COPY(NowPlayingList)
            DISABLE_MOVE(NowPlayingList)
        };
    }
}

#endif // !SPRING_PLAYER_NOW_PLAYING_LIST_H
