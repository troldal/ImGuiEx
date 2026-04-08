// ImGuiEx demo — Win32 + DirectX 11
// Shows the standard ImGui demo window and the ImGuiEx layout helper demo.

#include "ImGuiEx.h"
#include "imgui_layout.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>
#include <tchar.h>

// ---- D3D11 state -------------------------------------------------------
static ID3D11Device*            g_pd3dDevice           = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext    = nullptr;
static IDXGISwapChain*          g_pSwapChain           = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
static bool                     g_SwapChainOccluded    = false;
static UINT                     g_ResizeWidth          = 0;
static UINT                     g_ResizeHeight         = 0;

// ---- Forward declarations ----------------------------------------------
bool    CreateDeviceD3D(HWND hWnd);
void    CleanupDeviceD3D();
void    CreateRenderTarget();
void    CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ---- Layout demo -------------------------------------------------------
static void ShowLayoutDemo()
{
    ImGui::SetNextWindowSize(ImVec2(820, 620), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("ImGuiEx Layout Demo"))
    {
        ImGui::End();
        return;
    }

    if (!ImGui::BeginTabBar("##layout_tabs"))
    {
        ImGui::End();
        return;
    }

    // ---- 1. HStackScope ------------------------------------------------
    if (ImGui::BeginTabItem("HStackScope"))
    {
        ImGui::TextDisabled("item() calls SameLine(0, spacing) before every widget except the first.");
        ImGui::Separator();

        ImGui::Text("Toolbar — default 4 px spacing:");
        {
            ImGui::HStackScope hs;
            hs.item(); ImGui::SmallButton("New");
            hs.item(); ImGui::SmallButton("Open");
            hs.item(); ImGui::SmallButton("Save");
            hs.item(); ImGui::Separator();   // separators are items too
            hs.item(); ImGui::SmallButton("Undo");
            hs.item(); ImGui::SmallButton("Redo");
        }

        ImGui::Spacing();
        ImGui::Text("Wide spacing (20 px):");
        {
            ImGui::HStackScope hs(20.0f);
            hs.item(); ImGui::Button("Alpha");
            hs.item(); ImGui::Button("Beta");
            hs.item(); ImGui::Button("Gamma");
        }

        ImGui::Spacing();
        ImGui::Text("Label + input + button on one line:");
        {
            static char buf[64] = "hello";
            ImGui::HStackScope hs(8.0f);
            hs.item(); ImGui::Text("Name");
            hs.item(); ImGui::SetNextItemWidth(200);
                       ImGui::InputText("##name", buf, sizeof(buf));
            hs.item(); ImGui::Button("Search");
        }

        ImGui::EndTabItem();
    }

    // ---- 2. Springs & Spacer -------------------------------------------
    if (ImGui::BeginTabItem("Springs & Spacer"))
    {
        ImGui::TextDisabled("SpringH — pushes subsequent widgets to the right edge.");
        // Tab bar with a close button flush-right
        ImGui::BeginChild("##springh_demo", ImVec2(0, 32), ImGuiChildFlags_Borders);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("scene.json");
        ImGui::SameLine();
        ImGui::SpringH();
        ImGui::SmallButton("x");
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::TextDisabled("SpringV — pushes subsequent widgets to the bottom of the region.");
        // Status panel with footer pinned to bottom
        ImGui::BeginChild("##springv_demo", ImVec2(0, 110), ImGuiChildFlags_Borders);
        ImGui::Text("Status:  Running");
        ImGui::Text("Frames:  12 843");
        ImGui::Text("GPU:     4.2 ms");
        ImGui::SpringV();
        ImGui::Separator();
        if (ImGui::SmallButton("Stop")) {}
        ImGui::SameLine();
        if (ImGui::SmallButton("Pause")) {}
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::TextDisabled("Spacer — fixed-size invisible gap.");
        ImGui::Button("A");
        ImGui::SameLine(); ImGui::Spacer(32.0f); ImGui::SameLine();
        ImGui::Button("B  (+ 32 px horizontal gap)");
        ImGui::Button("C");
        ImGui::Spacer(0.0f, 18.0f);
        ImGui::Button("D  (+ 18 px vertical gap above)");

        ImGui::EndTabItem();
    }

    // ---- 3. SplitH -----------------------------------------------------
    if (ImGui::BeginTabItem("SplitH"))
    {
        ImGui::TextDisabled("30 / 70 split — sidebar + main content.");
        ImGui::Spacing();

        static int selected = 0;
        const char* items[] = { "Scenes", "Assets", "Shaders", "Textures", "Settings" };

        ImGui::SplitH(0.30f,
            [&]()   // left: sidebar
            {
                ImGui::TextColored({ 1, 0.75f, 0.3f, 1 }, "Sidebar  (30%%)");
                ImGui::Separator();
                for (int i = 0; i < IM_ARRAYSIZE(items); ++i)
                    if (ImGui::Selectable(items[i], selected == i))
                        selected = i;
            },
            [&]()   // right: content
            {
                ImGui::TextColored({ 0.4f, 0.9f, 1, 1 }, "Content  (70%% \xe2\x80\x93 spacing)");
                ImGui::Separator();
                ImGui::TextWrapped("Selected: \"%s\"\n\n"
                    "This pane uses ImVec2(0,0) to fill all remaining "
                    "horizontal space after the left child and the spacing gap.",
                    items[selected]);
            });

        ImGui::EndTabItem();
    }

    // ---- 4. SplitV -----------------------------------------------------
    if (ImGui::BeginTabItem("SplitV"))
    {
        ImGui::TextDisabled("70 / 30 split — viewport on top, log panel below.");
        ImGui::Spacing();

        static const char* logLines[] = {
            "[info]  Scene loaded",
            "[info]  Renderer initialised",
            "[warn]  Missing texture: diffuse.png",
            "[error] Shader compile failed: bloom.hlsl:14",
            "[info]  Fallback shader applied",
        };

        ImGui::SplitV(0.70f,
            [&]()   // top: viewport stand-in
            {
                ImGui::TextColored({ 1, 0.75f, 0.3f, 1 }, "Viewport  (70%%)");
                ImGui::Separator();
                ImVec2 sz = ImGui::GetContentRegionAvail();
                ImGui::Dummy(ImVec2(sz.x, sz.y - ImGui::GetTextLineHeightWithSpacing()));
                ImGui::Text("[ 3D view would render here ]");
            },
            [&]()   // bottom: log panel
            {
                ImGui::TextColored({ 0.4f, 0.9f, 1, 1 }, "Log  (30%% \xe2\x80\x93 spacing)");
                ImGui::Separator();
                for (const char* line : logLines)
                {
                    if (line[1] == 'w') ImGui::PushStyleColor(ImGuiCol_Text, { 1, 0.85f, 0.2f, 1 });
                    else if (line[1] == 'e') ImGui::PushStyleColor(ImGuiCol_Text, { 1, 0.4f, 0.4f, 1 });
                    else ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
                    ImGui::TextUnformatted(line);
                    ImGui::PopStyleColor();
                }
            });

        ImGui::EndTabItem();
    }

    // ---- 5. Padded -----------------------------------------------------
    if (ImGui::BeginTabItem("Padded"))
    {
        ImGui::TextDisabled("Padded: child sized avail - padding*2; inner cursor at (padding, padding).");
        ImGui::Spacing();

        static float pad = 16.0f;
        ImGui::SetNextItemWidth(200);
        ImGui::SliderFloat("padding", &pad, 4.0f, 40.0f, "%.0f px");
        ImGui::Spacing();

        ImGui::Padded(pad, [&]()
        {
            ImGui::TextColored({ 1, 0.75f, 0.3f, 1 }, "Padded region");
            ImGui::Text("Inner cursor starts at (%.0f, %.0f).", pad, pad);
            ImGui::Text("Child is avail - %.0f wide / tall.", pad * 2.0f);
            ImGui::Separator();
            static char field[64] = "editable";
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##field", field, sizeof(field));
            ImGui::Spacing();
            ImGui::Button("Button inside the padded region");
        });

        ImGui::EndTabItem();
    }

    // ---- 6. Columns + NextColumn ---------------------------------------
    if (ImGui::BeginTabItem("Columns"))
    {
        ImGui::TextDisabled("Columns: BeginTable with N equally stretched columns + optional flags.");
        ImGui::Spacing();

        struct Unit { const char* name; const char* type; int hp; float spd; };
        constexpr Unit units[] = {
            { "Knight",  "Melee",  120, 2.5f },
            { "Archer",  "Ranged",  80, 4.0f },
            { "Mage",    "Caster",  60, 3.0f },
            { "Paladin", "Melee",  150, 2.0f },
            { "Rogue",   "Melee",   70, 5.5f },
        };

        ImGui::Columns(4, [&]()
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("HP");
            ImGui::TableSetupColumn("Speed");
            ImGui::TableHeadersRow();

            for (const auto& u : units)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%s", u.name);
                ImGui::TableNextColumn(); ImGui::TextDisabled("%s", u.type);
                ImGui::TableNextColumn(); ImGui::Text("%d", u.hp);
                ImGui::TableNextColumn(); ImGui::Text("%.1f", u.spd);
            }
        },
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY);

        ImGui::EndTabItem();
    }

    // ---- 7. Group ------------------------------------------------------
    if (ImGui::BeginTabItem("Group"))
    {
        ImGui::TextDisabled("Group: bordered child or BeginGroup/EndGroup, with optional SeparatorText label.");
        ImGui::Spacing();

        ImGui::SplitH(0.5f,
            [&]()   // left column
            {
                // border=true (default), visible label
                ImGui::Group("Render Settings", [&]()
                {
                    static int   msaa  = 2;
                    static bool  vsync = true;
                    static float gamma = 2.2f;
                    ImGui::Combo("MSAA", &msaa, "Off\0x2\0x4\0x8\0");
                    ImGui::Checkbox("VSync", &vsync);
                    ImGui::SetNextItemWidth(-1);
                    ImGui::SliderFloat("##gamma", &gamma, 1.0f, 3.0f, "Gamma %.2f");
                });

                ImGui::Spacing();

                // border=true, label hidden (starts with ##)
                ImGui::TextDisabled("border=true, label hidden (##…):");
                ImGui::Group("##audio_panel", [&]()
                {
                    static float vol = 0.8f, music = 0.5f;
                    ImGui::Text("Audio");   // manual heading
                    ImGui::SetNextItemWidth(-1); ImGui::SliderFloat("##vol",   &vol,   0, 1, "Master %.0f%%");
                    ImGui::SetNextItemWidth(-1); ImGui::SliderFloat("##music", &music, 0, 1, "Music  %.0f%%");
                });
            },
            [&]()   // right column
            {
                // border=false, visible label
                ImGui::TextDisabled("border=false, label shown:");
                ImGui::Group("Build Info", [&]()
                {
                    ImGui::BulletText("Uses BeginGroup / EndGroup");
                    ImGui::BulletText("Label rendered as SeparatorText");
                    ImGui::BulletText("No child window created");
                }, false);

                ImGui::Spacing();

                // border=false, label hidden
                ImGui::TextDisabled("border=false, label hidden (##…):");
                ImGui::Group("##plain", [&]()
                {
                    ImGui::BulletText("No separator, no border");
                    ImGui::BulletText("Pure BeginGroup/EndGroup");
                }, false);
            });

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();
}

// ---- Entry point -------------------------------------------------------
int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
                   LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    WNDCLASSEXW wc = {
        sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        L"ImGuiExDemo", nullptr
    };
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowW(
        wc.lpszClassName, L"ImGuiEx Demo",
        WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // ---- ImGui setup ---------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // ---- Main loop -----------------------------------------------------
    const ImVec4 clearColor = { 0.45f, 0.55f, 0.60f, 1.00f };
    bool showDemoWindow  = true;
    bool showLayoutDemo  = true;
    bool done = false;

    while (!done)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        // Stall when occluded (e.g. screen locked)
        if (g_SwapChainOccluded &&
            g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Resize swap chain if the window was resized
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight,
                                        DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // ---- New ImGui frame -------------------------------------------
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&showDemoWindow);
        if (showLayoutDemo) ShowLayoutDemo();

        ImGui::Render();

        // ---- Render ----------------------------------------------------
        const float cc[4] = {
            clearColor.x * clearColor.w,
            clearColor.y * clearColor.w,
            clearColor.z * clearColor.w,
            clearColor.w
        };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, cc);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = g_pSwapChain->Present(1, 0); // vsync on
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // ---- Shutdown ------------------------------------------------------
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// ---- D3D11 helpers -----------------------------------------------------
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd          = {};
    sd.BufferCount                   = 2;
    sd.BufferDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                         = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                   = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                  = hWnd;
    sd.SampleDesc.Count              = 1;
    sd.Windowed                      = TRUE;
    sd.SwapEffect                    = DXGI_SWAP_EFFECT_DISCARD;

    constexpr D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL featureLevel;

    // Try hardware device first, fall back to WARP
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevels, 2, D3D11_SDK_VERSION,
        &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            featureLevels, 2, D3D11_SDK_VERSION,
            &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

    if (FAILED(res))
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain)          { g_pSwapChain->Release();          g_pSwapChain          = nullptr; }
    if (g_pd3dDeviceContext)   { g_pd3dDeviceContext->Release();   g_pd3dDeviceContext   = nullptr; }
    if (g_pd3dDevice)          { g_pd3dDevice->Release();          g_pd3dDevice          = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// ---- Window procedure --------------------------------------------------
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            g_ResizeWidth  = LOWORD(lParam);
            g_ResizeHeight = HIWORD(lParam);
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_KEYMENU) // suppress Alt menu
            return 0;
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

