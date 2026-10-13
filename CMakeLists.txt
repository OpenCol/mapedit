# mapedit-port: the 1994 Colonization map editor on SDL3 (docs/plan.md).
#
#   cmake -B build && cmake --build build && ctest --test-dir build
#
# The platform host, MSC 6's run-time (port/), and the imported editor (src/, compiled as
# an object library until it links: M2).
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

# MSC 6's run-time library as the editor calls it (msc_*)
add_library(msc STATIC port/msc_runtime.c)
target_include_directories(msc PUBLIC port)
target_compile_options(msc PRIVATE ${WARN})
if(NOT MSVC)
  target_compile_options(msc PUBLIC -fsigned-char)
endif()

# The imported editor (tools/import.py). Its warnings policy: the leftovers of byte-matching
# (stand-in declarations, unused locals that fixed MSC's stack layout) are not reported;
# everything that can change behaviour is.
file(GLOB EDITOR_SOURCES CONFIGURE_DEPENDS src/editor/*.c src/madsdev/*.c)
list(REMOVE_ITEM EDITOR_SOURCES ${CMAKE_SOURCE_DIR}/src/madsdev/error_1.c)   # unfinished upstream
add_library(editor OBJECT ${EDITOR_SOURCES})
target_include_directories(editor PRIVATE port src/include)
if(MSVC)
  target_compile_options(editor PRIVATE /W3)
else()
  target_compile_options(editor PRIVATE -fsigned-char -Wall -Wextra -Wno-unused-variable
    -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-pointer-sign
    -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses -Wno-misleading-indentation)
endif()

enable_testing()
add_executable(msc_selftest tests/msc_selftest.c)
target_link_libraries(msc_selftest PRIVATE msc)
target_compile_options(msc_selftest PRIVATE ${WARN})
add_test(NAME msc_selftest COMMAND msc_selftest)

add_executable(host_selftest tests/host_selftest.c)
target_link_libraries(host_selftest PRIVATE host)
target_compile_options(host_selftest PRIVATE ${WARN})
add_test(NAME host_selftest COMMAND host_selftest)
