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

#ifndef LIBSPRING_MUSIC_TRACK_H
#define LIBSPRING_MUSIC_TRACK_H

#include <chrono>
#include <memory>
#include <string>

#include <libspring_global.h>

namespace spring
{
    namespace music
    {
        class TrackPrivate;

        struct Track
        {
        public:
            using Milliseconds = std::chrono::milliseconds;
            using Seconds = std::chrono::seconds;

        public:
            explicit Track(TrackPrivate *priv) noexcept;
            ~Track() noexcept;

            Track(Track &&other) noexcept;
            Track &operator=(Track &&other) noexcept;

        public:
            const std::string &title() const noexcept;
            const std::string &album() const noexcept;
            const std::string &artist() const noexcept;
            Milliseconds duration() const noexcept;
            const std::string &filePath() const noexcept;
            std::size_t fileSize() const noexcept;
            const std::string &artwork() const noexcept;
            std::string url(std::uint32_t bitrate = 320) const noexcept;
            void trackData(DataFragmentReadyCallback callback, Seconds offset, void *userData) const
                noexcept;

        private:
            std::unique_ptr<TrackPrivate> priv_;

        private:
            DISABLE_COPY(Track)
        };
    } // namespace music
} // namespace spring

#endif // !LIBSPRING_MUSIC_TRACK_H
