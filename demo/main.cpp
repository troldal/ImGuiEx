// ImGuiEx demo — GLFW + OpenGL3
// Shows the standard ImGui demo window and, when built with
// -DENABLE_COLOR_TEXT_EDIT=ON, an ImGuiColorTextEdit demo window.

#include "ImGuiEx.hpp"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// ---- ColorTextEdit demo ------------------------------------------------
#ifdef IMGUIEX_ENABLE_COLOR_TEXT_EDIT
static void ShowColorTextEditDemo()
{
    static TextEditor editor = []()
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

    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("ColorTextEdit Demo",
                 nullptr,
                 ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Edit"))
        {
            bool ro = editor.IsReadOnlyEnabled();
            if (ImGui::MenuItem("Read-only", nullptr, &ro))
                editor.SetReadOnlyEnabled(ro);
            ImGui::Separator();
            if (ImGui::MenuItem("Undo", "Ctrl-Z", nullptr, editor.CanUndo()))
                editor.Undo();
            if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, editor.CanRedo()))
                editor.Redo();
            ImGui::Separator();
            if (ImGui::MenuItem("Copy",  "Ctrl-C", nullptr, editor.AnyCursorHasSelection()))
                editor.Copy();
            if (ImGui::MenuItem("Cut",   "Ctrl-X", nullptr, !ro && editor.AnyCursorHasSelection()))
                editor.Cut();
            if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro))
                editor.Paste();
            ImGui::Separator();
            if (ImGui::MenuItem("Select all"))
                editor.SelectAll();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Dark palette"))
                editor.SetPalette(TextEditor::GetDarkPalette());
            if (ImGui::MenuItem("Light palette"))
                editor.SetPalette(TextEditor::GetLightPalette());
            if (ImGui::MenuItem("Default palette"))
                editor.SetPalette(TextEditor::GetDefaultPalette());
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    auto cpos = editor.GetMainCursorPosition();
    ImGui::Text("Ln %d, Col %-3d | %s | %s | %s",
                cpos.line + 1, cpos.column + 1,
                editor.IsOverwriteEnabled() ? "Ovr" : "Ins",
                editor.CanUndo()            ? "*"   : " ",
                editor.GetLanguageName().c_str());

    editor.Render("##code");

    ImGui::End();
}
#endif // IMGUIEX_ENABLE_COLOR_TEXT_EDIT

static void glfw_error_callback(int error, const char* description)
{
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// ---- Entry point -------------------------------------------------------
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 800, "ImGuiEx Demo", nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // ---- ImGui setup ---------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // ---- Main loop -----------------------------------------------------
    const ImVec4 clearColor = { 0.45f, 0.55f, 0.60f, 1.00f };
    bool showDemoWindow       = true;
    bool showColorTextEdit    = true;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            glfwWaitEvents();
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&showDemoWindow);

#ifdef IMGUIEX_ENABLE_COLOR_TEXT_EDIT
        if (showColorTextEdit)
            ShowColorTextEditDemo();
#else
        (void)showColorTextEdit;
#endif

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clearColor.x * clearColor.w,
                     clearColor.y * clearColor.w,
                     clearColor.z * clearColor.w,
                     clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // ---- Shutdown ------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

