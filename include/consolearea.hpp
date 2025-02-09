#ifndef CONSOLEAREA_HPP
#define CONSOLEAREA_HPP

#include <vector>
#include <string>

#include "utils.hpp"
#include "charunit.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>


inline auto Log = spdlog::rotating_logger_mt("Log", "logs/latest.txt", 1024 * 1024, 128);


class ConsoleArea
{
public:
    ConsoleArea(Vector2u size)
        : size(size) {}
    ~ConsoleArea() {}

public:
    virtual void set_char(Vector2u pos,
                          CharUnit&& ch) = 0;
    virtual const CharUnit& get_char(Vector2u pos) const = 0;

    virtual void clear() {
        for (unsigned y = 0; y < size.y; y++)
            for (unsigned x = 0; x < size.x; x++)
                set_char({x, y}, CharUnit{CharUnit::none});
    }

public:
    void draw(ConsoleArea* area) const
    {
        for (unsigned y = 0; y < std::min<unsigned>(area->size.y, size.y); y++)
            for (unsigned x = 0; x < std::min<unsigned>(area->size.x, size.x); x++)
                area->set_char({x, y}, CharUnit{get_char({x, y})});
        constexpr auto j = sizeof(area);
    }

    void f(int ff) {}

public:
    Vector2u size;

};

inline static
void draw(ConsoleArea* area, Vector2u pos, std::string const& str, CharUnit::Meta const& styles = {})
{
    if (pos.x >= area->size.x or pos.y >= area->size.y)
        return;
    unsigned y = pos.y, x = pos.x, xs = area->size.x - pos.x;
    for (auto c : str)
    {
        if (x >= xs or c == '\n') {
            y = y + 1;
            x = pos.x;
            if (c == '\n')
                continue;
        }
        if (y >= area->size.y)
            break;
        area->set_char({x, y}, {c, styles});
        ++x;
    }
}



#endif // CONSOLEAREA_HPP
