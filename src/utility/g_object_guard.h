#ifndef SPRING_PLAYER_UTILITY_G_OBJECT_GUARD_H
#define SPRING_PLAYER_UTILITY_G_OBJECT_GUARD_H

#include <memory>

#include "forward_declarations.h"

namespace spring
{
    namespace player
    {
        namespace utility
        {
            template <typename GObject>
            using g_object_handle_t = std::unique_ptr<GObject, decltype(&g_object_unref)>;

            template <typename GObject> class GObjectGuard
            {
            public:
                using g_object_t = GObject;

            public:
                explicit GObjectGuard(g_object_t *object) noexcept
                  : handle_(object)
                {
                    if (handle_ != nullptr)
                    {
                        g_object_ref_sink(handle_);
                    }
                }
                ~GObjectGuard() noexcept
                {
                    if (handle_ != nullptr)
                    {
                        g_object_unref(handle_);
                        handle_ = nullptr;
                    }
                }

                GObjectGuard(const GObjectGuard &other)
                  : handle_(other.handle_)
                {
                    g_object_ref(handle_);
                }

                GObjectGuard &operator=(const GObjectGuard &other)
                {
                    handle_ = other.handle_;
                    g_object_ref(handle_);
                }

                GObjectGuard(GObjectGuard &&other)
                  : handle_(other.handle_)
                {
                    g_object_ref(handle_);
                }

                GObjectGuard &operator=(GObjectGuard &&other)
                {
                    handle_ = other.handle_;
                    g_object_ref(handle_);
                }

            public:
                GObjectGuard &operator=(g_object_t *object)
                {
                    this->~GObjectGuard();

                    handle_ = object;
                    if (handle_ != nullptr)
                    {
                        g_object_ref_sink(handle_);
                    }

                    return *this;
                }

            public:
                operator g_object_t *() { return handle_; }

            private:
                g_object_t *handle_;
            };
        }
    }
}

#endif // !SPRING_PLAYER_UTILITY_G_OBJECT_GUARD_H
