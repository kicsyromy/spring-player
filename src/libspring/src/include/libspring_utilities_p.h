#ifndef LIBSPRING_UTILITIES_H
#define LIBSPRING_UTILITIES_H

#include <string>
#include <type_traits>

namespace spring
{
    namespace utilities
    {
        struct CaseInsensitiveCompare
        {
            bool operator()(const std::string &s1, const std::string &s2) const;
            bool operator()(const char *s1, const char *s2) const;
        };
    }
}

#endif // !LIBSPRING_UTILITIES_H
