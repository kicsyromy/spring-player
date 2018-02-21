#ifndef SPRING_PLAYER_APPLICATION_SETTINGS_H
#define SPRING_PLAYER_APPLICATION_SETTINGS_H

#include <string>

namespace spring
{
    namespace player
    {
        namespace settings
        {
            enum class Page
            {
                Albums,
                Artists,
                Genres,
                Songs
            };

            void set_current_page(Page page) noexcept;
            Page get_current_page() noexcept;
            const std::string &home_directory() noexcept;
            const std::string &data_directory() noexcept;
            const std::string &config_directory() noexcept;
            const std::string &cache_directory() noexcept;
        }
    }
}

#endif // !SPRING_PLAYER_APPLICATION_SETTINGS_H
