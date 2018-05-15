#ifndef SPRING_PLAYER_RESOURCE_CACHE_H
#include "resource_cache.h"
#endif

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

template <std::size_t header_size>
void ResourceCache<header_size>::to_cache(const string_view &prefix,
                                          const string_view &resource_id,
                                          const Resource &resource) noexcept
{
    auto path = fmt::format("{}/{}", settings::cache_directory(), prefix);
    auto full_file_path = fmt::format("{}/{}", path, resource_id);
    bool created_but_invalid{ false };

    auto result = g_mkdir_with_parents(path.c_str(), 0755);
    if (result > 0)
    {
        auto err = errno;
        LOG_ERROR("ResourceCache: Failed to create directory {}: {}", path, std::strerror(err));
    }
    else
    {
        posix_fd_t resource_file(full_file_path.c_str(), O_CREAT | O_WRONLY);
        if (resource_file)
        {
            auto bytes_written = write(resource_file(), resource.header.data(), header_size);
            if (bytes_written < resource.header.size())
            {
                LOG_ERROR("ResourceCache: Failed to write file header for {}", full_file_path);
                created_but_invalid = true;
            }
            else
            {
                bytes_written = write(resource_file(), resource.buffer.data, resource.buffer.size);
                if (bytes_written < resource.buffer.size)
                {
                    LOG_ERROR("ResourceCache: Failed to write file data for {}", full_file_path);
                    created_but_invalid = true;
                }
            }
        }
        else
        {
            auto error = errno;
            LOG_ERROR("ResourceCache: Failed to save file {}: {}", full_file_path,
                      std::strerror(error));
        }
    }

    if (created_but_invalid)
    {
        auto result = remove(full_file_path.c_str());
        if (result < 0)
        {
            auto error = errno;
            LOG_ERROR("ResourceCache: Failed to remove incomplete cached file {}: {}",
                      full_file_path, std::strerror(error));
        }
    }
}

template <std::size_t header_size>
std::pair<typename ResourceCache<header_size>::Resource, bool> ResourceCache<
    header_size>::from_cache(const string_view &prefix, const string_view &resource_id) noexcept
{
    auto path = fmt::format("{}/{}", settings::cache_directory(), prefix);
    auto full_file_path = fmt::format("{}/{}", path, resource_id);

    Resource result;
    bool success{ true };

    posix_fd_t resource_file(full_file_path, O_RDONLY);
    if (!resource_file)
    {
        if (resource_file.error() != ENOENT)
        {
            LOG_ERROR("ResourceCache: Failed to open file {}: {}", full_file_path,
                      std::strerror(resource_file.error()));
            success = false;
        }
    }
    else
    {
        struct stat stbuf;
        if ((fstat(resource_file(), &stbuf) != 0) || (!S_ISREG(stbuf.st_mode)))
        {
            auto error = errno;
            LOG_ERROR("ResourceCache: Failed to determine file size for {}: {}", full_file_path,
                      std::strerror(error));
            success = false;
        }
        else
        {
            auto file_size{ static_cast<std::size_t>(stbuf.st_size) };
            if (file_size < header_size)
            {
                LOG_ERROR("ResourceCache: Corrupt file or bad header size for {}", full_file_path);
                success = false;
            }
            else
            {
                auto bytes_read = read(resource_file(), result.header.data(), header_size);
                if (bytes_read < header_size)
                {
                    auto error = errno;
                    LOG_ERROR("ResourceCache: Failed to read header for {}: {}", full_file_path,
                              std::strerror(error));
                    success = false;
                }
                else
                {
                    result.buffer.size = file_size - bytes_read;
                    result.buffer.data = new std::uint8_t[file_size - bytes_read];
                    bytes_read = read(resource_file(), result.buffer.data, result.buffer.size);
                    if (bytes_read == -1)
                    {
                        auto error = errno;
                        LOG_ERROR("ResourceCache: Failed to read data for {}: {}", full_file_path,
                                  std::strerror(error));
                        success = false;
                    }
                }
            }
        }
    }

    return { result, success };
}
