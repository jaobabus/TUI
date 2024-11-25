#pragma once

#include <memory>
#include <string_view>

#include "consolewidget.hpp"




template<typename T>
struct WidgetTypeS
{
    using type = T;
    static constexpr bool need_to_throw_by_default = true;
};

template<>
struct WidgetTypeS<void>
{
    using type = ViewNotifier;
    static constexpr bool need_to_throw_by_default = false;
};

template<typename T>
using WidgetType = typename WidgetTypeS<T>::type;
template<typename T>
inline constexpr bool WidgetNeedThrow = WidgetTypeS<T>::need_to_throw_by_default;


class WidgetRegistry
{
public:
    WidgetRegistry() {}

public:
    void set_global_prefix(std::string_view prefix);
    const std::string& get_global_prefix() const noexcept;

public:
    void register_widget(std::shared_ptr<ViewNotifier>&& ptr);
    void register_widget(std::string_view name, std::weak_ptr<ViewNotifier>&& ptr);

    template<typename T = void>
    std::shared_ptr<WidgetType<T>> find_widget(const std::string& name, bool throw_if_null = WidgetNeedThrow<T>) const
    {
        auto w = find_widget<void>(name, throw_if_null);
        return std::dynamic_pointer_cast<T>(w);
    }

private:
    std::map<std::string, std::weak_ptr<ViewNotifier>> _widgets;
    std::string _prefix;

};

template<>
std::shared_ptr<WidgetType<void>> WidgetRegistry::find_widget<void>(const std::string& name, bool throw_if_null) const;


extern WidgetRegistry global_registry;
