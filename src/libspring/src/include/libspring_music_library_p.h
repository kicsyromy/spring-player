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

#ifndef LIBSPRING_MUSIC_LIBRARY_P_H
#define LIBSPRING_MUSIC_LIBRARY_P_H

#include <memory>

#include <sequential.h>

#include "libspring_global.h"

namespace spring
{
    class PlexMediaServerPrivate;

    class MusicLibraryPrivate
    {
    private:
    public:
        MusicLibraryPrivate(const std::string key,
                            std::weak_ptr<PlexMediaServerPrivate> pms) noexcept;
        ~MusicLibraryPrivate() noexcept;

    private:
        std::string key_;
        std::weak_ptr<PlexMediaServerPrivate> pms_;

    private:
        DISABLE_COPY(MusicLibraryPrivate)

    private:
        friend class MusicLibrary;
    };
}

#endif // !LIBSPRING_MUSIC_LIBRARY_P_H
