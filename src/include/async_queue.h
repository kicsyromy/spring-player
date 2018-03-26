#ifndef SPRING_PLAYER_ASYNC_QUEUE_H
#define SPRING_PLAYER_ASYNC_QUEUE_H

#include <functional>
#include <string>

namespace spring
{
    namespace player
    {
        namespace async_queue
        {
            struct Request
            {
                const char *id;
                std::function<void()> request;
            };

            using Response = Request;

            void start_processing() noexcept;
            void stop_processing() noexcept;
            void push_back_request(Request &&request) noexcept;
            void push_front_request(Request &&request) noexcept;
            void post_response(Response &&response) noexcept;
        }
    }
}

#endif // !SPRING_PLAYER_ASYNC_QUEUE_H
