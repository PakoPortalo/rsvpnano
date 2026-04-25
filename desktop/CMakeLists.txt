cmake_minimum_required(VERSION 3.20)
project(rsvpnano_desktop CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")

# ---------- SDL2 ----------
# Try Homebrew paths (Apple Silicon and Intel)
find_package(SDL2 QUIET CONFIG
    HINTS /opt/homebrew/lib/cmake/SDL2
          /usr/local/lib/cmake/SDL2)

if(NOT SDL2_FOUND)
    # Fallback: pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    add_library(SDL2::SDL2 INTERFACE IMPORTED)
    target_include_directories(SDL2::SDL2 INTERFACE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(SDL2::SDL2 INTERFACE ${SDL2_LIBRARIES})
    target_compile_options(SDL2::SDL2 INTERFACE ${SDL2_CFLAGS_OTHER})
endif()

# ---------- Paths ----------
set(DESKTOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SHIM_DIR    ${DESKTOP_DIR}/shim)
set(SRC_DIR     ${DESKTOP_DIR}/../src)

# ---------- Source files from src/ ----------
# Include only the portable files; hardware .cpp files are replaced below.
set(SRC_FILES
    ${SRC_DIR}/app/App.cpp
    ${SRC_DIR}/display/DisplayManager.cpp
    ${SRC_DIR}/reader/ReadingLoop.cpp
    ${SRC_DIR}/main.cpp
)

# ---------- Platform replacement files ----------
set(PLATFORM_FILES
    ${DESKTOP_DIR}/platform/axs15231b_desktop.cpp
    ${DESKTOP_DIR}/platform/BoardConfig_desktop.cpp
    ${DESKTOP_DIR}/platform/ButtonHandler_desktop.cpp
    ${DESKTOP_DIR}/platform/TouchHandler_desktop.cpp
    ${DESKTOP_DIR}/platform/EpubConverter_desktop.cpp
    ${DESKTOP_DIR}/platform/UsbMassStorageManager_desktop.cpp
    ${DESKTOP_DIR}/platform/Wire_desktop.cpp
)

# StorageManager compiles unchanged — all its SD_MMC/FS calls go through shims
set(STORAGE_FILES
    ${SRC_DIR}/storage/StorageManager.cpp
)

# ---------- Shim implementation files ----------
set(SHIM_IMPL_FILES
    ${SHIM_DIR}/Arduino.cpp
    ${SHIM_DIR}/FS.cpp
    ${SHIM_DIR}/SD_MMC.cpp
    ${SHIM_DIR}/Preferences.cpp
)

# ---------- Executable ----------
add_executable(rsvpnano
    ${DESKTOP_DIR}/main_desktop.cpp
    ${SRC_FILES}
    ${STORAGE_FILES}
    ${PLATFORM_FILES}
    ${SHIM_IMPL_FILES}
)

# ---------- Compile definitions ----------
target_compile_definitions(rsvpnano PRIVATE
    DESKTOP_BUILD=1
    RSVP_ON_DEVICE_EPUB_CONVERSION=0
    RSVP_USB_TRANSFER_ENABLED=0
    RSVP_MAX_BOOK_WORDS=0
    CORE_DEBUG_LEVEL=0
)

# ---------- Include paths (ORDER MATTERS: shim must come first) ----------
target_include_directories(rsvpnano PRIVATE
    ${SHIM_DIR}           # shadows Arduino.h, Wire.h, FS.h, SD_MMC.h, etc.
    ${DESKTOP_DIR}        # for "platform/DesktopInput.h"
    ${SRC_DIR}            # for "app/App.h", "display/DisplayManager.h", etc.
)

# ---------- Link ----------
target_link_libraries(rsvpnano PRIVATE SDL2::SDL2)

# ---------- Warnings (suppress shim differences) ----------
target_compile_options(rsvpnano PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-missing-field-initializers
)

message(STATUS "rsvpnano desktop build configured")
message(STATUS "  Source:   ${SRC_DIR}")
message(STATUS "  Shims:    ${SHIM_DIR}")
message(STATUS "  Platform: ${DESKTOP_DIR}/platform")
