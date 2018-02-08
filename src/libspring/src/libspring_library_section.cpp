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

#include "libspring_library_section.h"
#include "libspring_library_section_p.h"

#include <unordered_map>

#include <json_format.h>

#include "libspring_logger_p.h"
#include "libspring_media_library_p.h"
#include "libspring_music_library_p.h"
#include "libspring_plex_media_server_p.h"

using namespace spring;

namespace
{
    const std::unordered_map<std::string, LibrarySection::Type> MEDIA_TYPE{
        { "artist", LibrarySection::Type::Music },
        { "movie", LibrarySection::Type::Movies },
        { "show", LibrarySection::Type::TVShows },
        { "movie", LibrarySection::Type::Videos }
    };
}

LibrarySectionPrivate::LibrarySectionPrivate(
    MediaDirectory &&container,
    std::weak_ptr<PlexMediaServerPrivate> pms) noexcept
  : key(std::move(container.get_key()))
  , title(std::move(container.get_title()))
  , type(MEDIA_TYPE.at(container.get_type()))
  , pms_(pms)
{
}

LibrarySectionPrivate::~LibrarySectionPrivate() noexcept = default;

LibrarySection::LibrarySection(LibrarySectionPrivate *priv) noexcept
  : priv_(priv)
{
}

LibrarySection::~LibrarySection() noexcept = default;

std::string LibrarySection::title() const noexcept
{
    return priv_->title;
}

LibrarySection::Type LibrarySection::type() const noexcept
{
    return priv_->type;
}

MediaLibrary LibrarySection::content() const noexcept
{
    MediaLibraryPrivate *mediaLibrary = nullptr;

    if (priv_->type == Type::Music)
    {
        mediaLibrary = new MediaLibraryPrivate{
            static_cast<MusicLibrary *>(nullptr), priv_->type,
            new MusicLibraryPrivate{ priv_->key, priv_->pms_ }
        };
    }
    else
    {
        LOG_WARN("Unimplemented type: %d", type());
    }

    return { mediaLibrary };
}
