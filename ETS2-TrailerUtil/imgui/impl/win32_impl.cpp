#include "../../kiero/kiero.h"
#include "win32_impl.h"
#include <Windows.h>
#include "../imgui.h"
#include "../imgui_impl_win32.h"

static WNDPROC oWndProc = NULL;
static HWND new_hwnd = NULL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK hkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return TRUE; // handled by ImGui

    return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}

void impl::win32::init(void* hwnd)
{
    new_hwnd = (HWND)hwnd;
    oWndProc = (WNDPROC)SetWindowLongPtr(new_hwnd, GWLP_WNDPROC, (LONG_PTR)hkWindowProc);
}

void impl::win32::shutdown()
{
    if (oWndProc) SetWindowLongPtr(new_hwnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
}
