cmake_minimum_required(VERSION 3.28)
project(ImGuiEx)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

# ---------------------------------------------------------------------------
# ImGui (docking branch) — fetched and configured via a community CMakeLists
# ---------------------------------------------------------------------------
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        docking   # pin to a commit hash for reproducibility, e.g. abc1234
        GIT_SHALLOW    TRUE
)

# FetchContent_Populate(name) is deprecated in CMake 3.30+ (CMP0169).
# The inject-CMakeLists trick requires it, so we opt-in to the old behaviour.
cmake_policy(SET CMP0169 OLD)

# The only community-default we override: Freetype triggers a hard FATAL_ERROR
# when not found, and we don't want to mandate it as a system dependency.
set(IMGUI_ENABLE_FREETYPE OFF CACHE BOOL "" FORCE)


FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    FetchContent_Populate(imgui)

    # ImGui ships no CMakeLists.txt — inject the community-maintained one
    file(
            DOWNLOAD
            "https://gist.githubusercontent.com/rokups/f771217b2d530d170db5cb1e08e9a8f4/raw/4c2c14374ab878ca2f45daabfed4c156468e4e27/CMakeLists.txt"
            "${imgui_SOURCE_DIR}/CMakeLists.txt"
            EXPECTED_HASH SHA256=fd62f69364ce13a4f7633a9b50ae6672c466bcc44be60c69c45c0c6e225bb086
    )

    # All IMGUI_IMPL_* backends and IMGUI_EXAMPLES are left at community defaults.
    # The community script creates each backend target when its headers/libraries
    # are found, and creates example targets (including example_win32_directx11)
    # when their backend targets exist — all inside add_subdirectory in the correct
    # order (backends before examples), so no pre-definition trick is needed.
    add_subdirectory(${imgui_SOURCE_DIR} ${imgui_BINARY_DIR})
endif ()

# ---------------------------------------------------------------------------
# ImGuiEx library
# ---------------------------------------------------------------------------
add_library(ImGuiEx
        src/ImGuiEx.cpp
        src/imgui_layout.cpp
)

target_include_directories(ImGuiEx
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(ImGuiEx
        PUBLIC imgui
)

# ---------------------------------------------------------------------------
# Demo
# ---------------------------------------------------------------------------
add_subdirectory(demo)

