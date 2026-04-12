// imspinner demo.
// This file is only compiled when ENABLE_IMSPINNER=ON; no #ifdefs needed.
//
// Displays a labelled grid of representative spinner variants.
// imspinner is header-only — no context management required.

#include "DemoApp.h"
#include <imspinner.h>
#include <imgui.h>

const wchar_t* GetDemoTitle() { return L"ImSpinner Demo"; }

// Draw one spinner cell: the spinner itself, then its name centered below.
// BeginGroup/EndGroup turns the (spinner + label) pair into a single layout
// item; SameLine then places the next cell to its right.
static void SpinnerCell(const char* name, auto fn)
{
    ImGui::BeginGroup();

    fn(); // spinner calls Dummy() internally to advance the cursor

    // Center the label under the spinner
    float spinner_w = ImGui::GetItemRectSize().x;
    float text_w    = ImGui::CalcTextSize(name).x;
    float offset    = (spinner_w - text_w) * 0.5f;
    if (offset > 0.f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    ImGui::TextUnformatted(name);

    ImGui::EndGroup();
    ImGui::SameLine(0.f, 12.f);
}

void ShowDemo()
{
    ImGui::SetNextWindowSize(ImVec2(900, 560), ImGuiCond_FirstUseEver);
    ImGui::Begin("ImSpinner Demo");

    const float  r  = 16.f;   // spinner radius
    const float  t  = 3.f;    // line thickness
    const ImColor cyan   (87,  200, 200, 255);
    const ImColor orange (230, 140,  50, 255);
    const ImColor green  ( 80, 200,  80, 255);
    const ImColor red    (220,  60,  60, 255);
    const ImColor white  (255, 255, 255, 220);
    const ImColor half   (255, 255, 255,  80);

    // helper so the loop below can track nextdot state per-spinner
    static float nd1 = 0.f, nd2 = 0.f;

    ImGui::SeparatorText("Arc / ring spinners");
    ImGui::Spacing();

    SpinnerCell("Rainbow",    [&]{ ImSpinner::SpinnerRainbow     ("##s1",  r, t, cyan,  2.8f); });
    SpinnerCell("Ang",        [&]{ ImSpinner::SpinnerAng         ("##s2",  r, t, cyan,  half, 2.8f); });
    SpinnerCell("TwinAng",    [&]{ ImSpinner::SpinnerTwinAng     ("##s3",  r, r*0.6f, t, cyan, orange, 2.8f); });
    SpinnerCell("TwinAng360", [&]{ ImSpinner::SpinnerTwinAng360  ("##s4",  r, r*0.6f, t, cyan, orange); });
    SpinnerCell("LoadingRing",[&]{ ImSpinner::SpinnerLoadingRing ("##s5",  r, t, cyan, half, 2.8f, 5); });
    SpinnerCell("Clock",      [&]{ ImSpinner::SpinnerClock       ("##s6",  r, t, cyan, half, 2.8f); });
    ImGui::NewLine();

    ImGui::Spacing();
    ImGui::SeparatorText("Pulse / fill spinners");
    ImGui::Spacing();

    SpinnerCell("Pulsar",     [&]{ ImSpinner::SpinnerPulsar      ("##s7",  r, t, half, 2.8f); });
    SpinnerCell("TwinPulsar", [&]{ ImSpinner::SpinnerTwinPulsar  ("##s8",  r, t, cyan, 2.8f); });
    SpinnerCell("FadePulsar", [&]{ ImSpinner::SpinnerFadePulsar  ("##s9",  r, cyan, 2.8f); });
    SpinnerCell("Filling",    [&]{ ImSpinner::SpinnerFilling      ("##s10", r, t, white, red, 2.8f); });
    SpinnerCell("Topup",      [&]{ ImSpinner::SpinnerTopup        ("##s11", r, r*0.6f, red, white, half, 2.0f); });
    ImGui::NewLine();

    ImGui::Spacing();
    ImGui::SeparatorText("Dot spinners");
    ImGui::Spacing();

    SpinnerCell("Dots",       [&]{ ImSpinner::SpinnerDots        ("##s12", &nd1, r, t, cyan); });
    SpinnerCell("VDots",      [&]{ ImSpinner::SpinnerVDots       ("##s13", r, t, cyan, half); });
    SpinnerCell("BounceDots", [&]{ ImSpinner::SpinnerBounceDots  ("##s14", r, t, cyan, 2.8f, 3); });
    SpinnerCell("FadeDots",   [&]{ ImSpinner::SpinnerFadeDots    ("##s15", r, t, cyan); });
    SpinnerCell("WaveDots",   [&]{ ImSpinner::SpinnerWaveDots    ("##s16", r, t, cyan); });
    SpinnerCell("MovingDots", [&]{ ImSpinner::SpinnerMovingDots  ("##s17", r, t, cyan); });
    SpinnerCell("RotateDots", [&]{ ImSpinner::SpinnerRotateDots  ("##s18", r, t, cyan); });
    SpinnerCell("Dots",       [&]{ ImSpinner::SpinnerDots        ("##s19", &nd2, r, t, green, 2.8f, 8); });
    ImGui::NewLine();

    ImGui::Spacing();
    ImGui::SeparatorText("Miscellaneous");
    ImGui::Spacing();

    SpinnerCell("CircularLines",[&]{ ImSpinner::SpinnerCircularLines("##s20", r, cyan); });
    SpinnerCell("ScaleDots",    [&]{ ImSpinner::SpinnerScaleDots    ("##s21", r, t, cyan); });
    SpinnerCell("GalaxyDots",   [&]{ ImSpinner::SpinnerGalaxyDots   ("##s22", r, t, cyan); });
    SpinnerCell("IncDots",      [&]{ ImSpinner::SpinnerIncDots      ("##s23", r, t, cyan); });
    ImGui::NewLine();

    ImGui::End();
}


