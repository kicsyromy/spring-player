#include "application_settings.h"

#include <array>

#include <gtk/gtk.h>

using namespace spring;
using namespace spring::player;

namespace
{
    struct SettingsInitializer
    {
        SettingsInitializer() noexcept
          : handle_(g_settings_new(APPLICATION_ID))
        {
        }
        ~SettingsInitializer() noexcept { g_object_unref(handle_); }
        operator GSettings *() noexcept { return handle_; }
        GSettings *handle_{ nullptr };
    } app_settings;

    enum Properties : std::size_t
    {
        PropertyCurrentPage,
        PropertyCount
    };

    const std::array<const char *, PropertyCount> properties{ "current-page" };
}

void settings::set_current_page(settings::Page page)
{
    g_settings_set_enum(app_settings, properties[PropertyCurrentPage],
                        static_cast<gint>(page));
}

settings::Page settings::get_current_page()
{
    return static_cast<settings::Page>(
        g_settings_get_enum(app_settings, properties[PropertyCurrentPage]));
}
