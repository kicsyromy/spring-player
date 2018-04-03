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

#include "libspring_plex_media_server.h"
#include "libspring_plex_media_server_p.h"

#include <json_format.h>
#include <sequential.h>

#include "libspring_library_section_p.h"
#include "libspring_logger.h"
#include "libspring_user_properties_p.h"

using namespace spring;

const char *PlexMediaServerPrivate::USER_AGENT{ "libspriNG/" LIBSPRING_VERSION_STR };

const char *PlexMediaServerPrivate::PLEX_HEADER_AUTH_KEY{ "X-Plex-Token" };

const HttpClient::http_header_array_t PlexMediaServerPrivate::PLEX_HEADERS{
    { "Accept", "application/json" },
    { "Content-Type", "application/x-www-form-urlencoded" },
    { "X-Plex-Device", "Generic" },
    { "X-Plex-Product", "SpringPlayer" },
    { "X-Plex-Version", LIBSPRING_VERSION_STR },
    { "X-Plex-Client-Identifier", "6sha3edzskjda732qmdwsjk" },
    { "X-Plex-Device-Name", "Linux" },
    { "X-Plex-Language", "en" },
    { "X-Plex-Model", "unknown" },
    { "X-Plex-Platform", "Linux" },
    { "X-Plex-Platform-Version", "4.4.0" },
    { "X-Plex-Provides", "client,controller,player,pubsub-player" }
};

namespace
{
    constexpr const char PLEX_AUTH_HOST[]{ "https://plex.tv" };
    constexpr const char PLEX_AUTH_PATH[]{ "/users/sign_in.json" };
}

PlexMediaServerPrivate::PlexMediaServerPrivate() noexcept
{
}

PlexMediaServerPrivate::~PlexMediaServerPrivate() noexcept = default;

Error PlexMediaServerPrivate::connect(const char *serverAddress,
                                      std::int32_t port,
                                      const char *username,
                                      const char *password,
                                      bool errorHandling) noexcept
{
    using namespace sequential_formats;

    Error error{};

    http_.setHost(PLEX_AUTH_HOST);
    http_.setUsername(username);
    http_.setPassword(password);

    auto request = http_.createRequest();
    request.setPath(PLEX_AUTH_PATH);
    request.setHeaders(PLEX_HEADERS);
    /* Send POST request */
    request.setBody("");

    auto result = request.send();

    if (result.error)
    {
        error = Error{ static_cast<Error::Code>(result.error.errorCode), result.error.message };
    }
    else
    {
        if (result.status.code() != HttpClient::Status::OK &&
            result.status.code() != HttpClient::Status::Created)
        {
            error = Error{ static_cast<Error::Code>(result.status.code()), result.status.name() };
        }
        else
        {
            JsonFormat format{ result.response.text };
            auto user = sequential::from_format<UserProperties>(format);
            authenticationToken_ = user.get_user().get_authentication_token();

            http_.setHost(serverAddress);
            http_.setPort(port);
            http_.disableAuthentication();
            http_.setSSLErrorHandling(static_cast<HttpClient::SSLErrorHandling>(errorHandling));

            url_ = http_.url();

            result = PlexMediaServerPrivate::request("/servers");
            format.parse(result.response.text);
            auto serverProperties = sequential::from_format<LibraryContainer>(format);
            name_ = serverProperties.get_MediaContainer().get_Server().at(0).get_name();
        }
    }

    return error;
}

Error PlexMediaServerPrivate::connect(const char *serverAddress,
                                      std::int32_t port,
                                      const char *token,
                                      bool errorHandling) noexcept
{
    http_.setHost(serverAddress);
    http_.setPort(port);
    http_.disableAuthentication();
    http_.setSSLErrorHandling(static_cast<HttpClient::SSLErrorHandling>(errorHandling));

    authenticationToken_ = token;

    return Error::noError();
}

HttpClient::Request PlexMediaServerPrivate::request() const noexcept
{
    auto request = http_.createRequest();

    request.setHeaders(PlexMediaServerPrivate::PLEX_HEADERS);
    request.setHeader({ PlexMediaServerPrivate::PLEX_HEADER_AUTH_KEY, authenticationToken_ });

    return request;
}

HttpClient::RequestResult PlexMediaServerPrivate::request(std::string &&path) const noexcept
{
    auto request = http_.createRequest();

    request.setPath(path);
    request.setHeaders(PlexMediaServerPrivate::PLEX_HEADERS);
    request.setHeader({ PlexMediaServerPrivate::PLEX_HEADER_AUTH_KEY, authenticationToken_ });

    return request.send();
}

PlexMediaServer::PlexMediaServer() noexcept
{
    priv_ = std::make_shared<PlexMediaServerPrivate>();
}

PlexMediaServer::~PlexMediaServer() noexcept = default;

PlexMediaServer::PlexMediaServer(PlexMediaServer &&other) noexcept
  : priv_(std::move(other.priv_))
{
}

PlexMediaServer &PlexMediaServer::operator=(PlexMediaServer &&other) noexcept
{
    priv_ = std::move(other.priv_);

    return *this;
}

Optional<const char *> PlexMediaServer::connect(
    const char *serverAddress,
    std::int32_t port,
    const char *username,
    const char *password,
    PlexMediaServer::SSLErrorHandling errorHandling) noexcept
{
    return { priv_->connect(serverAddress, port, username, password,
                            static_cast<bool>(errorHandling)),
             priv_->authenticationToken_.c_str() };
}

Error PlexMediaServer::connect(const char *serverAddress,
                               std::int32_t port,
                               const char *token,
                               PlexMediaServer::SSLErrorHandling errorHandling) noexcept
{
    return priv_->connect(serverAddress, port, token, static_cast<bool>(errorHandling));
}

const std::string &PlexMediaServer::name() const noexcept
{
    return priv_->name_;
}

std::vector<LibrarySection> PlexMediaServer::sections() const noexcept
{
    using namespace sequential_formats;

    /* TODO: Error handling */
    auto result = priv_->request(LibrarySectionPrivate::REQUEST_PATH);

    JsonFormat format{ result.response.text };
    auto mediaContainer =
        sequential::from_format<LibrarySectionPrivate::LibrarySectionContainer>(format);

    std::vector<LibrarySectionPrivate *> libraries;
    libraries.reserve(mediaContainer.get_MediaContainer().get_Directory().size());

    auto &directories = mediaContainer.get_MediaContainer().get_Directory();
    for (auto &directory : directories)
    {
        libraries.push_back(new LibrarySectionPrivate{ std::move(directory), priv_ });
    }

    return { libraries.begin(), libraries.end() };
}

std::string PlexMediaServer::customRequest(const char *path) const noexcept
{
    auto result = priv_->request(path);
    return result.response.text;
}
