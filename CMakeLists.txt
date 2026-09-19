cmake_minimum_required(VERSION 3.20)

project(ACEMU
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)

add_executable(ACEMU
    src/main.c
    src/app.c
    src/cpu.c
    src/memory.c
    src/crt.c
    src/ui.c
)

target_include_directories(ACEMU PRIVATE
    include
)

target_link_libraries(ACEMU PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
)

if (MSVC)
    string(REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    target_compile_options(ACEMU PRIVATE /W4)
else()
    target_compile_options(ACEMU PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()