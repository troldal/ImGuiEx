// ImPlot3D demo.
// This file is only compiled when ENABLE_IMPLOT3D=ON; no #ifdefs needed.
// Delegates entirely to ImPlot3D::ShowDemoWindow(), which opens its own window.

#include "DemoApp.h"
#include <implot3d.h>

const wchar_t* GetDemoTitle() { return L"ImPlot3D Demo"; }

void ShowDemo()
{
    // Create the ImPlot3D context once, after the ImGui context exists.
    // Intentionally leaked on exit (acceptable for a demo).
    static ImPlot3DContext* ctx = nullptr;
    if (!ctx)
        ctx = ImPlot3D::CreateContext();

    ImPlot3D::ShowDemoWindow();
}

