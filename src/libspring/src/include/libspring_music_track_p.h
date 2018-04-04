/*
 * Copyright (c) 2018 Romeo Calota
 *
 * This file is part of the SpriNG library
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Romeo Calota
 */

#ifndef LIBSPRING_MUSIC_TRACK_P_H
#define LIBSPRING_MUSIC_TRACK_P_H

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <sequential.h>

#include "libspring_global.h"
#include "libspring_music_track.h"

namespace spring
{
    class PlexMediaServerPrivate;

    namespace music
    {
        class TrackPrivate
        {
        public:
            struct LibraryContainer
            {
                struct media_container_t
                {
                    struct metadata_t
                    {
                        struct media_t
                        {
                            struct part_t
                            {
                                ATTRIBUTE(std::string, file)
                                ATTRIBUTE(std::string, key)
                                ATTRIBUTE(std::size_t, size)
                                INIT_ATTRIBUTES(file, key, size)
                            };

                            ATTRIBUTE(std::vector<part_t>, Part)
                            INIT_ATTRIBUTES(Part)
                        };

                        ATTRIBUTE(std::vector<media_t>, Media)
                        ATTRIBUTE(std::size_t, duration)
                        ATTRIBUTE(std::string, grandparentTitle) /* artist */
                        ATTRIBUTE(std::string, key)
                        ATTRIBUTE(std::string, parentTitle) /* album */
                        ATTRIBUTE(std::string, parentKey)   /* album id */
                        ATTRIBUTE(std::string, thumb)
                        ATTRIBUTE(std::string, title)
                        INIT_ATTRIBUTES(Media,
                                        duration,
                                        grandparentTitle,
                                        key,
                                        parentTitle,
                                        parentKey,
                                        thumb,
                                        title)
                    };

                    ATTRIBUTE(std::vector<metadata_t>, Metadata)
                    INIT_ATTRIBUTES(Metadata)
                };

                ATTRIBUTE(media_container_t, MediaContainer)
                INIT_ATTRIBUTES(MediaContainer)
            };

        public:
            using Seconds = Track::Seconds;

        private:
            using RawTrackMetadata = LibraryContainer::media_container_t::metadata_t;

        public:
            TrackPrivate(RawTrackMetadata &&metadata,
                         std::weak_ptr<PlexMediaServerPrivate> pms) noexcept;
            ~TrackPrivate() noexcept;

            std::string path(Seconds offset = Seconds{ 0 }, uint32_t bitrate = 320) const noexcept;

        private:
            std::string key_{};
            std::string path_{};
            std::string title_{};
            std::string album_{};
            std::string album_key_{};
            std::string artist_{};
            Track::Milliseconds duration_{ 0 };
            std::string filePath_{};
            std::size_t fileSize_{ 0 };
            std::string artworkPath_{};
            std::string artworkData_{};

            std::weak_ptr<PlexMediaServerPrivate> pms_;

        private:
            DISABLE_COPY(TrackPrivate)
            DISABLE_MOVE(TrackPrivate)

        private:
            friend class Track;
        };
    }
}

#endif // !LIBSPRING_MUSIC_TRACK_P_H
