#include "../../kiero/kiero.h"

#if KIERO_INCLUDE_D3D11

#include "d3d11_impl.h"
#include <d3d11.h>
#include <assert.h>

#include "win32_impl.h"

#include "../imgui.h"
#include "../imgui_impl_win32.h"
#include "../imgui_impl_dx11.h"

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

static bool init = false;
static ID3D11RenderTargetView* mainRenderTargetView = nullptr;
static ID3D11Device* device = nullptr;
static ID3D11DeviceContext* context = nullptr;
long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!init)
    {
        DXGI_SWAP_CHAIN_DESC desc;
        pSwapChain->GetDesc(&desc);

        pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);
        device->GetImmediateContext(&context);

        // Create render target view
        ID3D11Texture2D* pBackBuffer = nullptr;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
        device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
        pBackBuffer->Release();

        impl::win32::init(desc.OutputWindow);

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(desc.OutputWindow);
        ImGui_ImplDX11_Init(device, context);

        ImGuiIO& io = ImGui::GetIO();

        ImFont* defaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 15.0f);
        io.Fonts->Build();
        io.FontDefault = defaultFont;

        init = true;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    impl::tick();
    impl::render();

    ImGui::EndFrame();
    ImGui::Render();

    // Bind render target before drawing ImGui
    context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

void impl::d3d11::init()
{
	kiero::bind(8, (void**)&oPresent, hkPresent11);
}

void impl::d3d11::shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();

    if (mainRenderTargetView) mainRenderTargetView->Release();
    if (context) context->Release();
    if (device) device->Release();

    kiero::unbind(8);
}

#endif // KIERO_INCLUDE_D3D11