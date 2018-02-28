#include "async_queue.h"

#include <pthread.h>

#include <atomic>
#include <mutex>
#include <thread>

#include <gtk/gtk.h>

#include <libspring_logger.h>

using namespace spring;
using namespace spring::player;

namespace
{
    inline unsigned long current_thread_id() noexcept { return pthread_self(); }
}

namespace
{
    std::atomic_bool message_loop_running{ true };
    GAsyncQueue *message_queue{ nullptr };
    std::thread worker{};

    inline void clear_all_pending_requests() noexcept
    {
        using namespace async_queue;

        if (message_loop_running && message_queue != nullptr)
        {
            g_async_queue_push_front(
                message_queue,
                new Request{
                    "clear_message_queue", []() {
                        for (;;)
                        {
                            std::unique_ptr<Request> request{
                                static_cast<Request *>(
                                    g_async_queue_try_pop(message_queue))
                            };
                            if (request == nullptr)
                            {
                                break;
                            }
                        }
                    } });
        }
    }
}

void async_queue::start_processing() noexcept
{
    LOG_INFO("AsyncQueue: Main thread: 0x{:x}", current_thread_id());
    message_queue = g_async_queue_new();
    worker = std::thread{ []() {
        g_async_queue_ref(message_queue);
        LOG_INFO("AsyncQueue: Started worker thread 0x{:x}",
                 current_thread_id());

        for (;;)
        {
            std::unique_ptr<Request> request{ static_cast<Request *>(
                g_async_queue_pop(message_queue)) };

            if (!message_loop_running)
            {
                break;
            }

            if (request->request == nullptr)
            {
                LOG("AsyncQueue: Received empty request...");
            }
            else
            {
                if (message_loop_running)
                {
                    LOG_INFO("AsyncQueue: Executing request \"{}\" on thread "
                             "0x{:x}",
                             request->id, current_thread_id());

                    request->request();
                }
            }
        }

        g_async_queue_unref(message_queue);
    } };
}

void async_queue::stop_processing() noexcept
{
    if (worker.joinable())
    {
        clear_all_pending_requests();
        message_loop_running = false;
        worker.join();
    }

    if (message_queue != nullptr)
    {
        g_async_queue_unref(message_queue);
    }
}

void async_queue::push_back_request(Request *request) noexcept
{
    if (message_loop_running && message_queue != nullptr)
    {
        LOG_INFO("AsyncQueue: Posting message: \"{}\"", request->id);
        g_async_queue_push(message_queue, request);
    }
    else
    {
        LOG_WARN("AsyncQueue: Queue is not running, message \"{}\" lost",
                 request->id);
        delete request;
    }
}

void async_queue::push_front_request(Request *request) noexcept
{
    if (message_loop_running && message_queue != nullptr)
    {
        LOG_INFO("AsyncQueue: Posting message: \"{}\"", request->id);
        g_async_queue_push_front(message_queue, request);
    }
    else
    {
        LOG_WARN("AsyncQueue: Queue is not running, message \"{}\" lost",
                 request->id);
        delete request;
    }
}

void async_queue::post_response(Response *response) noexcept
{
    if (message_loop_running && response->request != nullptr)
    {
        LOG_INFO("AsyncQueue: Posting reply: \"{}\"", response->id);
        g_main_context_invoke(nullptr,
                              [](gpointer data) -> int {
                                  std::unique_ptr<Response> response{
                                      static_cast<Response *>(data)
                                  };
                                  response->request();
                                  return false;
                              },
                              response);
    }
}
