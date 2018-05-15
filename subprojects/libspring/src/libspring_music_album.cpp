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

#include "libspring_music_album.h"
#include "libspring_music_album_p.h"

#include <json_format.h>

#include "libspring_logger.h"
#include "libspring_music_track_p.h"
#include "libspring_plex_media_server_p.h"

using namespace spring;
using namespace spring::music;

AlbumPrivate::AlbumPrivate(RawAlbumMetadata &&metadata,
                           std::weak_ptr<PlexMediaServerPrivate> pms) noexcept
  : key_(std::move(metadata.get_key()))
  /* This micro-algorithm assumes that each key starts with '/library/metadata'*/
  /* and has exactly 5 charactes... there should be better ways to do this     */
  , id_(key_.c_str() + 18, 5)
  , title_(std::move(metadata.get_title()))
  , artist_(std::move(metadata.get_parentTitle()))
  , genre_(metadata.get_Genre().size() > 0 ? std::move(metadata.get_Genre().at(0).get_tag()) : "")
  , songCount_(metadata.get_leafCount())
  , artworkPath_(std::move(metadata.get_thumb()))
  , pms_(pms)
{
}

AlbumPrivate::~AlbumPrivate() noexcept = default;

Album::Album(AlbumPrivate *priv) noexcept
  : priv_(priv)
{
}

Album::~Album() noexcept = default;

Album::Album(Album &&other) noexcept
  : priv_(std::move(other.priv_))
{
}

Album &Album::operator=(Album &&other) noexcept
{
    priv_ = std::move(other.priv_);
    return *this;
}

const std::string &Album::id() const noexcept
{
    return priv_->id_;
}

const std::string &Album::title() const noexcept
{
    return priv_->title_;
}

const std::string &Album::artist() const noexcept
{
    return priv_->artist_;
}

const std::string &Album::genre() const noexcept
{
    return priv_->genre_;
}

std::size_t Album::songCount() const noexcept
{
    return priv_->songCount_;
}

const std::string &Album::artwork() const noexcept
{
    if (priv_->artworkData_.empty())
    {
        auto pms = priv_->pms_.lock();
        if (pms != nullptr)
        {
            /* TODO: Error handling */
            auto r = pms->request(priv_->artworkPath_.c_str());
            priv_->artworkData_ = std::move(r.response.text);
        }
        else
        {
            LOG_ERROR("Album: Invalid connection handle. PlexMediaServer "
                      "instance was deleted!");
        }
    }

    return priv_->artworkData_;
}

std::vector<Track> Album::tracks() const noexcept
{
    using namespace sequential_formats;

    std::vector<music::TrackPrivate *> result{};

    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        /* TODO: Error handling */
        auto r = pms->request(std::string{ priv_->key_ });
        auto body = std::move(r.response.text);

        JsonFormat format{ body };
        auto container = sequential::from_format<music::TrackPrivate::LibraryContainer>(format);

        auto &metadata = container.get_MediaContainer().get_Metadata();
        result.reserve(metadata.size());
        for (auto &m : metadata)
        {
            result.push_back(new music::TrackPrivate{ std::move(m), priv_->pms_ });
        }
    }
    else
    {
        LOG_ERROR("MusicLibrary: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return { result.begin(), result.end() };
}
