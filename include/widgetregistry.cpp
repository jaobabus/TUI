#include "widgetregistry.hpp"




void WidgetRegistry::set_global_prefix(std::string_view prefix)
{
    _prefix = prefix;
}

const std::string &WidgetRegistry::get_global_prefix() const noexcept
{
    return _prefix;
}

void WidgetRegistry::register_widget(std::shared_ptr<ViewNotifier>&& ptr)
{
    auto label = ptr->label;
    register_widget(label, std::move(ptr));
}

void WidgetRegistry::register_widget(std::string_view name, std::weak_ptr<ViewNotifier>&& ptr)
{
    auto path = _prefix;
    path += name;
    _widgets[path] = std::move(ptr);
}

template<>
std::shared_ptr<ViewNotifier> WidgetRegistry::find_widget<void>(const std::string& name, bool throw_if_null) const
{
    auto path = _prefix;
    path += name;
    auto it = _widgets.find(path);
    if (it == _widgets.end()) {
        if (throw_if_null) {
            auto prefix = (get_global_prefix().size() ? "(" + get_global_prefix() + ")" : "");
            throw std::runtime_error("Trying to access to null widget " + prefix + name);
        }
        return {};
    }
    return it->second.lock();
}


WidgetRegistry global_registry;
