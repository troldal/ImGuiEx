// imgui-knobs demo.
// This file is only compiled when ENABLE_IMGUI_KNOBS=ON; no #ifdefs needed.
//
// Shows every knob variant side by side with its name, plus a KnobInt
// example and a Stepped knob.

#include "DemoApp.h"
#include <imgui-knobs.h>
#include <imgui.h>

const wchar_t* GetDemoTitle() { return L"ImGui Knobs Demo"; }

void ShowDemo()
{
    ImGui::SetNextWindowSize(ImVec2(720, 420), ImGuiCond_FirstUseEver);
    ImGui::Begin("ImGui Knobs Demo");

    // Shared value so all float knobs track together.
    static float value  = 0.5f;
    static int   ival   = 5;
    static float freq   = 440.f;
    static float volume = 0.75f;
    static float pan    = 0.f;

    const float  knob_size = 60.f;
    const float  v_min     = 0.f;
    const float  v_max     = 1.f;

    // ---- All float variants -----------------------------------------------
    ImGui::SeparatorText("Float variants (shared value)");
    ImGui::Spacing();

    struct { const char* label; ImGuiKnobVariant variant; } variants[] = {
        { "Tick",      ImGuiKnobVariant_Tick      },
        { "Dot",       ImGuiKnobVariant_Dot       },
        { "Wiper",     ImGuiKnobVariant_Wiper     },
        { "WiperOnly", ImGuiKnobVariant_WiperOnly },
        { "WiperDot",  ImGuiKnobVariant_WiperDot  },
        { "Space",     ImGuiKnobVariant_Space     },
    };

    for (auto& v : variants)
    {
        ImGuiKnobs::Knob(v.label, &value, v_min, v_max,
                         /*speed*/0, "%.2f", v.variant, knob_size,
                         ImGuiKnobFlags_ValueTooltip);
        ImGui::SameLine();
    }
    ImGui::NewLine();

    // ---- Stepped (integer steps) ------------------------------------------
    ImGui::Spacing();
    ImGui::SeparatorText("Stepped variants");
    ImGui::Spacing();

    ImGuiKnobs::Knob("Stepped\nfloat", &value, v_min, v_max,
                     0, "%.2f", ImGuiKnobVariant_Stepped, knob_size,
                     ImGuiKnobFlags_ValueTooltip, /*steps*/8);
    ImGui::SameLine();
    ImGuiKnobs::KnobInt("Steps\nint", &ival, 0, 10,
                        0, "%d", ImGuiKnobVariant_Stepped, knob_size,
                        ImGuiKnobFlags_ValueTooltip, /*steps*/10);

    // ---- Practical audio-mixer example ------------------------------------
    ImGui::Spacing();
    ImGui::SeparatorText("Audio mixer example");
    ImGui::Spacing();

    ImGuiKnobs::Knob("Freq\n(Hz)", &freq,   20.f, 20000.f, 0, "%.0f",
                     ImGuiKnobVariant_Wiper, knob_size,
                     ImGuiKnobFlags_ValueTooltip | ImGuiKnobFlags_Logarithmic);
    ImGui::SameLine();
    ImGuiKnobs::Knob("Volume",     &volume,  0.f,     1.f, 0, "%.2f",
                     ImGuiKnobVariant_WiperDot, knob_size,
                     ImGuiKnobFlags_ValueTooltip);
    ImGui::SameLine();
    ImGuiKnobs::Knob("Pan",        &pan,    -1.f,     1.f, 0, "%.2f",
                     ImGuiKnobVariant_Dot, knob_size,
                     ImGuiKnobFlags_ValueTooltip);

    ImGui::Spacing();
    ImGui::Text("freq=%.1f Hz  volume=%.2f  pan=%.2f", freq, volume, pan);

    ImGui::End();
}

