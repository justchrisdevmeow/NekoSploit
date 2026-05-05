cmake_minimum_required(VERSION 3.20)
project(NekoSploit)

set(CMAKE_CXX_STANDARD 17)

# Injector executable
add_executable(neko_injector WIN32
    src/injector/main.cpp
    src/injector/gui.cpp
    src/injector/utils.cpp
)

target_compile_definitions(neko_injector PRIVATE _WIN32_WINNT=0x0601)
target_link_libraries(neko_injector comctl32 user32 gdi32)

# DLL
add_library(nekosploit SHARED
    src/dll/dllmain.cpp
    src/dll/hooks.cpp
    src/dll/executor.cpp
)

target_compile_definitions(nekosploit PRIVATE _WIN32_WINNT=0x0601)
target_link_libraries(nekosploit)
