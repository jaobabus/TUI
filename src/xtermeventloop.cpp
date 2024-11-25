#include "xtermeventloop.hpp"





XTermEventLoop::XTermEventLoop()
    : receiver(mutex, queue), sender(receiver), should_exit(false) {}

XTermEventLoop::~XTermEventLoop() {
    should_exit = true;
    thread.join();
}

void XTermEventLoop::loop() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 1;
    char buffer[128];
    auto recl = select(1, &fds, nullptr, nullptr, &tv);
    switch(recl)
    {
    case 0: break;
    case -1: break;
    default:
        if (auto s = read(STDIN_FILENO, buffer, sizeof(buffer)); s > 0) {
            for (size_t i = 0; i < s; i++)
                sender.on_char(buffer[i]);
        }
    }
}

bool XTermEventLoop::poll(ConsoleEventReceiver &recv) {
    AnyConsoleEvent event;
    {
        std::lock_guard lg(mutex);
        if (queue.empty())
            return false;
        event = std::move(queue.front());
        queue.pop();
    }
    std::visit([&](auto&& event) { recv.on_event(event); }, event);
    return true;
}

void XTermEventLoop::init() {
    run_thread();
    sender.init();
}

void XTermEventLoop::run_thread() {
    thread = std::thread([this]{ while (not should_exit) loop(); });
}
