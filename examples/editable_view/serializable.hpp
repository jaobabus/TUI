#pragma once

#include "textmap.hpp"
#include <minijson/json.h>
#include <consolezbuffer.hpp>



template<>
class mini_json::SerializeInfo<Vector2u> : public Vector2u
{
public:
    static constexpr auto fields()
    {
        using std::make_tuple;
        using mini_json::prop;
        return make_tuple
                (
                    prop("x")[&Vector2u::x],
                    prop("y")[&Vector2u::y]
                );
    }

};


struct TextMapStyle {
    struct InsertAt {
        Vector2u position;
        std::optional<unsigned> background;
        std::optional<unsigned> foreground;

        constexpr static auto json_properties() {
            return std::make_tuple(
                mini_json::prop("position")[&InsertAt::position],
                mini_json::prop("background")[&InsertAt::background],
                mini_json::prop("foreground")[&InsertAt::foreground]
            );
        }
    };

    struct Apply {
        Vector2u position;
        Vector2u size;
        std::optional<unsigned> foreground;

        constexpr static auto json_properties() {
            return std::make_tuple(
                mini_json::prop("position")[&Apply::position],
                mini_json::prop("size")[&Apply::size],
                mini_json::prop("foreground")[&Apply::foreground]
            );
        }
    };

    std::optional<InsertAt> insert_at;
    std::optional<Apply> apply;

    constexpr static auto json_properties() {
        return std::make_tuple(
            mini_json::prop("insert_at")[&TextMapStyle::insert_at],
            mini_json::prop("apply")[&TextMapStyle::apply]
        );
    }
};

struct TextMapData {
    Vector2u size;
    std::vector<std::string> data;
    std::vector<TextMapStyle> style;

    constexpr static auto json_properties() {
        return std::make_tuple(
            mini_json::prop("size")[&TextMapData::size],
            mini_json::prop("data")[&TextMapData::data],
            mini_json::prop("style")[&TextMapData::style]
        );
    }
};


template<>
class mini_json::SerializeInfo<TextMap>
{
public:
    static void load(TextMap* th, TextMapData&& data)
    {
        std::vector<TextMapStyle::InsertAt> insertes;
        std::vector<TextMapStyle::Apply> applies;
        for (auto& style : data.style) {
            if (style.apply)
                applies.emplace_back(*style.apply);
            if (style.insert_at)
                insertes.emplace_back(*style.insert_at);
        }
        std::sort(begin(insertes), end(insertes),
                  [&](auto& a, auto& b)
        {
            auto diff = Vector2i{a.position} - Vector2i{b.position};
            return diff.x + diff.y * data.size.x;
        });
        std::sort(begin(applies), end(applies),
                  [&](auto& a, auto& b)
        {
            auto diff = Vector2i{a.position} - Vector2i{b.position};
            return diff.x + diff.y * data.size.x;
        });

        std::string tex;
        for (auto& l : data.data)
            tex += l + "\n";
        tex.pop_back();
        (TextMap&)*th = TextMap{data.size};
        for (size_t i = 0; i < insertes.size(); i++)
        {
            auto ins = insertes[i];
            auto start = ins.position;
            auto stop = (i != insertes.size() - 1 ? insertes[i + 1].position : data.size);
            auto start_i = (start.x + start.y * data.size.x);
            auto count = start_i - (stop.x + stop.y * data.size.x);
            CharUnit::Meta meta;
            if (ins.background)
                meta.background = *ins.background;
            if (ins.foreground)
                meta.foreground = *ins.foreground;
            ::draw(th, start, tex.substr(start_i, count), meta);
        }
    }

    static const TextMapData& dump(const TextMap* th)
    {
        // Create a TextMapData instance representing the current state of the TextMap
        // Placeholder implementation
        static TextMapData data;
        (Vector2u&)data.size = th->size;

        data.data.resize(th->size.y);
        for (unsigned y = 0; y < th->size.y; ++y) {
            std::wstring line;
            for (unsigned x = 0; x < th->size.x; ++x) {
                CharUnit cu = th->get_char({x, y});
                line.push_back(cu.character);
            }
            data.data[y] = std::string(line.begin(), line.end());  // Convert wstring to string
        }

        // Dump insert_at styles into a vector to maintain sequence
        std::vector<TextMapStyle::InsertAt> insert_at_styles;
        for (unsigned y = 0; y < th->size.y; ++y) {
            for (unsigned x = 0; x < th->size.x; ++x) {
                CharUnit cu = th->get_char({x, y});
                if (insert_at_styles.empty() ||
                    (insert_at_styles.back().background != cu.meta.background.term_color ||
                     insert_at_styles.back().foreground != cu.meta.foreground.term_color)) {
                    insert_at_styles.push_back(TextMapStyle::InsertAt{{x, y}, cu.meta.background.term_color, cu.meta.foreground.term_color});
                }
            }
        }

        // Add insert_at styles to the styles vector
        for (const auto& insert_at : insert_at_styles) {
            data.style.push_back({});
            data.style.back().insert_at = insert_at;
        }

        return data;
    }

    static constexpr auto fields()
    {
        using std::make_tuple;
        return make_tuple
                (
                    mini_json::prop("text_map")[&SerializeInfo::dump](&SerializeInfo::load)
                );
    }

};
