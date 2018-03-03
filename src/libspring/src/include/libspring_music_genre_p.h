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

#ifndef LIBSPRING_MUSIC_GENRE_P_H
#define LIBSPRING_MUSIC_GENRE_P_H

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
        class GenrePrivate
        {
        public:
            struct LibraryContainer
            {
                struct media_container_t
                {
                    struct directory_t
                    {
                        ATTRIBUTE(std::string, fastKey)
                        ATTRIBUTE(std::string, title)
                        INIT_ATTRIBUTES(fastKey, title)
                    };

                    ATTRIBUTE(std::vector<directory_t>, Directory)
                    INIT_ATTRIBUTES(Directory)
                };

                ATTRIBUTE(media_container_t, MediaContainer)
                INIT_ATTRIBUTES(MediaContainer)
            };

        private:
            using RawGenreMetadata = LibraryContainer::media_container_t::directory_t;

        public:
            GenrePrivate(RawGenreMetadata &&metadata,
                         std::weak_ptr<PlexMediaServerPrivate> pms) noexcept;
            ~GenrePrivate() noexcept;

        private:
            std::string key_;
            std::string name_;

            std::weak_ptr<PlexMediaServerPrivate> pms_;

        private:
            DISABLE_COPY(GenrePrivate)
            DISABLE_MOVE(GenrePrivate)

        private:
            friend class Genre;
        };
    }
}

#endif // !LIBSPRING_MUSIC_GENRE_P_H
