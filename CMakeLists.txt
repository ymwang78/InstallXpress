cmake_minimum_required(VERSION 3.16)
project(InstallXpress VERSION 5.2.41 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-only framework
if(NOT WIN32)
    message(FATAL_ERROR "InstallXpress only supports Windows")
endif()

# Unicode build
add_definitions(-DUNICODE -D_UNICODE)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Third-party: 7z LZMA SDK ──────────────────────────────────────────────
file(GLOB_RECURSE LZMA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/libsrc/7z/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libsrc/7z/*.cpp)

add_library(lzma STATIC ${LZMA_SOURCES})
target_include_directories(lzma PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libsrc/7z)

# ── DuiLib UI framework ───────────────────────────────────────────────────
add_subdirectory(libsrc/DuiLib)

target_include_directories(duilib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libsrc/DuiLib)

# ── InstallXpress core library ────────────────────────────────────────────
find_package(Lua REQUIRED)

file(GLOB IX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/libsrc/InstallXpress/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libsrc/Utility/*.cpp)

add_library(installxpress_core STATIC ${IX_SOURCES})

target_include_directories(installxpress_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libsrc
    ${CMAKE_CURRENT_SOURCE_DIR}/libsrc/InstallXpress
    ${LUA_INCLUDE_DIR})

target_link_libraries(installxpress_core PUBLIC
    duilib
    lzma
    ${LUA_LIBRARIES}
    shlwapi
    shell32
    ole32
    oleaut32
    comctl32
    wininet)

# ── Demo installer projects ───────────────────────────────────────────────
# Each project is a standalone Win32 executable; add them as optional targets.
# They require their own .rc resources (7z archives) which must be present.
option(BUILD_PROJECTS "Build demo installer projects" OFF)

if(BUILD_PROJECTS)
    add_subdirectory(projects/TaijiMPC/Install)
endif()
