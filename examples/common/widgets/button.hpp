#pragma once

#include <minijson/json.h>

#include "consolewidget.hpp"
#include "textmap.hpp"
#include "widgetregistry.hpp"



class Button : public ViewNotifier
{
public:
    // Constructor
    Button(std::string label = "")
        : ViewNotifier(std::move(label)), pressed(false) {}
    virtual ~Button() {}

    virtual void press() {
        pressed = true;
        notify();
    }

    virtual void release() {
        pressed = false;
        notify();
    }

    virtual void click(conevent::MouseButton btn) {}

    bool is_pressed() const {
        return pressed;
    }

private:
    bool pressed;
};



class ButtonView : public ConsoleView
{
public:
    enum State
    {
        Normal,
        Hovered,
        Pressed,
        Disabled
    };

public:
    ButtonView(std::shared_ptr<Button> butt = {}, std::string&& label = "")
        : ConsoleView(std::move(label)), button(std::move(butt)), current_texture(Normal), textures(4, nullptr)
    {}
    virtual ~ButtonView() {}

    void notify(const ViewNotifier* notifier) override {
        // Log->trace("ButtonView({})::notify", label);
        // Redraw or update view based on the button state
    }

    void load_texture(std::shared_ptr<TextMap> texture, State state) {
        Log->trace("ButtonView({})::load_texture({})", label, (int)state);
        textures[state] = std::move(texture);
    }

    void draw(ConsoleArea* area) const override {
        if (textures[current_texture]) {
            textures[current_texture]->draw(area);
        } else {
            ::draw(area, {0, 0}, "<no tex:" + label + ">");
        }
    }

    bool contains(Vector2u pos) const override {
        return (textures[current_texture] ? pos < textures[current_texture]->size : pos.y == 0 and pos.x < 8);
    }

    const ViewNotifier* primary_widget() const override { return button.get(); }

    const std::vector<std::shared_ptr<TextMap>>& get_textures() const { return textures; }

public:
    void on_event(const conevent::MouseEnterExitEvent& event) override {
        if (event.is_enter) {
            current_texture = Hovered;
            mouse_inside = true;
        }
        else {
            current_texture = Normal;
            mouse_inside = false;
        }
    }
    void on_event(const conevent::MouseButtonEvent& event) override {
        if (event.is_down) {
            current_texture = Pressed;
            button->press();
        }
        else {
            current_texture = (mouse_inside ? Hovered : Normal);
            button->release();
            if (mouse_inside)
                button->click(event.button);
        }
    }

public:
    std::optional<std::string> widget_name() const
    {
        if (button)
            return button->label;
        return std::nullopt;
    }

    void resolve_widget(std::optional<std::string>&& name)
    {
        if (name)
            button = global_registry.find_widget<Button>(*name);
    }

    static constexpr auto json_properties()
    {
        using namespace mini_json;
        return make_tuple(prop("type") = "button-view",
                          prop("label")[&ButtonView::label],
                          prop("widget")[&ButtonView::widget_name](&ButtonView::resolve_widget),
                          prop("textures")[&ButtonView::textures]);
    }

private:
    std::shared_ptr<Button> button;
    State current_texture;
    std::vector<std::shared_ptr<TextMap>> textures;
};
