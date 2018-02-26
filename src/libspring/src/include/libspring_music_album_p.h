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

#ifndef LIBSPRING_MUSIC_ALBUM_P_H
#define LIBSPRING_MUSIC_ALBUM_P_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <sequential.h>

#include "libspring_global.h"

namespace spring
{
    class PlexMediaServerPrivate;

    namespace music
    {
        class AlbumPrivate
        {
        public:
            struct LibraryContainer
            {
                struct media_container_t
                {
                    struct genre_t
                    {
                        ATTRIBUTE(std::string, tag)
                        INIT_ATTRIBUTES(tag)
                    };

                    struct metadata_t
                    {
                        ATTRIBUTE(std::string, key)
                        ATTRIBUTE(std::string, title)
                        ATTRIBUTE(std::string, parentTitle)
                        ATTRIBUTE(std::string, thumb)
                        ATTRIBUTE(std::size_t, leafCount)
                        ATTRIBUTE(std::vector<genre_t>, Genre)
                        INIT_ATTRIBUTES(
                            key, title, parentTitle, thumb, leafCount, Genre)
                    };

                    ATTRIBUTE(std::vector<metadata_t>, Metadata)
                    INIT_ATTRIBUTES(Metadata)
                };

                ATTRIBUTE(media_container_t, MediaContainer)
                INIT_ATTRIBUTES(MediaContainer)
            };

        private:
            using RawAlbumMetadata =
                LibraryContainer::media_container_t::metadata_t;

        public:
            AlbumPrivate(RawAlbumMetadata &&metadata,
                         std::weak_ptr<PlexMediaServerPrivate> pms) noexcept;
            ~AlbumPrivate() noexcept;

        private:
            std::string key_{};
            std::string id_{};
            std::string title_{};
            std::string artist_{};
            std::string genre_{};
            std::size_t songCount_{ 0 };
            std::string artworkPath_{};
            std::string artworkData_{};

            std::weak_ptr<PlexMediaServerPrivate> pms_;

        private:
            DISABLE_COPY(AlbumPrivate)
            DISABLE_MOVE(AlbumPrivate)

        private:
            friend class Album;
        };
    }
}

#endif // !LIBSPRING_MUSIC_ALBUM_P_H
