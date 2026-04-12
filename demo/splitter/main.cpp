// Splitter demo.
// This file is only compiled when ENABLE_SPLITTER=ON; no #ifdefs needed.
//
// Demonstrates ImGui::Splitter() with three layouts:
//   - Vertical divider  (left | right)
//   - Horizontal divider  (top / bottom)
//   - Three-pane  (left | center | right)
//
// All child windows are sized so their sum always fills the available width
// or height.  ResyncPair() handles proportional rescaling on window resize.

#include "DemoApp.h"
#include <ImGuiEx.hpp>
#include <cmath>    // fabsf

const char* GetDemoTitle() { return "Splitter Demo"; }

// Proportionally rescale *s1 and *s2 so their sum equals new_total.
// No-op when the sum is already within 0.5 px of new_total.
static void ResyncPair(float new_total, float* s1, float* s2)
{
    const float old_total = *s1 + *s2;
    if (old_total > 0.f && fabsf(old_total - new_total) > 0.5f)
    {
        const float r = *s1 / old_total;
        *s1 = new_total * r;
        *s2 = new_total - *s1;
    }
}

// ---------------------------------------------------------------------------
static void VerticalSplitDemo()
{
    static float lft_w = 180.f, rgt_w = 380.f;
    constexpr float kThick = 4.f, kH = 200.f;

    ImGui::SeparatorText("Vertical divider  (left | right)");
    ImGui::Spacing();

    ResyncPair(ImGui::GetContentRegionAvail().x - kThick, &lft_w, &rgt_w);

    ImGui::BeginChild("##vl", {lft_w, kH}, ImGuiChildFlags_Borders);
    ImGui::SeparatorText("Left");
    for (int i = 0; i < 12; ++i)
        ImGui::Text("Item %d", i);
    ImGui::EndChild();

    ImGui::SameLine(0.f, 0.f);
    ImGui::Splitter(true, kThick, &lft_w, &rgt_w, 60.f, 60.f, kH);
    ImGui::SameLine(0.f, 0.f);

    ImGui::BeginChild("##vr", {rgt_w, kH}, ImGuiChildFlags_Borders);
    ImGui::SeparatorText("Right");
    ImGui::TextWrapped(
        "Drag the vertical bar on the left to resize the two panels. "
        "Both panels observe a 60 px minimum width.");
    ImGui::EndChild();

    ImGui::Spacing();
}

// ---------------------------------------------------------------------------
static void HorizontalSplitDemo()
{
    static float top_h = 110.f, bot_h = 90.f;
    constexpr float kThick = 4.f;

    ImGui::SeparatorText("Horizontal divider  (top / bottom)");
    ImGui::Spacing();

    const float avail_w = ImGui::GetContentRegionAvail().x;

    ImGui::BeginChild("##ht", {avail_w, top_h}, ImGuiChildFlags_Borders);
    ImGui::SeparatorText("Top");
    ImGui::TextWrapped("Drag the bar below to resize the top and bottom panels.");
    ImGui::EndChild();

    // No SameLine — the horizontal splitter stacks below the child.
    // Pass avail_w explicitly so the hit-area spans the full window width.
    ImGui::Splitter(false, kThick, &top_h, &bot_h, 40.f, 40.f, avail_w);

    ImGui::BeginChild("##hb", {avail_w, bot_h}, ImGuiChildFlags_Borders);
    ImGui::SeparatorText("Bottom / Log");
    for (int i = 0; i < 6; ++i)
        ImGui::Text("[info] Log line %d", i + 1);
    ImGui::EndChild();

    ImGui::Spacing();
}

// ---------------------------------------------------------------------------
static void ThreePaneDemo()
{
    static float lft_w = 130.f, mid_w = 220.f, rgt_w = 150.f;
    constexpr float kThick = 4.f, kH = 200.f;

    ImGui::SeparatorText("Three panes  (left | center | right)");
    ImGui::Spacing();

    // Keep all three panels filling the available width on window resize.
    {
        const float avail = ImGui::GetContentRegionAvail().x - 2.f * kThick;
        const float total = lft_w + mid_w + rgt_w;
        if (total > 0.f && fabsf(total - avail) > 0.5f)
        {
            lft_w = avail * (lft_w / total);
            mid_w = avail * (mid_w / total);
            rgt_w = avail - lft_w - mid_w;
        }
    }

    ImGui::BeginChild("##3l", {lft_w, kH}, ImGuiChildFlags_Borders);
    ImGui::SeparatorText("Left");
    ImGui::Text("File tree");
    for (int i = 0; i < 5; ++i)
        ImGui::Text("  folder_%d/", i);
    ImGui::EndChild();

    ImGui::SameLine(0.f, 0.f);
    ImGui::Splitter(true, kThick, &lft_w, &mid_w, 60.f, 60.f, kH);
    ImGui::SameLine(0.f, 0.f);

    ImGui::BeginChild("##3m", {mid_w, kH}, ImGuiChildFlags_Borders);
    ImGui::SeparatorText("Center");
    ImGui::TextWrapped(
        "Main content / editor area. "
        "Drag either bar to redistribute space between the three panels.");
    ImGui::EndChild();

    ImGui::SameLine(0.f, 0.f);
    ImGui::Splitter(true, kThick, &mid_w, &rgt_w, 60.f, 60.f, kH);
    ImGui::SameLine(0.f, 0.f);

    ImGui::BeginChild("##3r", {rgt_w, kH}, ImGuiChildFlags_Borders);
    ImGui::SeparatorText("Right");
    ImGui::Text("Properties");
    ImGui::EndChild();

    ImGui::Spacing();
}

// ---------------------------------------------------------------------------
void ShowDemo()
{
    ImGui::SetNextWindowSize(ImVec2(680, 720), ImGuiCond_FirstUseEver);
    ImGui::Begin("Splitter Demo");

    VerticalSplitDemo();
    HorizontalSplitDemo();
    ThreePaneDemo();

    ImGui::End();
}

