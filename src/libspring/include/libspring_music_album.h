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

#ifndef LIBSPRING_MUSIC_ALBUM_H
#define LIBSPRING_MUSIC_ALBUM_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <libspring_global.h>
#include <libspring_music_track.h>

namespace spring
{
    namespace music
    {
        class AlbumPrivate;

        class Album
        {
        public:
            Album(AlbumPrivate *priv) noexcept;
            ~Album() noexcept;

            Album(Album &&other) noexcept;
            Album &operator=(Album &&other) noexcept;

        public:
            const std::string &id() const noexcept;
            const std::string &title() const noexcept;
            const std::string &artist() const noexcept;
            const std::string &genre() const noexcept;
            std::size_t songCount() const noexcept;
            /* Just assume the image is a JPEG for now... */
            const std::string &artwork() const noexcept;
            std::vector<Track> tracks() const noexcept;

        private:
            std::unique_ptr<AlbumPrivate> priv_;

        private:
            DISABLE_COPY(Album)
        };
    } // namespace music
} // namespace spring

#endif // !LIBSPRING_MUSIC_ALBUM_H
