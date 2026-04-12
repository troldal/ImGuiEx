// imgui_toggle demo.
// This file is only compiled when ENABLE_IMGUI_TOGGLE=ON; no #ifdefs needed.
//
// Demonstrates the basic toggle widget, the flag-based variants, and the
// preset-config variants shipped by imgui_toggle_presets.h.

#include "DemoApp.h"
#include <imgui_toggle.h>
#include <imgui_toggle_presets.h>
#include <imgui.h>

const wchar_t* GetDemoTitle() { return L"ImGui Toggle Demo"; }

// Helper: draw a toggle with a status label on the same row.
static bool LabelledToggle(const char* id, const char* desc, bool* v,
                            ImGuiToggleFlags flags = ImGuiToggleFlags_None,
                            float speed = 1.0f,
                            float frame_rounding = -1.0f,
                            float knob_rounding  = -1.0f)
{
    bool changed = ImGui::Toggle(id, v, flags, speed, frame_rounding, knob_rounding);
    ImGui::SameLine();
    ImGui::Text("%-28s  %s", desc, *v ? "ON " : "off");
    return changed;
}

static bool LabelledToggleConfig(const char* id, const char* desc, bool* v,
                                  const ImGuiToggleConfig& cfg)
{
    bool changed = ImGui::Toggle(id, v, cfg);
    ImGui::SameLine();
    ImGui::Text("%-28s  %s", desc, *v ? "ON " : "off");
    return changed;
}

void ShowDemo()
{
    ImGui::SetNextWindowSize(ImVec2(540, 620), ImGuiCond_FirstUseEver);
    ImGui::Begin("ImGui Toggle Demo");

    // ---- Basic usage ------------------------------------------------------
    ImGui::SeparatorText("Basic (no animation)");
    ImGui::Spacing();

    static bool v_basic   = true;
    static bool v_rect    = false;
    static bool v_rounded = true;

    LabelledToggle("##basic",   "Default (square corners)",  &v_basic,
                   ImGuiToggleFlags_None);
    LabelledToggle("##rect",    "Rectangular frame+knob",    &v_rect,
                   ImGuiToggleFlags_None, 1.0f, 0.0f, 0.0f);
    LabelledToggle("##rounded", "Fully rounded (default)",   &v_rounded,
                   ImGuiToggleFlags_None, 1.0f, 1.0f, 1.0f);

    // ---- Animated ---------------------------------------------------------
    ImGui::Spacing();
    ImGui::SeparatorText("Animated");
    ImGui::Spacing();

    static bool v_anim_fast  = true;
    static bool v_anim_slow  = false;
    static bool v_anim_brd   = true;

    LabelledToggle("##anim_fast", "Animated  speed=1.0",
                   &v_anim_fast, ImGuiToggleFlags_Animated, 1.0f);
    LabelledToggle("##anim_slow", "Animated  speed=0.25",
                   &v_anim_slow, ImGuiToggleFlags_Animated, 0.25f);
    LabelledToggle("##anim_brd",  "Animated + bordered",
                   &v_anim_brd,
                   ImGuiToggleFlags_Animated |
                   ImGuiToggleFlags_BorderedFrame |
                   ImGuiToggleFlags_BorderedKnob,
                   1.0f);

    // ---- Bordered ---------------------------------------------------------
    ImGui::Spacing();
    ImGui::SeparatorText("Bordered (static)");
    ImGui::Spacing();

    static bool v_brd_frame = false;
    static bool v_brd_knob  = true;
    static bool v_brd_both  = false;

    LabelledToggle("##brd_frame", "Bordered frame only",
                   &v_brd_frame, ImGuiToggleFlags_BorderedFrame);
    LabelledToggle("##brd_knob",  "Bordered knob only",
                   &v_brd_knob,  ImGuiToggleFlags_BorderedKnob);
    LabelledToggle("##brd_both",  "Bordered frame + knob",
                   &v_brd_both,
                   ImGuiToggleFlags_BorderedFrame | ImGuiToggleFlags_BorderedKnob);

    // ---- Presets ----------------------------------------------------------
    ImGui::Spacing();
    ImGui::SeparatorText("Presets (ImGuiTogglePresets)");
    ImGui::Spacing();

    static bool v_pre_def  = true;
    static bool v_pre_rect = false;
    static bool v_pre_glow = true;

    LabelledToggleConfig("##pre_def",  "DefaultStyle()",
                         &v_pre_def,  ImGuiTogglePresets::DefaultStyle());
    LabelledToggleConfig("##pre_rect", "RectangleStyle()",
                         &v_pre_rect, ImGuiTogglePresets::RectangleStyle());
    LabelledToggleConfig("##pre_glow", "GlowingStyle()",
                         &v_pre_glow, ImGuiTogglePresets::GlowingStyle());

    // ---- Accessibility label ----------------------------------------------
    ImGui::Spacing();
    ImGui::SeparatorText("Accessibility (A11y label)");
    ImGui::Spacing();

    static bool v_a11y = true;
    LabelledToggle("##a11y", "A11y flag (shows ON/OFF text)",
                   &v_a11y,
                   ImGuiToggleFlags_Animated | ImGuiToggleFlags_A11y);

    // ---- Summary ----------------------------------------------------------
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("States:  basic=%d  rect=%d  rounded=%d  anim_fast=%d  anim_slow=%d",
                v_basic, v_rect, v_rounded, v_anim_fast, v_anim_slow);
    ImGui::Text("         presets: def=%d  rect=%d  glow=%d  a11y=%d",
                v_pre_def, v_pre_rect, v_pre_glow, v_a11y);

    ImGui::End();
}

