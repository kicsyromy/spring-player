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

#ifndef LIBSPRING_MEDIA_LIBRARY_P_H
#define LIBSPRING_MEDIA_LIBRARY_P_H

#include <type_traits>

#include "libspring_global.h"
#include "libspring_media_library.h"
#include <libspring_movie_library.h>
#include <libspring_music_library.h>
#include <libspring_tv_show_library.h>
#include <libspring_video_library.h>

namespace spring
{
    class MediaLibraryPrivate
    {
    private:
        struct Content
        {
        public:
            template <typename Library, typename... Args>
            Content(Library *, MediaLibrary::Type t, Args &&... args) noexcept
              : type_(t)
            {
                new (data_.data) Library{ std::forward<Args>(args)... };
            }

            ~Content() noexcept;

        private:
            MediaLibrary::Type type_{ MediaLibrary::Type::Unknown };
            union Data {
                Data() noexcept;
                ~Data() noexcept;

                MusicLibrary music;
                MovieLibrary movie;
                TVShowLibrary tvShows;
                VideoLibrary video;

                alignas(alignof(MusicLibrary)) unsigned char data[sizeof(MusicLibrary)];
            } data_{};

        private:
            friend class MediaLibrary;
        };

    public:
        template <typename Library, typename... Args>
        MediaLibraryPrivate(Library *l, MediaLibrary::Type t, Args &&... args) noexcept
          : content_(l, t, std::forward<Args>(args)...)
        {
        }

        ~MediaLibraryPrivate() noexcept;

    private:
        Content content_;

    private:
        DISABLE_COPY(MediaLibraryPrivate)
        DISABLE_MOVE(MediaLibraryPrivate)

    private:
        friend class MediaLibrary;
    };
}

#endif // !LIBSPRING_MEDIA_LIBRARY_P_H
