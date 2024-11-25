#pragma once

#include "client.hpp"
#include "server.hpp"


namespace udpio
{


class App
{
public:
    App() {}

public:
    void run();

private:
    Client client;
    Server server;

private:
    std::thread client_thread;
    std::thread server_thread;

};


}
