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

#include "libspring_http_client_p.h"

#include <algorithm>
#include <cctype>
#include <iostream>

#include "libspring_logger_p.h"
#include "libspring_vla_p.h"

using namespace spring;
using namespace std::chrono_literals;

namespace
{
    struct CUrlInitializer
    {
        CUrlInitializer() { curl_global_init(CURL_GLOBAL_ALL); }
        ~CUrlInitializer() { curl_global_cleanup(); }
    };
    CUrlInitializer cUrlInitializer;

    HttpClient::Error fromCUrlError(std::int32_t errorCode)
    {
        HttpClient::Error err;
        switch (errorCode)
        {
            default:
                err.errorCode = HttpClient::Error::Code::UnknownError;
                err.message = "An unknown error has occured";
                break;
            case CURLE_TOO_MANY_REDIRECTS:
            case CURLE_OK:
                err.errorCode = HttpClient::Error::Code::NoError;
                err.message = "";
                break;
            case CURLE_UNSUPPORTED_PROTOCOL:
                err.errorCode = HttpClient::Error::Code::UnsupportedProtocol;
                break;
            case CURLE_URL_MALFORMAT:
                err.errorCode = HttpClient::Error::Code::InvalidURL;
                break;
            case CURLE_COULDNT_RESOLVE_PROXY:
                err.errorCode = HttpClient::Error::Code::ProxyResolutionFailure;
                break;
            case CURLE_COULDNT_RESOLVE_HOST:
                err.errorCode = HttpClient::Error::Code::HostResolutionFailure;
                break;
            case CURLE_COULDNT_CONNECT:
                err.errorCode = HttpClient::Error::Code::ConnectionFailure;
                break;
            case CURLE_OPERATION_TIMEDOUT:
                err.errorCode = HttpClient::Error::Code::OperationTimedOut;
                break;
            case CURLE_SSL_CONNECT_ERROR:
                err.errorCode = HttpClient::Error::Code::SSLConnectError;
                break;
            case CURLE_PEER_FAILED_VERIFICATION:
                err.errorCode =
                    HttpClient::Error::Code::SSLRemoteCertificateError;
                break;
            case CURLE_GOT_NOTHING:
                err.errorCode = HttpClient::Error::Code::ResponseInvalid;
                break;
            case CURLE_SSL_ENGINE_NOTFOUND:
                err.errorCode = HttpClient::Error::Code::GenericSSLError;
                break;
            case CURLE_SSL_ENGINE_SETFAILED:
                err.errorCode = HttpClient::Error::Code::GenericSSLError;
                break;
            case CURLE_SEND_ERROR:
                err.errorCode = HttpClient::Error::Code::NetworkSendFailure;
                break;
            case CURLE_RECV_ERROR:
                err.errorCode = HttpClient::Error::Code::NetworkReceiveError;
                break;
            case CURLE_SSL_CERTPROBLEM:
                err.errorCode =
                    HttpClient::Error::Code::SSLLocalCertificateError;
                break;
            case CURLE_SSL_CIPHER:
                err.errorCode = HttpClient::Error::Code::GenericSSLError;
                break;
            case CURLE_SSL_CACERT:
                err.errorCode = HttpClient::Error::Code::SSLCacertError;
                break;
            case CURLE_USE_SSL_FAILED:
                err.errorCode = HttpClient::Error::Code::GenericSSLError;
                break;
            case CURLE_SSL_ENGINE_INITFAILED:
                err.errorCode = HttpClient::Error::Code::GenericSSLError;
                break;
            case CURLE_SSL_CACERT_BADFILE:
                err.errorCode = HttpClient::Error::Code::SSLCacertError;
                break;
            case CURLE_SSL_SHUTDOWN_FAILED:
                err.errorCode = HttpClient::Error::Code::GenericSSLError;
                break;
            case CURLE_SSL_CRL_BADFILE:
                err.errorCode = HttpClient::Error::Code::SSLCacertError;
                break;
            case CURLE_SSL_ISSUER_ERROR:
                err.errorCode = HttpClient::Error::Code::SSLCacertError;
                break;
        }

        if ((err.errorCode != HttpClient::Error::Code::NoError) &&
            (err.errorCode != HttpClient::Error::Code::UnknownError))
            err.message = curl_easy_strerror(static_cast<CURLcode>(errorCode));

        return err;
    }

    /* Appends a buffer of length size * nmemb to data. This function is called by CUrl for */
    /* subsequent chunks of data that constitute the response for a request.                */
    template <typename CallbackData>
    std::size_t writeCallback(void *data,
                              std::size_t size,
                              std::size_t nmemb,
                              CallbackData *callback)
    {
        return callback->function(reinterpret_cast<std::uint8_t *>(data),
                                  size * nmemb, callback->userData);
    }

    /* Appends a key-value pair, represented by ptr, of length size * nmemb, to the data map. */
    /* This function is called by CUrl for each line in the header of a response.             */
    std::size_t headerCallback(void *ptr,
                               std::size_t size,
                               std::size_t nmemb,
                               HttpClient::http_header_array_t *data)
    {
        std::string header(static_cast<char *>(ptr), size * nmemb);
        auto found = header.find("HTTP/");
        if (found == std::string::npos)
        {
            auto separatorPos = header.find_first_of(':');
            if (separatorPos != std::string::npos)
            {
                auto value = header.substr(separatorPos + 2);
                (*data)[header.substr(0, separatorPos)] =
                    value.substr(0, value.size() - 2);
                /*                  ^^^^^^^^^^^^^^^^          */
                /* Ignore CL/RF at the end of the header line */
            }
        }
        else
        {
            data->clear();
        }
        return size * nmemb;
    }
}

HttpClient::Status::Status(std::int32_t code) noexcept
  : statusCode_(Unknown)
  , name_(nullptr)
{
    switch (code)
    {
        default:
            name_ = "Unknown";
            break;
        case 200:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "OK";
            break;
        case 201:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Created";
            break;
        case 202:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Accepted";
            break;
        case 203:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Non-Authoritative Information";
            break;
        case 204:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "No Content";
            break;
        case 205:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Reset Content";
            break;
        case 206:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Partial Content";
            break;
        case 226:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "IM Used";
            break;
        case 300:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Multiple Choices";
            break;
        case 301:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Moved Permanently";
            break;
        case 302:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Found";
            break;
        case 303:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "See Other";
            break;
        case 304:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Not Modified";
            break;
        case 305:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Use Proxy";
            break;
        case 306:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Switch Proxy";
            break;
        case 307:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Temporary Redirect";
            break;
        case 308:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Permanent Redirect";
            break;
        case 400:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Bad Request";
            break;
        case 401:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Unauthorized";
            break;
        case 402:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Payment Required";
            break;
        case 403:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Forbidden";
            break;
        case 404:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Not Found";
            break;
        case 405:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Method Not Allowed";
            break;
        case 406:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Not Acceptable";
            break;
        case 407:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Proxy Authentication Required";
            break;
        case 408:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Request Time-Out";
            break;
        case 409:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Conflict";
            break;
        case 410:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Gone";
            break;
        case 411:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Length Required";
            break;
        case 412:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Precondition Failed";
            break;
        case 413:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Payload Too Large";
            break;
        case 414:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "URI Too Long";
            break;
        case 415:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Unsoported Media Type";
            break;
        case 416:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Range Not Satisfiable";
            break;
        case 417:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Expectation Failed";
            break;
        case 421:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Misdirected Request";
            break;
        case 426:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Upgrade Required";
            break;
        case 428:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Precondition Required";
            break;
        case 429:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Too Many Requests";
            break;
        case 431:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Request Header Fields Too Large";
            break;
        case 451:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Unavailable For Legal Reasons";
            break;
        case 500:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Internal Server Error";
            break;
        case 501:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Not Implemented";
            break;
        case 502:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Bad Gateway";
            break;
        case 503:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Service Unavailable";
            break;
        case 504:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Gateway Time-Out";
            break;
        case 505:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "HTTP Version Not Supported";
            break;
        case 506:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Variant Also Negotiates";
            break;
        case 510:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Not Extended";
            break;
        case 511:
            statusCode_ = static_cast<Status::Code>(code);
            name_ = "Network Authentication Required";
            break;
    }
}

HttpClient::HttpClient(const std::string &userAgent) noexcept
{
    handle_ = curl_easy_init();
    if (handle_ != nullptr)
    {
        curl_easy_setopt(handle_, CURLOPT_USERAGENT, userAgent.c_str());
        curl_easy_setopt(handle_, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(handle_, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(handle_, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(handle_, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(handle_, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(handle_, CURLOPT_HEADERFUNCTION, &headerCallback);
    }
    else
    {
        LOG_FATAL("Error %s while initiation connection.",
                  static_cast<std::string>(fromCUrlError(-1)).c_str());
    }
}

HttpClient::HttpClient(HttpClient &&) noexcept = default;
HttpClient &HttpClient::operator=(HttpClient &&other) noexcept
{
    hostname_ = std::move(other.hostname_);
    port_ = other.port_;
    authenticationEnabled_ = other.authenticationEnabled_;
    authentication_ = std::move(other.authentication_);
    sslErrorHandlingEnabled_ = other.sslErrorHandlingEnabled_;
    timeout_ = other.timeout_;
    handle_ = other.handle_;
    other.handle_ = nullptr;

    return *this;
}

HttpClient::~HttpClient() noexcept
{
    if (handle_ != nullptr)
    {
        curl_easy_cleanup(handle_);
    }
}

const std::string &HttpClient::host() const noexcept
{
    return hostname_;
}

void HttpClient::setHost(const std::string &hostname) noexcept
{
    hostname_ = hostname;
}

void HttpClient::setHost(std::string &&hostname) noexcept
{
    hostname_ = std::move(hostname);
}

HttpClient::http_port_t HttpClient::port() const noexcept
{
    return port_;
}

void HttpClient::setPort(http_port_t port) noexcept
{
    port_ = port;
}

std::string HttpClient::url() const noexcept
{
    return fmt::format("{}{}", hostname_,
                       port_ > 0 ? fmt::format(":{}", port_) : "");
}

bool HttpClient::authenticationRequired() const noexcept
{
    return authenticationEnabled_;
}

void HttpClient::enableAuthentication() noexcept
{
    authenticationEnabled_ = true;
}

void HttpClient::disableAuthentication() noexcept
{
    authenticationEnabled_ = false;
}

const std::string &HttpClient::username() const noexcept
{
    return authentication_.username;
}

void HttpClient::setUsername(const std::string &username) noexcept
{
    authenticationEnabled_ = true;
    authentication_.username = username;
}

void HttpClient::setUsername(std::string &&username) noexcept
{
    authenticationEnabled_ = true;
    authentication_.username = std::move(username);
}

const std::string &HttpClient::password() const noexcept
{
    return authentication_.password;
}

void HttpClient::setPassword(const std::string &password) noexcept
{
    authenticationEnabled_ = true;
    authentication_.password = password;
}

void HttpClient::setPassword(std::string &&password) noexcept
{
    authenticationEnabled_ = true;
    authentication_.password = std::move(password);
}

void HttpClient::setSSLErrorHandling(http_ssl_error_handling_t value) noexcept
{
    sslErrorHandlingEnabled_ = (value == http_ssl_error_handling_t::Aknowledge);
    if (!sslErrorHandlingEnabled_)
    {
        curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    else
    {
        curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYHOST, 1L);
    }
}

const HttpClient::milliseconds_t &HttpClient::timeout() const noexcept
{
    return timeout_;
}

void HttpClient::setTimeout(milliseconds_t value) noexcept
{
    timeout_ = value;
    curl_easy_setopt(handle_, CURLOPT_TIMEOUT_MS, timeout_.count());
}

HttpClient::Request::Request(CURL *handle, std::string &&url) noexcept
  : handle_(handle)
  , url_(std::move(url))
{
}

HttpClient::Request::Request(Request &&other) noexcept
  : handle_(other.handle_)
  , url_(std::move(other.url_))
  , path_(std::move(other.path_))
  , headers_(std::move(other.headers_))
{
}

HttpClient::Request &HttpClient::Request::operator=(Request &&other) noexcept
{
    handle_ = other.handle_;
    other.handle_ = nullptr;
    url_ = std::move(other.url_);
    path_ = std::move(other.path_);
    headers_ = std::move(other.headers_);
}

HttpClient::Request::~Request() noexcept
{
    if (handle_ != nullptr)
    {
        curl_easy_cleanup(handle_);
    }
}

void HttpClient::Request::setPath(const std::string &path) noexcept
{
    path_ = path;
}
void HttpClient::Request::setPath(std::string &&path) noexcept
{
    path_ = std::move(path);
}

void spring::HttpClient::Request::setBody(const std::string &data) noexcept
{
    curl_easy_setopt(handle_, CURLOPT_POSTFIELDSIZE, data.size());
    curl_easy_setopt(handle_, CURLOPT_COPYPOSTFIELDS, data.c_str());
}

void HttpClient::Request::setBody(std::string &&data) noexcept
{
    setBody(static_cast<const std::string &>(data));
}

void spring::HttpClient::Request::setHeaders(
    const HttpClient::http_header_array_t &headers) noexcept
{
    for (const auto &header : headers)
    {
        headers_[header.first] = header.second;
    }
}

void HttpClient::Request::setHeaders(
    HttpClient::http_header_array_t &&headers) noexcept
{
    for (auto &header : headers)
    {
        headers_[std::move(header.first)] = std::move(header.second);
    }
}

void spring::HttpClient::Request::setHeader(
    const HttpClient::http_header_t &header) noexcept
{
    headers_[header.first] = header.second;
}

void HttpClient::Request::setHeader(HttpClient::http_header_t &&header) noexcept
{
    headers_[std::move(header.first)] = std::move(header.second);
}

HttpClient::http_request_result_t HttpClient::Request::send() noexcept
{
    std::string text{};
    auto response = send(
        [](std::uint8_t *responseData, std::size_t responseSize,
           void *userData) -> std::size_t {
            auto text = static_cast<std::string *>(userData);
            text->append(reinterpret_cast<char *>(responseData), responseSize);
            return responseSize;
        },
        &text);

    if (text.back() == '\n')
    {
        text = text.substr(0, text.size() - 1);
    }

    response.response.text = std::move(text);
    return response;
}

HttpClient::http_request_result_t HttpClient::Request::send(
    write_callback_t callback, void *userData) noexcept
{
    std::int32_t httpStatus{ HttpClient::Status::Unknown };
    http_header_array_t responseHeaders{};
    http_error_t err = { HttpClient::Error::Code::InternalError,
                         "An internal connection handle is invalid. "
                         "This is most likely due to operating an a moved or "
                         "otherwise invalidated "
                         "instance of this object" };
    double elapsed = 0;

    struct CallbackData
    {
        write_callback_t function;
        void *userData;
    } callbackData{ callback, userData };

    if (handle_ != nullptr)
    {
        std::string url = fmt::format("{}{}", url_, path_);
        curl_easy_setopt(handle_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION,
                         &writeCallback<CallbackData>);

        curl_slist *headers = nullptr;
        std::vector<std::string> formatedHeaders;

        for (const auto &header : headers_)
        {
            formatedHeaders.emplace_back(
                fmt::format("{}: {}", header.first, header.second));
            headers =
                curl_slist_append(headers, formatedHeaders.back().c_str());
        }
        curl_easy_setopt(handle_, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(handle_, CURLOPT_WRITEDATA, &callbackData);
        curl_easy_setopt(handle_, CURLOPT_HEADERDATA, &responseHeaders);

        auto errCode = curl_easy_perform(handle_);
        err = fromCUrlError(errCode);

        curl_easy_getinfo(handle_, CURLINFO_RESPONSE_CODE, &httpStatus);
        curl_easy_getinfo(handle_, CURLINFO_TOTAL_TIME, &elapsed);

        curl_slist_free_all(headers);
    }

    return { http_status_t(httpStatus),
             { std::move(responseHeaders), "" },
             elapsed,
             err };
}

HttpClient::Request HttpClient::createRequest() const noexcept
{
    auto curl = curl_easy_duphandle(handle_);

    if (authenticationEnabled_)
    {
        curl_easy_setopt(curl, CURLOPT_USERPWD,
                         fmt::format("{}:{}", authentication_.username,
                                     authentication_.password)
                             .c_str());
    }

    return { curl, url() };
}
