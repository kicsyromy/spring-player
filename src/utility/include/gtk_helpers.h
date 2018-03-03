#ifndef SPRING_PLAYER_UTILITY_GTK_HELPERS_H
#define SPRING_PLAYER_UTILITY_GTK_HELPERS_H

#include "gtk_ref_guard.h"

#define get_widget_from_builder_simple(widget_name)                                                \
    widget_name##_ = utility::gtk_cast<std::remove_pointer<decltype(widget_name##_)>::type>(       \
        gtk_builder_get_object(builder, #widget_name))

#define get_guarded_widget_from_builder(widget_name)                                               \
    do                                                                                             \
    {                                                                                              \
        using widget_name##_t = decltype(widget_name##_);                                          \
        widget_name##_ = utility::gtk_cast<widget_name##_t::gtk_t>(                                \
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
            inline destination_t *gtk_cast(GtkRefGuard<source_t> &object)
            {
                return reinterpret_cast<destination_t *>(
                    static_cast<typename GtkRefGuard<source_t>::gtk_t *>(object));
            }

            template <typename GObjectType, typename SignalHandlerType, typename UserDataType>
            inline auto connect_g_signal(GObjectType *instance,
                                         const gchar *signal,
                                         SignalHandlerType signal_handler,
                                         UserDataType *user_data) noexcept
            {
                return g_signal_connect_data(gtk_cast<GObject>(instance), signal,
                                             reinterpret_cast<GCallback>(signal_handler),
                                             gtk_cast<gpointer>(user_data), nullptr,
                                             static_cast<GConnectFlags>(0));
            }

            template <typename GObjectType, typename SignalHandlerType, typename UserDataType>
            inline auto connect_g_signal(GtkRefGuard<GObjectType> &instance,
                                         const gchar *signal,
                                         SignalHandlerType signal_handler,
                                         UserDataType *user_data) noexcept
            {
                return g_signal_connect_data(gtk_cast<GObject>(instance), signal,
                                             reinterpret_cast<GCallback>(signal_handler),
                                             gtk_cast<gpointer>(user_data), nullptr,
                                             static_cast<GConnectFlags>(0));
            }
        }
    }
}

#endif // !SPRING_PLAYER_UTILITY_GTK_HELPERS_H
