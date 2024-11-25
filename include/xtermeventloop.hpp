#pragma once

#include "consoleevent.hpp"
#include <queue>




class XTermEventLoop
{
private:
    class EventReceiver : public ExtendedConsoleEventReceiver
    {
    public:
        EventReceiver(std::mutex& mutex, std::queue<AnyConsoleEvent>& queue)
            : mutex(mutex), queue(queue) {}

    public:
        void on_any_event(AnyConsoleEvent&& ev) override
        {
            std::lock_guard lg(mutex);
            queue.push(std::move(ev));
        }

    private:
        std::mutex& mutex;
        std::queue<AnyConsoleEvent>& queue;

    };

public:
    XTermEventLoop();
    ~XTermEventLoop();

private:
    void loop();

public:
    bool poll(ConsoleEventReceiver& recv);
    void init();

private:
    void run_thread();

private:
    std::mutex mutex;
    std::queue<AnyConsoleEvent> queue;
    EventReceiver receiver;
    XTermConsoleEventSender sender;
    std::thread thread;
    std::atomic<bool> should_exit;

};
