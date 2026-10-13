# mapedit-port: the 1994 Colonization map editor on SDL3 (docs/plan.md).
#
#   cmake -B build && cmake --build build && ctest --test-dir build
#
# M0 builds the platform host and its self-test. The imported editor (src/) joins at M1.
cmake_minimum_required(VERSION 3.16)
project(mapedit_port C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)

if(MSVC)
  set(WARN /W4)
else()
  set(WARN -Wall -Wextra)
endif()

add_library(host STATIC platform/host_sdl.c)
target_include_directories(host PUBLIC platform)
target_link_libraries(host PUBLIC SDL3::SDL3)
target_compile_options(host PRIVATE ${WARN})

enable_testing()
add_executable(host_selftest tests/host_selftest.c)
target_link_libraries(host_selftest PRIVATE host)
target_compile_options(host_selftest PRIVATE ${WARN})
add_test(NAME host_selftest COMMAND host_selftest)
