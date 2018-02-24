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

#ifndef LIBSPRING_GLOBAL_H
#define LIBSPRING_GLOBAL_H

#include <cstdint>

#include <libspring_config.h>

#ifdef _MSC_VER
#define EXPORT_SYMBOL __declspec(dllexport)
#define IMPORT_SYMBOL __declspec(dllimport)
#elif defined(__GNUC__)
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#define IMPORT_SYMBOL
#else
#define EXPORT_SYMBOL
#define IMPORT_SYMBOL
#pragma warning Unknown dynamic link import / export semantics.
#endif

#ifndef LIBSPRING_NO_IMPORT_SEMANTICS
#ifdef GEARBOX_LIB
#define SPRING EXPORT_SYMBOL
#else
#define SPRING IMPORT_SYMBOL
#endif
#else
#define SPRING_API
#endif

#define DEFAULT_COPY(klass)                                                    \
    klass(const klass &) noexcept = default;                                   \
    klass &operator=(const klass &) noexcept = default;

#define DISABLE_COPY(klass)                                                    \
    klass(const klass &) noexcept = delete;                                    \
    klass &operator=(const klass &) noexcept = delete;

#define DEFAULT_MOVE(klass)                                                    \
    klass(klass &&) noexcept = default;                                        \
    klass &operator=(klass &&) noexcept = default;

#define DISABLE_MOVE(klass)                                                    \
    klass(klass &&) noexcept = delete;                                         \
    klass &operator=(klass &&) noexcept = delete;

namespace spring
{
    using DataFragmentReadyCallback = std::size_t (*)(
        std::uint8_t *responseData, std::size_t responseSize, void *userData);
}

#endif // LIBSPRING_GLOBAL_H
