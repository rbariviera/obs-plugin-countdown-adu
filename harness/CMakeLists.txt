cmake_minimum_required(VERSION 3.24)

# Standalone UI preview harness for the CountdownDock widget.
# This project is intentionally independent from the OBS build tree: it does
# NOT link libobs. It reuses the plugin's widget sources plus a lightweight
# obs_module_text() stub so the dock can be previewed in seconds.
#
# Configure once (point CMAKE_PREFIX_PATH at the Qt6 from obs-deps):
#   cmake -B build -G Ninja \
#     -DCMAKE_PREFIX_PATH="<repo>/.deps/obs-deps-qt6-2026-07-15-universal"
#
# Then iterate:
#   cmake --build build && ./build/countdown-harness

project(countdown-harness LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PLUGIN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(PLUGIN_SRC "${PLUGIN_ROOT}/src")
set(PLUGIN_DATA "${PLUGIN_ROOT}/data")
set(LOCALE_FILE "${PLUGIN_DATA}/locale/en-US.ini")

add_executable(
  countdown-harness
  main.cpp
  obs-module-stub.cpp
  "${PLUGIN_SRC}/countdown-dock.cpp"
  "${PLUGIN_SRC}/repeat-button.cpp"
  "${PLUGIN_SRC}/settings-dialog.cpp"
)

# stub-include provides a minimal <obs-module.h> so the shared widget sources
# compile without libobs. It must come after PLUGIN_SRC.
target_include_directories(
  countdown-harness
  PRIVATE "${PLUGIN_SRC}" "${CMAKE_CURRENT_SOURCE_DIR}/stub-include"
)

# Provide absolute paths so the stub finds bundled data (locale, fonts)
# regardless of the current working directory.
target_compile_definitions(
  countdown-harness
  PRIVATE COUNTDOWN_LOCALE_FILE="${LOCALE_FILE}" COUNTDOWN_DATA_DIR="${PLUGIN_DATA}"
)

target_link_libraries(countdown-harness PRIVATE Qt6::Widgets)
