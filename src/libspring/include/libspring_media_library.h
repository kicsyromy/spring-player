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

#ifndef LIBSPRING_MEDIA_LIBRARY_H
#define LIBSPRING_MEDIA_LIBRARY_H

#include <memory>

#include <libspring_global.h>

namespace spring
{
    class MediaLibraryPrivate;
    class MusicLibrary;
    class MovieLibrary;
    class TVShowLibrary;
    class VideoLibrary;

    class MediaLibrary
    {
    public:
        enum class Type
        {
            Unknown = -1,
            Music,
            Movies,
            TVShows,
            Videos
        };

    public:
        MediaLibrary(MediaLibraryPrivate *priv) noexcept;
        ~MediaLibrary() noexcept;

        MediaLibrary(MediaLibrary &&other) noexcept;
        MediaLibrary &operator=(MediaLibrary &&other) noexcept;

    public:
        operator MusicLibrary &() noexcept;
        operator MovieLibrary &() noexcept;
        operator TVShowLibrary &() noexcept;
        operator VideoLibrary &() noexcept;

    private:
        std::unique_ptr<MediaLibraryPrivate> priv_;

    private:
        DISABLE_COPY(MediaLibrary)
    };
}

#endif // !LIBSPRING_MEDIA_LIBRARY_H
