#pragma once

#include "consolearea.hpp"
#include <cstdint>
#include <map>

#include "consoleevent_t.hpp"
#include "escapeparsehelper.hpp"





class ConsoleEventSender
{
public:
    virtual bool on_char(char chr) = 0;
};



class ConsoleEventReceiver
{
public:
    virtual void on_event(const conevent::FocusEvent&) {}
    virtual void on_event(const conevent::MouseEnterExitEvent&) {}
    virtual void on_event(const conevent::MouseButtonEvent&) {}
    virtual void on_event(const conevent::MouseMoveEvent&) {}
    virtual void on_event(const conevent::KeyboardEvent&) {}
    virtual void on_event(const conevent::XTermStatusEvent&) {}
    virtual void on_event(const conevent::XTermDSRPositionEvent&) {}
};



using AnyConsoleEvent = std::variant<conevent::FocusEvent, conevent::MouseEnterExitEvent, conevent::MouseButtonEvent, conevent::MouseMoveEvent, conevent::KeyboardEvent, conevent::XTermStatusEvent, conevent::XTermDSRPositionEvent>;

class ExtendedConsoleEventReceiver : public ConsoleEventReceiver
{
public:
    void on_event(const conevent::FocusEvent& event) override{
        on_any_event({event});
    }
    void on_event(const conevent::MouseEnterExitEvent& event) override {
        on_any_event({event});
    }
    void on_event(const conevent::MouseButtonEvent& event) override {
        on_any_event({event});
    }
    void on_event(const conevent::MouseMoveEvent& event) override {
        on_any_event({event});
    }
    void on_event(const conevent::KeyboardEvent& event) override {
        on_any_event({event});
    }
    void on_event(const conevent::XTermStatusEvent& event) override {
        on_any_event({event});
    }
    void on_event(const conevent::XTermDSRPositionEvent& event) override {
        on_any_event({event});
    }

public:
    virtual void on_any_event(AnyConsoleEvent&& ev) = 0;

};



class XTermConsoleEventSender : public ConsoleEventSender
{
public:
    XTermConsoleEventSender(ConsoleEventReceiver& receiver)
        : _receiver(receiver), _base_rule(make_xterm_rules()), _parse_context(_base_rule.get()) {}

public:
    void init() {
        printf("\e[?1000;1003;1006;1015h");
    }

public:
    bool on_char(char chr) override
    {
        try
        {
            _parse_context.next(chr);
            return true;
        }
        catch (const std::runtime_error& e) {
            Log->error("Runtime error: \"{}\", stack: \"{}\"", e.what(), escape_string(_parse_context.stack()));
            return false;
        }
    }

private:
    static conevent::MouseButton translate_cb(int btn)
    {
        using conevent::MouseButton;
        int idx = btn & 3;
        if (idx == 3)
            return MouseButton::Unspecified;
        if (btn & 0x80)
            throw std::runtime_error("Not supported MB6, MB7");
            // return (MouseButton)((int)MouseButton::MB6 + idx);
        if (btn & 0x40)
            return (MouseButton)((int)MouseButton::MB4 + idx);
        return (MouseButton)((int)MouseButton::MB1 + idx);
    };

private:
    void xterm_arrow(int count, char chr);
    void xterm_status(int status);
    void xterm_fn_key(int key);
    void xterm_btn_urxvt_M(int btn, int x, int y);
    void xterm_btn_sgr_M(int btn, int x, int y);
    void xterm_btn_sgr_m(int btn, int x, int y);
    void xterm_dsr(int x, int y);
    std::unique_ptr<escparse::BaseContainerRule> make_xterm_rules();

private:
    ConsoleEventReceiver& _receiver;
    const std::unique_ptr<escparse::BaseContainerRule> _base_rule;
    escparse::ParseContext _parse_context;
    Vector2u _last_mouse_pos{0, 0};
    conevent::CtrlStatus _mouse_status;

};
