#pragma once

// Contract every demo executable must satisfy.
// DemoFramework.cpp provides main() and the full GLFW/OpenGL3/ImGui render loop;
// it calls these two functions on each iteration.

/// Returns the text shown in the window title bar.
const char* GetDemoTitle();

/// Called once per ImGui frame.  Render whatever UI the demo wants to show.
void ShowDemo();
