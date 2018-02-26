#ifndef SPRING_PLAYER_UTILITY_COMPATIBITLITY_H
#define SPRING_PLAYER_UTILITY_COMPATIBITLITY_H

#if __has_include(<string_view>)
#include <string_view>
#elif __has_include(<experimental/string_view>)
#include <experimental/string_view>
#else
#error Missing string_view header
#endif

namespace spring
{
    namespace player
    {
        namespace utility
        {
#if __has_include(<string_view>)
            using string_view = std::string_view;
#else
            using string_view = std::experimental::string_view;
#endif
        }
    }
}
namespace std
{
}

#endif // !SPRING_PLAYER_UTILITY_COMPATIBITLITY_H
