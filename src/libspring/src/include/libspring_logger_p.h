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

#ifndef LIBSPRING_LOGGER_H
#define LIBSPRING_LOGGER_H

#include <cstdio>
#include <cstring>

constexpr const std::size_t BUFFER_SIZE{ 8192 };
extern thread_local char logbuffer[BUFFER_SIZE + 1];

#ifdef _MSC_VER
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif // _MSC_VER

#define LOG(...)                                                               \
    do                                                                         \
    {                                                                          \
        snprintf(logbuffer, BUFFER_SIZE, __VA_ARGS__);                         \
        fprintf(stderr, "INFO: %s: %d: %s:\n%s\n",                             \
                strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__, __func__,     \
                logbuffer);                                                    \
    } while (0)

#define LOG_INFO(...) LOG(__VA_ARGS__)

#define LOG_WARN(...)                                                          \
    do                                                                         \
    {                                                                          \
        snprintf(logbuffer, BUFFER_SIZE, __VA_ARGS__);                         \
        fprintf(stderr, "WARNING: %s: %d: %s:\n%s\n",                          \
                strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__, __func__,     \
                logbuffer);                                                    \
    } while (0)

#define LOG_ERROR(...)                                                         \
    do                                                                         \
    {                                                                          \
        snprintf(logbuffer, BUFFER_SIZE, __VA_ARGS__);                         \
        fprintf(stderr, "ERROR: %s: %d: %s:\n%s\n",                            \
                strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__, __func__,     \
                logbuffer);                                                    \
    } while (0)

#define LOG_FATAL(...)                                                         \
    do                                                                         \
    {                                                                          \
        snprintf(logbuffer, BUFFER_SIZE, __VA_ARGS__);                         \
        fprintf(stderr, "FATAL: %s: %d: %s:\n%s\n",                            \
                strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__, __func__,     \
                logbuffer);                                                    \
        std::terminate();                                                      \
    } while (0)

#ifdef LIBSPRING_LOG_DEBUG
#define LOG_DEBUG(...)                                                         \
    do                                                                         \
    {                                                                          \
        snprintf(logbuffer, BUFFER_SIZE, __VA_ARGS__);                         \
        fprintf(stderr, "DEBUG: %s: %d: %s:\n%s\n",                            \
                strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__, __func__,     \
                logbuffer);                                                    \
    } while (0)
#else
#define LOG_DEBUG(...)
#endif // LIBSPRING_LOG_DEBUG

#endif // LIBSPRING_LOGGER_H
