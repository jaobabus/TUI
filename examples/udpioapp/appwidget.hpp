#pragma once

#include "consolewidget.hpp"




class AppWidget : public ViewNotifier
{
public:
    std::atomic<bool> need_closed = false;

};
