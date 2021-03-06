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

#ifndef LIBSPRING_PLEX_MEDIA_SERVER_H
#define LIBSPRING_PLEX_MEDIA_SERVER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include <libspring_error.h>
#include <libspring_global.h>
#include <libspring_library_section.h>
#include <libspring_optional.h>

namespace spring
{
    class PlexMediaServerPrivate;

    class PlexMediaServer
    {
    public:
        enum class SSLErrorHandling
        {
            Ignore,
            Acknowledge
        };

    public:
        PlexMediaServer() noexcept;
        ~PlexMediaServer() noexcept;

        PlexMediaServer(PlexMediaServer &&other) noexcept;
        PlexMediaServer &operator=(PlexMediaServer &&other) noexcept;

    public:
        Optional<const char *> connect(
            const char *serverAddress,
            std::int32_t port,
            const char *username,
            const char *password,
            SSLErrorHandling errorHandling = SSLErrorHandling::Acknowledge) noexcept;

        Error connect(const char *serverAddress,
                      std::int32_t port,
                      const char *token,
                      SSLErrorHandling errorHandling = SSLErrorHandling::Acknowledge) noexcept;

        const std::string &name() const noexcept;

        std::vector<LibrarySection> sections() const noexcept;
        std::string customRequest(const char *path) const noexcept;

    private:
        std::shared_ptr<PlexMediaServerPrivate> priv_;

    private:
        DISABLE_COPY(PlexMediaServer)
    };
} // namespace spring

#endif // !LIBSPRING_PLEX_MEDIA_SERVER_H
