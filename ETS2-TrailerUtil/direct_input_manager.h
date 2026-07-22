#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "Dinput8.lib")
#pragma comment(lib, "Dxguid.lib")

inline HWND hwndFromProcessId(DWORD pid)
{
    struct HwndData { DWORD pid; HWND hwnd = nullptr; } data{ pid };

    auto callback = [](HWND h, LPARAM lParam) -> BOOL {
        auto* pData = reinterpret_cast<HwndData*>(lParam);
        DWORD wndPid = 0;
        GetWindowThreadProcessId(h, &wndPid);

        if (wndPid == pData->pid && GetWindow(h, GW_OWNER) == NULL && IsWindowVisible(h)) {
            pData->hwnd = h;
            return FALSE;
        }

        return TRUE;
        };

    EnumWindows(callback, reinterpret_cast<LPARAM>(&data));
    return data.hwnd;
}


namespace dinput_manager
{
    struct InputDevice
    {
        LPDIRECTINPUTDEVICE8 device;
        DIDEVICEINSTANCE info;
    };

    LPDIRECTINPUT8 pDirectInput = nullptr;
    std::vector<InputDevice> inputDevices;

    const std::vector<InputDevice>& GetInputDevices()
    {
        return inputDevices;
    }

    BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
    {
        LPDIRECTINPUTDEVICE8 newDevice = nullptr;

        if (SUCCEEDED(pDirectInput->CreateDevice(pdidInstance->guidInstance, &newDevice, nullptr)))
        {
            if (SUCCEEDED(newDevice->SetDataFormat(&c_dfDIJoystick2)) &&
                SUCCEEDED(newDevice->SetCooperativeLevel((HWND)pContext, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
            {
                newDevice->Acquire();
                inputDevices.push_back({ newDevice, *pdidInstance });
            }
            else
            {
                newDevice->Release();
            }
        }

        return DIENUM_CONTINUE;
    }


    void InitDirectInput()
    {
        DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&pDirectInput, nullptr);
        inputDevices.clear();

        DWORD pid = GetCurrentProcessId();
        HWND hwnd = hwndFromProcessId(pid);
        pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, hwnd, DIEDFL_ATTACHEDONLY);
    }


    void ShutdownDirectInput()
    {
        // Release all devices
        for (auto& device : inputDevices)
        {
            if (device.device)
            {
                device.device->Unacquire();
                device.device->Release();
                device.device = nullptr;
            }
        }
        inputDevices.clear();

        // Release DirectInput interface
        if (pDirectInput)
        {
            pDirectInput->Release();
            pDirectInput = nullptr;
        }
    }
}