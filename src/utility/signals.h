#ifndef SPRING_PLAYER_UTILITY_SIGNALS_H
#define SPRING_PLAYER_UTILITY_SIGNALS_H

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "forward_declarations.h"

#define signal(name, ...)                                                                          \
private:                                                                                           \
    utility::Signal<__VA_ARGS__> signal_##name##_;                                                 \
    template <typename... Args> inline void emit_##name(Args &&... args) const noexcept            \
    {                                                                                              \
        signal_##name##_.emit(std::forward<Args>(args)...);                                        \
    }                                                                                              \
    template <typename... Args> inline void emit_queued_##name(Args &&... args) const noexcept     \
    {                                                                                              \
        signal_##name##_.emit_queued(std::forward<Args>(args)...);                                 \
    }                                                                                              \
                                                                                                   \
public:                                                                                            \
    using signal_##name##_t = decltype(signal_##name##_);                                          \
    template <typename CalleeType, typename CallbackT, typename UserDataT = void>                  \
    inline void on_##name(CalleeType *callee, CallbackT handler,                                   \
                          UserDataT *user_data = nullptr) noexcept                                 \
    {                                                                                              \
        signal_##name##_.connect(callee, handler, user_data ? user_data : callee);                 \
    }                                                                                              \
    template <typename CalleeType> inline void disconnect_##name(CalleeType *callee) noexcept      \
    {                                                                                              \
        signal_##name##_.disconnect(callee);                                                       \
    }

namespace spring
{
    namespace player
    {
        namespace utility
        {
            template <typename... Args> class Signal
            {
            public:
                using signature_t = void (*)(Args..., void *);

            private:
                template <typename Function, typename Tuple, size_t... I>
                inline static auto call(Function f, Tuple t, std::index_sequence<I...>) noexcept
                {
                    return f(std::get<I>(t)...);
                }

                template <typename Function, typename Tuple>
                inline static auto call(Function f, Tuple t) noexcept
                {
                    static constexpr auto size = std::tuple_size<Tuple>::value;
                    return call(f, t, std::make_index_sequence<size>{});
                }

            private:
                struct QueuedData
                {
                    signature_t notify;
                    std::tuple<Args..., void *> args;
                    std::weak_ptr<void> lifeline;
                };

            public:
                inline Signal() noexcept {}
                inline ~Signal() noexcept {}

            public:
                template <typename CalleeType, typename CallbackT, typename UserDataT>
                void connect(CalleeType *callee, CallbackT callback, UserDataT *user_data) noexcept
                {
                    clients_.emplace(callee, connections_.size());
                    connections_.emplace_back((signature_t)callback,
                                              static_cast<void *>(user_data));
                }

                template <typename CalleeType> void disconnect(CalleeType *callee) noexcept
                {
                    auto it = clients_.find(callee);
                    if (it != clients_.end())
                    {
                        auto &connection = connections_[it->second];
                        connection.first = nullptr;
                        clients_.erase(it);
                    }
                }

                void emit(Args &&... args) const noexcept
                {
                    for (const auto &connection : connections_)
                    {
                        if (connection.first != nullptr)
                        {
                            connection.first(std::forward<Args>(args)..., connection.second);
                        }
                    }
                }

                void emit_queued(Args &&... args) const noexcept
                {
                    for (const auto &connection : connections_)
                    {
                        auto data = new QueuedData{ connection.first,
                                                    std::make_tuple(std::forward<Args>(args)...,
                                                                    connection.second),
                                                    lifeline_ };

                        g_idle_add(
                            [](void *d) -> int {
                                auto data =
                                    std::unique_ptr<QueuedData>{ static_cast<QueuedData *>(d) };

                                if (data->lifeline.lock() != nullptr)
                                {
                                    if (data->notify != nullptr)
                                    {
                                        call(data->notify, data->args);
                                    }
                                }

                                return false;
                            },
                            data);
                    }
                }

            private:
                std::vector<std::pair<signature_t, void *>> connections_{};
                std::unordered_map<void *, std::size_t> clients_{};
                mutable std::shared_ptr<void> lifeline_{ std::make_shared<char>() };
            };
        }
    }
}

#endif // !SPRING_PLAYER_UTILITY_SIGNALS_H
