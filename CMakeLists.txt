cmake_minimum_required(VERSION 3.16)

set(PROJECT tui_framwork)
project(${PROJECT})

set(CMAKE_CXX_STANDARD 20)

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


set(EXAMPLES_SOURCES
    examples/button/main.cpp)


add_executable(${PROJECT} ${SOURCES} ${EXAMPLES_SOURCES})
target_include_directories(${PROJECT} PUBLIC include)
target_link_libraries(${PROJECT} PUBLIC spdlog::spdlog)
