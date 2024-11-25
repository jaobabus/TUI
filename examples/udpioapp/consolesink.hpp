#pragma once

#include <sys/ioctl.h>
#include <termios.h>

#include "consolearea.hpp"
#include "textmap.hpp"




class ConsoleSink : public ConsoleArea
{
public:
    ConsoleSink(Vector2u size = {0, 0}) : ConsoleArea(size), console(size) {}

    Vector2u update_size() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        auto now = size;
        size = {w.ws_col, w.ws_row};
        if (size != now)
            console.resize(size);
        return now;
    }

    void init() {
        update_size();
        termios tios;
        tcgetattr(fileno(stdin), &tios);
        tios.c_lflag &= ~ECHO & ~ICANON;
        tcsetattr(fileno(stdin), 0, &tios);
        fwrite(std::string(24, '\n').data(), 24, 1, stdout);
        fwrite("\e[24A", 5, 1, stdout);
        fwrite("\e7", 2, 1, stdout);
        printf("\e[6n\n");
        char buffer[30];
        size_t i = 0;
        for(char ch = 0; ch != 'R'; i++) {
            auto ret = read(STDIN_FILENO, &ch, 1);
            if (ret <= 0)
                throw std::runtime_error("Error read cursor position");
            buffer[i] = ch;
        }
        buffer[i] = '\0';
        Vector2u& pos = home_position;
        if (sscanf(buffer, "\e[%u;%uR", &pos.y, &pos.x) != 2)
            throw std::runtime_error("Error parse position: " + std::string(buffer));
        pos.x -= 1, pos.y -= 1;
        Log->info("Console pos at ({}, {})", pos.x, pos.y);
    }

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
