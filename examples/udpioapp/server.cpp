#include "server.hpp"
#include "appwidget.hpp"
#include "../common/widgets/textarea.hpp"
#include "widgetregistry.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <queue>
#include <sys/socket.h>


namespace
{
using namespace udpio;


struct SocketIP4
{
    SocketIP4(int fd = -1) : fd(fd) {}
    SocketIP4(const SocketIP4&) = delete;
    SocketIP4(SocketIP4&& mv) : fd(mv.fd) { mv.fd = -1; }
    ~SocketIP4() { close(fd); }

    SocketIP4& operator=(SocketIP4&& mv) { fd = mv.fd; mv.fd = -1; return *this; }

    ssize_t send_to(std::string_view data, sockaddr_in addr)
    {
        return sendto(fd, data.data(), data.size(), 0, (const sockaddr*)&addr, sizeof(addr));
    }

    std::pair<std::string, sockaddr_in> poll()
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1;
        char buffer[4096];
        auto recl = select(1, &fds, nullptr, nullptr, &tv);
        switch(recl)
        {
        case 0: break;
        case -1: break;
        default: {
            sockaddr_in ip4;
            socklen_t len = sizeof(ip4);
            if (auto s = recvfrom(fd, buffer, sizeof(buffer), 0, (sockaddr*)&ip4, &len); s > 0)
                return std::make_pair(std::string{buffer, buffer + s}, ip4);
        }
        }
        return {};
    }

    int fd;
};


std::string dump(sockaddr_in addr)
{
    std::ostringstream oss;
    oss << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port);
    return oss.str();
}


class Receiver
{
public:
    struct Task
    {
        std::chrono::steady_clock::time_point expire_time;
        std::function<void()> callback;

        bool operator<(const Task& r) const {
            return (ssize_t)expire_time.time_since_epoch().count() - (ssize_t)r.expire_time.time_since_epoch().count();
        }
    };

public:
    Receiver() { poll_loop_thr = std::thread([this] { poll_loop(); }); }
    ~Receiver() { sock.fd = -2; poll_loop_thr.join(); }

public:
    std::shared_ptr<CallbackButton> bind = std::make_shared<CallbackButton>("bind-button");
    std::shared_ptr<TextArea> address_area = std::make_shared<TextArea>("address-area");
    std::shared_ptr<TextArea> packet_area = std::make_shared<TextArea>("packet-area");

public:
    void on_bind(std::string_view saddr)
    {
        std::lock_guard lg(mtx);
        auto addr = parse(saddr);
        if (addr.sin_port == 0)
        {
            packet_area->load("Bind error: invalid address");
            return;
        }
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0)
        {
            packet_area->load("Bind error: " + std::string(strerror(errno)));
            return;
        }
        sock = SocketIP4{fd};
        packet_area->load("Waiting packets from " + dump(addr) + "...");
    }

    void on_packet(std::string_view raw, sockaddr_in addr)
    {
        std::lock_guard lg(mtx);
        auto data = dump(addr) + escape_string(raw);
        packet_area->load(data);
    }

    void tick()
    {
        std::lock_guard lg(mtx);
        if (tasks.size()) {
            auto& top = tasks.top();
            if (top.expire_time >= std::chrono::steady_clock::now())
            {
                top.callback();
                tasks.pop();
            }
        }
    }

public:
    void poll_loop()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::lock_guard lg(mtx);
            if (sock.fd == -1)
                continue;
            if (sock.fd == -2)
                return;
            auto [data, addr] = sock.poll();

        }
    }

private:
    static sockaddr_in parse(std::string_view saddr)
    {
        std::istringstream iss{std::string{saddr}};
        char seps[5] = {0};
        uint8_t address[4];
        uint16_t port;
        iss >> address[3] >> seps[0] >> address[2] >> seps[1] >> address[1] >> seps[2] >> address[0] >> seps[3];
        iss >> port;
        if (not iss or std::string(seps) != "...:")
            return {0, 0};
        return {AF_INET, htons(port), {(uint32_t&)address}};
    }

private:
    std::mutex mtx;
    SocketIP4 sock;
    std::thread poll_loop_thr;
    std::priority_queue<Task> tasks;

};


}



class udpio::Server::PImpl
{
public:
    Receiver receiver;

public:
    void tick()
    {
        receiver.tick();
    }

};

Server::Server() {
    impl = new PImpl;
}

udpio::Server::~Server()
{
    delete impl;
}

void udpio::Server::init()
{
    add_widget(impl->receiver.bind);
    add_widget(impl->receiver.address_area);
    add_widget(impl->receiver.packet_area);
}

void udpio::Server::run()
{
    while (not global_registry.find_widget<AppWidget>("app")->need_closed)
    {
        impl->tick();
        std::this_thread::sleep_for(std::chrono::microseconds(33));
    }
}

void Server::add_widget(std::shared_ptr<ViewNotifier> ptr)
{
    auto& label = ptr->label;
    global_registry.register_widget(label, std::move(ptr));
}
