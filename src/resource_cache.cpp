#include "resource_cache.h"

#include <cstring>

#include <fmt/format.h>

#include <libspring_logger.h>

#include "application_settings.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

ResourceCache::ResourceCache() noexcept = default;
ResourceCache::~ResourceCache() noexcept = default;

std::pair<std::string, bool> ResourceCache::from_cache(const string_view &prefix,
                                                       const string_view &resource_id) noexcept
{
    auto path = fmt::format("{}/{}", settings::cache_directory(), prefix);
    auto full_file_path = fmt::format("{}/{}", path, resource_id);

    GError *error{ nullptr };
    char *file_content{ nullptr };
    std::size_t file_size{ 0 };

    auto success = g_file_get_contents(full_file_path.c_str(), &file_content, &file_size, &error);
    if (!success)
    {
        if (error->code != G_FILE_ERROR_NOENT)
        {
            LOG_ERROR("ResourceCache: Failed to open file {}: {}", full_file_path, error->message);
        }
        else
        {
            success = true;
        }
    }

    g_clear_error(&error);

    return { file_content ? std::string{ file_content, file_size } : std::string{ "" }, success };
}

void ResourceCache::to_cache(const string_view &prefix,
                             const string_view &resource_id,
                             const char *data,
                             std::size_t size) noexcept
{
    auto path = fmt::format("{}/{}", settings::cache_directory(), prefix);
    auto full_file_path = fmt::format("{}/{}", path, resource_id);

    auto result = g_mkdir_with_parents(path.c_str(), 0755);
    if (result > 0)
    {
        auto err = errno;
        LOG_ERROR("ResourceCache: Failed to create directory {}: {}", path, std::strerror(err));
    }
    else
    {
        GError *error{ nullptr };
        auto success =
            g_file_set_contents(full_file_path.c_str(), data, static_cast<gssize>(size), &error);

        if (!success)
        {
            LOG_ERROR("ResourceCache: Failed to save file {}: {}", full_file_path, error->message);
        }

        g_clear_error(&error);
    }
}

void ResourceCache::to_cache(const string_view &prefix,
                             const string_view &resource_id,
                             GdkPixbuf *pixbuf) noexcept
{
    auto path = fmt::format("{}/{}", settings::cache_directory(), prefix);
    auto full_file_path = fmt::format("{}/{}", path, resource_id);

    auto result = g_mkdir_with_parents(path.c_str(), 0755);
    if (result > 0)
    {
        auto err = errno;
        LOG_ERROR("ResourceCache: Failed to create directory {}: {}", path, std::strerror(err));
    }
    else
    {
        GError *error{ nullptr };
        gdk_pixbuf_save(pixbuf, full_file_path.c_str(), "png", &error, nullptr);
        if (error != nullptr)
        {
            LOG_ERROR("ResourceCache: Failed to save pixbuf {} to disk: {}", full_file_path,
                      error->message);
        }

        g_clear_error(&error);
    }
}
