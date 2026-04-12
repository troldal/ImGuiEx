// Stack Layout demo.
// Compiled only when ENABLE_STACK_LAYOUT=ON.
//
// Demonstrates ImGui::BeginHorizontal / EndHorizontal, BeginVertical /
// EndVertical, and Spring — the elastic spacer that makes right-alignment
// and centering trivial without manual arithmetic.

#include "DemoApp.h"
#include <ImGuiEx.h>
#include <imgui.h>

const wchar_t* GetDemoTitle() { return L"Stack Layout Demo"; }

// ---------------------------------------------------------------------------
static void ToolbarDemo()
{
    ImGui::SeparatorText("Toolbar  (right-aligned button via Spring)");
    ImGui::Spacing();

    ImGui::BeginHorizontal("toolbar", ImVec2(ImGui::GetContentRegionAvail().x, 0));

    if (ImGui::Button("New"))  {}
    if (ImGui::Button("Open")) {}
    if (ImGui::Button("Save")) {}

    ImGui::Spring();   // pushes everything after it to the right

    if (ImGui::Button("Help")) {}

    ImGui::EndHorizontal();

    ImGui::Spacing();
}

// ---------------------------------------------------------------------------
static void CenteredButtonDemo()
{
    ImGui::SeparatorText("Centered button via Spring on both sides");
    ImGui::Spacing();

    ImGui::BeginHorizontal("centered", ImVec2(ImGui::GetContentRegionAvail().x, 0));

    ImGui::Spring();              // left elastic gap
    ImGui::Button("Centered!");
    ImGui::Spring();              // right elastic gap (same weight → centred)

    ImGui::EndHorizontal();

    ImGui::Spacing();
}

// ---------------------------------------------------------------------------
static void AlignmentDemo()
{
    ImGui::SeparatorText("Per-layout alignment  (top / centre / bottom)");
    ImGui::Spacing();

    const float row_h = 60.f;

    // Top-aligned row
    ImGui::BeginHorizontal("top_row", ImVec2(ImGui::GetContentRegionAvail().x, row_h), 0.0f);
    ImGui::Button("Top-A");
    ImGui::Button("Top-B\nTwo lines");
    ImGui::Button("Top-C");
    ImGui::EndHorizontal();

    // Centre-aligned row
    ImGui::BeginHorizontal("mid_row", ImVec2(ImGui::GetContentRegionAvail().x, row_h), 0.5f);
    ImGui::Button("Mid-A");
    ImGui::Button("Mid-B\nTwo lines");
    ImGui::Button("Mid-C");
    ImGui::EndHorizontal();

    // Bottom-aligned row
    ImGui::BeginHorizontal("bot_row", ImVec2(ImGui::GetContentRegionAvail().x, row_h), 1.0f);
    ImGui::Button("Bot-A");
    ImGui::Button("Bot-B\nTwo lines");
    ImGui::Button("Bot-C");
    ImGui::EndHorizontal();

    ImGui::Spacing();
}

// ---------------------------------------------------------------------------
static void VerticalLayoutDemo()
{
    ImGui::SeparatorText("Vertical layout with centred label");
    ImGui::Spacing();

    const float col_w = 120.f;

    ImGui::BeginVertical("vert", ImVec2(col_w, 160.f));

    ImGui::Spring();   // top gap
    ImGui::Text("Centred");
    ImGui::Text("vertically");
    ImGui::Spring();   // bottom gap (same weight → centred)

    ImGui::EndVertical();

    ImGui::SameLine();

    ImGui::BeginChild("##desc", ImVec2(0, 160.f), ImGuiChildFlags_Borders);
    ImGui::TextWrapped(
        "BeginVertical with two Springs of equal weight centres its "
        "content along the vertical axis — no manual offset arithmetic needed.");
    ImGui::EndChild();

    ImGui::Spacing();
}

// ---------------------------------------------------------------------------
static void SuspendResumeDemo()
{
    ImGui::SeparatorText("SuspendLayout / ResumeLayout");
    ImGui::Spacing();

    ImGui::BeginHorizontal("suspend_row", ImVec2(ImGui::GetContentRegionAvail().x, 0));

    ImGui::Button("In layout");
    ImGui::Spring();

    ImGui::SuspendLayout();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8.f); // manual nudge outside layout
    ImGui::Button("Suspended (manual pos)");
    ImGui::ResumeLayout();

    ImGui::Spring();
    ImGui::Button("Back in layout");

    ImGui::EndHorizontal();

    ImGui::Spacing();
}

// ---------------------------------------------------------------------------
// NOTE: This demo must be called LAST in ShowDemo() — it sizes its
// BeginVertical to the remaining window height, so any content rendered
// after it would be clipped.
// ---------------------------------------------------------------------------
static void BottomPinnedDemo()
{
    ImGui::SeparatorText("Bottom-pinned buttons  (BeginVertical + Spring)");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "BeginVertical is sized to the remaining window height each frame. "
        "A single Spring() between the content and the button row keeps the "
        "buttons pinned to the bottom edge — resize the window vertically to see it.");
    ImGui::Spacing();

    // Capture available space AFTER the description text above has been laid out.
    // This value changes every frame as the user resizes the window, so the
    // spring automatically absorbs the difference.
    const ImVec2 avail = ImGui::GetContentRegionAvail();

    ImGui::BeginVertical("##pinned", avail);

    ImGui::SeparatorText("Content");
    ImGui::Text("Item 1");
    ImGui::Text("Item 2");
    ImGui::Text("Item 3");
    static char buf[128] = "Editable field";
    ImGui::InputText("Name", buf, sizeof(buf));

    // Spring absorbs all remaining vertical space, pushing the button row down.
    ImGui::Spring();

    // Button row — right-aligned via a nested horizontal layout.
    ImGui::BeginHorizontal("##btns", ImVec2(ImGui::GetContentRegionAvail().x, 0));
    ImGui::Spring();
    if (ImGui::Button("Cancel")) {}
    if (ImGui::Button("OK"))     {}
    ImGui::EndHorizontal();

    ImGui::EndVertical();
}

// ---------------------------------------------------------------------------
void ShowDemo()
{
    ImGui::SetNextWindowSize(ImVec2(680, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Stack Layout Demo");

    ToolbarDemo();
    CenteredButtonDemo();
    AlignmentDemo();
    VerticalLayoutDemo();
    SuspendResumeDemo();
    BottomPinnedDemo(); // must be last — consumes all remaining window height

    ImGui::End();
}
