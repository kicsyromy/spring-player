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

#include "libspring_music_genre.h"
#include "libspring_music_genre_p.h"

#include "libspring_plex_media_server_p.h"

using namespace spring;
using namespace spring::music;

GenrePrivate::GenrePrivate(RawGenreMetadata &&metadata,
                           std::weak_ptr<PlexMediaServerPrivate> pms) noexcept
  : key_(std::move(metadata.get_fastKey()))
  , name_(std::move(metadata.get_title()))
  , pms_(pms)
{
}

GenrePrivate::~GenrePrivate() noexcept = default;

Genre::Genre(GenrePrivate *priv) noexcept
  : priv_(priv)
{
}

Genre::~Genre() noexcept = default;

const std::string &Genre::name() const noexcept
{
    return priv_->name_;
}
