#ifndef SPRING_PLAYER_UTILITY_EXPONENTIAL_BLUR_H
#define SPRING_PLAYER_UTILITY_EXPONENTIAL_BLUR_H

#include <array>
#include <cmath>
#include <cstdint>
#include <thread>

namespace spring
{
    namespace player
    {
        namespace utility
        {
            using image_data_t = std::uint8_t *;

            template <std::size_t channel_count> class detail
            {
                static constexpr auto ALPHA_PRECISION{ 16 };
                static constexpr auto PARAM_PRECISION{ 7 };

                using count_t = decltype(channel_count);

                static inline void exponential_blur_inner(
                    std::uint8_t *pixel,
                    std::array<std::int32_t, channel_count> &channels,
                    std::int32_t alpha) noexcept
                {
                    for (count_t it = 0; it < channel_count; ++it)
                    {
                        auto &channel = channels[it];
                        channel +=
                            (alpha * ((pixel[it] << PARAM_PRECISION) - channel)) >> ALPHA_PRECISION;
                        pixel[it] = static_cast<std::uint8_t>(channel >> PARAM_PRECISION);
                    }
                }

                static inline void exponential_blur_rows(image_data_t pixels,
                                                         std::int32_t width,
                                                         std::int32_t startRow,
                                                         std::int32_t endRow,
                                                         std::int32_t startX,
                                                         std::int32_t endX,
                                                         std::int32_t alpha) noexcept
                {
                    for (auto rowIndex = startRow; rowIndex < endRow; rowIndex++)
                    {
                        auto *row = pixels + rowIndex * width * channel_count;

                        std::array<std::int32_t, channel_count> channels;
                        for (count_t it = 0; it < channel_count; ++it)
                        {
                            channels[it] = row[it] << PARAM_PRECISION;
                        }

                        for (auto index = startX + 1; index < endX; ++index)
                        {
                            exponential_blur_inner(row + index * channel_count, channels, alpha);
                        }

                        for (auto index = endX - 2; index >= startX; --index)
                        {
                            exponential_blur_inner(row + index * channel_count, channels, alpha);
                        }
                    }
                }

                static inline void exponential_blur_columns(image_data_t pixels,
                                                            std::int32_t width,
                                                            std::int32_t startColumn,
                                                            std::int32_t endColumn,
                                                            std::int32_t startY,
                                                            std::int32_t endY,
                                                            std::int32_t alpha) noexcept
                {
                    for (auto columnIndex = startColumn; columnIndex < endColumn; columnIndex++)
                    {
                        auto *column = pixels + columnIndex * channel_count;

                        std::array<std::int32_t, channel_count> channels;
                        for (count_t it = 0; it < channel_count; ++it)
                        {
                            channels[it] = column[it] << PARAM_PRECISION;
                        }

                        for (auto index = width * (startY + 1); index < (endY - 1) * width;
                             index += width)
                        {
                            exponential_blur_inner(column + index * channel_count, channels, alpha);
                        }

                        for (auto index = (endY - 2) * width; index >= startY; index -= width)
                        {
                            exponential_blur_inner(column + index * channel_count, channels, alpha);
                        }
                    }
                }

                template <std::size_t cc>
                friend void exponential_blur(image_data_t pixels,
                                             std::int32_t width,
                                             std::int32_t height,
                                             std::int32_t radius) noexcept;
            };

            template <std::size_t channel_count>
            inline void exponential_blur(image_data_t pixels,
                                         std::int32_t width,
                                         std::int32_t height,
                                         std::int32_t radius) noexcept
            {
                using impl = detail<channel_count>;

                auto alpha = static_cast<std::int32_t>((1 << impl::ALPHA_PRECISION) *
                                                       (1.0 - std::exp(-2.3 / (radius + 1.0))));

                std::thread t1{ [&] {
                    impl::exponential_blur_rows(pixels, width, 0, height / 2, 0, width, alpha);
                } };
                impl::exponential_blur_rows(pixels, width, height / 2, height, 0, width, alpha);
                t1.join();

                std::thread t2{ [&] {
                    impl::exponential_blur_columns(pixels, width, 0, width / 2, 0, height, alpha);
                } };
                impl::exponential_blur_columns(pixels, width, width / 2, width, 0, height, alpha);
                t2.join();
            }
        }
    }
}

#endif // !SPRING_PLAYER_UTILITY_EXPONENTIAL_BLUR_H
