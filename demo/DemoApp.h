#pragma once

// Contract every demo executable must satisfy.
// DemoApp.cpp provides WinMain and the full Win32/DX11/ImGui render loop;
// it calls these two functions on each iteration.

/// Returns the text shown in the window title bar.
const wchar_t* GetDemoTitle();

/// Called once per ImGui frame.  Render whatever UI the demo wants to show.
void ShowDemo();

