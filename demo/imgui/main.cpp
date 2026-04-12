// ImGui built-in demo window.

#include "DemoApp.h"
#include <imgui.h>

const wchar_t* GetDemoTitle() { return L"ImGui Demo"; }

static bool s_show = true;

void ShowDemo()
{
    ImGui::ShowDemoWindow(&s_show);
}

