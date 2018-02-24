#ifndef SPRING_PLAYER_UTILITY_H
#define SPRING_PLAYER_UTILITY_H

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include <gtk/gtk.h>
#include <unicode/unistr.h>

#include <utfcpp.h>

#define get_widget_from_builder_simple(widget_name)                            \
    widget_name##_ =                                                           \
        gtk_cast<std::remove_pointer<decltype(widget_name##_)>::type>(         \
            gtk_builder_get_object(builder, #widget_name))

#define get_guarded_widget_from_builder(widget_name)                           \
    do                                                                         \
    {                                                                          \
        using widget_name##_t = decltype(widget_name##_);                      \
        widget_name##_ = gtk_cast<widget_name##_t::gtk_t>(                     \
            gtk_builder_get_object(builder, #widget_name));                    \
    } while (false)

#define get_widget_from_builder_new(widget_type, widget_name)                  \
    auto widget_name =                                                         \
        gtk_cast<widget_type>(gtk_builder_get_object(builder, #widget_name))

#define get_widget_from_builder(widget_type, widget_name)                      \
    gtk_cast<widget_type>(gtk_builder_get_object(builder, #widget_name))

#define signal(name, ...)                                                      \
private:                                                                       \
    struct                                                                     \
    {                                                                          \
        void (*notify)(__VA_ARGS__, void *){ nullptr };                        \
        void *user_data{ nullptr };                                            \
    } signal_##name##_{};                                                      \
                                                                               \
    template <typename... Args>                                                \
    inline void emit_##name(Args &&... args) const noexcept                    \
    {                                                                          \
        signal_##name##_.notify(std::forward<Args>(args)...,                   \
                                signal_##name##_.user_data);                   \
    }                                                                          \
                                                                               \
public:                                                                        \
    using signal_##name##_t = decltype(signal_##name##_.notify);               \
    inline void on_##name(signal_##name##_t handler, void *user_data) noexcept \
    {                                                                          \
        signal_##name##_.notify = handler;                                     \
        signal_##name##_.user_data = user_data;                                \
    }

namespace spring
{
    namespace player
    {
        template <typename Widget> class GtkRefGuard
        {
        public:
            using gtk_t = Widget;

        public:
            explicit GtkRefGuard(Widget *widget) noexcept
              : handle_(widget)
            {
                if (handle_ != nullptr)
                {
                    g_object_ref_sink(handle_);
                }
            }
            ~GtkRefGuard() noexcept
            {
                if (handle_ != nullptr)
                {
                    g_object_unref(handle_);
                    handle_ = nullptr;
                }
            }

            GtkRefGuard(const GtkRefGuard &other)
              : handle_(other.handle_)
            {
                g_object_ref(handle_);
            }

            GtkRefGuard &operator=(const GtkRefGuard &other)
            {
                handle_ = other.handle_;
                g_object_ref(handle_);
            }

            GtkRefGuard(GtkRefGuard &&other)
              : handle_(other.handle_)
            {
                g_object_ref(handle_);
            }

            GtkRefGuard &operator=(GtkRefGuard &&other)
            {
                handle_ = other.handle_;
                g_object_ref(handle_);
            }

        public:
            GtkRefGuard &operator=(Widget *widget)
            {
                this->~GtkRefGuard();

                handle_ = widget;
                if (handle_ != nullptr)
                {
                    g_object_ref_sink(handle_);
                }

                return *this;
            }

        public:
            operator Widget *() { return handle_; }

        private:
            Widget *handle_;
        };

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

        template <typename GObjectType,
                  typename SignalHandlerType,
                  typename UserDataType>
        inline auto connect_g_signal(GObjectType *instance,
                                     const gchar *signal,
                                     SignalHandlerType signal_handler,
                                     UserDataType *user_data) noexcept
        {
            return g_signal_connect_data(
                gtk_cast<GObject>(instance), signal,
                reinterpret_cast<GCallback>(signal_handler),
                gtk_cast<gpointer>(user_data), nullptr,
                static_cast<GConnectFlags>(0));
        }

        template <int width, int height>
        inline void load_image_from_data_scaled(const std::string &data,
                                                GtkImage *result) noexcept
        {
            auto loader = gdk_pixbuf_loader_new();
            gdk_pixbuf_loader_write(
                loader, reinterpret_cast<const guchar *>(data.data()),
                data.size(), nullptr);
            auto pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
            auto scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height,
                                                         GDK_INTERP_HYPER);
            gtk_image_set_from_pixbuf(result, scaled_pixbuf);

            g_object_unref(scaled_pixbuf);
            g_object_unref(pixbuf);

            gdk_pixbuf_loader_close(loader, nullptr);
        }

        /* Shamelessly adapted from:                                                         */
        /* https://en.wikipedia.org/wiki/Levenshtein_distance#Iterative_with_two_matrix_rows */
        inline std::int32_t levenshtein_distance(
            const std::string_view &from, const std::string_view &to) noexcept
        {
            std::size_t s1_size = utf8::distance(from.begin(), from.end());
            std::size_t s2_size = utf8::distance(to.begin(), to.end());

            std::string s1{};
            std::string s2{};

            {
                icu::UnicodeString s1_utf8(from.data(), from.size(), US_INV);
                icu::UnicodeString s2_utf8(to.data(), to.size(), US_INV);

                s1_utf8.toLower();
                s2_utf8.toLower();

                s1_utf8.toUTF8String(s1);
                s2_utf8.toUTF8String(s2);
            }

            std::vector<std::int32_t> previous_distances(s2_size + 1, 0);
            std::vector<std::int32_t> current_distances(s2_size + 1, 0);

            for (std::size_t it = 0; it <= s2_size; ++it)
            {
                previous_distances[it] = static_cast<std::int32_t>(it);
            }

            auto s1_code_point = s1.begin();
            for (std::size_t it = 0; it < s1_size;
                 ++it, utf8::next(s1_code_point))
            {
                current_distances[0] = static_cast<std::int32_t>(it) + 1;

                auto s2_code_point = s2.begin();
                for (std::size_t jt = 0; jt < s2_size; ++jt)
                {
                    const std::int32_t deletion_cost{
                        previous_distances[jt + 1] + 1
                    };
                    const std::int32_t insertion_cost{ current_distances[jt] +
                                                       1 };
                    std::int32_t substitution_cost{ 0 };
                    if (utf8::peek_next(s1_code_point) ==
                        utf8::next(s2_code_point))
                    {
                        substitution_cost = previous_distances[jt];
                    }
                    else
                    {
                        substitution_cost = previous_distances[jt] + 1;
                    }

                    current_distances[jt + 1] =
                        std::min(std::min(deletion_cost, insertion_cost),
                                 substitution_cost);
                }

                for (std::size_t it = 0; it < current_distances.size(); ++it)
                {
                    std::swap(previous_distances[it], current_distances[it]);
                }
            }

            return previous_distances[s2.size()];
        }

        inline bool fuzzy_match(const std::string_view &s1,
                                const std::string_view &s2,
                                std::int32_t precision)
        {
            std::size_t s1_size = utf8::distance(s1.begin(), s1.end());
            std::size_t s2_size = utf8::distance(s2.begin(), s2.end());

            if (s2_size > s1_size)
            {
                const auto iterations = s2_size - s1_size;
                auto s2_window_begin = s2.begin();
                for (std::size_t advance = 0; advance < iterations;
                     ++advance, utf8::next(s2_window_begin))
                {
                    auto s2_window_end = s2_window_begin;
                    utf8::advance(s2_window_end, s1_size);
                    std::string_view s2_window{
                        s2_window_begin, static_cast<std::size_t>(std::distance(
                                             s2_window_begin, s2_window_end))
                    };
                    if (levenshtein_distance(s1, s2_window) <= precision)
                    {
                        return true;
                    }
                }
            }
            else if (s1_size > s2_size)
            {
                const auto iterations = s1_size - s2_size;
                auto s1_window_begin = s1.begin();
                for (std::size_t advance = 0; advance < iterations;
                     ++advance, utf8::next(s1_window_begin))
                {
                    auto s1_window_end = s1_window_begin;
                    utf8::advance(s1_window_end, s2_size);
                    std::string_view s1_window{
                        s1_window_begin, static_cast<std::size_t>(std::distance(
                                             s1_window_begin, s1_window_end))
                    };
                    if (levenshtein_distance(s2, s1_window) <= precision)
                    {
                        return true;
                    }
                }
            }
            else
            {
                return (levenshtein_distance(s1, s2) <= precision);
            }

            return false;
        }
    }
}

#endif // !SPRING_PLAYER_UTILITY_H
