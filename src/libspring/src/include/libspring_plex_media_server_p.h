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

#ifndef LIBSPRING_PLEX_MEDIA_SERVER_P_H
#define LIBSPRING_PLEX_MEDIA_SERVER_P_H

#include <memory>

#include <sequential.h>

#include "libspring_error.h"
#include "libspring_global.h"
#include "libspring_http_client_p.h"

namespace spring
{
    class PlexMediaServerPrivate
    {
    public:
        struct LibraryContainer
        {
            struct media_container_t
            {
                struct server_t
                {
                    ATTRIBUTE(std::string, name)
                    INIT_ATTRIBUTES(name)
                };

                ATTRIBUTE(std::vector<server_t>, Server)
                INIT_ATTRIBUTES(Server)
            };

            ATTRIBUTE(media_container_t, MediaContainer)
            INIT_ATTRIBUTES(MediaContainer)
        };

    public:
        static const char *USER_AGENT;
        static const char *PLEX_HEADER_AUTH_KEY;
        static const HttpClient::http_header_array_t PLEX_HEADERS;

    public:
        PlexMediaServerPrivate() noexcept;
        ~PlexMediaServerPrivate() noexcept;

    public:
        Error connect(const char *serverAddress,
                      std::int32_t port,
                      const char *username,
                      const char *password,
                      bool errorHandling) noexcept;

        Error connect(const char *serverAddress,
                      std::int32_t port,
                      const char *token,
                      bool errorHandling) noexcept;

        inline const std::string &authenticationToken() const noexcept
        {
            return authenticationToken_;
        }

        inline const std::string &clientUUID() const noexcept { return clientUUID_; }

        inline const std::string &url() const noexcept { return url_; }

        HttpClient::Request request() const noexcept;
        HttpClient::RequestResult request(std::string &&path) const noexcept;

    private:
        HttpClient http_{ USER_AGENT };
        std::string url_{};
        std::string authenticationToken_{};
        std::string clientUUID_{ "6sha3edzskjda732qmdwsjk" };
        std::string name_{};

    private:
        DISABLE_COPY(PlexMediaServerPrivate)
        DISABLE_MOVE(PlexMediaServerPrivate)

    private:
        friend class PlexMediaServer;
    };
}

#endif // !LIBSPRING_PLEX_MEDIA_SERVER_P_H
