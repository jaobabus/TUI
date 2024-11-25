#include "udpioapp.hpp"
#include "appwidget.hpp"
#include "widgetregistry.hpp"



void udpio::App::run()
{
    server_thread = std::thread([this]
    {
        server.init();
        server.run();
    });
    client_thread = std::thread([this]
    {
        client.run();
    });
    while (not global_registry.find_widget<AppWidget>("app")->need_closed)
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    client_thread.join();
    server_thread.join();
}


extern int udpio_example_run();
int udpio_example_run()
{
    global_registry.set_global_prefix("udpio/");
    auto app_widget = std::make_shared<AppWidget>();
    global_registry.register_widget("app", app_widget);
    udpio::App app{};
    app.run();
    return 0;
}
