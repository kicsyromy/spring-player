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

            void start_processing();
            void stop_processing();
            void post_request(Request *request);
            void clear_all_pending_requests();
            void post_response(Response *response);
        }
    }
}

#endif // !SPRING_PLAYER_ASYNC_QUEUE_H
