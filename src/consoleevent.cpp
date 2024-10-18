#include "consoleevent.hpp"



using namespace conevent;


void XTermConsoleEventSender::xterm_arrow(int count, char chr)
{
    const static std::map<char, Key> mapping = {
        {'A', Key::Up},
        {'B', Key::Down},
        {'C', Key::Right},
        {'D', Key::Left},
    };
    KeyboardEvent event;
    event.key = mapping.at(chr);
    event.count = count;
    _receiver.on_event(event);
}

void XTermConsoleEventSender::xterm_status(int status)
{
    if (status != 3 and status != 0)
        throw std::runtime_error("Unknown  xterm status " + std::to_string(status));
    _receiver.on_event(XTermStatusEvent{status});
}

void XTermConsoleEventSender::xterm_fn_key(int key)
{
    KeyboardEvent event;
    auto fn_index = key - 11;
    if (fn_index >= 0 && fn_index < 12) {
        event.key = static_cast<Key>(fn_index + static_cast<int>(Key::F1));
        _receiver.on_event(event);
    } else {
        throw std::runtime_error("Unknown Fn key " + std::to_string(fn_index));
    }
}

void XTermConsoleEventSender::xterm_btn_urxvt_M(int btn, int x, int y)
{
    CtrlStatus status;
    status = (btn & 4 ? CtrlStatus::Shift : CtrlStatus::None)
            | (btn & 8 ? CtrlStatus::Meta : CtrlStatus::None)
            | (btn & 16 ? CtrlStatus::Control : CtrlStatus::None);
    if (btn & 32) {
        MouseMoveEvent event;
        event.status = status;
        event.position = { static_cast<uint32_t>(x - 1), static_cast<uint32_t>(y - 1) };
        event.previous = _last_mouse_pos;
        _last_mouse_pos = event.position;
        _receiver.on_event(event);
    }
    else {
        MouseButtonEvent event;
        event.status = status;
        event.position = { static_cast<uint32_t>(x - 1), static_cast<uint32_t>(y - 1) };
        event.button = translate_cb(btn);
        event.is_down = (event.button != MouseButton::Unspecified);
        _receiver.on_event(event);
    }
}

void XTermConsoleEventSender::xterm_btn_sgr_M(int btn, int x, int y)
{
    CtrlStatus status;
    status = (btn & 4 ? CtrlStatus::Shift : CtrlStatus::None)
            | (btn & 8 ? CtrlStatus::Meta : CtrlStatus::None)
            | (btn & 16 ? CtrlStatus::Control : CtrlStatus::None);
    if (btn & 32) {
        MouseMoveEvent event;
        event.status = status;
        event.position = { static_cast<uint32_t>(x - 1), static_cast<uint32_t>(y - 1) };
        event.previous = _last_mouse_pos;
        _last_mouse_pos = event.position;
        _receiver.on_event(event);
    }
    else {
        MouseButtonEvent event;
        event.status = status;
        event.position = { static_cast<uint32_t>(x - 1), static_cast<uint32_t>(y - 1) };
        event.button = translate_cb(btn);
        event.is_down = (event.button != MouseButton::Unspecified);
        _receiver.on_event(event);
    }
}

void XTermConsoleEventSender::xterm_btn_sgr_m(int btn, int x, int y)
{
    CtrlStatus status;
    status = (btn & 4 ? CtrlStatus::Shift : CtrlStatus::None)
            | (btn & 8 ? CtrlStatus::Meta : CtrlStatus::None)
            | (btn & 16 ? CtrlStatus::Control : CtrlStatus::None);
    if (btn & 32) {
        throw std::runtime_error("Motion at 'm' mod?");
    }
    else {
        MouseButtonEvent event;
        event.status = status;
        event.position = { static_cast<uint32_t>(x - 1), static_cast<uint32_t>(y - 1) };
        event.button = translate_cb(btn);
        event.is_down = false;
        _receiver.on_event(event);
    }
}

void XTermConsoleEventSender::xterm_dsr(int x, int y)
{
    MouseMoveEvent event;
    event.position = { static_cast<uint32_t>(x - 1), static_cast<uint32_t>(y - 1) };
    _receiver.on_event(event);
}
