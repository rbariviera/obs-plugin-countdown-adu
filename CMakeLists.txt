cmake_minimum_required(VERSION 3.28...3.30)

find_package(Qt6 REQUIRED Widgets)

add_library(obs-plugin-countdown-adu MODULE)
add_library(OBS::plugin-countdown-adu ALIAS obs-plugin-countdown-adu)

# Generate plugin-support.c with name/version metadata (mirrors obs-plugintemplate)
set(CMAKE_PROJECT_NAME obs-plugin-countdown-adu)
set(CMAKE_PROJECT_VERSION ${OBS_VERSION_CANONICAL})
configure_file(src/plugin-support.c.in ${CMAKE_CURRENT_BINARY_DIR}/plugin-support.c @ONLY)

target_sources(
  obs-plugin-countdown-adu
  PRIVATE
    src/plugin-main.cpp
    src/countdown-dock.cpp
    src/countdown-dock.hpp
    src/repeat-button.cpp
    src/repeat-button.hpp
    src/plugin-support.h
    ${CMAKE_CURRENT_BINARY_DIR}/plugin-support.c
)

target_include_directories(obs-plugin-countdown-adu PRIVATE src)

target_compile_options(
  obs-plugin-countdown-adu
  PRIVATE $<$<PLATFORM_ID:Darwin>:-Wno-quoted-include-in-framework-header> $<$<PLATFORM_ID:Darwin>:-Wno-comma>
)

target_link_libraries(
  obs-plugin-countdown-adu
  PRIVATE OBS::libobs OBS::frontend-api Qt::Widgets
)

if(OS_WINDOWS)
  configure_file(cmake/windows/obs-module.rc.in obs-plugin-countdown-adu.rc)
  target_sources(obs-plugin-countdown-adu PRIVATE obs-plugin-countdown-adu.rc)
endif()

set_target_properties_obs(
  obs-plugin-countdown-adu
  PROPERTIES FOLDER plugins
             PREFIX ""
             AUTOMOC ON
)
