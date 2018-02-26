#include "application_settings.h"

#include <array>

#include <gtk/gtk.h>

#include <fmt/format.h>

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

    const std::string home_directory = []() -> std::string {
        auto HOME = getenv("HOME");
        return { HOME };
    }();
    const std::string data_directory = []() -> std::string {
        auto XDG_DATA_HOME = getenv("XDG_DATA_HOME");
        if (XDG_DATA_HOME != nullptr)
        {
            return { XDG_DATA_HOME };
        }
        else
        {
            return fmt::format("{}/.local/share", home_directory);
        }
    }();

    const std::string config_directory = []() -> std::string {
        auto XDG_CONFIG_HOME = getenv("XDG_CONFIG_HOME");
        if (XDG_CONFIG_HOME != nullptr)
        {
            return { XDG_CONFIG_HOME };
        }
        else
        {
            return fmt::format("{}/.config", home_directory);
        }
    }();

    const std::string cache_directory = []() -> std::string {
        auto XDG_CACHE_HOME = getenv("XDG_CACHE_HOME");
        if (XDG_CACHE_HOME != nullptr)
        {
            return { XDG_CACHE_HOME };
        }
        else
        {
            return fmt::format("{}/.cache", home_directory);
        }
    }();
}

void settings::set_current_page(settings::Page page) noexcept
{
    g_settings_set_enum(app_settings, properties[PropertyCurrentPage],
                        static_cast<gint>(page));
}

settings::Page settings::get_current_page() noexcept
{
    return static_cast<settings::Page>(
        g_settings_get_enum(app_settings, properties[PropertyCurrentPage]));
}

const std::string &settings::home_directory() noexcept
{
    return ::home_directory;
}

const std::string &settings::data_directory() noexcept
{
    return ::data_directory;
}

const std::string &settings::config_directory() noexcept
{
    return ::config_directory;
}

const std::string &settings::cache_directory() noexcept
{
    return ::cache_directory;
}
