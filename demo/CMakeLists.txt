# ---------------------------------------------------------------------------
# DemoFramework — shared GLFW/OpenGL3/ImGui render loop
# Built once and linked by all demos (no dependency stacking).
# ---------------------------------------------------------------------------
add_library(DemoFramework STATIC
        DemoFramework.cpp
)

target_include_directories(DemoFramework PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DemoFramework PUBLIC
        imgui
        imgui-glfw
        imgui-opengl3
)

# ---------------------------------------------------------------------------
# Include directories for all enabled extensions
# This ensures that when demos include ImGuiEx.h, all extension headers
# are available even if they don't directly link that extension.
# ---------------------------------------------------------------------------
set(DEMO_EXTENSION_INCLUDE_DIRS)
if (IMGUIEX_ENABLE_COLOR_TEXT_EDIT)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${imguicolortextedit_SOURCE_DIR})
endif ()
if (IMGUIEX_ENABLE_NODE_EDITOR)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${imguinodeeditor_SOURCE_DIR})
endif ()
if (IMGUIEX_ENABLE_IMNODES)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${imnodes_SOURCE_DIR})
endif ()
if (IMGUIEX_ENABLE_NODE_FLOW)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${imnodeflow_SOURCE_DIR}/include)
endif ()
if (IMGUIEX_ENABLE_IMPLOT)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${implot_SOURCE_DIR})
endif ()
if (IMGUIEX_ENABLE_IMPLOT3D)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${implot3d_SOURCE_DIR})
endif ()
if (IMGUIEX_ENABLE_IMGUI_KNOBS)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${imguiknobs_SOURCE_DIR})
endif ()
if (IMGUIEX_ENABLE_IMSPINNER)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${imspinner_SOURCE_DIR})
endif ()
if (IMGUIEX_ENABLE_IMGUI_TOGGLE)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${imgui_toggle_SOURCE_DIR})
endif ()
if (IMGUIEX_ENABLE_DATE_PICKER)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${imguidatepicker_SOURCE_DIR})
endif ()
if (IMGUIEX_ENABLE_IMGUI_KEYBOARD)
    list(APPEND DEMO_EXTENSION_INCLUDE_DIRS ${imgui_keyboard_SOURCE_DIR})
endif ()

# ---------------------------------------------------------------------------
# add_imgui_demo_standalone(<target> <extension_lib> <sources...>)
#
# Creates a self-contained executable that links:
#   - DemoFramework (once, shared by all demos)
#   - ImGuiEx (to get all compile definitions for #ifdef guards)
#   - Only the specific extension library needed
# This prevents dependency stacking where the last demo depends on all others.
# ---------------------------------------------------------------------------
function(add_imgui_demo_standalone target extension_lib)
    add_executable(${target}
            ${ARGN}
    )
    target_include_directories(${target} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
            ${DEMO_EXTENSION_INCLUDE_DIRS}
    )
    target_link_libraries(${target} PRIVATE
            DemoFramework
            ImGuiEx
            ${extension_lib}
    )
endfunction()

# ---------------------------------------------------------------------------
# ImGui built-in demo  (always built)
# ---------------------------------------------------------------------------
add_executable(DemoImGui imgui/main.cpp)
target_include_directories(DemoImGui PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${DEMO_EXTENSION_INCLUDE_DIRS}
)
target_link_libraries(DemoImGui PRIVATE
        DemoFramework
        ImGuiEx
)

# ---------------------------------------------------------------------------
# Extension demos  (built only when the matching option is ON)
# Each demo links ONLY its specific extension, not the full ImGuiEx stack.
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_COLOR_TEXT_EDIT)
    add_imgui_demo_standalone(DemoColorTextEdit ImGuiColorTextEdit color_text_edit/main.cpp)
endif ()

if (IMGUIEX_ENABLE_NODE_EDITOR)
    add_imgui_demo_standalone(DemoNodeEditor ImGuiNodeEditor node_editor/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMNODES)
    add_imgui_demo_standalone(DemoImNodes imnodes imnodes/main.cpp)
endif ()

if (IMGUIEX_ENABLE_NODE_FLOW)
    add_imgui_demo_standalone(DemoImNodeFlow ImNodeFlow node_flow/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMPLOT)
    add_imgui_demo_standalone(DemoImPlot implot implot/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMPLOT3D)
    add_imgui_demo_standalone(DemoImPlot3D implot3d implot3d/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMGUI_KNOBS)
    add_imgui_demo_standalone(DemoImGuiKnobs imguiknobs imgui_knobs/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMSPINNER)
    add_imgui_demo_standalone(DemoImSpinner imspinner imspinner/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMGUI_TOGGLE)
    add_imgui_demo_standalone(DemoImGuiToggle imgui_toggle imgui_toggle/main.cpp)
endif ()

if (IMGUIEX_ENABLE_SPLITTER)
    add_executable(DemoSplitter splitter/main.cpp)
    target_include_directories(DemoSplitter PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
            ${DEMO_EXTENSION_INCLUDE_DIRS}
    )
    target_link_libraries(DemoSplitter PRIVATE
            DemoFramework
            ImGuiEx
    )
endif ()

if (IMGUIEX_ENABLE_STACK_LAYOUT)
    add_executable(DemoStackLayout stack_layout/main.cpp)
    target_include_directories(DemoStackLayout PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
            ${DEMO_EXTENSION_INCLUDE_DIRS}
    )
    target_link_libraries(DemoStackLayout PRIVATE
            DemoFramework
            ImGuiEx
            imgui_stacklayout
    )
endif ()

if (IMGUIEX_ENABLE_DATE_PICKER)
    add_imgui_demo_standalone(DemoDatePicker ImGuiDatePicker date_picker/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMGUI_KEYBOARD)
    add_imgui_demo_standalone(DemoImGuiKeyboard imgui_keyboard imgui_keyboard/main.cpp)
endif ()
