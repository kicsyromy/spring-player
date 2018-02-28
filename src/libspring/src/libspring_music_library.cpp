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

#include "libspring_music_library.h"
#include "libspring_music_library_p.h"

#include <json_format.h>

#include "libspring_library_section_p.h"
#include "libspring_logger.h"
#include "libspring_music_album_p.h"
#include "libspring_music_artist_p.h"
#include "libspring_music_genre_p.h"
#include "libspring_music_track_p.h"
#include "libspring_plex_media_server_p.h"

using namespace spring;

MusicLibraryPrivate::MusicLibraryPrivate(
    std::string key, std::weak_ptr<PlexMediaServerPrivate> pms) noexcept
  : key_(key)
  , pms_(pms)
{
}

MusicLibraryPrivate::~MusicLibraryPrivate() noexcept = default;

MusicLibrary::MusicLibrary(MusicLibraryPrivate *priv) noexcept
  : priv_(priv)
{
}

MusicLibrary::~MusicLibrary() noexcept = default;

MusicLibrary::MusicLibrary(MusicLibrary &&other) noexcept
  : priv_(std::move(other.priv_))
{
}

MusicLibrary &MusicLibrary::operator=(MusicLibrary &&other) noexcept
{
    priv_ = std::move(other.priv_);
    return *this;
}

std::vector<MusicLibrary::Album> MusicLibrary::albums() const noexcept
{
    using namespace sequential_formats;

    std::vector<music::AlbumPrivate *> result{};

    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        /* TODO: Error handling */
        auto r =
            pms->request(std::string{ LibrarySectionPrivate::REQUEST_PATH } +
                         "/" + priv_->key_ + "/albums");
        auto body = std::move(r.response.text);

        JsonFormat format{ body };
        auto container =
            sequential::from_format<music::AlbumPrivate::LibraryContainer>(
                format);
        auto &metadata = container.get_MediaContainer().get_Metadata();
        result.reserve(metadata.size());
        for (auto &m : metadata)
        {
            result.push_back(
                new music::AlbumPrivate{ std::move(m), priv_->pms_ });
        }
    }
    else
    {
        LOG_ERROR("MusicLibrary: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return { result.begin(), result.end() };
}

std::vector<MusicLibrary::Artist> MusicLibrary::artists() const noexcept
{
    using namespace sequential_formats;

    std::vector<music::ArtistPrivate *> result{};

    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        /* TODO: Error handling */
        auto r =
            pms->request(std::string{ LibrarySectionPrivate::REQUEST_PATH } +
                         "/" + priv_->key_ + "/all");
        auto body = std::move(r.response.text);

        JsonFormat format{ body };
        auto container =
            sequential::from_format<music::ArtistPrivate::LibraryContainer>(
                format);
        auto &metadata = container.get_MediaContainer().get_Metadata();
        result.reserve(metadata.size());
        for (auto &m : metadata)
        {
            result.push_back(
                new music::ArtistPrivate{ std::move(m), priv_->pms_ });
        }
    }
    else
    {
        LOG_ERROR("MusicLibrary: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return { result.begin(), result.end() };
}

std::vector<MusicLibrary::Genre> MusicLibrary::genres() const noexcept
{
    using namespace sequential_formats;

    std::vector<music::GenrePrivate *> result{};

    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        /* TODO: Error handling */
        auto r =
            pms->request(std::string{ LibrarySectionPrivate::REQUEST_PATH } +
                         "/" + priv_->key_ + "/genre");
        auto body = std::move(r.response.text);

        JsonFormat format{ body };
        auto container =
            sequential::from_format<music::GenrePrivate::LibraryContainer>(
                format);

        auto &metadata = container.get_MediaContainer().get_Directory();
        result.reserve(metadata.size());
        for (auto &m : metadata)
        {
            result.push_back(
                new music::GenrePrivate{ std::move(m), priv_->pms_ });
        }
    }
    else
    {
        LOG_ERROR("MusicLibrary: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return { result.begin(), result.end() };
}

std::vector<MusicLibrary::Track> MusicLibrary::tracks() const noexcept
{
    using namespace sequential_formats;

    std::vector<music::TrackPrivate *> result{};

    auto pms = priv_->pms_.lock();
    if (pms != nullptr)
    {
        /* TODO: Error handling */
        auto r =
            pms->request(std::string{ LibrarySectionPrivate::REQUEST_PATH } +
                         "/" + priv_->key_ + "/all?type=10");
        auto body = std::move(r.response.text);

        JsonFormat format{ body };
        auto container =
            sequential::from_format<music::TrackPrivate::LibraryContainer>(
                format);

        auto &metadata = container.get_MediaContainer().get_Metadata();
        result.reserve(metadata.size());
        for (auto &m : metadata)
        {
            result.push_back(
                new music::TrackPrivate{ std::move(m), priv_->pms_ });
        }
    }
    else
    {
        LOG_ERROR("MusicLibrary: Invalid connection handle. PlexMediaServer "
                  "instance was deleted!");
    }

    return { result.begin(), result.end() };
}
