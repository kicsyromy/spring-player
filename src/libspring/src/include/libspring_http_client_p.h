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

#ifndef LIBSPRING_HTTP_CLIENT_P_H
#define LIBSPRING_HTTP_CLIENT_P_H

#include <chrono>
#include <map>

#include <curl/curl.h>

#include <fmt/format.h>

#include "libspring_global.h"
#include "libspring_utilities_p.h"

namespace spring
{
    class HttpClient
    {
    public:
        using milliseconds_t = decltype(std::chrono::milliseconds(0));
        using http_header_t = std::pair<std::string, std::string>;
        using http_header_array_t = std::
            map<std::string, std::string, utilities::CaseInsensitiveCompare>;
        using http_port_t = std::int32_t;

    public:
        enum class RequestType
        {
            GET,
            POST
        };

        enum class SSLErrorHandling
        {
            Ignore,
            Aknowledge
        };

        struct Status
        {
            enum Code
            {
                /* SUCCESS */
                OK = 200,
                Created = 201,
                Accepted = 202,
                NonAuthoritativeInformation = 203,
                NoContent = 204,
                ResetContent = 205,
                PartialContent = 206,
                IMUsed = 226,

                /* REDIRECTION */
                MultipleChoices = 300,
                MovedPermanently = 301,
                Found = 302,
                SeeOther = 303,
                NotModified = 304,
                UseProxy = 305,
                SwitchProxy = 306,
                TemporaryRedirect = 307,
                PermanentRedirect = 308,

                /* CLIENT ERROR */
                BadRequest = 400,
                Unauthorized = 401,
                PaymentRequired = 402,
                Forbidden = 403,
                NotFound = 404,
                MethodNotAllowed = 405,
                NotAcceptable = 406,
                ProxyAuthenticationRequired = 407,
                RequestTimedOut = 408,
                Conflict = 409,
                Gone = 410,
                LengthRequired = 411,
                PreconditionFailed = 412,
                PayloadTooLarge = 413,
                URITooLong = 414,
                UnsupportedMediaType = 415,
                RangeNotSatisfiable = 416,
                ExpectationFailed = 417,
                MisdirectedRequest = 421,
                UpgradeRequired = 426,
                PreconditionRequired = 428,
                TooManyRequests = 429,
                RequestHeaderFieldsTooLarge = 431,
                UnavailableForLegalReasons = 451,

                /* SERVER ERROR */
                InternalServerError = 500,
                NotImplemented = 501,
                BadGateway = 502,
                ServiceUnavailable = 503,
                GatewayTimedOut = 504,
                HTTPVersionNotSupported = 505,
                VariantAlsoNegotiates = 506,
                NotExtended = 510,
                NetworkAuthenticationRequired = 511,

                /* Fallback */
                Unknown = 999
            };

            explicit Status(std::int32_t code) noexcept;
            explicit Status(Status::Code value) noexcept
              : Status(static_cast<std::int32_t>(value))
            {
            }
            inline Status::Code code() const noexcept { return statusCode_; }
            inline const char *name() const noexcept { return name_; }
            inline bool operator==(std::int32_t code) const noexcept
            {
                return code == static_cast<std::int32_t>(statusCode_);
            }
            inline bool operator==(Status::Code code) const noexcept
            {
                return code == statusCode_;
            }
            inline operator std::int32_t() const noexcept
            {
                return static_cast<std::int32_t>(statusCode_);
            }
            inline operator std::string() const noexcept
            {
                return fmt::format("'{} {}'", static_cast<int>(statusCode_),
                                   name_);
            }

        private:
            Status::Code statusCode_;
            const char *name_;
        };

        struct Error
        {
            enum class Code
            {
                NoError = 0,
                ConnectionFailure,
                RetryRequest,
                ConnectionClosed,
                ResponseInvalid,
                HeaderInvalid,
                HeaderEmpty,
                HostResolutionFailure,
                InternalError,
                InvalidURL,
                RequestInvalid,
                NetworkReceiveError,
                NetworkSendFailure,
                OperationTimedOut,
                ProxyResolutionFailure,
                CookieDeclined,
                SSLConnectError,
                SSLLocalCertificateError,
                SSLRemoteCertificateError,
                SSLCacertError,
                SSLRequestUnsecure,
                GenericSSLError,
                UnsupportedProtocol,
                UnknownError = 999
            };
            Error::Code errorCode;
            const char *message;
            inline operator bool() const noexcept
            {
                return errorCode != Error::Code::NoError;
            }
            inline operator std::string() const noexcept
            {
                return fmt::format("'{} {}'", static_cast<int>(errorCode),
                                   message);
            }
        };

        struct RequestResult
        {
            Status status;
            struct
            {
                http_header_array_t headers;
                std::string text;
            } response;
            double elapsed;
            Error error;
        };

    public:
        using http_request_t = RequestType;
        using http_ssl_error_handling_t = SSLErrorHandling;
        using http_status_t = Status;
        using http_error_t = Error;
        using http_request_result_t = RequestResult;

    public:
        explicit HttpClient(const std::string &userAgent) noexcept;
        HttpClient(HttpClient &&) noexcept;
        HttpClient &operator=(HttpClient &&) noexcept;
        ~HttpClient() noexcept;

    public:
        const std::string &host() const noexcept;
        void setHost(const std::string &hostname) noexcept;
        void setHost(std::string &&hostname) noexcept;

        http_port_t port() const noexcept;
        void setPort(http_port_t port) noexcept;

        std::string url() const noexcept;

        bool authenticationRequired() const noexcept;
        void enableAuthentication() noexcept;
        void disableAuthentication() noexcept;
        const std::string &username() const noexcept;
        void setUsername(const std::string &username) noexcept;
        void setUsername(std::string &&username) noexcept;

        const std::string &password() const noexcept;
        void setPassword(const std::string &password) noexcept;
        void setPassword(std::string &&password) noexcept;

        void setSSLErrorHandling(http_ssl_error_handling_t value) noexcept;

        const milliseconds_t &timeout() const noexcept;
        void setTimeout(milliseconds_t value) noexcept;

    public:
        class Request
        {
        public:
            Request(CURL *handle, std::string &&url) noexcept;
            Request(Request &&other) noexcept;
            Request &operator=(Request &&) noexcept;
            ~Request() noexcept;

        public:
            void setPath(const std::string &path) noexcept;
            void setPath(std::string &&path) noexcept;

            void setBody(const std::string &data) noexcept;
            void setBody(std::string &&data) noexcept;

            void setHeaders(const http_header_array_t &headers) noexcept;
            void setHeaders(http_header_array_t &&headers) noexcept;

            void setHeader(const http_header_t &header) noexcept;
            void setHeader(http_header_t &&header) noexcept;

        public:
            http_request_result_t send() noexcept;

            using write_callback_t = std::size_t (*)(std::uint8_t *responseData,
                                                     std::size_t responseSize,
                                                     void *userData);
            http_request_result_t send(write_callback_t callback,
                                       void *userData) noexcept;

        private:
            CURL *handle_{ nullptr };
            std::string url_{};
            std::string path_{ "/" };
            http_header_array_t headers_{};

        private:
            DISABLE_COPY(Request)
        };
        Request createRequest() const noexcept;

    private:
        std::string hostname_{};
        http_port_t port_{ -1 };
        bool authenticationEnabled_{ false };
        struct
        {
            std::string username{};
            std::string password{};
        } authentication_{};
        bool sslErrorHandlingEnabled_{ true };
        milliseconds_t timeout_{};

    private:
        CURL *handle_{ nullptr };

    private:
        DISABLE_COPY(HttpClient)
    };
}

#endif // !LIBSPRING_HTTP_CLIENT_P_H
