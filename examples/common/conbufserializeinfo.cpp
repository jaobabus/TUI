#include "conbufserializeinfo.hpp"





template<typename T, typename Variant>
static void set_view_if(Variant& cont, std::shared_ptr<T>&& view)
{
    if (view)
        cont = view;
}

template<typename... Args>
static void set_view(std::variant<Args...>& cont, std::shared_ptr<ConsoleView> view)
{
    (set_view_if(cont, std::dynamic_pointer_cast<typename Args::element_type>(view)), ...);
}

AllViewsVariant mini_json::SerializeInfo<ConsoleViewWidget>::dump_view(const ConsoleViewWidget* buf)
{
    AllViewsVariant view;
    set_view(view, buf->get_view());
    return view;
}

void mini_json::SerializeInfo<ConsoleViewWidget>::load_view(ConsoleViewWidget* buf, AllViewsVariant&& view)
{
    std::visit([&](auto v) { buf->set_view(v); }, view);
}



std::vector<std::shared_ptr<ConsoleViewWidget>> mini_json::SerializeInfo<ConsoleZBuffer>::dump_views(const ConsoleZBuffer* buf)
{
    std::vector<std::shared_ptr<ConsoleViewWidget>> views;
    for (auto& view : buf->get_views()) {
        views.push_back(view);
    }
    return views;
}

void mini_json::SerializeInfo<ConsoleZBuffer>::load_views(ConsoleZBuffer* buf, std::vector<std::shared_ptr<ConsoleViewWidget>>&& views)
{
    for (auto&& view : views) {
        buf->add_view(std::move(view));
    }
}
