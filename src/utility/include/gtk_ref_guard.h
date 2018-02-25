#ifndef SPRING_PLAYER_UTILITY_GTK_REF_GUARD_H
#define SPRING_PLAYER_UTILITY_GTK_REF_GUARD_H

#include <gtk/gtk.h>

namespace spring
{
    namespace player
    {
        namespace utility
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
        }
    }
}

#endif // !SPRING_PLAYER_UTILITY_GTK_REF_GUARD_H
