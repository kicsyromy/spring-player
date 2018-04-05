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

#ifndef LIBSPRING_OPTIONAL_H
#define LIBSPRING_OPTIONAL_H

#include <libspring_error.h>
#include <libspring_global.h>

namespace spring
{
    template <typename T> struct Optional
    {
        Optional(Error &&error, T &&value) noexcept
          : error(std::move(error))
          , value(std::move(value))
        {
        }

        ~Optional() noexcept = default;

        Optional(Optional &&other) noexcept
        {
            error = std::move(other.error);
            value = std::move(other.value);
        }

        Optional &operator=(Optional &&other) noexcept
        {
            error = std::move(other.error);
            value = std::move(other.value);
        }

        constexpr operator T() noexcept { return std::move(value); }

        Error error;
        T value;

    private:
        DISABLE_COPY(Optional)
    };
} // namespace spring

#endif // LIBGEARBOX_RETURN_TYPE_H
