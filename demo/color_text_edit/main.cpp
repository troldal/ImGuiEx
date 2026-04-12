// ImGuiColorTextEdit demo.
// This file is only compiled when ENABLE_COLOR_TEXT_EDIT=ON; no #ifdefs needed.

#include "DemoApp.h"
#include "ImGuiEx.h"   // includes TextEditor.h via IMGUIEX_ENABLE_COLOR_TEXT_EDIT

const wchar_t* GetDemoTitle() { return L"ImGuiColorTextEdit Demo"; }

void ShowDemo()
{
    // Function-local static: initialized on first call (during the render loop,
    // after ImGui is fully set up).  A file-scope static would run before main()
    // and leave the editor empty due to static-initialization-order issues.
    static TextEditor s_editor = []()
    {
        TextEditor ed;
        ed.SetLanguage(TextEditor::Language::Cpp());
        ed.SetText(
            "// ImGuiColorTextEdit demo\n"
            "#include <iostream>\n"
            "\n"
            "int main()\n"
            "{\n"
            "    // Edit me!\n"
            "    std::cout << \"Hello, ImGuiEx!\\n\";\n"
            "    return 0;\n"
            "}\n"
        );
        return ed;
    }();
    ImGui::SetNextWindowSize(ImVec2(900, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("ColorTextEdit Demo", nullptr,
                 ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Edit"))
        {
            bool ro = s_editor.IsReadOnlyEnabled();
            if (ImGui::MenuItem("Read-only", nullptr, &ro))
                s_editor.SetReadOnlyEnabled(ro);
            ImGui::Separator();
            if (ImGui::MenuItem("Undo", "Ctrl-Z", nullptr, s_editor.CanUndo()))
                s_editor.Undo();
            if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, s_editor.CanRedo()))
                s_editor.Redo();
            ImGui::Separator();
            if (ImGui::MenuItem("Copy",  "Ctrl-C", nullptr, s_editor.AnyCursorHasSelection()))
                s_editor.Copy();
            if (ImGui::MenuItem("Cut",   "Ctrl-X", nullptr, !ro && s_editor.AnyCursorHasSelection()))
                s_editor.Cut();
            if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro))
                s_editor.Paste();
            ImGui::Separator();
            if (ImGui::MenuItem("Select all"))
                s_editor.SelectAll();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Dark palette"))
                s_editor.SetPalette(TextEditor::GetDarkPalette());
            if (ImGui::MenuItem("Light palette"))
                s_editor.SetPalette(TextEditor::GetLightPalette());
            if (ImGui::MenuItem("Default palette"))
                s_editor.SetPalette(TextEditor::GetDefaultPalette());
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    auto cpos = s_editor.GetMainCursorPosition();
    ImGui::Text("Ln %d, Col %-3d | %s | %s | %s",
                cpos.line + 1, cpos.column + 1,
                s_editor.IsOverwriteEnabled() ? "Ovr" : "Ins",
                s_editor.CanUndo()            ? "*"   : " ",
                s_editor.GetLanguageName().c_str());

    s_editor.Render("##code");

    ImGui::End();
}

