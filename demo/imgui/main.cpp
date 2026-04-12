// ImGui built-in demo window.

#include "DemoApp.h"
#include <imgui.h>

const char* GetDemoTitle() { return "ImGui Demo"; }

static bool s_show = true;

void ShowDemo()
{
    ImGui::ShowDemoWindow(&s_show);
}

