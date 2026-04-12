#pragma once

// ImGuiEx — aggregator header.
// Include this single header to get access to every enabled extension.
// Extensions are controlled via CMake options (e.g. -DENABLE_COLOR_TEXT_EDIT=ON).

#include <imgui.h>

// ---------------------------------------------------------------------------
// ImGuiColorTextEdit  (ENABLE_COLOR_TEXT_EDIT)
// ---------------------------------------------------------------------------
#ifdef IMGUIEX_ENABLE_COLOR_TEXT_EDIT
#include <TextEditor.h>
#endif

// ---------------------------------------------------------------------------
// imgui-node-editor  (ENABLE_NODE_EDITOR)
// ---------------------------------------------------------------------------
#ifdef IMGUIEX_ENABLE_NODE_EDITOR
#include <imgui_node_editor.h>
#endif

// ---------------------------------------------------------------------------
// imgui_toggle  (ENABLE_IMGUI_TOGGLE)
// ---------------------------------------------------------------------------
#ifdef IMGUIEX_ENABLE_IMGUI_TOGGLE
#include <imgui_toggle.h>
#include <imgui_toggle_presets.h>
#endif

// ---------------------------------------------------------------------------
// Stack Layout  (ENABLE_STACK_LAYOUT)
// Provides ImGui::BeginHorizontal / EndHorizontal, BeginVertical /
// EndVertical, Spring, SuspendLayout, ResumeLayout.
//
// The API is injected into imgui.h by CMake patches applied to the fetched
// ImGui source, so no extra include is needed here.  This block only serves
// as documentation and emits the compile-time guard consumed by demos.
// ---------------------------------------------------------------------------
#ifdef IMGUIEX_ENABLE_STACK_LAYOUT
// Stack Layout API is declared in imgui.h (patched at configure time).
// See src/imgui_stacklayout.cpp for the implementation.
#endif // IMGUIEX_ENABLE_STACK_LAYOUT

// ---------------------------------------------------------------------------
// ImGuiDatePicker  (ENABLE_DATE_PICKER)
// Provides ImGui::DatePicker and ImGui::DatePickerEx.
// Both take a std::tm& that is updated in-place when the user selects a date.
// Returns true on change (same convention as other ImGui input widgets).
// ---------------------------------------------------------------------------
#ifdef IMGUIEX_ENABLE_DATE_PICKER
#include <ImGuiDatePicker.hpp>
#endif // IMGUIEX_ENABLE_DATE_PICKER

// ---------------------------------------------------------------------------
// imgui_keyboard  (ENABLE_IMGUI_KEYBOARD)
// Provides ImKeyboard::Keyboard(), ImKeyboard::KeyboardDemo(), and helpers
// for highlighting keys and recording keybindings.
// ---------------------------------------------------------------------------
#ifdef IMGUIEX_ENABLE_IMGUI_KEYBOARD
#include <imgui_keyboard.h>
#endif // IMGUIEX_ENABLE_IMGUI_KEYBOARD

// ---------------------------------------------------------------------------
// Splitter  (ENABLE_SPLITTER)
// Extends the ImGui namespace with a draggable panel-divider widget.
// Uses only the public ImGui API; imgui_internal.h is NOT required.
//
// Note: imgui_internal.h's SplitterBehavior() (referenced in issue #319)
// no longer exists in ImGui 1.92+.  This implementation uses
// InvisibleButton + MouseDelta and relies on no internal symbols.
//
// Typical usage — left/right split:
//   BeginChild("##left",  {size1, h}, ...); ... EndChild();
//   SameLine(0.f, 0.f);
//   Splitter(true, 4.f, &size1, &size2, 50.f, 50.f, h);
//   SameLine(0.f, 0.f);
//   BeginChild("##right", {size2, h}, ...); ... EndChild();
//
// Typical usage — top/bottom split (no SameLine needed):
//   BeginChild("##top",    {0, size1}, ...); ... EndChild();
//   Splitter(false, 4.f, &size1, &size2, 40.f, 40.f);
//   BeginChild("##bottom", {0, size2}, ...); ... EndChild();
// ---------------------------------------------------------------------------
#ifdef IMGUIEX_ENABLE_SPLITTER
namespace ImGui {

/// Renders a thin draggable divider between two adjacent child panels.
///
/// @param split_vertically  true  = vertical bar separating left/right panels.
///                          false = horizontal bar separating top/bottom panels.
/// @param thickness         Width (or height) of the hit-area in pixels.
/// @param size1             [in/out] Size of the first  panel along the split axis.
/// @param size2             [in/out] Size of the second panel along the split axis.
/// @param min_size1         Minimum allowed size for the first  panel (pixels).
/// @param min_size2         Minimum allowed size for the second panel (pixels).
/// @param long_axis_size    Length of the divider along its long axis (pixels).
///                          Pass -1 (default) to stretch to fill available space.
/// @return                  true while the user is actively dragging.
inline bool Splitter(bool split_vertically, float thickness,
                     float* size1, float* size2,
                     float min_size1 = 8.f, float min_size2 = 8.f,
                     float long_axis_size = -1.f)
{
    const ImVec2 avail   = GetContentRegionAvail();
    const float  long_sz = (long_axis_size >= 0.f)
                           ? long_axis_size
                           : (split_vertically ? avail.y : avail.x);

    // Use the address of size1 as a unique ID seed so multiple Splitter
    // calls in the same frame do not share an ID and trigger the
    // "items with identical ID" assert.
    PushID(static_cast<const void*>(size1));
    InvisibleButton("##s", split_vertically ? ImVec2(thickness, long_sz)
                                            : ImVec2(long_sz,  thickness));
    PopID();

    if (IsItemHovered() || IsItemActive())
        SetMouseCursor(split_vertically ? ImGuiMouseCursor_ResizeEW
                                        : ImGuiMouseCursor_ResizeNS);

    if (IsItemActive())
    {
        const float delta = split_vertically ? GetIO().MouseDelta.x
                                             : GetIO().MouseDelta.y;
        *size1 += delta;
        *size2 -= delta;

        if (*size1 < min_size1) { *size2 -= (min_size1 - *size1); *size1 = min_size1; }
        if (*size2 < min_size2) { *size1 -= (min_size2 - *size2); *size2 = min_size2; }
    }

    return IsItemActive();
}

} // namespace ImGui
#endif // IMGUIEX_ENABLE_SPLITTER
