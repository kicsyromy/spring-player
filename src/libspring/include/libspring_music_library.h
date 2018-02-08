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

#ifndef LIBSPRING_MUSIC_LIBRARY_H
#define LIBSPRING_MUSIC_LIBRARY_H

#include <memory>
#include <vector>

#include <libspring_global.h>
#include <libspring_music_album.h>
#include <libspring_music_artist.h>
#include <libspring_music_genre.h>
#include <libspring_music_track.h>

namespace spring
{
    class MusicLibraryPrivate;

    class MusicLibrary
    {
    private:
        using Album = spring::music::Album;
        using Artist = spring::music::Artist;
        using Genre = spring::music::Genre;
        using Track = spring::music::Track;

    public:
        MusicLibrary(MusicLibraryPrivate *priv) noexcept;
        ~MusicLibrary() noexcept;

        MusicLibrary(MusicLibrary &&other) noexcept;
        MusicLibrary &operator=(MusicLibrary &&other) noexcept;

    public:
        std::vector<Album> albums() const noexcept;
        std::vector<Artist> artists() const noexcept;
        std::vector<Genre> genres() const noexcept;
        std::vector<Track> tracks() const noexcept;

    private:
        std::unique_ptr<MusicLibraryPrivate> priv_;

    private:
        DISABLE_COPY(MusicLibrary)
    };
}

#endif // !LIBSPRING_MUSIC_LIBRARY_H
