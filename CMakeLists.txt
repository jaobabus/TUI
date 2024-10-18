cmake_minimum_required(VERSION 3.16)

project(tui_framwork LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tui_framwork main.cpp)

include(GNUInstallDirs)
install(TARGETS tui_framwork
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
