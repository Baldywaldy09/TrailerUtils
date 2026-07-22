#include "rendering.h"
#include "imgui/impl/win32_impl.h"

kiero::RenderType::Enum RenderManager::DetectRenderDevice()
{
    HMODULE TruckersMP = GetModuleHandleA("core_ets2mp.dll");
    if (TruckersMP) {
        return kiero::RenderType::D3D11;
    }

    HMODULE vulkan = GetModuleHandleA("vulkan-1.dll");
    if (vulkan) {
        return kiero::RenderType::Vulkan;
    }

    HMODULE opengl = GetModuleHandleA("opengl32.dll");
    if (opengl) {
        return kiero::RenderType::OpenGL;
    }

    HMODULE d3d12core = GetModuleHandleA("d3d12core.dll");
    if (d3d12core) {
        return kiero::RenderType::D3D12;
    }

    return kiero::RenderType::D3D11;
}


RenderManager::RenderManager(kiero::RenderType::Enum rendering_api)
{
    if (rendering_api == kiero::RenderType::Auto)
        render_device = DetectRenderDevice();
    else
        render_device = rendering_api;

    kiero::init(render_device);

    switch (render_device)
    {
        case kiero::RenderType::D3D11: impl::d3d11::init(); break;
        case kiero::RenderType::D3D12: impl::d3d12::init(); break;
        case kiero::RenderType::OpenGL: impl::opengl::init(); break;
        case kiero::RenderType::Vulkan: impl::vulkan::init(); break;
    }

    impl::dinput::init();
}

RenderManager::~RenderManager()
{
    switch (render_device)
    {
        case kiero::RenderType::D3D11: impl::d3d11::shutdown(); break;
        case kiero::RenderType::D3D12: impl::d3d12::shutdown(); break;
        case kiero::RenderType::OpenGL: impl::opengl::shutdown(); break;
        case kiero::RenderType::Vulkan: impl::vulkan::shutdown(); break;
    }

    impl::win32::shutdown();

    kiero::shutdown();
}

void RenderManager::toggle_menu()
{
    menu_visible = !menu_visible;
    impl::set_menu_state(menu_visible);
}

void RenderManager::set_menu_state(bool state) { impl::set_menu_state(state); }

void RenderManager::toggle_mouse()
{
    mouse_visible = !mouse_visible;
    impl::set_mouse_state(mouse_visible);
}

void RenderManager::set_mouse_state(bool state) { impl::set_mouse_state(state); }