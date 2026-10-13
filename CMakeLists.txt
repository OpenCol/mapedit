# mapedit-port: the 1994 Colonization map editor on SDL3 (docs/plan.md).
#
#   cmake -B build && cmake --build build && ctest --test-dir build
#
# The platform host, MSC 6's run-time (port/), and the imported editor (src/, compiled as
# an object library until it links: M2).
cmake_minimum_required(VERSION 3.16)
project(mapedit_port VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
# the editor's objects also go into a shared library (the oracle tests' oracle_lib)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

# The editor (src/, from mapedit-decomp: docs/origin.md). Its warnings policy: the leftovers of byte-matching
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
target_include_directories(portlib PRIVATE port platform src/include)
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
set(MAPEDIT_MAIN port/main.c port/game_dir.c)
add_executable(mapedit ${MAPEDIT_MAIN} $<TARGET_OBJECTS:editor> $<TARGET_OBJECTS:portlib>)
target_include_directories(mapedit PRIVATE port platform)
target_link_libraries(mapedit PRIVATE msc host)
if(NOT MSVC)
  target_compile_options(mapedit PRIVATE -fsigned-char)
endif()
# macOS: the same program as an application, "Colonization Map Editor.app". A release links
# SDL3 statically (tools/release.py), so the bundle is the one executable; a build against a
# shared SDL3 (Homebrew's) runs on the machine that has it.
if(APPLE)
  add_executable(mapedit_app MACOSX_BUNDLE ${MAPEDIT_MAIN} $<TARGET_OBJECTS:editor> $<TARGET_OBJECTS:portlib>)
  target_include_directories(mapedit_app PRIVATE port platform)
  target_link_libraries(mapedit_app PRIVATE msc host)
  target_compile_options(mapedit_app PRIVATE -fsigned-char)
  set(ICON ${CMAKE_SOURCE_DIR}/platform/mapedit.icns)
  if(EXISTS ${ICON})
    target_sources(mapedit_app PRIVATE ${ICON})
    set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  endif()
  set_target_properties(mapedit_app PROPERTIES
    OUTPUT_NAME "Colonization Map Editor"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "Colonization Map Editor"
    MACOSX_BUNDLE_GUI_IDENTIFIER org.opencol.mapedit
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_ICON_FILE mapedit.icns)
endif()

if(MINGW)
  # a Windows program, not a console one: a double click opens the editor alone
  set_target_properties(mapedit PROPERTIES WIN32_EXECUTABLE ON)
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
  # Screens and saved files of tests/golden's sessions (needs the game in $MAPEDIT_GAME)
  add_test(NAME golden COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/golden.py
           --port $<TARGET_FILE:mapedit>)
  set_tests_properties(golden PROPERTIES SKIP_RETURN_CODE 77)
endif()
