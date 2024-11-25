#pragma once

#include <filesystem>
#include <queue>
#include <thread>

#include "common/widgets/button.hpp"
#include "consolezbuffer.hpp"
#include "eventroot.hpp"
#include "textmap.hpp"
#include "serializable.hpp"
#include "xtermeventloop.hpp"



namespace editable
{



class ConsoleSink : public ConsoleArea
{
public:
    ConsoleSink(Vector2u size = {0, 0}) : ConsoleArea(size), console(size) {}

    Vector2u update_size();
    void init();

    Vector2u homepos() const {
        return home_position;
    }

    void set_char(Vector2u pos, CharUnit&& ch) override {
        if (pos < size) {
            console.set_char(pos, std::move(ch));
        }
    }

    const CharUnit& get_char(Vector2u pos) const override {
        if (pos < size) {
            return console.get_char(pos);
        }
        return CharUnit::none;
    }

    void draw() {
        // Восстанавливаем положение для цикла
        fwrite("\e8", 2, 1, stdout);
        fwrite("\e7", 2, 1, stdout);
        auto w = console.print();
        fwrite(w.data(), w.size(), 1, stdout);
        fflush(stdout);
    }

private:
    STDIOTextArea console;
    Vector2u home_position;
};


class EditableViewApp
{
public:
    EditableViewApp()
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



}
