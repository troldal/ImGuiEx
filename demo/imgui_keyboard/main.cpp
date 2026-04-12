// imgui_keyboard demo.
// Compiled only when ENABLE_IMGUI_KEYBOARD=ON.
//
// Demonstrates ImKeyboard::Keyboard() from the imgui_keyboard library by
// mgerhardy (https://github.com/mgerhardy/imgui_keyboard).
//
// ShowDemo() delegates to ImKeyboard::KeyboardDemo(), which provides:
//   - Layout selector  (QWERTY, QWERTZ, AZERTY, Colemak, Dvorak, Numpad,
//                       ISO variants, Apple ANSI/ISO)
//   - Flag toggles     (ShowPressed, NoShiftLabels, ShowBothLabels,
//                       ShowIcons, NoNumpad, Recordable)
//   - Key highlighting (WASD, arrow keys, numpad, or any single key)
//   - Style editor     (sizes and colours)

#include "DemoApp.h"
#include <ImGuiEx.hpp>

const char* GetDemoTitle() { return "ImGui Keyboard Demo"; }

void ShowDemo()
{
    ImGui::SetNextWindowSize(ImVec2(1100, 620), ImGuiCond_FirstUseEver);
    ImGui::Begin("ImGui Keyboard Demo");

    ImKeyboard::KeyboardDemo();

    ImGui::End();
}

