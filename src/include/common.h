#ifndef SPRING_PLAYER_UTILITY_H
#define SPRING_PLAYER_UTILITY_H

#include <memory>

#include <gtk/gtk.h>

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

        template <int width, int height>
        void load_image_from_data_scaled(const std::string &data,
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
    }
}

#endif // !SPRING_PLAYER_UTILITY_H
