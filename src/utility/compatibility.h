#ifndef SPRING_PLAYER_UTILITY_COMPATIBITLITY_H
#define SPRING_PLAYER_UTILITY_COMPATIBITLITY_H

#if __has_include(<string_view>)
#include <string_view>
#define SPRING_PLAYER_STRING_VIEW_T std::string_view
#elif __has_include(<experimental/string_view>)
#include <experimental/string_view>
#define SPRING_PLAYER_STRING_VIEW_T std::experimental::string_view
#else
#error Missing string_view header
#define SPRING_PLAYER_STRING_VIEW_T
#endif

namespace spring
{
    namespace player
    {
        namespace utility
        {
            using string_view = SPRING_PLAYER_STRING_VIEW_T;
        }
    } // namespace player
} // namespace spring
namespace std
{
}

#endif // !SPRING_PLAYER_UTILITY_COMPATIBITLITY_H
