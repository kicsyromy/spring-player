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
#include "libspring_logger_p.h"
#include "libspring_user_properties_p.h"

using namespace spring;

const char *PlexMediaServerPrivate::USER_AGENT{
    "libspriNG/" LIBSPRING_VERSION_STR
};

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

PlexMediaServerPrivate::PlexMediaServerPrivate(const char *serverAddress,
                                               int32_t port,
                                               const char *username,
                                               const char *password,
                                               bool errorHandling) noexcept
{
    using namespace sequential_formats;

    //    http_->setHost(PLEX_AUTH_HOST);
    //    http_->setUsername(username);
    //    http_->setPassword(password);

    //    auto request = http_->createRequest();
    //    request.setPath(PLEX_AUTH_PATH);
    //    request.setHeaders(PLEX_HEADERS);
    //    /* Send POST request */
    //    request.setBody("");

    //    auto result = request.send();

    //    JsonFormat format{ result.response.text };
    //    auto user = sequential::from_format<UserProperties>(format);
    char token[100];
    FILE *f = fopen("/home/kicsyromy/workspace/token.txt", "r");
    fscanf(f, "%s", token);
    authenticationToken_ = token;
    fclose(f);
    //    std::cout << user.get_user().get_authentication_token();
    //    authenticationToken_ = user.get_user().get_authentication_token();

    http_.setHost(serverAddress);
    http_.setPort(port);
    http_.disableAuthentication();
    http_.setSSLErrorHandling(
        static_cast<HttpClient::SSLErrorHandling>(errorHandling));

    url_ = http_.url();
}

PlexMediaServerPrivate::~PlexMediaServerPrivate() noexcept = default;

HttpClient::RequestResult PlexMediaServerPrivate::request(
    std::string &&path) const noexcept
{
    auto request = http_.createRequest();

    request.setPath(path);
    request.setHeaders(PlexMediaServerPrivate::PLEX_HEADERS);
    request.setHeader(
        { PlexMediaServerPrivate::PLEX_HEADER_AUTH_KEY, authenticationToken_ });

#ifdef LIBSPRING_LOG_DEBUG
    auto result = request.send();
    std::stringstream ss;
    ss << "Request:\n  Headers:";
    for (const auto &header : PlexMediaServerPrivate::PLEX_HEADERS)
    {
        ss << fmt::format("\n    {}: {}", header.first.c_str(),
                          header.second.c_str());
    }
    ss << fmt::format("\n    {}: {}",
                      PlexMediaServerPrivate::PLEX_HEADER_AUTH_KEY,
                      authenticationToken_.c_str());
    ss << fmt::format("\n  Path: {}", path);

    ss << "\n\nResponse:\n  Headers:";
    ss << fmt::format("\n    {} {}", static_cast<int>(result.status.code()),
                      result.status.name());
    for (const auto &header : result.response.headers)
    {
        ss << fmt::format("\n    {}: {}", header.first.c_str(),
                          header.second.c_str());
    }
    ss << "\n  Body:";

    //    nlohmann::json json{ std::move(
    //        nlohmann::json::parse(result.response.text)) };
    //    std::string body;
    //    nlohmann::detail::serializer<nlohmann::json> s(
    //        nlohmann::detail::output_adapter<char>(body), ' ');
    //    s.dump(json, true, false, 2, 2);
    //    body.replace(0, 2, "");
    //    body.replace(body.end() - 2, body.end(), "");
    //    ss << '\n' << body;
    //    auto log = ss.str();
    //    LOG_DEBUG("%s", log.c_str());

    return std::move(result);
#else
    return request.send();
#endif
}

PlexMediaServer::PlexMediaServer(
    const char *serverAddress,
    int32_t port,
    const char *username,
    const char *password,
    PlexMediaServer::SSLErrorHandling errorHandling) noexcept
  : priv_(std::make_shared<PlexMediaServerPrivate>(
        serverAddress,
        port,
        username,
        password,
        static_cast<bool>(errorHandling)))
{
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

std::vector<LibrarySection> PlexMediaServer::sections() const noexcept
{
    using namespace sequential_formats;

    /* TODO: Error handling */
    auto result = priv_->request(LibrarySectionPrivate::REQUEST_PATH);

    JsonFormat format{ result.response.text };
    auto mediaContainer =
        sequential::from_format<LibrarySectionPrivate::LibrarySectionContainer>(
            format);

    std::vector<LibrarySectionPrivate *> libraries;
    libraries.reserve(
        mediaContainer.get_MediaContainer().get_Directory().size());

    auto &directories = mediaContainer.get_MediaContainer().get_Directory();
    for (auto &directory : directories)
    {
        libraries.push_back(
            new LibrarySectionPrivate{ std::move(directory), priv_ });
    }

    return { libraries.begin(), libraries.end() };
}

std::string PlexMediaServer::customRequest(const char *path) const noexcept
{
    auto result = priv_->request(path);
    //    auto json = nlohmann::json::parse(result.response.text, nullptr, false);
    //    if (json.type() != nlohmann::json::value_t::null)
    //        return json.dump(2);
    //    else
    return result.response.text;
}
