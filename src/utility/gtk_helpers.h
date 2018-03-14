#ifndef SPRING_PLAYER_UTILITY_GTK_HELPERS_H
#define SPRING_PLAYER_UTILITY_GTK_HELPERS_H

#include <cstdint>
#include <type_traits>

#include "forward_declarations.h"
#include "g_object_guard.h"

#define get_widget_from_builder_simple(widget_name)                                                \
    widget_name##_ = utility::gtk_cast<std::remove_pointer<decltype(widget_name##_)>::type>(       \
        gtk_builder_get_object(builder, #widget_name))

#define get_guarded_widget_from_builder(widget_name)                                               \
    do                                                                                             \
    {                                                                                              \
        using widget_name##_t = decltype(widget_name##_);                                          \
        widget_name##_ = utility::gtk_cast<widget_name##_t::g_object_t>(                           \
            gtk_builder_get_object(builder, #widget_name));                                        \
    } while (false)

#define get_widget_from_builder_new(widget_type, widget_name)                                      \
    auto widget_name = utility::gtk_cast<widget_type>(gtk_builder_get_object(builder, #widget_name))

#define get_widget_from_builder(widget_type, widget_name)                                          \
    utility::gtk_cast<widget_type>(gtk_builder_get_object(builder, #widget_name))

namespace spring
{
    namespace player
    {
        namespace utility
        {
            template <typename destination_t, typename source_t>
            inline destination_t *gtk_cast(source_t *object)
            {
                return reinterpret_cast<destination_t *>(object);
            }

            template <typename destination_t, typename source_t>
            inline destination_t &gtk_cast(source_t &object)
            {
                return reinterpret_cast<destination_t &>(object);
            }

            template <typename destination_t, typename source_t>
            inline destination_t *gtk_cast(GObjectGuard<source_t> &object)
            {
                return reinterpret_cast<destination_t *>(
                    static_cast<typename GObjectGuard<source_t>::g_object_t *>(object));
            }

            template <typename GObjectType, typename SignalHandlerType, typename UserDataType>
            inline auto connect_g_signal(GObjectType *instance,
                                         const char *signal,
                                         SignalHandlerType signal_handler,
                                         UserDataType *user_data) noexcept
            {
                return g_signal_connect_data(static_cast<void *>(instance), signal,
                                             reinterpret_cast<void (*)()>(signal_handler),
                                             static_cast<void *>(user_data), nullptr,
                                             static_cast<GConnectFlags>(0));
            }

            template <typename GObjectType, typename SignalHandlerType, typename UserDataType>
            inline auto connect_g_signal(GObjectGuard<GObjectType> &instance,
                                         const char *signal,
                                         SignalHandlerType signal_handler,
                                         UserDataType *user_data) noexcept
            {
                return g_signal_connect_data(static_cast<void *>(instance), signal,
                                             reinterpret_cast<void (*)()>(signal_handler),
                                             static_cast<void *>(user_data), nullptr,
                                             static_cast<GConnectFlags>(0));
            }
        }
    }
}

#endif // !SPRING_PLAYER_UTILITY_GTK_HELPERS_H
