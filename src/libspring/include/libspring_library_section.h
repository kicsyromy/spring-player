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

#ifndef LIBSPRING_LIBRARY_SECTION_H
#define LIBSPRING_LIBRARY_SECTION_H

#include <memory>
#include <string>

#include <libspring_global.h>
#include <libspring_media_library.h>

namespace spring
{
    class LibrarySectionPrivate;

    class LibrarySection
    {
    public:
        using Type = MediaLibrary::Type;

    public:
        LibrarySection(LibrarySectionPrivate *priv) noexcept;
        ~LibrarySection() noexcept;

    public:
        std::string title() const noexcept;
        Type type() const noexcept;
        MediaLibrary content() const noexcept;

    private:
        std::unique_ptr<LibrarySectionPrivate> priv_;

    private:
        DISABLE_COPY(LibrarySection)
    };
}

#endif // !LIBSPRING_LIBRARY_SECTION_H
