#pragma once

#include "consolezbuffer.hpp"
#include <consolewidget.hpp>




class EventRoot : public ConsoleEventReceiver
{
public:
    EventRoot(ConsoleZBuffer& buffer)
        : _buffer{buffer},
          _hower_view{nullptr},
          _focus_view{nullptr}
        {}


public:
    void on_event(const conevent::MouseButtonEvent& event) override {
        if (_hower_view)
            _hower_view->on_event(event);
    }

    void on_event(const conevent::MouseMoveEvent& event) override {
        auto widget_under = _buffer.get_widget_at(event.position);
        auto view_under = (widget_under ? widget_under->get_view() : nullptr);
        if (_hower_view != view_under.get())
        {
            if (_hower_view) {
                _hower_view->on_event(event);
                _hower_view->on_event(conevent::MouseEnterExitEvent{false, event.position, event.status});
            }
            if (view_under) {
                view_under->on_event(event);
                view_under->on_event(conevent::MouseEnterExitEvent{true, event.position, event.status});
            }
            _hower_view = view_under.get();
        }
        if (_hower_view)
            _hower_view->on_event(event);
    }

    void on_event(const conevent::KeyboardEvent& event) override {
        if (_focus_view)
            _focus_view->on_event(event);
    }


private:
    ConsoleZBuffer& _buffer;
    ConsoleView* _hower_view;
    ConsoleView* _focus_view;

};

