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

#include "libspring_music_artist.h"
#include "libspring_music_artist_p.h"

#include "libspring_logger_p.h"
#include "libspring_plex_media_server_p.h"

using namespace spring;
using namespace spring::music;

ArtistPrivate::ArtistPrivate(RawArtistMetadata &&metadata,
                             std::weak_ptr<PlexMediaServerPrivate> pms) noexcept
  : key_(std::move(metadata.get_key()))
  , name_(std::move(metadata.get_title()))
  , summary_(std::move(metadata.get_summary()))
  , country_(metadata.get_Country().size() > 0 ?
                 std::move(metadata.get_Country().at(0).get_tag()) :
                 "")
  , genre_(metadata.get_Genre().size() > 0 ?
               std::move(metadata.get_Genre().at(0).get_tag()) :
               "")
  , thumbnailPath_(std::move(metadata.get_thumb()))
  , pms_(pms)
{
}

ArtistPrivate::~ArtistPrivate() noexcept = default;

Artist::Artist(ArtistPrivate *priv) noexcept
  : priv_(priv)
{
}

Artist::~Artist() noexcept = default;

const std::string &Artist::name() const noexcept
{
    return priv_->name_;
}

const std::string &Artist::summary() const noexcept
{
    return priv_->summary_;
}

const std::string &Artist::country() const noexcept
{
    return priv_->country_;
}

const std::string &Artist::genre() const noexcept
{
    return priv_->genre_;
}

std::string Artist::thumbnail() const noexcept
{
    std::string result{};

    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        /* TODO: Error handling */
        auto r = pms->request(priv_->thumbnailPath_.c_str());
        result = std::move(r.response.text);
    }
    else
    {
        LOG_ERROR("Artist: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return std::move(result);
}
