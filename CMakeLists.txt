cmake_minimum_required(VERSION 3.20)

# macOS: arm64 by default; packaging/make_mac.sh builds a universal
# (arm64 + x86_64) app. 10.15 is the first release with std::filesystem.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
  set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "")
endif()

project(JumpStart4 LANGUAGES C CXX)
if(APPLE)
  enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# By default SDL3 is built from source and linked statically, so the app is
# self-contained. -DJS4_SYSTEM_SDL=ON links an installed
# SDL3 (e.g. Homebrew's) instead, which is quicker for development.
option(JS4_SYSTEM_SDL "Link the system/Homebrew SDL3 instead of building it" OFF)
if(JS4_SYSTEM_SDL)
  find_package(SDL3 REQUIRED CONFIG)
  set(JS4_SDL_TARGET SDL3::SDL3)
else()
  include(FetchContent)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  set(SDL_TESTS OFF CACHE BOOL "" FORCE)
  set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.18
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(SDL3)
  set(JS4_SDL_TARGET SDL3::SDL3-static)
endif()

file(GLOB ENGINE_SRC CONFIGURE_DEPENDS src/engine/*.cpp)
if(JS4_GAME_UNITS)
  # Development aid: build only the listed game units (plus stubs).
  set(GAME_SRC src/game/zz_stubs.cpp)
  foreach(u ${JS4_GAME_UNITS})
    list(APPEND GAME_SRC src/game/${u}.cpp)
  endforeach()
else()
  file(GLOB GAME_SRC CONFIGURE_DEPENDS src/game/*.cpp)
endif()
if(APPLE)
  set(PLATFORM_SRC src/platform/print_mac.mm)
elseif(WIN32)
  set(PLATFORM_SRC src/platform/print_win.cpp)
else()
  set(PLATFORM_SRC src/platform/print_other.cpp)
endif()

set(APP_NAME "JumpStart 4th Grade")
set(APP_VERSION "1.0.0")

add_executable(js4 MACOSX_BUNDLE WIN32 src/main.cpp ${ENGINE_SRC} ${GAME_SRC} ${PLATFORM_SRC})
target_include_directories(js4 PRIVATE src)
target_link_libraries(js4 PRIVATE ${JS4_SDL_TARGET})
if(NOT APPLE)
  target_link_options(js4 PRIVATE $<$<CONFIG:Release>:-s>)  # strip release builds
endif()
target_compile_options(js4 PRIVATE -Wall -Wno-unused-variable -Wno-unused-but-set-variable
  -Wno-unused-function -Wno-deprecated-declarations -fno-strict-aliasing)

if(APPLE)
  target_link_libraries(js4 PRIVATE "-framework AppKit")
  set_target_properties(js4 PROPERTIES
    OUTPUT_NAME "${APP_NAME}"
    MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.maikopruett.jumpstart4"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${APP_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "1"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/res/Info.plist.in")
  # Ad-hoc sign the bundle so it launches on Apple Silicon after being copied.
  add_custom_command(TARGET js4 POST_BUILD
    COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:js4>"
    VERBATIM)
elseif(WIN32)
  set_target_properties(js4 PROPERTIES OUTPUT_NAME "JumpStart4")
  target_link_libraries(js4 PRIVATE comdlg32 gdi32)
  if(MINGW)
    # No MinGW runtime DLLs to ship.
    target_link_options(js4 PRIVATE -static -static-libgcc -static-libstdc++)
  endif()
  # Version info, plus the game's icon when packaging supplies one (it is
  # extracted from the user's disc, so it is not part of the source tree).
  set(JS4_WIN_ICON "" CACHE FILEPATH "Icon (.ico) to embed in the executable")
  if(JS4_WIN_ICON)
    set(JS4_ICON_LINE "1 ICON \"${JS4_WIN_ICON}\"")
  endif()
  configure_file(res/js4.rc.in ${CMAKE_BINARY_DIR}/js4.rc @ONLY)
  target_sources(js4 PRIVATE ${CMAKE_BINARY_DIR}/js4.rc)
else()
  set_target_properties(js4 PROPERTIES OUTPUT_NAME "jumpstart4")
  # Only glibc is needed at run time; SDL loads X11/Wayland/audio itself.
  target_link_options(js4 PRIVATE -static-libgcc -static-libstdc++)
endif()

# Packaging helper: extracts the icon and artwork from the user's game data
# (used by the scripts in packaging/).
add_executable(packtool EXCLUDE_FROM_ALL packaging/packtool.cpp src/engine/core.cpp src/engine/vfs.cpp
  src/engine/hostfs.cpp
  src/engine/resource.cpp src/engine/gfx.cpp src/engine/exeres.cpp)
target_include_directories(packtool PRIVATE src)
target_link_libraries(packtool PRIVATE ${JS4_SDL_TARGET})
