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

#ifndef LIBSPRING_MUSIC_ARTIST_P_H
#define LIBSPRING_MUSIC_ARTIST_P_H

#include <memory>
#include <string>
#include <vector>

#include <sequential.h>

#include "libspring_global.h"

namespace spring
{
    class PlexMediaServerPrivate;

    namespace music
    {
        class ArtistPrivate
        {
        public:
            struct LibraryContainer
            {
                struct media_container_t
                {
                    struct country_t
                    {
                        ATTRIBUTE(std::string, tag)
                        INIT_ATTRIBUTES(tag)
                    };

                    struct genre_t
                    {
                        ATTRIBUTE(std::string, tag)
                        INIT_ATTRIBUTES(tag)
                    };

                    struct metadata_t
                    {
                        ATTRIBUTE(std::vector<country_t>, Country)
                        ATTRIBUTE(std::vector<genre_t>, Genre)
                        ATTRIBUTE(std::string, key)
                        ATTRIBUTE(std::string, summary)
                        ATTRIBUTE(std::string, thumb)
                        ATTRIBUTE(std::string, title)
                        INIT_ATTRIBUTES(Country, Genre, key, summary, thumb, title)
                    };

                    ATTRIBUTE(std::int32_t, librarySectionID)
                    ATTRIBUTE(std::vector<metadata_t>, Metadata)
                    INIT_ATTRIBUTES(librarySectionID, Metadata)
                };

                ATTRIBUTE(media_container_t, MediaContainer)
                INIT_ATTRIBUTES(MediaContainer)
            };

        private:
            using RawArtistMetadata = LibraryContainer::media_container_t::metadata_t;

        public:
            ArtistPrivate(RawArtistMetadata &&metadata,
                          std::int32_t sectionId,
                          std::weak_ptr<PlexMediaServerPrivate> pms) noexcept;
            ~ArtistPrivate() noexcept;

        private:
            std::string key_{};
            std::string id_{};
            std::string name_{};
            std::string summary_{};
            std::string country_{};
            std::string genre_{};
            std::string thumbnailPath_{};
            std::int32_t librarySectionId_{};
            std::string artworkData_{};

            std::weak_ptr<PlexMediaServerPrivate> pms_{};

        private:
            DISABLE_COPY(ArtistPrivate)
            DISABLE_MOVE(ArtistPrivate)

        private:
            friend class Artist;
        };
    } // namespace music
} // namespace spring

#endif // !LIBSPRING_MUSIC_ARTIST_P_H
