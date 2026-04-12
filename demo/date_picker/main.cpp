// DatePicker demo.
// Compiled only when ENABLE_DATE_PICKER=ON.
//
// Demonstrates ImGui::DatePicker and ImGui::DatePickerEx from the
// ImGuiDatePicker library by DnA-IntRicate.
//
// Both functions take a std::tm& and return true whenever the selected
// date changes — the same convention as ImGui's other input widgets.

#include "DemoApp.h"
#include <ImGuiEx.h>
#include <ctime>
#include <cstdio>

const wchar_t* GetDemoTitle() { return L"ImGuiDatePicker Demo"; }

// ---------------------------------------------------------------------------
// Helper: initialise a tm to today's local date (time fields zeroed).
// ---------------------------------------------------------------------------
static std::tm TodayLocal()
{
    std::time_t now = std::time(nullptr);
    std::tm t{};
#ifdef _WIN32
    localtime_s(&t, &now);   // MSVC / MinGW safe version
#else
    localtime_r(&now, &t);
#endif
    return t;
}

// ---------------------------------------------------------------------------
// Helper: format a tm as "YYYY-MM-DD".
// ---------------------------------------------------------------------------
static void FormatDate(const std::tm& t, char* buf, int buf_sz)
{
    std::snprintf(buf, buf_sz, "%04d-%02d-%02d",
                  t.tm_year + 1900,
                  t.tm_mon  + 1,
                  t.tm_mday);
}

// ---------------------------------------------------------------------------
void ShowDemo()
{
    ImGui::SetNextWindowSize(ImVec2(520, 420), ImGuiCond_FirstUseEver);
    ImGui::Begin("ImGuiDatePicker Demo");

    // ---- Basic DatePicker ---------------------------------------------------
    ImGui::SeparatorText("DatePicker  (basic)");
    ImGui::Spacing();

    static std::tm date1 = TodayLocal();
    static char    label1[32];
    FormatDate(date1, label1, sizeof(label1));

    ImGui::PushID("date1");
    if (ImGui::DatePicker("##picker", date1))
        FormatDate(date1, label1, sizeof(label1));
    ImGui::PopID();

    ImGui::SameLine();
    ImGui::TextDisabled("Selected: %s", label1);

    ImGui::Spacing();

    // ---- DatePicker with clamped popup (stays inside window border) ---------
    ImGui::SeparatorText("DatePicker  (clampToBorder = true)");
    ImGui::Spacing();

    static std::tm date2 = TodayLocal();
    static char    label2[32];
    FormatDate(date2, label2, sizeof(label2));

    ImGui::PushID("date2");
    if (ImGui::DatePicker("##picker", date2, /*clampToBorder=*/true))
        FormatDate(date2, label2, sizeof(label2));
    ImGui::PopID();

    ImGui::SameLine();
    ImGui::TextDisabled("Selected: %s", label2);

    ImGui::Spacing();

    // ---- DatePickerEx with a narrower item spacing --------------------------
    ImGui::SeparatorText("DatePickerEx  (custom itemSpacing, no alt font)");
    ImGui::Spacing();

    static std::tm date3 = TodayLocal();
    static char    label3[32];
    FormatDate(date3, label3, sizeof(label3));

    // Pass nullptr for the alt font to use the current default font.
    ImGui::PushID("date3");
    if (ImGui::DatePickerEx("##picker", date3, /*altFont=*/nullptr,
                            /*clampToBorder=*/false, /*itemSpacing=*/100.0f))
        FormatDate(date3, label3, sizeof(label3));
    ImGui::PopID();

    ImGui::SameLine();
    ImGui::TextDisabled("Selected: %s", label3);

    ImGui::Spacing();

    // ---- Date-range selection (two pickers side-by-side) --------------------
    ImGui::SeparatorText("Date range  (start / end)");
    ImGui::Spacing();

    static std::tm range_start = TodayLocal();
    static std::tm range_end   = TodayLocal();
    static char    rs_label[32], re_label[32];
    FormatDate(range_start, rs_label, sizeof(rs_label));
    FormatDate(range_end,   re_label, sizeof(re_label));

    ImGui::Text("From:");
    ImGui::SameLine();
    ImGui::PushID("range_start");
    if (ImGui::DatePicker("##picker", range_start))
        FormatDate(range_start, rs_label, sizeof(rs_label));
    ImGui::PopID();

    ImGui::SameLine(0.f, 16.f);

    ImGui::Text("To:");
    ImGui::SameLine();
    ImGui::PushID("range_end");
    if (ImGui::DatePicker("##picker", range_end))
        FormatDate(range_end, re_label, sizeof(re_label));
    ImGui::PopID();

    ImGui::Spacing();
    ImGui::TextDisabled("Range: %s  →  %s", rs_label, re_label);

    ImGui::End();
}

