#pragma once

#include <vector>
#include <minijson/json.h>
#include "consolewidget.hpp"
#include "textmap.hpp"
#include "utils.hpp"
#include "widgetregistry.hpp"




inline
std::string encode_utf8(wchar_t ch)
{
    if ((uint32_t)ch < 127)
        return std::string(1, ch);
    else
        return "?";
}


class TextArea : public ViewNotifier
{
public:
    using ViewNotifier::ViewNotifier;

public:
    virtual void insert_char(wchar_t ch, Vector2u pos)
    {
        if (not editable)
            return;
        if (pos.y < lines.size() and pos.x <= lines[pos.y].size()
                or pos.y == lines.size() and pos.x == 0)
        {
            if (pos.y == lines.size())
                lines.push_back("");
            lines[pos.y].insert(pos.x, encode_utf8(ch));
            this->notify();
        }
    }

    virtual void insert_line(unsigned y)
    {
        if (not editable)
            return;
        if (y < lines.size())
        {
            lines.insert(lines.begin() + y, std::string{""});
            this->notify();
        }
    }

    virtual void pop_char(Vector2u pos)
    {
        if (not editable)
            return;
        if (pos.y < lines.size() and pos.x < lines[pos.y].size())
        {
            lines[pos.y].erase(pos.x, 1);
            this->notify();
        }
    }

    void clear()
    {
        lines.clear();
        this->notify();
    }

    void load(std::string_view data)
    {
        clear();
        lines.push_back("");
        for (auto c : data) {
            if (c == '\n')
                lines.push_back("");
            else if (c == '\r')
                continue;
            else
                lines.emplace_back(std::string{1, c});
        }
        this->notify();
    }

public:
    bool editable = true;

private:
    std::vector<std::string> lines;

};



class TextAreaView : public ConsoleView {
public:
    TextAreaView(std::string&& label = {}, Vector2u size = {0, 0}, std::shared_ptr<TextArea> area = {})
        : ConsoleView(std::move(label)), text_map(size), area(area) {}

public:
    void draw(ConsoleArea* area) const override {
        text_map.draw(area);
    }

    bool contains(Vector2u pos) const override {
        return pos < text_map.size;
    }

public:
    void on_event(const conevent::MouseButtonEvent& event) override {
        if (event.is_down && contains(event.position)) {
            cursor_pos = event.position;
        }
    }

    void on_event(const conevent::KeyboardEvent& event) override {
        if (contains(cursor_pos)) {
            if (event.key == Key::Backspace) {
                if (cursor_pos.x > 0) {
                    cursor_pos.x--;
                    area->pop_char(cursor_pos);
                }
            } else if (event.key == Key::Enter) {
                cursor_pos.x = 0;
                area->insert_line(cursor_pos.y);
                cursor_pos.y++;
            } else {
                wchar_t ch = static_cast<wchar_t>(event.key);
                area->insert_char(ch, cursor_pos);
                cursor_pos.x++;
            }
        }
    }

public:
    std::optional<std::string> widget_name() const
    {
        if (area)
            return area->label;
        return std::nullopt;
    }

    void bind_widget(std::optional<std::string>&& name)
    {
        if (name)
            area = global_registry.find_widget<TextArea>(*name);
    }

    Vector2u area_size() const
    {
        return text_map.size;
    }

    void set_area_size(Vector2u size)
    {
        text_map.resize(size);
    }

    static constexpr auto json_properties()
    {
        using namespace mini_json;
        return std::make_tuple
                (
                    prop("widget")[&TextAreaView::widget_name](&TextAreaView::bind_widget),
                    prop("size")[&TextAreaView::area_size](&TextAreaView::set_area_size),
                    prop("label")[&TextAreaView::label]
                );
    }

private:
    TextMap text_map;
    Vector2u cursor_pos{0, 0};
    std::shared_ptr<TextArea> area;

};
