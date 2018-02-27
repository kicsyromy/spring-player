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
#include <cstdlib>

#include <fmt/format.h>

#ifdef _MSC_VER
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif // _MSC_VER

#define LOG(...)                                                               \
    do                                                                         \
    {                                                                          \
        fmt::MemoryWriter output;                                              \
        output.write("   INFO: {:<20}: {:<4} ** {:<20} **: ",                  \
                     strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__,          \
                     __func__);                                                \
        output.write(__VA_ARGS__);                                             \
        output.write("\n");                                                    \
        std::fputs(output.c_str(), stderr);                                    \
    } while (0)

#define LOG_INFO(...) LOG(__VA_ARGS__)

#define LOG_WARN(...)                                                          \
    do                                                                         \
    {                                                                          \
        fmt::MemoryWriter output;                                              \
        output.write("WARNING: {:<20}: {:<4} ** {:<20} **: ",                  \
                     strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__,          \
                     __func__);                                                \
        output.write(__VA_ARGS__);                                             \
        output.write("\n");                                                    \
        std::fputs(output.c_str(), stderr);                                    \
    } while (0)

#define LOG_ERROR(...)                                                         \
    do                                                                         \
    {                                                                          \
        fmt::MemoryWriter output;                                              \
        output.write("  ERROR: {:<20}: {:<4} ** {:<20} **: ",                  \
                     strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__,          \
                     __func__);                                                \
        output.write(__VA_ARGS__);                                             \
        output.write("\n");                                                    \
        std::fputs(output.c_str(), stderr);                                    \
    } while (0)

#define LOG_FATAL(...)                                                         \
    do                                                                         \
    {                                                                          \
        fmt::MemoryWriter output;                                              \
        output.write("  FATAL: {:<20}: {:<4} ** {:<20} **: ",                  \
                     strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__,          \
                     __func__);                                                \
        output.write(__VA_ARGS__);                                             \
        output.write("\n");                                                    \
        std::fputs(output.c_str(), stderr);                                    \
        std::exit(EXIT_FAILURE);                                               \
    } while (0)

#ifdef LIBSPRING_LOG_DEBUG
#define LOG_DEBUG(...)                                                         \
    do                                                                         \
    {                                                                          \
        fmt::MemoryWriter output;                                              \
        output.write("  DEBUG: {:<20}: {:<4} ** {:<20} **: ",                  \
                     strrchr(__FILE__, PATH_SEPARATOR) + 1, __LINE__,          \
                     __func__);                                                \
        output.write(__VA_ARGS__);                                             \
        output.write("\n");                                                    \
        std::fputs(output.c_str(), stderr);                                    \
    } while (0)
#else
#define LOG_DEBUG(...)
#endif // LIBSPRING_LOG_DEBUG

#endif // LIBSPRING_LOGGER_H
