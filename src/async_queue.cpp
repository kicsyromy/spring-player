#include "async_queue.h"

#include <atomic>
#include <mutex>
#include <thread>

#include <gtk/gtk.h>

using namespace spring;
using namespace spring::player;

namespace
{
    std::atomic_bool message_loop_running{ true };
    //    std::mutex message_discard_lock {};
    GAsyncQueue *message_queue{ nullptr };
    std::thread worker{};
}

void async_queue::start_processing()
{
    g_warning("Main thread %p", std::this_thread::get_id());
    message_queue = g_async_queue_new();
    worker = std::thread{ []() {
        g_async_queue_ref(message_queue);
        g_warning("Started worker thread %p", std::this_thread::get_id());

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
                g_warning("AsyncQueue: Received empty request...");
            }
            else
            {
                if (message_loop_running)
                {
                    g_warning("Executing request \"%s\" on thread %p",
                              request->id, std::this_thread::get_id());

                    request->request();
                }
            }
        }

        g_async_queue_unref(message_queue);
    } };
}

void async_queue::stop_processing()
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

void async_queue::post_request(Request *request)
{
    if (message_loop_running && message_queue != nullptr)
    {
        g_warning("AsyncQueue: posting message: \"%s\"", request->id);
        g_async_queue_push(message_queue, request);
    }
    else
    {
        g_warning("AsyncQueue: queue is not running, message \"%s\" lost",
                  request->id);
        delete request;
    }
}

void async_queue::clear_all_pending_requests()
{
    if (message_loop_running && message_queue != nullptr)
    {
        g_async_queue_push_front(
            message_queue,
            new Request{ "clear_message_queue", []() {
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

void async_queue::post_response(Response *response)
{
    if (message_loop_running && response->request != nullptr)
    {
        g_warning("AsyncQueue: posting reply: \"%s\"", response->id);
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
