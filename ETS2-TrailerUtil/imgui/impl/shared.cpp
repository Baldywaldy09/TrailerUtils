#define _CRT_SECURE_NO_WARNINGS

#include "shared.h"
#include <Windows.h>
#include <stdio.h>
#include "../imgui.h"
#include "dinput_impl.h"
#include "../../dllmain.h"

bool show_menu = false;
bool mouse_shown = false;

bool mousekey_down = false;
bool renderkey_down = false;

void impl::set_menu_state(bool state)
{
    show_menu = state;
}

void impl::set_mouse_state(bool state)
{
    mouse_shown = state;

    if (!state) {
        impl::dinput::hide_mouse();
    }
    else {
        impl::dinput::show_mouse();
    }
}

void impl::tick()
{
    // Wait for key input
    if (GetAsyncKeyState(VK_INSERT) & 0x8000)
    {
        if (!renderkey_down)
        {
            set_menu_state(!show_menu);
            set_mouse_state(show_menu);
        }
        renderkey_down = true;
    }
    else renderkey_down = false;

    if (show_menu && GetAsyncKeyState(VK_HOME) & 0x8000)
    {
        if (!mousekey_down)
        {
            set_mouse_state(!mouse_shown);
        }
        mousekey_down = true;
    }
    else mousekey_down = false;

    dllmain::main_tick();
}

void impl::render()
{
	if (!show_menu)
		return;


    dllmain::render_tick();
}