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
  # a global defined in two places is an error everywhere, as it is on Windows
  add_compile_options(-fno-common)
endif()

add_library(host STATIC platform/host_sdl.c)
target_include_directories(host PUBLIC platform)
target_link_libraries(host PUBLIC SDL3::SDL3)
target_compile_options(host PRIVATE ${WARN})

find_package(Python3 COMPONENTS Interpreter REQUIRED)

# MSC 6's run-time library as the editor calls it (msc_*)
add_library(msc STATIC port/msc_runtime.c)
target_include_directories(msc PUBLIC port)
target_link_libraries(msc PUBLIC SDL3::SDL3)
target_compile_options(msc PRIVATE ${WARN})
if(NOT MSVC)
  target_compile_options(msc PUBLIC -fsigned-char)
endif()

# The imported editor (tools/import.py). Its warnings policy: the leftovers of byte-matching
# (stand-in declarations, unused locals that fixed MSC's stack layout) are not reported;
# everything that can change behaviour is.
file(GLOB EDITOR_SOURCES CONFIGURE_DEPENDS src/editor/*.c src/madsdev/*.c)
add_library(editor OBJECT ${EDITOR_SOURCES})
target_include_directories(editor PRIVATE port src/include)
if(MSVC)
  target_compile_options(editor PRIVATE /W3)
else()
  target_compile_options(editor PRIVATE -fsigned-char -Wall -Wextra -Wno-unused-variable
    -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-pointer-sign
    -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses -Wno-misleading-indentation)
endif()

# What the port writes for the functions the import refused (inline assembly): port/lib
file(GLOB PORT_LIB_SOURCES CONFIGURE_DEPENDS port/lib/*.c)
add_library(portlib OBJECT ${PORT_LIB_SOURCES})
target_include_directories(portlib PRIVATE port platform)
target_compile_options(portlib PRIVATE ${WARN})
if(NOT MSVC)
  target_compile_options(portlib PRIVATE -fsigned-char)
endif()

# The oracle tests (tests/oracle) call the port's functions through this library, beside the
# original's machine code run in an emulator. Symbols nothing calls yet may stay unresolved.
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/stubs.c
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/unresolved.py --stubs ${CMAKE_BINARY_DIR}/stubs.c
          $<TARGET_OBJECTS:editor> $<TARGET_OBJECTS:portlib> $<TARGET_FILE:msc>
  DEPENDS editor portlib msc ${CMAKE_SOURCE_DIR}/tools/unresolved.py
  COMMAND_EXPAND_LISTS
  COMMENT "weak stand-ins for what nothing defines yet")
add_library(oracle_lib SHARED $<TARGET_OBJECTS:editor> $<TARGET_OBJECTS:portlib> ${CMAKE_BINARY_DIR}/stubs.c)
target_link_libraries(oracle_lib PRIVATE msc host)
if(APPLE)
  target_link_options(oracle_lib PRIVATE -undefined dynamic_lookup)
endif()

# The editor (M2: links; M3: runs)
add_executable(mapedit port/main.c $<TARGET_OBJECTS:editor> $<TARGET_OBJECTS:portlib>)
target_include_directories(mapedit PRIVATE port platform)
target_link_libraries(mapedit PRIVATE msc host)
if(NOT MSVC)
  target_compile_options(mapedit PRIVATE -fsigned-char)
endif()
if(WIN32)
  # SDL3.dll beside mapedit.exe, where Windows looks for it
  add_custom_command(TARGET mapedit POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:mapedit> $<TARGET_FILE_DIR:mapedit>
    COMMAND_EXPAND_LISTS)
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

# Per-function comparisons with the original (needs the player's MAPEDIT.EXE in $MAPEDIT_EXE
# and Python's unicorn package; skipped without them)
if(Python3_FOUND)
  add_test(NAME oracle COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/oracle/run.py
           --lib $<TARGET_FILE:oracle_lib>)
  set_tests_properties(oracle PROPERTIES SKIP_RETURN_CODE 77)
endif()
