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

#ifndef LIBGEARBOX_ERROR_H
#define LIBGEARBOX_ERROR_H

#include <string>

#include <libspring_global.h>

namespace spring
{
    class SPRING_API Error
    {
    public:
        enum class Code
        {
            Ok = 0,
            RequestConnectionFailure,
            RequestRetry,
            RequestConnectionClosed,
            RequestResponseInvalid,
            RequestHeaderInvalid,
            RequestHeaderEmpty,
            RequestHostResolutionFailure,
            RequestInternalError,
            RequestInvalidURL,
            RequestInvalid,
            RequestNetworkReceiveError,
            RequestNetworkSendFailure,
            RequestOperationTimedOut,
            RequestProxyResolutionFailure,
            RequestCookieDeclined,
            RequestSSLConnectError,
            RequestSSLLocalCertificateError,
            RequestSSLRemoteCertificateError,
            RequestSSLCacertError,
            RequestSSLRequestUnsecure,
            RequestGenericSSLError,
            RequestUnsupportedProtocol,

            HttpOK = 200,
            HttpCreated = 201,
            HttpAccepted = 202,
            HttpNonAuthoritativeInformation = 203,
            HttpNoContent = 204,
            HttpResetContent = 205,
            HttpPartialContent = 206,
            HttpIMUsed = 226,

            HttpMultipleChoices = 300,
            HttpMovedPermanently = 301,
            HttpFound = 302,
            HttpSeeOther = 303,
            HttpNotModified = 304,
            HttpUseProxy = 305,
            HttpSwitchProxy = 306,
            HttpTemporaryRedirect = 307,
            HttpPermanentRedirect = 308,

            HttpBadRequest = 400,
            HttpUnauthorized = 401,
            HttpPaymentRequired = 402,
            HttpForbidden = 403,
            HttpNotFound = 404,
            HttpMethodNotAllowed = 405,
            HttpNotAcceptable = 406,
            HttpProxyAuthenticationRequired = 407,
            HttpRequestTimedOut = 408,
            HttpConflict = 409,
            HttpGone = 410,
            HttpLengthRequired = 411,
            HttpPreconditionFailed = 412,
            HttpPayloadTooLarge = 413,
            HttpURITooLong = 414,
            HttpUnsupportedMediaType = 415,
            HttpRangeNotSatisfiable = 416,
            HttpExpectationFailed = 417,
            HttpMisdirectedRequest = 421,
            HttpUpgradeRequired = 426,
            HttpPreconditionRequired = 428,
            HttpTooManyRequests = 429,
            HttpRequestHeaderFieldsTooLarge = 431,
            HttpUnavailableForLegalReasons = 451,

            HttpInternalServerError = 500,
            HttpNotImplemented = 501,
            HttpBadGateway = 502,
            HttpServiceUnavailable = 503,
            HttpGatewayTimedOut = 504,
            HttpHTTPVersionNotSupported = 505,
            HttpVariantAlsoNegotiates = 506,
            HttpNotExtended = 510,
            HttpNetworkAuthenticationRequired = 511,

            SpringSessionInvalid = 600,
            SpringReserved_1,
            SpringReserved_2,
            SpringReserved_3,
            SpringReserved_4,
            SpringReserved_5,
            SpringReserved_6,
            SpringReserved_7,
            SpringReserved_8,

            UnknownError = 999
        };

    public:
        Error() noexcept;
        Error(Error::Code errorCode, std::string &&message) noexcept;
        Error(Error &&other) noexcept;
        Error &operator=(Error &&other) noexcept;
        Error &operator=(std::pair<Error::Code, std::string> &&other) noexcept;

    public:
        inline Error::Code errorCode() const noexcept { return errorCode_; }
        inline const std::string &message() const noexcept { return message_; }

        inline operator bool() const noexcept { return errorCode_ != Error::Code::Ok; }

    public:
        static inline Error noError() noexcept { return {}; }

    private:
        Error::Code errorCode_;
        std::string message_;

    private:
        DISABLE_COPY(Error)
    };
} // namespace spring

#endif // LIBGEARBOX_ERROR_H
