// ImNodeFlow demo.
// This file is only compiled when ENABLE_NODE_FLOW=ON; no #ifdefs needed.
//
// Shows a simple math flow graph:
//
//   [Const A] --\
//               +--> [Add] --> sum -----> [Display (sum)]
//   [Const B] --/
//               \--> [Mul] --> product -> [Display (product)]
//
// Right-click on the canvas background to add new nodes.
// Drag from an output socket to an input socket to connect them.

#include "DemoApp.h"
#include <ImNodeFlow.h>
#include <imgui.h>

const wchar_t* GetDemoTitle() { return L"ImNodeFlow Demo"; }

// ---------------------------------------------------------------------------
// Node definitions
// ---------------------------------------------------------------------------

// Exposes a single float constant via a DragFloat widget.
class ConstNode : public ImFlow::BaseNode
{
public:
    explicit ConstNode(float initial = 1.0f) : m_value(initial)
    {
        setTitle("Constant");
        setStyle(ImFlow::NodeStyle::cyan());
        addOUT<float>("Value")->behaviour([this]() { return m_value; });
    }
    void draw() override
    {
        ImGui::SetNextItemWidth(90.f);
        ImGui::DragFloat("##val", &m_value, 0.1f);
    }
private:
    float m_value;
};

// Adds two float inputs.
class AddNode : public ImFlow::BaseNode
{
public:
    AddNode()
    {
        setTitle("Add");
        setStyle(ImFlow::NodeStyle::green());
        addIN<float>("A", 0.f, ImFlow::ConnectionFilter::SameType());
        addIN<float>("B", 0.f, ImFlow::ConnectionFilter::SameType());
        addOUT<float>("Sum")->behaviour([this]() {
            return getInVal<float>("A") + getInVal<float>("B");
        });
    }
};

// Multiplies two float inputs.
class MulNode : public ImFlow::BaseNode
{
public:
    MulNode()
    {
        setTitle("Multiply");
        setStyle(ImFlow::NodeStyle::brown());
        addIN<float>("A", 1.f, ImFlow::ConnectionFilter::SameType());
        addIN<float>("B", 1.f, ImFlow::ConnectionFilter::SameType());
        addOUT<float>("Product")->behaviour([this]() {
            return getInVal<float>("A") * getInVal<float>("B");
        });
    }
};

// Receives a float and displays its live-computed value.
class DisplayNode : public ImFlow::BaseNode
{
public:
    DisplayNode()
    {
        setTitle("Display");
        setStyle(ImFlow::NodeStyle::red());
        addIN<float>("Value", 0.f, ImFlow::ConnectionFilter::SameType());
    }
    void draw() override
    {
        ImGui::Text("= %.3f", getInVal<float>("Value"));
    }
};

// ---------------------------------------------------------------------------
// ShowDemo
// ---------------------------------------------------------------------------

void ShowDemo()
{
    // The editor lives for the duration of the demo.
    // Static local variables are accessible inside lambdas without capture.
    static ImFlow::ImNodeFlow s_inf("NodeFlowDemo");
    static bool               s_initialized = false;

    if (!s_initialized)
    {
        s_initialized = true;

        // Build initial graph.
        auto constA   = s_inf.addNode<ConstNode>  (ImVec2{ 40,  50}, 3.f);
        auto constB   = s_inf.addNode<ConstNode>  (ImVec2{ 40, 210}, 5.f);
        auto addNode  = s_inf.addNode<AddNode>     (ImVec2{260,  50});
        auto mulNode  = s_inf.addNode<MulNode>     (ImVec2{260, 210});
        auto dispSum  = s_inf.addNode<DisplayNode> (ImVec2{500,  50});
        auto dispProd = s_inf.addNode<DisplayNode> (ImVec2{500, 210});

        constA->outPin("Value")->createLink(addNode->inPin("A"));
        constB->outPin("Value")->createLink(addNode->inPin("B"));
        constA->outPin("Value")->createLink(mulNode->inPin("A"));
        constB->outPin("Value")->createLink(mulNode->inPin("B"));
        addNode->outPin("Sum")->createLink(dispSum->inPin("Value"));
        mulNode->outPin("Product")->createLink(dispProd->inPin("Value"));

        // Right-click popup: place new nodes at the mouse cursor.
        s_inf.rightClickPopUpContent([](ImFlow::BaseNode* /*node*/) {
            if (ImGui::MenuItem("Add: Constant"))  s_inf.placeNode<ConstNode>();
            if (ImGui::MenuItem("Add: Add"))       s_inf.placeNode<AddNode>();
            if (ImGui::MenuItem("Add: Multiply"))  s_inf.placeNode<MulNode>();
            if (ImGui::MenuItem("Add: Display"))   s_inf.placeNode<DisplayNode>();
        });
    }

    ImGui::SetNextWindowSize(ImVec2(960, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("ImNodeFlow Demo");

    ImGui::TextWrapped(
        "Drag from an output socket (right side) to an input socket (left side) "
        "to connect nodes.  Right-click the canvas to add new nodes.  "
        "Adjust the Constant values with DragFloat to see results update live.");
    ImGui::Spacing();

    s_inf.setSize(ImGui::GetContentRegionAvail());
    s_inf.update();

    ImGui::End();
}
