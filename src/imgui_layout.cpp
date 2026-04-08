#include "imgui_layout.h"

namespace ImGui {

// ---------------------------------------------------------------------------
// HStackScope
// ---------------------------------------------------------------------------

HStackScope::HStackScope(float sp)
    : spacing(sp), first(true)
{
}

void HStackScope::item()
{
    if (!first)
        SameLine(0.0f, spacing);
    first = false;
}

} // namespace ImGui

