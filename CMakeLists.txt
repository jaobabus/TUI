cmake_minimum_required(VERSION 3.16)

set(PROJECT tui_framwork)
project(${PROJECT})

set(CMAKE_CXX_STANDARD 20)
add_compile_options("-ftemplate-backtrace-limit=0")


find_package(spdlog REQUIRED)


set(SOURCES
    include/utils.hpp
    src/utils.cpp
    include/consolearea.hpp
    include/consolewidget.hpp
    src/consolewidget.cpp
    include/buttonwidget.hpp
    src/buttonwidget.cpp
    include/textmap.hpp
    src/textmap.cpp
    include/properties.hpp
    src/properties.cpp
    include/kbevent.hpp
    include/consoleevent_t.hpp
    include/escapeparsehelper.hpp
    src/escapeparsehelper.cpp
    include/consoleevent.hpp
    src/consoleevent.cpp
    src/consoleevent_rules.cpp
    src/coprintf.cpp
    include/consolezbuffer.hpp
    src/consolezbuffer.cpp
    include/asciiescape.hpp
    include/asciistyles.hpp
    include/termcolor.hpp
    src/termcolor.cpp
    include/charunit.hpp
    src/charunit.cpp
    main.cpp
    )


set(EXAMPLES_EDITABLE_VIEW_SOURCES
    examples/editable_view/serializable.hpp
    examples/editable_view/editable.hpp
    examples/editable_view/editable.cpp
    examples/editable_view/layout.hpp)


set(EXAMPLES_SOURCES
    examples/button/main.cpp

    ${EXAMPLES_EDITABLE_VIEW_SOURCES})


add_executable(${PROJECT} ${SOURCES} ${EXAMPLES_SOURCES}
    include/eventroot.hpp src/eventroot.cpp
    examples/udpioapp/udpioapp.hpp examples/udpioapp/udpioapp.cpp
    include/xtermeventloop.hpp src/xtermeventloop.cpp
    examples/udpioapp/server.hpp examples/udpioapp/server.cpp
    examples/common/widgets/button.hpp
    examples/common/widgets/textarea.hpp examples/common/widgets/textarea.cpp
    examples/udpioapp/common.hpp examples/udpioapp/common.cpp
    examples/udpioapp/client.hpp examples/udpioapp/client.cpp
    examples/udpioapp/consolesink.hpp
    include/widgetregistry.hpp include/widgetregistry.cpp
    examples/udpioapp/appwidget.hpp
    examples/common/conbufserializeinfo.hpp examples/common/conbufserializeinfo.cpp
    examples/common/allviews.hpp)
target_include_directories(${PROJECT} PUBLIC include examples)
target_link_libraries(${PROJECT} PUBLIC spdlog::spdlog)
