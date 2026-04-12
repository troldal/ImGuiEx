// imnodes demo.
// This file is only compiled when ENABLE_IMNODES=ON; no #ifdefs needed.
//
// Shows a small interactive node graph:
//   [Inputs]  --A-->  [Add]  --A+B-->  [Result]
//             --B-->
//
// Drag from any output pin to an input pin to create a new link.
// Links can be detached by dragging from the connected pin
// (requires ImNodesAttributeFlags_EnableLinkDetachWithDragClick,
//  which is set up below).

#include "DemoApp.h"
#include <imnodes.h>
#include <imgui.h>
#include <vector>
#include <algorithm>

const wchar_t* GetDemoTitle() { return L"imnodes Demo"; }

// ---------------------------------------------------------------------------
// Node / attribute / link ID layout
//
//  Node 1 "Inputs"  — output attrs: 10 (A out), 11 (B out)
//  Node 2 "Add"     — input  attrs: 20 (A in),  21 (B in)
//                   — output attr:  22 (A+B out)
//  Node 3 "Result"  — input  attr:  30 (Value in)
// ---------------------------------------------------------------------------

struct Link
{
    int id;
    int start_attr;
    int end_attr;
};

static std::vector<Link> s_links;
static int               s_next_link_id = 100;

void ShowDemo()
{
    // Create the imnodes context once, after the ImGui context exists.
    // Intentionally leaked on exit (acceptable for a demo).
    static ImNodesContext* ctx = nullptr;
    if (!ctx)
    {
        ctx = ImNodes::CreateContext();

        // Allow links to be detached by dragging from a connected pin.
        ImNodes::GetIO().LinkDetachWithModifierClick.Modifier =
            &ImGui::GetIO().KeyCtrl;

        // Pre-wire the initial graph.
        s_links.push_back({s_next_link_id++, 10, 20});   // A out  -> A in
        s_links.push_back({s_next_link_id++, 11, 21});   // B out  -> B in
        s_links.push_back({s_next_link_id++, 22, 30});   // A+B    -> Value in
    }

    ImGui::SetNextWindowSize(ImVec2(900, 620), ImGuiCond_FirstUseEver);
    ImGui::Begin("imnodes Demo");

    ImGui::TextWrapped(
        "Drag from an output pin to an input pin to create a link.  "
        "Ctrl+drag a connected pin to detach its link.");
    ImGui::Spacing();

    ImNodes::BeginNodeEditor();

    // ---- Set initial positions on first frame ----------------------------
    static bool positioned = false;
    if (!positioned)
    {
        positioned = true;
        ImNodes::SetNodeEditorSpacePos(1, ImVec2( 40, 120));
        ImNodes::SetNodeEditorSpacePos(2, ImVec2(280, 100));
        ImNodes::SetNodeEditorSpacePos(3, ImVec2(540, 120));
    }

    // ---- Node 1: Inputs --------------------------------------------------
    ImNodes::BeginNode(1);
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Inputs");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginOutputAttribute(10);
            ImGui::Text("A");
        ImNodes::EndOutputAttribute();

        ImNodes::BeginOutputAttribute(11);
            ImGui::Text("B");
        ImNodes::EndOutputAttribute();
    ImNodes::EndNode();

    // ---- Node 2: Add -----------------------------------------------------
    ImNodes::BeginNode(2);
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Add");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(20);
            ImGui::Text("A");
        ImNodes::EndInputAttribute();

        ImNodes::BeginInputAttribute(21);
            ImGui::Text("B");
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(22);
            ImGui::Text("A + B");
        ImNodes::EndOutputAttribute();
    ImNodes::EndNode();

    // ---- Node 3: Result --------------------------------------------------
    ImNodes::BeginNode(3);
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Result");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(30);
            ImGui::Text("Value");
        ImNodes::EndInputAttribute();
    ImNodes::EndNode();

    // ---- Render existing links -------------------------------------------
    for (const Link& lnk : s_links)
        ImNodes::Link(lnk.id, lnk.start_attr, lnk.end_attr);

    ImNodes::MiniMap(0.15f, ImNodesMiniMapLocation_BottomRight);

    ImNodes::EndNodeEditor();

    // ---- Handle new link creation ----------------------------------------
    {
        int start_attr, end_attr;
        if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
            s_links.push_back({s_next_link_id++, start_attr, end_attr});
    }

    // ---- Handle link destruction (detach or external delete) -------------
    {
        int link_id;
        if (ImNodes::IsLinkDestroyed(&link_id))
        {
            auto it = std::find_if(
                s_links.begin(), s_links.end(),
                [link_id](const Link& l) { return l.id == link_id; });
            if (it != s_links.end())
                s_links.erase(it);
        }
    }

    ImGui::End();
}

