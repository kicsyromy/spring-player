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

#include "libspring_music_track.h"
#include "libspring_music_track_p.h"

#include <json_format.h>

#include "libspring_logger.h"
#include "libspring_music_album_p.h"
#include "libspring_plex_media_server_p.h"

using namespace spring;
using namespace spring::music;

namespace
{
    constexpr const char MUSIC_TRANSCODE_REQUEST_PATH[]{
        "/music/:/transcode/universal/start.mp3?path=http://127.0.0.1:32400"
    };
}

TrackPrivate::TrackPrivate(RawTrackMetadata &&metadata,
                           std::weak_ptr<PlexMediaServerPrivate> pms) noexcept
  : path_(std::move(metadata.get_key()))
  , title_(std::move(metadata.get_title()))
  , album_(std::move(metadata.get_parentTitle()))
  , album_key_(std::move(metadata.get_parentKey()))
  , artist_(std::move(metadata.get_grandparentTitle()))
  , duration_(metadata.get_duration())
  , artworkPath_(std::move(metadata.get_thumb()))
  , pms_(pms)
{
    auto &mediaList = metadata.get_Media();
    if (!mediaList.empty())
    {
        auto &partList = mediaList.at(0).get_Part();
        if (!partList.empty())
        {
            auto &part = partList.at(0);
            key_ = std::move(part.get_key());
            filePath_ = std::move(part.get_file());
            fileSize_ = part.get_size();
        }
    }
}

TrackPrivate::~TrackPrivate() noexcept = default;

std::string TrackPrivate::path(Seconds offset, std::uint32_t bitrate) const noexcept
{
    auto pms = pms_.lock();
    if (pms != nullptr)
    {
        return fmt::format("{}{}"
                           "&session={}"
                           "&protocol=http"
                           "&maxAudioBitrate={}"
                           "&directPlay=1"
                           "&offset={}"
                           "&X-Plex-Device=Web&{}={}",
                           MUSIC_TRANSCODE_REQUEST_PATH, path_, pms->clientUUID(), bitrate,
                           offset.count(), PlexMediaServerPrivate::PLEX_HEADER_AUTH_KEY,
                           pms->authenticationToken());
    }
    else
    {
        LOG_ERROR("Track: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return {};
}

Track::Track(TrackPrivate *priv) noexcept
  : priv_(priv)
{
}

Track::~Track() noexcept = default;

Track::Track(Track &&other) noexcept
  : priv_(std::move(other.priv_))
{
}

Track &Track::operator=(Track &&other) noexcept
{
    priv_ = std::move(other.priv_);

    return *this;
}

const std::string &Track::title() const noexcept
{
    return priv_->title_;
}

const std::string &Track::album() const noexcept
{
    return priv_->album_;
}

const std::string &Track::artist() const noexcept
{
    return priv_->artist_;
}

Track::Milliseconds Track::duration() const noexcept
{
    return priv_->duration_;
}

const std::string &Track::filePath() const noexcept
{
    return priv_->filePath_;
}

std::size_t Track::fileSize() const noexcept
{
    return priv_->fileSize_;
}

const std::string &Track::artwork() const noexcept
{
    if (priv_->artworkData_.empty())
    {
        auto pms = priv_->pms_.lock();
        if (pms != nullptr)
        {
            /* TODO: Error handling */
            if (!priv_->artworkPath_.empty())
            {
                auto r = pms->request(priv_->artworkPath_.c_str());
                priv_->artworkData_ = std::move(r.response.text);
            }
            else /* some tracks don't have artwork so use album artwork instead */
            {
                using namespace sequential_formats;

                auto r = pms->request(priv_->album_key_.c_str());
                JsonFormat format{ r.response.text };
                auto mediaContainer =
                    sequential::from_format<music::AlbumPrivate::LibraryContainer>(format);
                r = pms->request(
                    mediaContainer.get_MediaContainer().get_Metadata().at(0).get_thumb().c_str());
                priv_->artworkData_ = std::move(r.response.text);
            }
        }
        else
        {
            LOG_ERROR("Track: Invalid connection handle. PlexMediaServer "
                      "instance was deleted!");
        }
    }

    return priv_->artworkData_;
}

std::string Track::url(std::uint32_t bitrate) const noexcept
{
    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        return fmt::format("{}{}", pms->url(),
                           std::move(priv_->path(music::Track::Seconds{ 0 }, bitrate)));
    }
    else
    {
        LOG_ERROR("Track: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
        return {};
    }
}

void Track::trackData(DataFragmentReadyCallback callback, Seconds offset, void *userData) const
    noexcept
{
    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        auto request = pms->request();
        request.setPath(priv_->path(offset));
        auto response = request.send(callback, userData);
    }
    else
    {
        LOG_ERROR("Track: Invalid connection handle. PlexMediaServer instance was deleted!");
    }
}
