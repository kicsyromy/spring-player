#ifndef SPRING_PLAYER_APPLICATION_SETTINGS_H
#define SPRING_PLAYER_APPLICATION_SETTINGS_H

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

            void set_current_page(Page page);
            Page get_current_page();
        }
    }
}

#endif // !SPRING_PLAYER_APPLICATION_SETTINGS_H
