# ---------------------------------------------------------------------------
# add_imgui_demo(<target> <sources...>)
#
# Creates a WIN32 executable that compiles the shared DemoApp.cpp framework
# together with the provided demo sources.  Links the minimum set of libs
# required to run an ImGui Win32+DX11 application.
# DemoApp.h is accessible via the include path (CMAKE_CURRENT_SOURCE_DIR).
# ---------------------------------------------------------------------------
function(add_imgui_demo target)
    add_executable(${target} WIN32
            DemoApp.cpp
            ${ARGN}
    )
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${target} PRIVATE
            imgui
            imgui-win32
            imgui-dx11
            d3d11
    )
endfunction()

# ---------------------------------------------------------------------------
# ImGui built-in demo  (always built)
# ---------------------------------------------------------------------------
add_imgui_demo(ImGuiDemo imgui/main.cpp)

# ---------------------------------------------------------------------------
# Extension demos  (built only when the matching option is ON)
# ---------------------------------------------------------------------------
if (ENABLE_COLOR_TEXT_EDIT)
    add_imgui_demo(ColorTextEditDemo color_text_edit/main.cpp)
    target_link_libraries(ColorTextEditDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_NODE_EDITOR)
    add_imgui_demo(NodeEditorDemo node_editor/main.cpp)
    target_link_libraries(NodeEditorDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_IMNODES)
    add_imgui_demo(ImNodesDemo imnodes/main.cpp)
    target_link_libraries(ImNodesDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_NODE_FLOW)
    add_imgui_demo(ImNodeFlowDemo node_flow/main.cpp)
    target_link_libraries(ImNodeFlowDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_IMPLOT)
    add_imgui_demo(ImPlotDemo implot/main.cpp)
    target_link_libraries(ImPlotDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_IMPLOT3D)
    add_imgui_demo(ImPlot3DDemo implot3d/main.cpp)
    target_link_libraries(ImPlot3DDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_IMGUI_KNOBS)
    add_imgui_demo(ImGuiKnobsDemo imgui_knobs/main.cpp)
    target_link_libraries(ImGuiKnobsDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_IMSPINNER)
    add_imgui_demo(ImSpinnerDemo imspinner/main.cpp)
    target_link_libraries(ImSpinnerDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_IMGUI_TOGGLE)
    add_imgui_demo(ImGuiToggleDemo imgui_toggle/main.cpp)
    target_link_libraries(ImGuiToggleDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_SPLITTER)
    add_imgui_demo(SplitterDemo splitter/main.cpp)
    target_link_libraries(SplitterDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_STACK_LAYOUT)
    add_imgui_demo(StackLayoutDemo stack_layout/main.cpp)
    target_link_libraries(StackLayoutDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_DATE_PICKER)
    add_imgui_demo(DatePickerDemo date_picker/main.cpp)
    target_link_libraries(DatePickerDemo PRIVATE ImGuiEx)
endif ()

if (ENABLE_IMGUI_KEYBOARD)
    add_imgui_demo(ImGuiKeyboardDemo imgui_keyboard/main.cpp)
    target_link_libraries(ImGuiKeyboardDemo PRIVATE ImGuiEx)
endif ()

