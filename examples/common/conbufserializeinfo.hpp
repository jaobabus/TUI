#pragma once

#include "allviews.hpp"
#include "consolezbuffer.hpp"




template<>
class mini_json::SerializeInfo<ConsoleViewWidget>
{
public:
    static AllViewsVariant dump_view(const ConsoleViewWidget* buf);
    static void load_view(ConsoleViewWidget* buf, AllViewsVariant&& view);

public:
    static constexpr auto fields()
    {
        using std::make_tuple;
        using mini_json::prop;
        return make_tuple
                (
                    prop("type") = "console-view-widget",
                    prop("position")[&ConsoleViewWidget::get_position](&ConsoleViewWidget::set_position),
                    prop("view")[&SerializeInfo::dump_view](&SerializeInfo::load_view)
                );
    }

};




template<>
class mini_json::SerializeInfo<ConsoleZBuffer>
{
public:
    static std::vector<std::shared_ptr<ConsoleViewWidget>> dump_views(const ConsoleZBuffer* buf);
    static void load_views(ConsoleZBuffer* buf, std::vector<std::shared_ptr<ConsoleViewWidget>>&& views);

public:
    static constexpr auto fields()
    {
        using std::make_tuple;
        using mini_json::prop;
        return make_tuple
                (
                    prop("type") = "console-buffer",
                    prop("views")[&SerializeInfo::dump_views](&SerializeInfo::load_views)
                    );
    }

};
