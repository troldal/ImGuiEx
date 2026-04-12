// ImGuiEx demo — Win32 + DirectX 11
// Shows the standard ImGui demo window and, when built with
// -DENABLE_COLOR_TEXT_EDIT=ON, an ImGuiColorTextEdit demo window.

#include "ImGuiEx.h"
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

// ---- ColorTextEdit demo ------------------------------------------------
#ifdef IMGUIEX_ENABLE_COLOR_TEXT_EDIT
static void ShowColorTextEditDemo()
{
    static TextEditor editor = []()
    {
        TextEditor ed;
        ed.SetLanguage(TextEditor::Language::Cpp());
        ed.SetText(
            "// ImGuiColorTextEdit demo\n"
            "#include <iostream>\n"
            "\n"
            "int main()\n"
            "{\n"
            "    // Edit me!\n"
            "    std::cout << \"Hello, ImGuiEx!\\n\";\n"
            "    return 0;\n"
            "}\n"
        );
        return ed;
    }();

    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("ColorTextEdit Demo",
                 nullptr,
                 ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Edit"))
        {
            bool ro = editor.IsReadOnlyEnabled();
            if (ImGui::MenuItem("Read-only", nullptr, &ro))
                editor.SetReadOnlyEnabled(ro);
            ImGui::Separator();
            if (ImGui::MenuItem("Undo", "Ctrl-Z", nullptr, editor.CanUndo()))
                editor.Undo();
            if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, editor.CanRedo()))
                editor.Redo();
            ImGui::Separator();
            if (ImGui::MenuItem("Copy",  "Ctrl-C", nullptr, editor.AnyCursorHasSelection()))
                editor.Copy();
            if (ImGui::MenuItem("Cut",   "Ctrl-X", nullptr, !ro && editor.AnyCursorHasSelection()))
                editor.Cut();
            if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro))
                editor.Paste();
            ImGui::Separator();
            if (ImGui::MenuItem("Select all"))
                editor.SelectAll();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Dark palette"))
                editor.SetPalette(TextEditor::GetDarkPalette());
            if (ImGui::MenuItem("Light palette"))
                editor.SetPalette(TextEditor::GetLightPalette());
            if (ImGui::MenuItem("Default palette"))
                editor.SetPalette(TextEditor::GetDefaultPalette());
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    auto cpos = editor.GetMainCursorPosition();
    ImGui::Text("Ln %d, Col %-3d | %s | %s | %s",
                cpos.line + 1, cpos.column + 1,
                editor.IsOverwriteEnabled() ? "Ovr" : "Ins",
                editor.CanUndo()            ? "*"   : " ",
                editor.GetLanguageName().c_str());

    editor.Render("##code");

    ImGui::End();
}
#endif // IMGUIEX_ENABLE_COLOR_TEXT_EDIT

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
    bool showDemoWindow       = true;
    bool showColorTextEdit    = true;
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

        if (g_SwapChainOccluded &&
            g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight,
                                        DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&showDemoWindow);

#ifdef IMGUIEX_ENABLE_COLOR_TEXT_EDIT
        if (showColorTextEdit)
            ShowColorTextEditDemo();
#else
        (void)showColorTextEdit;
#endif

        ImGui::Render();

        const float cc[4] = {
            clearColor.x * clearColor.w,
            clearColor.y * clearColor.w,
            clearColor.z * clearColor.w,
            clearColor.w
        };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, cc);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = g_pSwapChain->Present(1, 0);
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

