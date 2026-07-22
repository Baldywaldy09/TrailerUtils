#pragma once
#include "kiero/kiero.h"

# include "imgui/impl/d3d11_impl.h"
# include "imgui/impl/d3d12_impl.h"
# include "imgui/impl/vulkan_impl.h"
# include "imgui/impl/opengl_impl.h"
# include "imgui/impl/dinput_impl.h"

#include <Windows.h>
#include <string>

class RenderManager
{
private:
    kiero::RenderType::Enum render_device;
    bool menu_visible = false;
    bool mouse_visible = false;

    kiero::RenderType::Enum DetectRenderDevice();

public:
    RenderManager(kiero::RenderType::Enum rendering_api = kiero::RenderType::Auto);
    ~RenderManager();

    kiero::RenderType::Enum GetRenderDevice() { return render_device; }

    void toggle_menu();
    void set_menu_state(bool state);

    void toggle_mouse();
    void set_mouse_state(bool state);
};
