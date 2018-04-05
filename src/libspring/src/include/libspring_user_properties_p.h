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

#ifndef LIBSPRING_USER_P_H
#define LIBSPRING_USER_P_H

#include <cstdint>
#include <string>
#include <vector>

#include <sequential.h>

namespace spring
{
    class UserProperties
    {
        struct properties_t
        {
            struct subscription_t
            {
                ATTRIBUTE(bool, active)
                ATTRIBUTE(std::string, status)
                ATTRIBUTE(std::string, plan)
                ATTRIBUTE(std::vector<std::string>, features)
                INIT_ATTRIBUTES(active, status, plan, features)
            };

            struct roles_t
            {
                ATTRIBUTE(std::vector<std::string>, roles)
                INIT_ATTRIBUTES(roles)
            };

            ATTRIBUTE(std::int32_t, id)
            ATTRIBUTE(std::string, uuid)
            ATTRIBUTE(std::string, email)
            ATTRIBUTE(std::string, joined_at)
            ATTRIBUTE(std::string, username)
            ATTRIBUTE(std::string, title)
            ATTRIBUTE(std::string, thumb)
            ATTRIBUTE(bool, hasPassword)
            ATTRIBUTE(std::string, authToken)
            ATTRIBUTE(std::string, authentication_token)
            ATTRIBUTE(subscription_t, subscription)
            ATTRIBUTE(roles_t, roles)
            ATTRIBUTE(std::vector<std::string>, entitlements)
            ATTRIBUTE(std::string, confirmedAt)
            ATTRIBUTE(std::int32_t, forumId)
            ATTRIBUTE(bool, rememberMe)
            INIT_ATTRIBUTES(id,
                            uuid,
                            email,
                            joined_at,
                            username,
                            title,
                            thumb,
                            hasPassword,
                            authToken,
                            authentication_token,
                            subscription,
                            roles,
                            entitlements,
                            confirmedAt,
                            forumId,
                            rememberMe)
        };

        ATTRIBUTE(properties_t, user)
        INIT_ATTRIBUTES(user)
    };
} // namespace spring

#endif // !LIBSPRING_USER_P_H
