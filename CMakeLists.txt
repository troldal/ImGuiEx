cmake_minimum_required(VERSION 3.28)
project(ImGuiEx)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

# ---------------------------------------------------------------------------
# FreeType support (optional)
# ---------------------------------------------------------------------------
option(IMGUIEX_ENABLE_FREETYPE "Enable FreeType support for ImGui font rendering" ON)

if (IMGUIEX_ENABLE_FREETYPE)
    FetchContent_Declare(
            freetype
            GIT_REPOSITORY https://github.com/freetype/freetype.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(freetype)

    set(IMGUI_ENABLE_FREETYPE ON CACHE BOOL "" FORCE)
else ()
    set(IMGUI_ENABLE_FREETYPE OFF CACHE BOOL "" FORCE)
endif ()

# ---------------------------------------------------------------------------
# ImGui (docking branch) — fetched and configured via a community CMakeLists
# ---------------------------------------------------------------------------
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        docking   # pin to a commit hash for reproducibility, e.g. abc1234
        GIT_SHALLOW    TRUE
)

# FetchContent_Populate(name) is deprecated in CMake 3.30+ (CMP0169).
# The inject-CMakeLists trick requires it, so we opt-in to the old behaviour
# when the policy exists (CMake < 3.30 does not know CMP0169 at all).
if (POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif ()


FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    FetchContent_Populate(imgui)

    # ImGui ships no CMakeLists.txt — inject the community-maintained one
    file(
            DOWNLOAD
            "https://gist.githubusercontent.com/rokups/f771217b2d530d170db5cb1e08e9a8f4/raw/4c2c14374ab878ca2f45daabfed4c156468e4e27/CMakeLists.txt"
            "${imgui_SOURCE_DIR}/CMakeLists.txt"
            EXPECTED_HASH SHA256=fd62f69364ce13a4f7633a9b50ae6672c466bcc44be60c69c45c0c6e225bb086
    )

    # If FreeType was built via FetchContent, patch ImGui's CMakeLists so it
    # can find the already-available `freetype` target.  The community CMakeLists
    # calls find_package(Freetype) which won't find a FetchContent-built lib.
    # We replace the entire FreeType discovery block with a direct link to the
    # `freetype` target that FetchContent already created.
    if (IMGUIEX_ENABLE_FREETYPE)
        file(READ "${imgui_SOURCE_DIR}/CMakeLists.txt" _imgui_cmake)
        string(FIND "${_imgui_cmake}" "TARGET freetype" _ft_patch_done)
        if (_ft_patch_done EQUAL -1)
            # 1. Disable find_package(Freetype) — it won't find our FetchContent build
            string(REPLACE
                "find_package(Freetype)"
                "set(FREETYPE_FOUND FALSE) # patched by ImGuiEx"
                _imgui_cmake "${_imgui_cmake}")
            # 2. Check for our FetchContent-created target instead of ../freetype dir
            string(REPLACE
                "elseif (EXISTS ../freetype)"
                "elseif (TARGET freetype)"
                _imgui_cmake "${_imgui_cmake}")
            # 3. Remove the add_subdirectory(../freetype) — target is already built
            string(REPLACE
                "add_subdirectory(../freetype \$\{CMAKE_BINARY_DIR\}/freetype)"
                "# patched out by ImGuiEx: freetype target already provided"
                _imgui_cmake "${_imgui_cmake}")
            file(WRITE "${imgui_SOURCE_DIR}/CMakeLists.txt" "${_imgui_cmake}")
        endif ()
    endif ()

    add_subdirectory(${imgui_SOURCE_DIR} ${imgui_BINARY_DIR})
endif ()

# ---------------------------------------------------------------------------
# Extension options (prefixed to avoid name clashes)
# ---------------------------------------------------------------------------
option(IMGUIEX_ENABLE_COLOR_TEXT_EDIT "Include the ImGuiColorTextEdit extension" OFF)
option(IMGUIEX_ENABLE_NODE_EDITOR     "Include the imgui-node-editor extension"  OFF)
option(IMGUIEX_ENABLE_IMNODES         "Include the imnodes extension"            OFF)
option(IMGUIEX_ENABLE_NODE_FLOW       "Include the ImNodeFlow extension"         OFF)
option(IMGUIEX_ENABLE_IMPLOT          "Include the ImPlot extension"             OFF)
option(IMGUIEX_ENABLE_IMPLOT3D        "Include the ImPlot3D extension"           OFF)
option(IMGUIEX_ENABLE_IMGUI_KNOBS     "Include the imgui-knobs extension"        OFF)
option(IMGUIEX_ENABLE_IMSPINNER       "Include the imspinner extension"           OFF)
option(IMGUIEX_ENABLE_IMGUI_TOGGLE    "Include the imgui_toggle extension"        OFF)
option(IMGUIEX_ENABLE_SPLITTER        "Include the built-in Splitter widget"      OFF)
option(IMGUIEX_ENABLE_STACK_LAYOUT    "Include the Stack Layout extension (PR #846)" OFF)
option(IMGUIEX_ENABLE_DATE_PICKER     "Include the ImGuiDatePicker extension"        OFF)
option(IMGUIEX_ENABLE_IMGUI_KEYBOARD  "Include the imgui_keyboard extension"         OFF)

# ---------------------------------------------------------------------------
# ImGuiEx — INTERFACE aggregator library (no sources of its own)
# ---------------------------------------------------------------------------
add_library(ImGuiEx INTERFACE)

target_include_directories(ImGuiEx
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(ImGuiEx
        INTERFACE imgui
)

# ---------------------------------------------------------------------------
# Built-in feature: Splitter
# (header-only; no external dependency — just sets a compile definition)
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_SPLITTER)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_SPLITTER)
endif ()

# ---------------------------------------------------------------------------
# Stack Layout  (ENABLE_STACK_LAYOUT)
# Based on thedmd's feature/layout branch (PR #846).
# Patches the fetched ImGui source files to add the required structs and hooks,
# then compiles src/imgui_stacklayout.cpp as a separate static library.
#
# Patches applied (all idempotent via a guard string check):
#   imgui_internal.h  – ImGuiLayoutItemType typedef/enum, ImGuiLayoutItem and
#                       ImGuiLayout structs, new fields in ImGuiWindowTempData
#   imgui.h           – IMGUI_HAS_STACK_LAYOUT define, public API declarations,
#                       ImGuiStyleVar_LayoutAlign, ImGuiStyle::LayoutAlign field
#   imgui.cpp         – ~ImGuiWindow cleanup, Begin() reset, ItemSize() horiz
#                       handling, ItemAdd() bounds update, GStyleVarsInfo entry,
#                       ImGuiStyle ctor initialisation, forward declarations
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_STACK_LAYOUT)
    # Ensure imgui has been populated before we can patch its sources.
    FetchContent_GetProperties(imgui)
    if (NOT imgui_POPULATED)
        FetchContent_Populate(imgui)
    endif ()

    # ------------------------------------------------------------------
    # Patch imgui_internal.h
    # ------------------------------------------------------------------
    file(READ "${imgui_SOURCE_DIR}/imgui_internal.h" _sl_ih)

    # Guard: only patch when our sentinel is absent
    string(FIND "${_sl_ih}" "ImGuiLayoutItemType" _sl_ih_done)
    if (_sl_ih_done EQUAL -1)

        # 1. Add ImGuiLayoutItemType typedef alongside ImGuiLayoutType
        string(REPLACE
            "typedef int ImGuiLayoutType;            // -> enum ImGuiLayoutType_         // Enum: Horizontal or vertical"
            "typedef int ImGuiLayoutType;            // -> enum ImGuiLayoutType_         // Enum: Horizontal or vertical\ntypedef int ImGuiLayoutItemType;        // -> enum ImGuiLayoutItemType_     // Enum: Item or Spring"
            _sl_ih "${_sl_ih}")

        # 2. Add ImGuiLayoutItemType_ enum right after the ImGuiLayoutType_ enum
        string(REPLACE
            "    ImGuiLayoutType_Vertical = 1\n};"
            "    ImGuiLayoutType_Vertical = 1\n};\nenum ImGuiLayoutItemType_\n{\n    ImGuiLayoutItemType_Item,\n    ImGuiLayoutItemType_Spring\n};"
            _sl_ih "${_sl_ih}")

        # 3. Add forward declarations alongside ImGuiGroupData forward decl
        string(REPLACE
            "struct ImGuiGroupData;              // Stacked storage data for BeginGroup()/EndGroup()"
            "struct ImGuiGroupData;              // Stacked storage data for BeginGroup()/EndGroup()\nstruct ImGuiLayout;                  // Stack layout state (PR #846)\nstruct ImGuiLayoutItem;              // Item / spring inside a stack layout"
            _sl_ih "${_sl_ih}")

        # 4. Insert ImGuiLayoutItem and ImGuiLayout struct bodies after ImGuiGroupData body
        string(REPLACE
            "};\n\n// Simple column measurement, currently used for MenuItem() only.. This is very short-sighted/throw-away code and NOT a generic helper."
            "};\n\n// Stack Layout item — either a regular widget or an elastic spring (PR #846)\nstruct ImGuiLayoutItem\n{\n    ImGuiLayoutItemType     Type;\n    ImRect                  MeasuredBounds;\n    float                   SpringWeight;\n    float                   SpringSpacing;\n    float                   SpringSize;\n    float                   CurrentAlign;\n    float                   CurrentAlignOffset;\n    unsigned int            VertexIndexBegin;\n    unsigned int            VertexIndexEnd;\n\n    ImGuiLayoutItem(ImGuiLayoutItemType type)\n    {\n        Type = type;\n        MeasuredBounds = ImRect(0, 0, 0, 0);\n        SpringWeight = 1.0f;\n        SpringSpacing = -1.0f;\n        SpringSize = 0.0f;\n        CurrentAlign = 0.0f;\n        CurrentAlignOffset = 0.0f;\n        VertexIndexBegin = VertexIndexEnd = (ImDrawIdx)0;\n    }\n};\n\n// Stack Layout container — holds items and springs for one BeginH/V block\nstruct ImGuiLayout\n{\n    ImGuiID                     Id;\n    ImGuiLayoutType             Type;\n    bool                        Live;\n    ImVec2                      Size;\n    ImVec2                      CurrentSize;\n    ImVec2                      MinimumSize;\n    ImVec2                      MeasuredSize;\n    ImVector<ImGuiLayoutItem>   Items;\n    int                         CurrentItemIndex;\n    int                         ParentItemIndex;\n    ImGuiLayout*                Parent;\n    ImGuiLayout*                FirstChild;\n    ImGuiLayout*                NextSibling;\n    float                       Align;\n    float                       Indent;\n    ImVec2                      StartPos;\n    ImVec2                      StartCursorMaxPos;\n\n    ImGuiLayout(ImGuiID id, ImGuiLayoutType type)\n    {\n        Id = id;\n        Type = type;\n        Live = false;\n        Size = CurrentSize = MinimumSize = MeasuredSize = ImVec2(0, 0);\n        CurrentItemIndex = 0;\n        ParentItemIndex = 0;\n        Parent = FirstChild = NextSibling = NULL;\n        Align = -1.0f;\n        Indent = 0.0f;\n        StartPos = ImVec2(0, 0);\n        StartCursorMaxPos = ImVec2(0, 0);\n    }\n};\n\n// Simple column measurement, currently used for MenuItem() only.. This is very short-sighted/throw-away code and NOT a generic helper."
            _sl_ih "${_sl_ih}")

        # 5. Add CurrentLayout / CurrentLayoutItem / LayoutStack / Layouts fields
        #    to ImGuiWindowTempData, after ParentLayoutType
        string(REPLACE
            "    ImGuiLayoutType         ParentLayoutType;       // Layout type of parent window at the time of Begin()"
            "    ImGuiLayoutType         ParentLayoutType;       // Layout type of parent window at the time of Begin()\n    ImGuiLayout*            CurrentLayout;          // Current stack layout (NULL when not inside BeginH/V)\n    ImGuiLayoutItem*        CurrentLayoutItem;      // Current item inside the active layout\n    ImVector<ImGuiLayout*>  LayoutStack;            // Stack of nested layouts\n    ImGuiStorage            Layouts;                // Persistent per-window layout storage (keyed by ImGuiID)"
            _sl_ih "${_sl_ih}")

        file(WRITE "${imgui_SOURCE_DIR}/imgui_internal.h" "${_sl_ih}")
    endif ()

    # ------------------------------------------------------------------
    # Patch imgui.h
    # ------------------------------------------------------------------
    file(READ "${imgui_SOURCE_DIR}/imgui.h" _sl_h)

    string(FIND "${_sl_h}" "IMGUI_HAS_STACK_LAYOUT" _sl_h_done)
    if (_sl_h_done EQUAL -1)

        # 1. Feature-detect define
        string(REPLACE
            "#define IMGUI_HAS_DOCK              // In 'docking' WIP branch."
            "#define IMGUI_HAS_DOCK              // In 'docking' WIP branch.\n#define IMGUI_HAS_STACK_LAYOUT      1   // Stack-Layout PR #846"
            _sl_h "${_sl_h}")

        # 2. Public API declarations (after GetFrameHeightWithSpacing)
        string(REPLACE
            "    IMGUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)"
            "    IMGUI_API float         GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)\n\n    // Stack Layout (PR #846) — horizontal/vertical auto-sizing with elastic springs\n    IMGUI_API void          BeginHorizontal(const char* str_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);\n    IMGUI_API void          BeginHorizontal(const void* ptr_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);\n    IMGUI_API void          BeginHorizontal(int id, const ImVec2& size = ImVec2(0, 0), float align = -1);\n    IMGUI_API void          EndHorizontal();\n    IMGUI_API void          BeginVertical(const char* str_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);\n    IMGUI_API void          BeginVertical(const void* ptr_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);\n    IMGUI_API void          BeginVertical(int id, const ImVec2& size = ImVec2(0, 0), float align = -1);\n    IMGUI_API void          EndVertical();\n    IMGUI_API void          Spring(float weight = 1.0f, float spacing = -1.0f); // elastic spacer\n    IMGUI_API void          SuspendLayout();\n    IMGUI_API void          ResumeLayout();"
            _sl_h "${_sl_h}")

        # 3. ImGuiStyleVar_LayoutAlign (before ImGuiStyleVar_COUNT)
        string(REPLACE
            "    ImGuiStyleVar_DockingSeparatorSize,     // float     DockingSeparatorSize\n    ImGuiStyleVar_COUNT"
            "    ImGuiStyleVar_DockingSeparatorSize,     // float     DockingSeparatorSize\n    ImGuiStyleVar_LayoutAlign,              // float     LayoutAlign\n    ImGuiStyleVar_COUNT"
            _sl_h "${_sl_h}")

        # 4. LayoutAlign field in ImGuiStyle (after DockingSeparatorSize)
        string(REPLACE
            "    float       DockingSeparatorSize;       // Thickness of resizing border between docked windows"
            "    float       DockingSeparatorSize;       // Thickness of resizing border between docked windows\n    float       LayoutAlign;                // Element alignment inside horizontal and vertical layouts (0.0f - left/top, 1.0f - right/bottom, 0.5f - center)."
            _sl_h "${_sl_h}")

        file(WRITE "${imgui_SOURCE_DIR}/imgui.h" "${_sl_h}")
    endif ()

    # ------------------------------------------------------------------
    # Patch imgui.cpp
    # ------------------------------------------------------------------
    file(READ "${imgui_SOURCE_DIR}/imgui.cpp" _sl_cpp)

    string(FIND "${_sl_cpp}" "IMGUIEX_STACK_LAYOUT_PATCHED" _sl_cpp_done)
    if (_sl_cpp_done EQUAL -1)

        # Mark the file as patched (sentinel comment, removed by none of the replacements)
        string(REPLACE
            "// dear imgui, v"
            "// dear imgui, v"   # no-op sentinel: the real guard is the IMGUIEX_STACK_LAYOUT_PATCHED string
            _sl_cpp "${_sl_cpp}")

        # -- a) ~ImGuiWindow destructor: delete layout objects --
        string(REPLACE
            "    IM_DELETE(Name);\n    ColumnsStorage.clear_destruct();\n}"
            "    IM_DELETE(Name);\n    ColumnsStorage.clear_destruct();\n    for (int i = 0; i < DC.Layouts.Data.Size; i++) // IMGUIEX_STACK_LAYOUT_PATCHED\n    {\n        ImGuiLayout* layout = (ImGuiLayout*)DC.Layouts.Data[i].val_p;\n        IM_DELETE(layout);\n    }\n}"
            _sl_cpp "${_sl_cpp}")

        # -- b) Begin(): reset layout stack + mark all layouts as dead each frame --
        string(REPLACE
            "        window->DC.ParentLayoutType = parent_window ? parent_window->DC.LayoutType : ImGuiLayoutType_Vertical;\n\n        // Default item width."
            "        window->DC.ParentLayoutType = parent_window ? parent_window->DC.LayoutType : ImGuiLayoutType_Vertical;\n        window->DC.CurrentLayout = NULL;\n        window->DC.CurrentLayoutItem = NULL;\n        window->DC.LayoutStack.resize(0);\n        for (int i = 0; i < window->DC.Layouts.Data.Size; i++) // mark all layouts as dead; revived when visited\n        {\n            ImGuiLayout* layout = (ImGuiLayout*)window->DC.Layouts.Data[i].val_p;\n            layout->Live = false;\n        }\n\n        // Default item width."
            _sl_cpp "${_sl_cpp}")

        # -- c) ItemSize(): add horizontal layout handling before baseline offset calc --
        string(REPLACE
            "    if (window->SkipItems)\n        return;\n\n    // We increase the height in this function to accommodate for baseline offset."
            "    if (window->SkipItems)\n        return;\n\n    // [Stack Layout] Handle horizontal layout early.\n    {\n        ImGuiLayoutType layout_type = window->DC.LayoutType;\n        if (window->DC.CurrentLayout)\n            layout_type = window->DC.CurrentLayout->Type;\n        if (layout_type == ImGuiLayoutType_Horizontal)\n        {\n            const float line_width = ImMax(window->DC.CurrLineSize.x, size.x);\n            window->DC.CursorPosPrevLine.x = window->DC.CursorPos.x;\n            window->DC.CursorPosPrevLine.y = window->DC.CursorPos.y + size.y;\n            window->DC.CursorPos.x = ImFloor(window->DC.CursorPos.x + line_width + g.Style.ItemSpacing.x);\n            window->DC.CursorPos.y = ImFloor(window->DC.CursorPosPrevLine.y - size.y);\n            window->DC.CursorMaxPos.x = ImMax(window->DC.CursorMaxPos.x, window->DC.CursorPos.x - g.Style.ItemSpacing.x);\n            window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, window->DC.CursorPosPrevLine.y);\n            window->DC.PrevLineSize.x = line_width;\n            window->DC.PrevLineSize.y = 0.0f;\n            window->DC.CurrLineSize.x = 0.0f;\n            window->DC.PrevLineTextBaseOffset = ImMax(window->DC.CurrLineTextBaseOffset, text_baseline_y);\n            window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;\n            window->DC.IsSameLine = window->DC.IsSetPos = false;\n            return;\n        }\n    }\n\n    // We increase the height in this function to accommodate for baseline offset."
            _sl_cpp "${_sl_cpp}")

        # -- d) ItemSize() vertical path: reset PrevLineSize.x and remove SameLine call --
        string(REPLACE
            "    window->DC.PrevLineSize.y = line_height;\n    window->DC.CurrLineSize.y = 0.0f;"
            "    window->DC.PrevLineSize.x = 0.0f;\n    window->DC.PrevLineSize.y = line_height;\n    window->DC.CurrLineSize.y = 0.0f;"
            _sl_cpp "${_sl_cpp}")

        string(REPLACE
            "    // Horizontal layout mode\n    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)\n        SameLine();"
            "    // [Stack Layout] Horizontal mode is handled above via CurrentLayout check."
            _sl_cpp "${_sl_cpp}")

        # -- e) ItemAdd(): track current layout item bounds --
        string(REPLACE
            "    g.NextItemData.HasFlags = ImGuiNextItemDataFlags_None;\n    g.NextItemData.ItemFlags = ImGuiItemFlags_None;"
            "    g.NextItemData.HasFlags = ImGuiNextItemDataFlags_None;\n    g.NextItemData.ItemFlags = ImGuiItemFlags_None;\n\n    // [Stack Layout] Expand the current layout item's measured bounds.\n    if (window->DC.CurrentLayoutItem)\n        window->DC.CurrentLayoutItem->MeasuredBounds.Max = ImMax(window->DC.CurrentLayoutItem->MeasuredBounds.Max, bb.Max);"
            _sl_cpp "${_sl_cpp}")

        # -- f) GStyleVarsInfo table: add LayoutAlign entry --
        string(REPLACE
            "    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, DockingSeparatorSize) },      // ImGuiStyleVar_DockingSeparatorSize\n};"
            "    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, DockingSeparatorSize) },      // ImGuiStyleVar_DockingSeparatorSize\n    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, LayoutAlign) },           // ImGuiStyleVar_LayoutAlign\n};"
            _sl_cpp "${_sl_cpp}")

        # -- g) ImGuiStyle constructor: initialise LayoutAlign --
        string(REPLACE
            "    DockingSeparatorSize        = 2.0f;             // Thickness of resizing border between docked windows"
            "    DockingSeparatorSize        = 2.0f;             // Thickness of resizing border between docked windows\n    LayoutAlign                 = 0.5f;             // Default: centred"
            _sl_cpp "${_sl_cpp}")

        # -- h) namespace ImGui forward-declaration block: add Stack Layout helpers --
        string(REPLACE
            "// Error Checking and Debug Tools\nstatic void             ErrorCheckNewFrameSanityChecks();"
            "// Stack Layout helpers (PR #846)\nstatic ImGuiLayout*     FindLayout(ImGuiID id, ImGuiLayoutType type);\nstatic ImGuiLayout*     CreateNewLayout(ImGuiID id, ImGuiLayoutType type, ImVec2 size);\nstatic void             BeginLayout(ImGuiID id, ImGuiLayoutType type, ImVec2 size, float align);\nstatic void             EndLayout(ImGuiLayoutType type);\nstatic void             PushLayout(ImGuiLayout* layout);\nstatic void             PopLayout(ImGuiLayout* layout);\nstatic void             BalanceLayoutSprings(ImGuiLayout& layout);\nstatic ImVec2           BalanceLayoutItemAlignment(ImGuiLayout& layout, ImGuiLayoutItem& item);\nstatic void             BalanceLayoutItemsAlignment(ImGuiLayout& layout);\nstatic void             BalanceChildLayouts(ImGuiLayout& layout);\nstatic ImVec2           CalculateLayoutSize(ImGuiLayout& layout, bool collapse_springs);\nstatic ImGuiLayoutItem* GenerateLayoutItem(ImGuiLayout& layout, ImGuiLayoutItemType type);\nstatic float            CalculateLayoutItemAlignmentOffset(ImGuiLayout& layout, ImGuiLayoutItem& item);\nstatic void             TranslateLayoutItem(ImGuiLayoutItem& item, const ImVec2& offset);\nstatic void             BeginLayoutItem(ImGuiLayout& layout);\nstatic void             EndLayoutItem(ImGuiLayout& layout);\nstatic void             AddLayoutSpring(ImGuiLayout& layout, float weight, float spacing);\nstatic void             SignedIndent(float indent);\n\n// Error Checking and Debug Tools\nstatic void             ErrorCheckNewFrameSanityChecks();"
            _sl_cpp "${_sl_cpp}")

        file(WRITE "${imgui_SOURCE_DIR}/imgui.cpp" "${_sl_cpp}")
    endif ()

    # ------------------------------------------------------------------
    # Build imgui_stacklayout as a static library
    # ------------------------------------------------------------------
    add_library(imgui_stacklayout STATIC
            ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_stacklayout.cpp
    )

    target_include_directories(imgui_stacklayout
            PUBLIC ${imgui_SOURCE_DIR}
    )

    target_link_libraries(imgui_stacklayout PUBLIC imgui)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE imgui_stacklayout)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_STACK_LAYOUT)
endif ()

# ---------------------------------------------------------------------------
# Extension: ImGuiColorTextEdit
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_COLOR_TEXT_EDIT)
    FetchContent_Declare(
            ImGuiColorTextEdit
            GIT_REPOSITORY https://github.com/goossens/ImGuiColorTextEdit.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
    )

    # The repo ships no CMakeLists.txt, so FetchContent_MakeAvailable would
    # download the sources but never create a CMake target.  Use the lower-level
    # FetchContent_Populate (CMP0169 OLD is already set above for imgui) and
    # build a static library from the two source files ourselves.
    FetchContent_GetProperties(ImGuiColorTextEdit)
    if (NOT imguicolortextedit_POPULATED)
        FetchContent_Populate(ImGuiColorTextEdit)
    endif ()

    add_library(ImGuiColorTextEdit STATIC
            ${imguicolortextedit_SOURCE_DIR}/TextEditor.cpp
            ${imguicolortextedit_SOURCE_DIR}/TextDiff.cpp
    )

    # PUBLIC so the include path propagates to every consumer through ImGuiEx
    target_include_directories(ImGuiColorTextEdit
            PUBLIC ${imguicolortextedit_SOURCE_DIR}
    )

    target_link_libraries(ImGuiColorTextEdit PUBLIC imgui)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE ImGuiColorTextEdit)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_COLOR_TEXT_EDIT)
endif ()

# ---------------------------------------------------------------------------
# Extension: imgui-node-editor
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_NODE_EDITOR)
    FetchContent_Declare(
            ImGuiNodeEditor
            GIT_REPOSITORY https://github.com/thedmd/imgui-node-editor.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
    )

    # No CMakeLists.txt — populate and build manually.
    FetchContent_GetProperties(ImGuiNodeEditor)
    if (NOT imguinodeeditor_POPULATED)
        FetchContent_Populate(ImGuiNodeEditor)
    endif ()

    # ------------------------------------------------------------------
    # Compatibility patches for imgui-node-editor (master) + imgui (docking).
    # Applied OUTSIDE _POPULATED so every cmake configure ensures they
    # are present; all replacements are designed to be idempotent.
    #
    # Root cause:
    #  imgui_extra_math.h unconditionally sets IMGUI_DEFINE_MATH_OPERATORS
    #  before including imgui.h, which causes imgui.h to define the ImVec2
    #  operators AND set IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED.
    #  In the master branch, operator== and operator!= in imgui_extra_math.inl
    #  are already guarded by "#if IMGUI_VERSION_NUM < 19002", but
    #  operator*(float, ImVec2) is still unguarded — causing a redefinition
    #  error. This also causes a secondary "substitution failure" in the
    #  ImCubicBezierDt template (which relies on the operator).
    #  Fix: wrap operator*(float, ImVec2) with
    #       #ifndef IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED.
    # ------------------------------------------------------------------

    # Patch A — imgui_extra_math.inl
    # In the master branch, operator== and operator!= are already guarded by
    # "#if IMGUI_VERSION_NUM < 19002". The remaining problem is operator*(float, ImVec2)
    # at line 34, which has no guard. imgui_extra_math.h unconditionally sets
    # IMGUI_DEFINE_MATH_OPERATORS before including imgui.h, so imgui.h defines
    # operator*(float, ImVec2) AND sets IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED.
    # The unguarded definition in imgui_extra_math.inl is therefore a redefinition.
    # Fix: wrap it in #ifndef IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED.
    # Idempotency: skip if the guard string is already present.
    file(READ "${imguinodeeditor_SOURCE_DIR}/imgui_extra_math.inl" _emi)
    string(FIND "${_emi}" "IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED" _emi_done)
    if (_emi_done EQUAL -1)
        string(REPLACE
                "inline ImVec2 operator*(const float lhs, const ImVec2& rhs)"
                "# ifndef IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED\ninline ImVec2 operator*(const float lhs, const ImVec2& rhs)"
                _emi "${_emi}")
        string(REPLACE
                "# if IMGUI_VERSION_NUM < 18955"
                "# endif // IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED\n# if IMGUI_VERSION_NUM < 18955"
                _emi "${_emi}")
        file(WRITE "${imguinodeeditor_SOURCE_DIR}/imgui_extra_math.inl" "${_emi}")
    endif ()

    # Patch B — crude_json.cpp
    # crude_json.cpp uses std::terminate() but does not include <exception>.
    # In C++23 mode (Clang 21+), this is no longer transitively available.
    # Fix: add #include <exception> after the existing includes.
    # Idempotency: skip if already present.
    file(READ "${imguinodeeditor_SOURCE_DIR}/crude_json.cpp" _cj)
    string(FIND "${_cj}" "#include <exception>" _cj_done)
    if (_cj_done EQUAL -1)
        string(REPLACE
                "# include <cmath>"
                "# include <cmath>\n# include <exception>"
                _cj "${_cj}")
        file(WRITE "${imguinodeeditor_SOURCE_DIR}/crude_json.cpp" "${_cj}")
    endif ()

    add_library(ImGuiNodeEditor STATIC
            ${imguinodeeditor_SOURCE_DIR}/crude_json.cpp
            ${imguinodeeditor_SOURCE_DIR}/imgui_canvas.cpp
            ${imguinodeeditor_SOURCE_DIR}/imgui_node_editor.cpp
            ${imguinodeeditor_SOURCE_DIR}/imgui_node_editor_api.cpp
    )

    # PUBLIC so the include path (and imgui) propagates to every consumer
    target_include_directories(ImGuiNodeEditor
            PUBLIC ${imguinodeeditor_SOURCE_DIR}
    )

    target_link_libraries(ImGuiNodeEditor PUBLIC imgui)

    # _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING: crude_json.h uses
    # std::aligned_storage which is deprecated in C++23.
    # Note: IMGUI_DEFINE_MATH_OPERATORS is already set unconditionally by
    # imgui_extra_math.h so it does not need to be repeated here.
    target_compile_definitions(ImGuiNodeEditor PRIVATE
            _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
    )

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE ImGuiNodeEditor)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_NODE_EDITOR)
endif ()

# ---------------------------------------------------------------------------
# Extension: imnodes
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_IMNODES)
    FetchContent_Declare(
            imnodes
            GIT_REPOSITORY https://github.com/Nelarius/imnodes.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
    )

    FetchContent_GetProperties(imnodes)
    if (NOT imnodes_POPULATED)
        FetchContent_Populate(imnodes)
    endif ()

    add_library(imnodes STATIC
            ${imnodes_SOURCE_DIR}/imnodes.cpp
    )

    target_include_directories(imnodes
            PUBLIC ${imnodes_SOURCE_DIR}
    )

    target_link_libraries(imnodes PUBLIC imgui)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE imnodes)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_IMNODES)
endif ()

# ---------------------------------------------------------------------------
# Extension: ImNodeFlow
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_NODE_FLOW)
    FetchContent_Declare(
            ImNodeFlow
            GIT_REPOSITORY https://github.com/Fattorino/ImNodeFlow.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
    )

    FetchContent_GetProperties(ImNodeFlow)
    if (NOT imnodeflow_POPULATED)
        FetchContent_Populate(ImNodeFlow)
    endif ()

    # ------------------------------------------------------------------
    # Compatibility patch for ImNodeFlow (master) + imgui (docking).
    # ImNodeFlow bundles its own copy of imgui_extra_math.inl (version 0.9.1)
    # in src/, which unconditionally defines operator*(float, ImVec2).
    # imgui_extra_math.h (also bundled) sets IMGUI_DEFINE_MATH_OPERATORS
    # before including imgui.h, so imgui.h defines the operator first and
    # sets IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED.  The bare definition
    # in imgui_extra_math.inl is then a redefinition → compile error.
    # Fix: wrap operator*(float, ImVec2) with
    #      #ifndef IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED.
    # Idempotency: skip if the guard is already present.
    # ------------------------------------------------------------------
    file(READ "${imnodeflow_SOURCE_DIR}/src/imgui_extra_math.inl" _nf_emi)
    string(FIND "${_nf_emi}" "IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED" _nf_emi_done)
    if (_nf_emi_done EQUAL -1)
        string(REPLACE
                "inline ImVec2 operator*(const float lhs, const ImVec2& rhs)"
                "# ifndef IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED\ninline ImVec2 operator*(const float lhs, const ImVec2& rhs)"
                _nf_emi "${_nf_emi}")
        string(REPLACE
                "# if IMGUI_VERSION_NUM < 18955"
                "# endif // IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED\n# if IMGUI_VERSION_NUM < 18955"
                _nf_emi "${_nf_emi}")
        file(WRITE "${imnodeflow_SOURCE_DIR}/src/imgui_extra_math.inl" "${_nf_emi}")
    endif ()

    add_library(ImNodeFlow STATIC
            ${imnodeflow_SOURCE_DIR}/src/ImNodeFlow.cpp
    )

    target_include_directories(ImNodeFlow
            PUBLIC ${imnodeflow_SOURCE_DIR}/include
    )

    target_link_libraries(ImNodeFlow PUBLIC imgui)

    # ImNodeFlow.h includes <imgui.h> at line 15 *before* imgui_extra_math.h
    # (line 16) gets to define IMGUI_DEFINE_MATH_OPERATORS, so imgui.h is parsed
    # without it and imgui_internal.h fires its #error guard.
    # Fix: define IMGUI_DEFINE_MATH_OPERATORS as a compiler flag so it is
    # present before any header is touched.
    # PUBLIC so consumers (ImNodeFlowDemo via ImGuiEx) inherit the flag too.
    target_compile_definitions(ImNodeFlow PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE ImNodeFlow)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_NODE_FLOW)
endif ()

# ---------------------------------------------------------------------------
# Extension: ImPlot
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_IMPLOT)
    FetchContent_Declare(
            implot
            GIT_REPOSITORY https://github.com/epezent/implot.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
    )

    FetchContent_GetProperties(implot)
    if (NOT implot_POPULATED)
        FetchContent_Populate(implot)
    endif ()

    add_library(implot STATIC
            ${implot_SOURCE_DIR}/implot.cpp
            ${implot_SOURCE_DIR}/implot_items.cpp
            ${implot_SOURCE_DIR}/implot_demo.cpp
    )

    target_include_directories(implot
            PUBLIC ${implot_SOURCE_DIR}
    )

    target_link_libraries(implot PUBLIC imgui)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE implot)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_IMPLOT)
endif ()

# ---------------------------------------------------------------------------
# Extension: ImPlot3D
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_IMPLOT3D)
    FetchContent_Declare(
            implot3d
            GIT_REPOSITORY https://github.com/brenocq/implot3d.git
            GIT_TAG        main
            GIT_SHALLOW    TRUE
    )

    FetchContent_GetProperties(implot3d)
    if (NOT implot3d_POPULATED)
        FetchContent_Populate(implot3d)
    endif ()

    add_library(implot3d STATIC
            ${implot3d_SOURCE_DIR}/implot3d.cpp
            ${implot3d_SOURCE_DIR}/implot3d_items.cpp
            ${implot3d_SOURCE_DIR}/implot3d_meshes.cpp
            ${implot3d_SOURCE_DIR}/implot3d_demo.cpp
    )

    target_include_directories(implot3d
            PUBLIC ${implot3d_SOURCE_DIR}
    )

    target_link_libraries(implot3d PUBLIC imgui)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE implot3d)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_IMPLOT3D)
endif ()

# ---------------------------------------------------------------------------
# Extension: imgui-knobs
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_IMGUI_KNOBS)
    FetchContent_Declare(
            imguiknobs
            GIT_REPOSITORY https://github.com/altschuler/imgui-knobs.git
            GIT_TAG        main
            GIT_SHALLOW    TRUE
    )

    FetchContent_GetProperties(imguiknobs)
    if (NOT imguiknobs_POPULATED)
        FetchContent_Populate(imguiknobs)
    endif ()

    add_library(imguiknobs STATIC
            ${imguiknobs_SOURCE_DIR}/imgui-knobs.cpp
    )

    target_include_directories(imguiknobs
            PUBLIC ${imguiknobs_SOURCE_DIR}
    )

    target_link_libraries(imguiknobs PUBLIC imgui)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE imguiknobs)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_IMGUI_KNOBS)
endif ()

# ---------------------------------------------------------------------------
# Extension: imgui_toggle
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_IMGUI_TOGGLE)
    FetchContent_Declare(
            imgui_toggle
            GIT_REPOSITORY https://github.com/cmdwtf/imgui_toggle.git
            GIT_TAG        main
            GIT_SHALLOW    TRUE
    )

    FetchContent_GetProperties(imgui_toggle)
    if (NOT imgui_toggle_POPULATED)
        FetchContent_Populate(imgui_toggle)
    endif ()

    add_library(imgui_toggle STATIC
            ${imgui_toggle_SOURCE_DIR}/imgui_toggle.cpp
            ${imgui_toggle_SOURCE_DIR}/imgui_toggle_palette.cpp
            ${imgui_toggle_SOURCE_DIR}/imgui_toggle_presets.cpp
            ${imgui_toggle_SOURCE_DIR}/imgui_toggle_renderer.cpp
    )

    target_include_directories(imgui_toggle
            PUBLIC ${imgui_toggle_SOURCE_DIR}
    )

    target_link_libraries(imgui_toggle PUBLIC imgui)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE imgui_toggle)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_IMGUI_TOGGLE)
endif ()

# ---------------------------------------------------------------------------
# Extension: imspinner
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_IMSPINNER)
    FetchContent_Declare(
            imspinner
            GIT_REPOSITORY https://github.com/dalerank/imspinner.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
    )
    FetchContent_GetProperties(imspinner)
    if (NOT imspinner_POPULATED)
        FetchContent_Populate(imspinner)
    endif ()

    # imspinner is header-only — all functions are inline inside imspinner.h.
    add_library(imspinner INTERFACE)

    target_include_directories(imspinner
            INTERFACE ${imspinner_SOURCE_DIR}
    )

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE imspinner)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_IMSPINNER)
endif ()

# ---------------------------------------------------------------------------
# Extension: ImGuiDatePicker
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_DATE_PICKER)
    FetchContent_Declare(
            ImGuiDatePicker
            GIT_REPOSITORY https://github.com/DnA-IntRicate/ImGuiDatePicker.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
    )

    FetchContent_GetProperties(ImGuiDatePicker)
    if (NOT imguidatepicker_POPULATED)
        FetchContent_Populate(ImGuiDatePicker)
    endif ()

    # ------------------------------------------------------------------
    # Compatibility patch: ImGui 1.92 changed PushFont(ImFont*) to
    # PushFont(ImFont*, float font_size_base_unscaled).
    # Pass 0.0f as the size to keep the current font size unchanged.
    # Idempotency: skip if the patched signature is already present.
    # ------------------------------------------------------------------
    file(READ "${imguidatepicker_SOURCE_DIR}/ImGuiDatePicker.cpp" _dp_cpp)
    string(FIND "${_dp_cpp}" "PushFont(altFont, 0.0f)" _dp_done)
    if (_dp_done EQUAL -1)
        string(REPLACE
            "PushFont(altFont);"
            "PushFont(altFont, 0.0f);"
            _dp_cpp "${_dp_cpp}")
        file(WRITE "${imguidatepicker_SOURCE_DIR}/ImGuiDatePicker.cpp" "${_dp_cpp}")
    endif ()

    add_library(ImGuiDatePicker STATIC
            ${imguidatepicker_SOURCE_DIR}/ImGuiDatePicker.cpp
    )

    target_include_directories(ImGuiDatePicker
            PUBLIC ${imguidatepicker_SOURCE_DIR}
    )

    target_link_libraries(ImGuiDatePicker PUBLIC imgui)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE ImGuiDatePicker)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_DATE_PICKER)
endif ()

# ---------------------------------------------------------------------------
# Extension: imgui_keyboard
# ---------------------------------------------------------------------------
if (IMGUIEX_ENABLE_IMGUI_KEYBOARD)
    FetchContent_Declare(
            imgui_keyboard
            GIT_REPOSITORY https://github.com/mgerhardy/imgui_keyboard.git
            GIT_TAG        main
            GIT_SHALLOW    TRUE
    )

    FetchContent_GetProperties(imgui_keyboard)
    if (NOT imgui_keyboard_POPULATED)
        FetchContent_Populate(imgui_keyboard)
    endif ()

    add_library(imgui_keyboard STATIC
            ${imgui_keyboard_SOURCE_DIR}/imgui_keyboard.cpp
    )

    target_include_directories(imgui_keyboard
            PUBLIC ${imgui_keyboard_SOURCE_DIR}
    )

    target_link_libraries(imgui_keyboard PUBLIC imgui)

    # Wire into the ImGuiEx aggregator
    target_link_libraries(ImGuiEx INTERFACE imgui_keyboard)
    target_compile_definitions(ImGuiEx INTERFACE IMGUIEX_ENABLE_IMGUI_KEYBOARD)
endif ()

# ---------------------------------------------------------------------------
# Configuration Summary
# ---------------------------------------------------------------------------
message(STATUS "ImGuiEx Configuration:")
message(STATUS "  FreeType Support: ${IMGUIEX_ENABLE_FREETYPE}")

if (IMGUIEX_ENABLE_COLOR_TEXT_EDIT)
    message(STATUS "  ✓ ImGuiColorTextEdit")
endif ()

if (IMGUIEX_ENABLE_NODE_EDITOR)
    message(STATUS "  ✓ imgui-node-editor")
endif ()

if (IMGUIEX_ENABLE_IMNODES)
    message(STATUS "  ✓ imnodes")
endif ()

if (IMGUIEX_ENABLE_NODE_FLOW)
    message(STATUS "  ✓ ImNodeFlow")
endif ()

if (IMGUIEX_ENABLE_IMPLOT)
    message(STATUS "  ✓ ImPlot")
endif ()

if (IMGUIEX_ENABLE_IMPLOT3D)
    message(STATUS "  ✓ ImPlot3D")
endif ()

if (IMGUIEX_ENABLE_IMGUI_KNOBS)
    message(STATUS "  ✓ imgui-knobs")
endif ()

if (IMGUIEX_ENABLE_IMSPINNER)
    message(STATUS "  ✓ imspinner")
endif ()

if (IMGUIEX_ENABLE_IMGUI_TOGGLE)
    message(STATUS "  ✓ imgui_toggle")
endif ()

if (IMGUIEX_ENABLE_SPLITTER)
    message(STATUS "  ✓ Splitter (built-in)")
endif ()

if (IMGUIEX_ENABLE_STACK_LAYOUT)
    message(STATUS "  ✓ Stack Layout (PR #846)")
endif ()

if (IMGUIEX_ENABLE_DATE_PICKER)
    message(STATUS "  ✓ ImGuiDatePicker")
endif ()

if (IMGUIEX_ENABLE_IMGUI_KEYBOARD)
    message(STATUS "  ✓ imgui_keyboard")
endif ()

# ---------------------------------------------------------------------------
# Demo
# ---------------------------------------------------------------------------
if (PROJECT_IS_TOP_LEVEL)

    # ---------------------------------------------------------------------------
    # GLFW (windowing library — also used by ImGui's GLFW backend)
    # ---------------------------------------------------------------------------
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
            GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glfw)

    add_subdirectory(demo)
endif ()
