#include "application_settings.h"

#include <array>

#include <gtk/gtk.h>

#include <libspring_logger.h>

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
            LOG_INFO("ApplicationSettings: Initializing...");
        }
        ~SettingsInitializer() noexcept { g_object_unref(handle_); }
        operator GSettings *() noexcept { return handle_; }
        GSettings *handle_{ nullptr };
    } app_settings;

    constexpr std::array<const char *,
                         static_cast<std::size_t>(settings::Page::Count)>
        PAGES{ "Page::Albums", "Page::Artists", "Page::Genres", "Page::Songs" };

    enum Properties : std::size_t
    {
        PropertyCurrentPage,
        PropertyCount
    };
    constexpr std::array<const char *, PropertyCount> properties{
        "current-page"
    };

    std::string home_directory{};
    std::string data_directory{};
    std::string config_directory{};
    std::string cache_directory{};
}

void settings::set_current_page(settings::Page page) noexcept
{
    LOG_INFO("ApplicationSettings: Saving selected page {} to settings",
             PAGES[static_cast<std::size_t>(page)]);

    g_settings_set_enum(app_settings, properties[PropertyCurrentPage],
                        static_cast<gint>(page));
}

settings::Page settings::get_current_page() noexcept
{
    auto page = static_cast<settings::Page>(
        g_settings_get_enum(app_settings, properties[PropertyCurrentPage]));

    LOG_INFO("ApplicationSettings: Loaded current page {} from settings",
             PAGES[static_cast<std::size_t>(page)]);

    return page;
}

const std::string &settings::home_directory() noexcept
{
    if (::home_directory.empty())
    {
        ::home_directory = []() -> std::string {
            auto HOME = getenv("HOME");
            return { HOME };
        }();
    }

    return ::home_directory;
}

const std::string &settings::data_directory() noexcept
{
    if (::data_directory.empty())
    {
        ::data_directory = []() -> std::string {
            auto XDG_DATA_HOME = getenv("XDG_DATA_HOME");
            if (XDG_DATA_HOME != nullptr)
            {
                return fmt::format("{}/" APPLICATION_ID, XDG_DATA_HOME);
            }
            else
            {
                return fmt::format("{}/.local/share/" APPLICATION_ID,
                                   home_directory());
            }
        }();
    }

    return ::data_directory;
}

const std::string &settings::config_directory() noexcept
{
    if (::config_directory.empty())
    {
        ::config_directory = []() -> std::string {
            auto XDG_CONFIG_HOME = getenv("XDG_CONFIG_HOME");
            if (XDG_CONFIG_HOME != nullptr)
            {
                return fmt::format("{}/" APPLICATION_ID, XDG_CONFIG_HOME);
            }
            else
            {
                return fmt::format("{}/.config/" APPLICATION_ID,
                                   home_directory());
            }
        }();
    }

    return ::config_directory;
}

const std::string &settings::cache_directory() noexcept
{
    if (::cache_directory.empty())
    {
        ::cache_directory = []() -> std::string {
            auto XDG_CACHE_HOME = getenv("XDG_CACHE_HOME");
            if (XDG_CACHE_HOME != nullptr)
            {
                return fmt::format("{}/" APPLICATION_ID, XDG_CACHE_HOME);
            }
            else
            {
                return fmt::format("{}/.cache/" APPLICATION_ID,
                                   home_directory());
            }
        }();
    }

    return ::cache_directory;
}
