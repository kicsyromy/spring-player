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

/*!
    \class spring::Error
    \brief Represents an error state consisting of an error code and a message

    Error is returned by any method call that makes an HTTP request. It can be
    stand-alone, or, accompanied by some data, returned by the server, wrapped
    in spring::Optional.
*/

#include "libspring_error.h"

/*!
    \enum spring::Error::Code

    \brief Error types including request failures, HTTP response errors, and library internal errors

    \var spring::Error::Ok
    \brief No error occured

    \var spring::Error::RequestConnectionFailure
    \brief There was a failure in establishing a connection to the server

    \var spring::Error::RequestRetry
    \brief The request needs to be sent once more

    \var spring::Error::RequestConnectionClosed
    \brief The connection was closed by the server

    \var spring::Error::RequestResponseInvalid
    \brief The response from the server was malformed

    \var spring::Error::RequestHeaderInvalid
    \brief One or more headers in the request were malformed. This could be due to incompatible server version

    \var spring::Error::RequestHeaderEmpty
    \brief The server sent an empty header

    \var spring::Error::RequestHostResolutionFailure
    \brief The requested host is not available

    \var spring::Error::RequestInternalError
    \brief An internal error in the library has occured

    \var spring::Error::RequestInvalidURL
    \brief The requested URL is invalid. The URL consists of the host and path

    \var spring::Error::RequestInvalid
    \brief The request is malformed

    \var spring::Error::RequestNetworkReceiveError
    \brief Could not receive any data from the server

    \var spring::Error::RequestNetworkSendFailure
    \brief Could not send any data to the server

    \var spring::Error::RequestOperationTimedOut
    \brief The request timed out

    \var spring::Error::RequestProxyResolutionFailure
    \brief Failed to connect to the system proxy

    \var spring::Error::RequestCookieDeclined
    \brief The supplied cookie was rejected by the server. With the current implementation this is not applicable

    \var spring::Error::RequestSSLConnectError
    \brief An attempt to establish a secure connection fails for reasons which cannot be expressed more specifically

    \var spring::Error::RequestSSLLocalCertificateError
    \brief A client certificate is required to authenticate an SSL connection

    \var spring::Error::RequestSSLRemoteCertificateError
    \brief The server supplied an untrusted or invalid certificate

    \var spring::Error::RequestSSLCacertError
    \brief The issuing Certificate Authority, that generated the server's certificate, is unknown

    \var spring::Error::RequestSSLRequestUnsecure
    \brief The application is moving from an SSL to an non-SSL connection because of a redirect

    \var spring::Error::RequestGenericSSLError
    \brief A place-holder error for a variety of SSL related errors

    \var spring::Error::RequestUnsupportedProtocol
    \brief The URL has an unsupported protocol

    \var spring::Error::HttpOK
    \brief Standard response for successful HTTP requests

    \var spring::Error::HttpCreated
    \brief The request has been fulfilled, resulting in the creation of a new resource

    \var spring::Error::HttpAccepted
    \brief The request has been accepted for processing, but the processing has not been completed

    \var spring::Error::HttpNonAuthoritativeInformation
    \brief The server is a transforming proxy that received a 200 OK from its origin, but is returning a modified version of the origin's response

    \var spring::Error::HttpNoContent
    \brief The server successfully processed the request and is not returning any content

    \var spring::Error::HttpResetContent
    \brief The server successfully processed the request, but is not returning any content

    \var spring::Error::HttpPartialContent
    \brief The server is delivering only part of the resource due to a range header sent by the client

    \var spring::Error::HttpIMUsed
    \brief The server has fulfilled a request for the resource, and the response is a representation of the result of one or more instance-manipulations applied to the current instance

    \var spring::Error::HttpMultipleChoices
    \brief Indicates multiple options for the resource from which the client may choose

    \var spring::Error::HttpMovedPermanently
    \brief This and all future requests should be directed to the given URI

    \var spring::Error::HttpFound
    \brief The response to the request can be found under another URI

    \var spring::Error::HttpSeeOther
    \brief The response to the request can be found under another URI

    \var spring::Error::HttpNotModified
    \brief Indicates that the resource has not been modified since the version specified by the request headers

    \var spring::Error::HttpUseProxy
    \brief The requested resource is available only through a proxy, the address for which is provided in the response

    \var spring::Error::HttpSwitchProxy
    \brief No longer used. Originally meant "Subsequent requests should use the specified proxy"

    \var spring::Error::HttpTemporaryRedirect
    \brief In this case, the request should be repeated with another URI; however, future requests should still use the original URI

    \var spring::Error::HttpPermanentRedirect
    \brief The request and all future requests should be repeated using another URI

    \var spring::Error::HttpBadRequest
    \brief The server cannot or will not process the request due to an apparent client error (e.g., malformed request syntax, too large size, invalid request message framing, or deceptive request routing)

    \var spring::Error::HttpUnauthorized
    \brief Similar to 403 Forbidden, but specifically for use when authentication is required and has failed or has not yet been provided

    \var spring::Error::HttpPaymentRequired
    \brief Reserved for future use

    \var spring::Error::HttpForbidden
    \brief The request was valid, but the server is refusing action

    \var spring::Error::HttpNotFound
    \brief The requested resource could not be found but may be available in the future

    \var spring::Error::HttpMethodNotAllowed
    \brief A request method is not supported for the requested resource; for example, a GET request on a form that requires data to be presented via POST

    \var spring::Error::HttpNotAcceptable
    \brief The requested resource is capable of generating only content not acceptable according to the Accept headers sent in the request

    \var spring::Error::HttpProxyAuthenticationRequired
    \brief The client must first authenticate itself with the proxy

    \var spring::Error::HttpRequestTimedOut
    \brief The client did not produce a request within the time that the server was prepared to wait. The client MAY repeat the request without modifications at any later time

    \var spring::Error::HttpConflict
    \brief Indicates that the request could not be processed because of conflict in the request; used by Transmission to signal an invalid session id

    \var spring::Error::HttpGone
    \brief Indicates that the resource requested is no longer available and will not be available again

    \var spring::Error::HttpLengthRequired
    \brief The request did not specify the length of its content

    \var spring::Error::HttpPreconditionFailed
    \brief The server does not meet one of the preconditions that the requester put on the request

    \var spring::Error::HttpPayloadTooLarge
    \brief The request is larger than the server is willing or able to process

    \var spring::Error::HttpURITooLong
    \brief The URI provided was too long for the server to process

    \var spring::Error::HttpUnsupportedMediaType
    \brief The request entity has a media type which the server or resource does not support

    \var spring::Error::HttpRangeNotSatisfiable
    \brief The client has asked for a portion of the file, but the server cannot supply that portion

    \var spring::Error::HttpExpectationFailed
    \brief The server cannot meet the requirements of the Expect request-header field

    \var spring::Error::HttpMisdirectedRequest
    \brief The server cannot meet the requirements of the Expect request-header field

    \var spring::Error::HttpUpgradeRequired
    \brief The client should switch to a different protocol such as TLS/1.0, given in the Upgrade header field

    \var spring::Error::HttpPreconditionRequired
    \brief The origin server requires the request to be conditional

    \var spring::Error::HttpTooManyRequests
    \brief The user has sent too many requests in a given amount of time

    \var spring::Error::HttpRequestHeaderFieldsTooLarge
    \brief The server is unwilling to process the request because either an individual header field, or all the header fields collectively, are too large

    \var spring::Error::HttpUnavailableForLegalReasons
    \brief A server operator has received a legal demand to deny access to a resource or to a set of resources that includes the requested resource

    \var spring::Error::HttpInternalServerError
    \brief A generic error message, given when an unexpected condition was encountered and no more specific message is suitable

    \var spring::Error::HttpNotImplemented
    \brief The server either does not recognize the request method, or it lacks the ability to fulfill the request

    \var spring::Error::HttpBadGateway
    \brief The server was acting as a gateway or proxy and received an invalid response from the upstream server

    \var spring::Error::HttpServiceUnavailable
    \brief The server is currently unavailable (because it is overloaded or down for maintenance)

    \var spring::Error::HttpGatewayTimedOut
    \brief The server was acting as a gateway or proxy and did not receive a timely response from the upstream server

    \var spring::Error::HttpHTTPVersionNotSupported
    \brief The server does not support the HTTP protocol version used in the request

    \var spring::Error::HttpVariantAlsoNegotiates
    \brief Transparent content negotiation for the request results in a circular reference

    \var spring::Error::HttpNotExtended
    \brief The server is unable to store the representation needed to complete the request

    \var spring::Error::HttpNetworkAuthenticationRequired
    \brief The client needs to authenticate to gain network access

    \var spring::Error::SpringSessionInvalid
    \brief The spring::PlexMediaServer used is invalid; this could mean that the session was
           destroyed before or during a request

    \var spring::Error::SpringReserved_1
    \brief Reserved for future use

    \var spring::Error::SpringReserved_2
    \brief Reserved for future use

    \var spring::Error::SpringReserved_3
    \brief Reserved for future use

    \var spring::Error::SpringReserved_4
    \brief Reserved for future use

    \var spring::Error::SpringReserved_5
    \brief Reserved for future use

    \var spring::Error::SpringReserved_6
    \brief Reserved for future use

    \var spring::Error::SpringReserved_7
    \brief Reserved for future use

    \var spring::Error::SpringReserved_8
    \brief Reserved for future use

    \var spring::Error::UnknownError
    \brief An unknown/unhandled error has occured
*/

using namespace spring;

/*!
    Constructs an instance of spring::Error
*/
Error::Error() noexcept
  : errorCode_(Error::Code::Ok)
  , message_()
{
}

/*!
    Constructs an instance of spring::Error with the supplied spring::Error::Code and message
*/
Error::Error(Error::Code errorCode, std::string &&message) noexcept
  : errorCode_(errorCode)
  , message_(message)
{
}

/*!
    Move constructor
*/
Error::Error(Error &&other) noexcept
  : errorCode_(other.errorCode_)
  , message_(std::move(other.message_))
{
}

/*!
    Move asignment operator
*/
Error &Error::operator=(Error &&other) noexcept
{
    errorCode_ = other.errorCode_;
    message_ = std::move(other.message_);

    return *this;
}

/*!
    Initializes an existing instance of spring::Error from a pair of spring::Error::Code and a message
*/
Error &Error::operator=(std::pair<Error::Code, std::string> &&other) noexcept
{
    errorCode_ = other.first;
    message_ = std::move(other.second);

    return *this;
}

/*!
    \fn gearbox::Error::errorCode() const noexcept

    Returns the spring::Error::Code from the instance of spring::Error
*/

/*!
    \fn gearbox::Error::message() const noexcept

    Returns a human readable message from the instance of spring::Error, that describes the error
*/

/*!
    \fn spring::Error::operator bool() const

    Implicit covnversion operator to check whether there is an error or not
*/
