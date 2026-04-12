# ---------------------------------------------------------------------------
# DemoFramework — shared Win32/DX11/ImGui render loop
# Built once and linked by all demos (no dependency stacking).
# ---------------------------------------------------------------------------
add_library(DemoFramework STATIC
        DemoFramework.cpp
)

target_include_directories(DemoFramework PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DemoFramework PRIVATE
        imgui
        imgui-win32
        imgui-dx11
        d3d11
)

# ---------------------------------------------------------------------------
# add_imgui_demo_standalone(<target> <extension_lib> <sources...>)
#
# Creates a self-contained WIN32 executable that links:
#   - DemoFramework (once, shared by all demos)
#   - Only the specific extension library needed
# This prevents dependency stacking where the last demo depends on all others.
# ---------------------------------------------------------------------------
function(add_imgui_demo_standalone target extension_lib)
    add_executable(${target} WIN32
            ${ARGN}
    )
    target_include_directories(${target} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
    target_link_libraries(${target} PRIVATE
            DemoFramework
            ${extension_lib}
    )
endfunction()

# ---------------------------------------------------------------------------
# ImGui built-in demo  (always built)
# ---------------------------------------------------------------------------
add_executable(ImGuiDemo WIN32 imgui/main.cpp)
target_include_directories(ImGuiDemo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_link_libraries(ImGuiDemo PRIVATE DemoFramework)

# ---------------------------------------------------------------------------
# Extension demos  (built only when the matching option is ON)
# Each demo links ONLY its specific extension, not the full ImGuiEx stack.
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_COLOR_TEXT_EDIT)
    add_imgui_demo_standalone(ColorTextEditDemo ImGuiColorTextEdit color_text_edit/main.cpp)
endif ()

if (IMGUIEX_ENABLE_NODE_EDITOR)
    add_imgui_demo_standalone(NodeEditorDemo ImGuiNodeEditor node_editor/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMNODES)
    add_imgui_demo_standalone(ImNodesDemo imnodes imnodes/main.cpp)
endif ()

if (IMGUIEX_ENABLE_NODE_FLOW)
    add_imgui_demo_standalone(ImNodeFlowDemo ImNodeFlow node_flow/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMPLOT)
    add_imgui_demo_standalone(ImPlotDemo implot implot/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMPLOT3D)
    add_imgui_demo_standalone(ImPlot3DDemo implot3d implot3d/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMGUI_KNOBS)
    add_imgui_demo_standalone(ImGuiKnobsDemo imguiknobs imgui_knobs/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMSPINNER)
    add_imgui_demo_standalone(ImSpinnerDemo imspinner imspinner/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMGUI_TOGGLE)
    add_imgui_demo_standalone(ImGuiToggleDemo imgui_toggle imgui_toggle/main.cpp)
endif ()

if (IMGUIEX_ENABLE_SPLITTER)
    add_executable(SplitterDemo WIN32 splitter/main.cpp)
    target_include_directories(SplitterDemo PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
    target_link_libraries(SplitterDemo PRIVATE DemoFramework)
endif ()

if (IMGUIEX_ENABLE_STACK_LAYOUT)
    add_executable(StackLayoutDemo WIN32 stack_layout/main.cpp)
    target_include_directories(StackLayoutDemo PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
    target_link_libraries(StackLayoutDemo PRIVATE DemoFramework imgui_stacklayout)
endif ()

if (IMGUIEX_ENABLE_DATE_PICKER)
    add_imgui_demo_standalone(DatePickerDemo ImGuiDatePicker date_picker/main.cpp)
endif ()

if (IMGUIEX_ENABLE_IMGUI_KEYBOARD)
    add_imgui_demo_standalone(ImGuiKeyboardDemo imgui_keyboard imgui_keyboard/main.cpp)
endif ()

