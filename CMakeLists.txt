cmake_minimum_required(VERSION 3.20)
project(NekoSploit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Injector executable
add_executable(neko_injector WIN32
    src/injector/main.cpp
    src/injector/gui.cpp
    src/injector/utils.cpp
    src/injector/scanner.cpp
)

target_compile_definitions(neko_injector PRIVATE _WIN32_WINNT=0x0601)
target_link_libraries(neko_injector comctl32 user32 gdi32 psapi)

# DLL
add_library(nekosploit SHARED
    src/dll/dllmain.cpp
    src/dll/hooks.cpp
    src/dll/offsets.cpp
)

target_compile_definitions(nekosploit PRIVATE _WIN32_WINNT=0x0601)
target_link_libraries(nekosploit psapi)

# Set both to output to the same folder
set_target_properties(neko_injector PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(nekosploit PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
