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
                Songs,
                Count
            };

            void set_current_page(Page page) noexcept;
            Page get_current_page() noexcept;
            const std::string &home_directory() noexcept;
            const std::string &data_directory() noexcept;
            const std::string &config_directory() noexcept;
            const std::string &cache_directory() noexcept;
        } // namespace settings
    }     // namespace player
} // namespace spring

#endif // !SPRING_PLAYER_APPLICATION_SETTINGS_H
