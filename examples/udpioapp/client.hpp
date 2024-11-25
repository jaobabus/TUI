#pragma once

#include "consolesink.hpp"
#include "eventroot.hpp"
#include "xtermeventloop.hpp"
#include <filesystem>



class Client
{
public:
    Client()
        : _root{_buffer} {}

private:
    void init();
    void make_default();
    void load_view(const std::filesystem::path& view);
    void save_view(const std::filesystem::path& view);
    void _run();

public:
    void run();
    void save();

private:
    ConsoleSink _sink;
    XTermEventLoop _loop;
    EventRoot _root;
    ConsoleZBuffer _buffer;

};

