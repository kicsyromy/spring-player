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

#include <json_format.h>

#include "libspring_logger.h"
#include "libspring_music_album_p.h"
#include "libspring_music_track_p.h"
#include "libspring_plex_media_server_p.h"

using namespace spring;
using namespace spring::music;

ArtistPrivate::ArtistPrivate(RawArtistMetadata &&metadata,
                             std::weak_ptr<PlexMediaServerPrivate> pms) noexcept
  : key_(std::move(metadata.get_key()))
  /* This micro-algorithm assumes that each key starts with '/library/metadata'*/
  /* and has exactly 5 charactes... there should be better ways to do this     */
  , id_(key_.c_str() + 18, 5)
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
    auto index = key_.rfind('/');
    key_.replace(index, std::string::npos, "");
}

ArtistPrivate::~ArtistPrivate() noexcept = default;

Artist::Artist(ArtistPrivate *priv) noexcept
  : priv_(priv)
{
}

Artist::Artist(Artist &&other) noexcept
  : priv_(std::move(other.priv_))
{
}

Artist &Artist::operator=(Artist &&other) noexcept
{
    priv_ = std::move(other.priv_);
    return *this;
}

const std::string &Artist::id() const noexcept
{
    return priv_->id_;
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

std::vector<Album> Artist::albums() const noexcept
{
    using namespace sequential_formats;

    std::vector<AlbumPrivate *> result{};

    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        /* TODO: Error handling */
        auto r = pms->request(priv_->key_ + "/children");
        auto body = std::move(r.response.text);

        JsonFormat format{ body };
        auto container =
            sequential::from_format<AlbumPrivate::LibraryContainer>(format);

        auto &metadata = container.get_MediaContainer().get_Metadata();
        result.reserve(metadata.size());
        for (auto &m : metadata)
        {
            result.push_back(new AlbumPrivate{ std::move(m), priv_->pms_ });
        }
    }
    else
    {
        LOG_ERROR("Artist: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return { result.begin(), result.end() };
}

std::vector<Track> Artist::tracks() const noexcept
{
    using namespace sequential_formats;

    std::vector<TrackPrivate *> result{};

    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        /* TODO: Error handling */
        auto r = pms->request(priv_->key_ + "/allLeaves");
        auto body = std::move(r.response.text);

        JsonFormat format{ body };
        auto container =
            sequential::from_format<TrackPrivate::LibraryContainer>(format);

        auto &metadata = container.get_MediaContainer().get_Metadata();
        result.reserve(metadata.size());
        for (auto &m : metadata)
        {
            result.push_back(new TrackPrivate{ std::move(m), priv_->pms_ });
        }
    }
    else
    {
        LOG_ERROR("Artist: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return { result.begin(), result.end() };
}

std::vector<Track> Artist::popularTracks() const noexcept
{
    using namespace sequential_formats;

    std::vector<music::TrackPrivate *> result{};

    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
    }
    else
    {
        LOG_ERROR("Artist: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return { result.begin(), result.end() };
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

    return result;
}
