#pragma once
#include <windows.h>
#include <iostream>
#include <fstream>

#include "dinput_impl.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "Dinput8.lib")
#pragma comment(lib, "Dxguid.lib")

#include "MinHook/MinHook.h"
#include "../imgui.h"

typedef HRESULT(__stdcall* GetDeviceDataT)(IDirectInputDevice8*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
GetDeviceDataT pGetDeviceData = nullptr;

typedef BOOL(WINAPI* hk_SetCursorPos)(int, int);
hk_SetCursorPos origSetCursorPos = NULL;

bool showMouse = false;

// Get the func from a vTable
uintptr_t getDIDFunctionAddress(LPDIRECTINPUTDEVICE8 lpdid, int index)
{
    uintptr_t avTable = *(uintptr_t*)(lpdid);
    uintptr_t pFunction = avTable + index * sizeof(uintptr_t);
    uintptr_t aFunction = *(uintptr_t*)(pFunction);

    return aFunction;
}

HRESULT __stdcall hookGetDeviceData(IDirectInputDevice8* pThis, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
    HRESULT result = pGetDeviceData(pThis, cbObjectData, rgdod, pdwInOut, dwFlags);

    if (result == DI_OK) {
        if (showMouse) {
            *pdwInOut = 0;
        }
    }

    return result;
}

HMODULE GetCurrentModule()
{
    HMODULE hModule = nullptr;
    // Use address of a function or variable in your DLL as reference
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCTSTR)GetCurrentModule, &hModule);
    return hModule;
}

BOOL WINAPI HOOK_SetCursorPos(int X, int Y) {
    if (showMouse) return true; // Stop windows from resetting the mouse position

    return origSetCursorPos(X, Y);
}


namespace impl
{
    namespace dinput {
        void hide_mouse()
        {
            showMouse = false;

            ImGuiIO* io = &ImGui::GetIO();
            if (io) io->MouseDrawCursor = showMouse;
        }

        void show_mouse()
        {
            showMouse = true;

            ImGuiIO* io = &ImGui::GetIO();
            if (io) io->MouseDrawCursor = showMouse;
        }

        void init()
        {
            // Create a dummy mouse, this will be used to get the address for the GetDeviceData function
            // This function is used to get data for devices so if we want to stop the mouse we stop the data

            // Create
            IDirectInput8* pDirectInput = nullptr;
            DirectInput8Create(GetCurrentModule(), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&pDirectInput, NULL);

            LPDIRECTINPUTDEVICE8 lpdiMouse;
            pDirectInput->CreateDevice(GUID_SysMouse, &lpdiMouse, NULL);

            // Hook the get device data function
            LPVOID GetDeviceData = reinterpret_cast<LPVOID>(getDIDFunctionAddress(lpdiMouse, 10));
            MH_CreateHook(GetDeviceData, hookGetDeviceData, (LPVOID*)&pGetDeviceData);
            MH_EnableHook(GetDeviceData);

            // Now that we have the correct function location we no longer need Direct Input 8 and its devices so we kill it
            lpdiMouse->Release();
            pDirectInput->Release();


            MH_CreateHookApi(L"user32", "SetCursorPos", &HOOK_SetCursorPos, reinterpret_cast<LPVOID*>(&origSetCursorPos));
            MH_EnableHook(MH_ALL_HOOKS);
        }
    }
}