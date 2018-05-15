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

#include "libspring_media_library.h"
#include "libspring_media_library_p.h"

using namespace spring;

MediaLibraryPrivate::Content::~Content() noexcept
{
    switch (type_)
    {
        default:
        case MediaLibrary::Type::Movies:
        case MediaLibrary::Type::TVShows:
        case MediaLibrary::Type::Videos:
            break;
        case MediaLibrary::Type::Music:
            data_.music.~MusicLibrary();
            break;
    }
}

MediaLibraryPrivate::Content::Data::Data() noexcept
{
}

MediaLibraryPrivate::Content::Data::~Data() noexcept
{
}

MediaLibraryPrivate::~MediaLibraryPrivate() noexcept = default;

MediaLibrary::MediaLibrary(MediaLibraryPrivate *priv) noexcept
  : priv_(priv)
{
}

MediaLibrary::~MediaLibrary() noexcept = default;

MediaLibrary::MediaLibrary(MediaLibrary &&other) noexcept
  : priv_(std::move(other.priv_))
{
}
MediaLibrary &MediaLibrary::operator=(MediaLibrary &&other) noexcept
{
    priv_ = std::move(other.priv_);
    return *this;
}

spring::MediaLibrary::operator MusicLibrary &() noexcept
{
    return priv_->content_.data_.music;
}

spring::MediaLibrary::operator MovieLibrary &() noexcept
{
    return priv_->content_.data_.movie;
}

spring::MediaLibrary::operator TVShowLibrary &() noexcept
{
    return priv_->content_.data_.tvShows;
}

spring::MediaLibrary::operator VideoLibrary &() noexcept
{
    return priv_->content_.data_.video;
}
