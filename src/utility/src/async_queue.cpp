#include <atomic>
#include <thread>

#include <gtk/gtk.h>

#include <pthread.h>

#include <blockingconcurrentqueue.h>

#include <libspring_logger.h>

#include "utility/async_queue.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;
using namespace moodycamel;

namespace
{
    std::atomic_bool message_loop_running{ true };
    BlockingConcurrentQueue<async_queue::Request> message_queue{};
    ConcurrentQueue<async_queue::Request> prio_queue{};
    std::thread worker{};

    constexpr auto SENTINEL_REQUEST_ID{ "6228d6fc-f492-4e09-bc82-2c83d351fe97" };

    inline unsigned long current_thread_id() noexcept { return pthread_self(); }

    void handle_request(async_queue::Request &request) noexcept
    {
        if (request.request == nullptr)
        {
            if (request.id != SENTINEL_REQUEST_ID)
            {
                LOG("AsyncQueue: Received empty request...");
            }
        }
        else
        {
            if (message_loop_running)
            {
                LOG_INFO("AsyncQueue: Executing request \"{}\" on thread "
                         "0x{:x}",
                         request.id, current_thread_id());

                request.request();
            }
        }
    }

    void send_sentinel_request() noexcept
    {
        message_queue.enqueue(async_queue::Request{ SENTINEL_REQUEST_ID, nullptr });
    }
} // namespace

void async_queue::start_processing() noexcept
{
    LOG_INFO("AsyncQueue: Main thread: 0x{:x}", current_thread_id());
    worker = std::thread{ []() {
        LOG_INFO("AsyncQueue: Started worker thread 0x{:x}", current_thread_id());

        for (;;)
        {
            Request request;

            if (!prio_queue.try_dequeue(request))
            {
                message_queue.wait_dequeue(request);
            }

            if (!message_loop_running)
            {
                break;
            }

            handle_request(request);
        }
    } };
}

void async_queue::stop_processing() noexcept
{
    if (worker.joinable())
    {
        message_loop_running = false;
        send_sentinel_request();
        worker.join();
    }
}

void async_queue::push_back_request(Request &&request) noexcept
{
    if (message_loop_running)
    {
        LOG_INFO("AsyncQueue: Posting message: \"{}\"", request.id);
        message_queue.enqueue(std::move(request));
    }
    else
    {
        LOG_WARN("AsyncQueue: Queue is not running, message \"{}\" lost", request.id);
    }
}

void async_queue::push_front_request(Request &&request) noexcept
{
    if (message_loop_running)
    {
        LOG_INFO("AsyncQueue: Posting priority message: \"{}\"", request.id);
        prio_queue.enqueue(std::move(request));
        send_sentinel_request();
    }
    else
    {
        LOG_WARN("AsyncQueue: Queue is not running, message \"{}\" lost", request.id);
    }
}

void async_queue::post_response(Response &&response) noexcept
{
    if (message_loop_running && response.request != nullptr)
    {
        LOG_INFO("AsyncQueue: Posting reply: \"{}\"", response.id);
        auto r = new Response;
        *r = std::move(response);
        g_main_context_invoke(
            nullptr,
            [](gpointer data) -> int {
                std::unique_ptr<Response> response{ static_cast<Response *>(data) };
                response->request();
                return false;
            },
            r);
    }
}
