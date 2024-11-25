#pragma once

#include "../common/widgets/button.hpp"
#include "consolewidget.hpp"
#include <string_view>




namespace udpio
{


class CallbackButton : public Button
{
public:
    using Button::Button;

public:
    std::function<void()> on_press;
    std::function<void()> on_release;
    std::function<void(conevent::MouseButton)> on_click;

public:
    void press() override { if (on_press) on_press(); Button::press(); }
    void release() override { if (on_release) on_release(); Button::release(); }
    void click(conevent::MouseButton btn) override { if (on_click) on_click(btn); Button::click(btn); }

};


class Server
{
public:
    Server();
    ~Server();

public:
    void init();
    void run();

private:
    void add_widget(std::shared_ptr<ViewNotifier> ptr);

private:
    class PImpl;
    PImpl* impl;

};


}
