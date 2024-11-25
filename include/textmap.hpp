#ifndef TEXTMAP_HPP
#define TEXTMAP_HPP

#include "consolearea.hpp"



class TextMap : public ConsoleArea
{
public:
    TextMap(Vector2u size = {0, 0})
        : ConsoleArea(size)
    {
        _map.resize(size.x * size.y, CharUnit::none);
    }

    void set_char(Vector2u pos, CharUnit&& ch) override
    {
        if (pos < size)
            _map[pos.x + pos.y * size.x] = std::move(ch);
    }

    CharUnit const& get_char(Vector2u pos) const override
    {
        if (pos < size)
            return _map[pos.x + pos.y * size.x];
        return CharUnit::none;
    }

public:
    static TextMap create_from(std::string const& tex);
    void load_from(std::string const& tex);
    void resize(Vector2u size);

public:
    TextMap strip() const;

protected:
    std::vector<CharUnit> _map;

};


class STDIOTextArea : public TextMap
{
public:
    using TextMap::TextMap;

public:
    std::string print(std::string&& old = {}) const;

};



#endif // TEXTMAP_HPP
