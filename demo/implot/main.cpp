// ImPlot demo.
// This file is only compiled when ENABLE_IMPLOT=ON; no #ifdefs needed.
// Delegates entirely to ImPlot::ShowDemoWindow(), which opens its own window.

#include "DemoApp.h"
#include <ImGuiEx.hpp>

const char* GetDemoTitle() { return "ImPlot Demo"; }

void ShowDemo()
{
    // Create the ImPlot context once, after the ImGui context exists.
    // Intentionally leaked on exit (acceptable for a demo).
    static ImPlotContext* ctx = nullptr;
    if (!ctx)
        ctx = ImPlot::CreateContext();

    ImPlot::ShowDemoWindow();
}

