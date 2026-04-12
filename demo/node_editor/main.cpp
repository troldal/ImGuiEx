// imgui-node-editor demo.
// This file is only compiled when ENABLE_NODE_EDITOR=ON; no #ifdefs needed.

#include "DemoApp.h"
#include <ImGuiEx.hpp>

namespace ed = ax::NodeEditor;

const char* GetDemoTitle() { return "ImGui Node Editor Demo"; }

void ShowDemo()
{
    // Created on first call (after ImGui context exists); intentionally leaked
    // on exit — acceptable for a demo.
    static ed::EditorContext* ctx = nullptr;
    if (!ctx)
        ctx = ed::CreateEditor();

    ed::SetCurrentEditor(ctx);

    ImGui::SetNextWindowSize(ImVec2(1000, 750), ImGuiCond_FirstUseEver);
    ImGui::Begin("Node Editor Demo");

    ed::Begin("editor");

    // Node/pin/link IDs must all be unique within the same editor context.
    // Convention used here: nodes 1-9, pins 10-19, links 20-29.

    // ---- Node 1: Source ------------------------------------------------
    ed::BeginNode(1);
        ImGui::Text("Source");
        ImGui::Dummy(ImVec2(80, 0));
        ed::BeginPin(10, ed::PinKind::Output);
            ImGui::Text("Value ->");
        ed::EndPin();
    ed::EndNode();

    // ---- Node 2: Process -----------------------------------------------
    ed::BeginNode(2);
        ImGui::Text("Process");
        ImGui::Dummy(ImVec2(80, 0));
        ed::BeginPin(11, ed::PinKind::Input);
            ImGui::Text("-> A");
        ed::EndPin();
        ImGui::SameLine();
        ed::BeginPin(12, ed::PinKind::Output);
            ImGui::Text("Result ->");
        ed::EndPin();
    ed::EndNode();

    // ---- Node 3: Sink --------------------------------------------------
    ed::BeginNode(3);
        ImGui::Text("Sink");
        ImGui::Dummy(ImVec2(80, 0));
        ed::BeginPin(13, ed::PinKind::Input);
            ImGui::Text("-> In");
        ed::EndPin();
    ed::EndNode();

    // ---- Links ---------------------------------------------------------
    ed::Link(20, 10, 11);   // Source.Value   -> Process.A
    ed::Link(21, 12, 13);   // Process.Result -> Sink.In

    ed::End();

    ImGui::End();

    ed::SetCurrentEditor(nullptr);
}

