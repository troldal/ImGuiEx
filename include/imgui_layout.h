#pragma once

#include <imgui.h>
#include <utility>  // std::forward
#include <cmath>    // floorf

// Layout helper primitives for Dear ImGui.
// All symbols live in namespace ImGui for consistent call-site syntax.
//
// Template functions are defined entirely here.
// Non-template, non-inline functions (HStackScope) are defined in imgui_layout.cpp.

namespace ImGui {

// ---------------------------------------------------------------------------
// HStackScope — RAII horizontal stack manager
//
// Usage:
//   ImGui::HStackScope hs;
//   hs.item(); ImGui::Button("A");
//   hs.item(); ImGui::Button("B");   // SameLine inserted automatically
// ---------------------------------------------------------------------------
struct HStackScope
{
    float spacing;  // spacing forwarded to SameLine
    bool  first;    // true until item() is called for the first time

    explicit HStackScope(float spacing = 4.0f);

    /// Call once before each widget. Inserts SameLine (with this->spacing)
    /// between every widget, but never before the very first one.
    void item();
};

// ---------------------------------------------------------------------------
// SpringH / SpringV — flexible space consumers
// ---------------------------------------------------------------------------

/// Consumes all remaining horizontal space with a zero-height Dummy, then
/// calls SameLine — effectively pushing the next widget to the right edge.
inline void SpringH()
{
    const ImVec2 avail = GetContentRegionAvail();
    Dummy(ImVec2(avail.x, 0.0f));
    SameLine();
}

/// Consumes all remaining vertical space with a zero-width Dummy, pushing
/// the next widget to the bottom of the current layout region.
inline void SpringV()
{
    const ImVec2 avail = GetContentRegionAvail();
    Dummy(ImVec2(0.0f, avail.y));
}

// ---------------------------------------------------------------------------
// Spacer — fixed-size invisible gap
// ---------------------------------------------------------------------------

/// Inserts a fixed-size invisible gap via Dummy. h defaults to 0.
inline void Spacer(float w, float h = 0.0f)
{
    Dummy(ImVec2(w, h));
}

// Note: ImGui::NextColumn() already exists in imgui for the legacy Columns() API.
// For use inside ImGui::Columns() (which uses BeginTable internally) call
// ImGui::TableNextColumn() directly.

// ---------------------------------------------------------------------------
// SplitH — horizontal two-pane split
//
// Divides the available width into two BeginChild regions.
// Left  gets  avail.x * ratio  pixels.
// Right gets  the remainder minus spacing  (ImVec2(0,0) = fill remaining).
// ---------------------------------------------------------------------------
template <typename LeftFn, typename RightFn>
void SplitH(float ratio, LeftFn&& leftFn, RightFn&& rightFn,
            float spacing = 4.0f)
{
    const ImVec2 avail  = GetContentRegionAvail();
    const float  leftW  = floorf(avail.x * ratio);

    BeginChild("##split_left", ImVec2(leftW, 0.0f));
    std::forward<LeftFn>(leftFn)();
    EndChild();

    SameLine(0.0f, spacing);

    // ImVec2(0,0) fills from the current cursor to the right/bottom edge,
    // which is exactly avail.x - leftW - spacing wide.
    BeginChild("##split_right", ImVec2(0.0f, 0.0f));
    std::forward<RightFn>(rightFn)();
    EndChild();
}

// ---------------------------------------------------------------------------
// SplitV — vertical two-pane split
//
// Divides the available height into two BeginChild regions.
// Top    gets  avail.y * ratio  pixels.
// Bottom gets  the remainder minus spacing.
// ---------------------------------------------------------------------------
template <typename TopFn, typename BottomFn>
void SplitV(float ratio, TopFn&& topFn, BottomFn&& bottomFn,
            float spacing = 4.0f)
{
    const ImVec2 avail  = GetContentRegionAvail();
    const float  topH   = floorf(avail.y * ratio);
    const float  startY = GetCursorPosY();

    BeginChild("##split_top", ImVec2(0.0f, topH));
    std::forward<TopFn>(topFn)();
    EndChild();

    // Override ImGui's automatic ItemSpacing and place the bottom child at
    // exactly startY + topH + spacing.
    SetCursorPosY(startY + topH + spacing);

    BeginChild("##split_bottom", ImVec2(0.0f, 0.0f));
    std::forward<BottomFn>(bottomFn)();
    EndChild();
}

// ---------------------------------------------------------------------------
// Padded — content region with uniform inner padding
//
// Creates a BeginChild that is smaller than the available area by padding*2
// on each axis, then offsets the internal cursor to (padding, padding) so
// that content is inset from all four sides.
// ---------------------------------------------------------------------------
template <typename Fn>
void Padded(float padding, Fn&& fn)
{
    const ImVec2 avail = GetContentRegionAvail();

    BeginChild("##padded",
               ImVec2(avail.x - padding * 2.0f,
                      avail.y - padding * 2.0f));
    SetCursorPos(ImVec2(padding, padding));
    std::forward<Fn>(fn)();
    EndChild();
}

// ---------------------------------------------------------------------------
// Columns — equal-width table layout
//
// Opens a BeginTable with `count` equally stretched columns.
// Use ImGui::TableNextColumn() to advance through cells (TableNextRow() to
// start a new row).  EndTable is only called when BeginTable succeeds.
// ---------------------------------------------------------------------------
template <typename Fn>
void Columns(int count, Fn&& fn,
             ImGuiTableFlags flags = ImGuiTableFlags_None)
{
    if (BeginTable("##cols", count,
                   flags | ImGuiTableFlags_SizingStretchSame))
    {
        std::forward<Fn>(fn)();
        EndTable();
    }
}

// ---------------------------------------------------------------------------
// Group — labelled / bordered content region
//
// border == true  → BeginChild with ImGuiChildFlags_Borders + ChildRounding 4
// border == false → BeginGroup / EndGroup
//
// If label is non-null and does not start with '#', a SeparatorText is
// rendered at the top of the region.
// ---------------------------------------------------------------------------
template <typename Fn>
void Group(const char* label, Fn&& fn, bool border = true)
{
    const bool showLabel = label != nullptr && label[0] != '#';

    if (border)
    {
        PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
        BeginChild(label, ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        if (showLabel)
            SeparatorText(label);
        std::forward<Fn>(fn)();
        EndChild();
        PopStyleVar();
    }
    else
    {
        BeginGroup();
        if (showLabel)
            SeparatorText(label);
        std::forward<Fn>(fn)();
        EndGroup();
    }
}

} // namespace ImGui






