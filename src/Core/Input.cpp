//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "pch.hpp"
#include "Input.hpp"

#include <Utils/Assert.hpp>

#define _GAMING_DESKTOP

#ifdef _GAMING_DESKTOP

// I can't find the GameInput.h header in the GDK for Desktop yet
#include <Xinput.h>
#pragma comment(lib, "xinput9_1_0.lib")

#define USE_KEYBOARD_MOUSE
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#else

// This is what we should use on *all* platforms, but see previous comment
#include <GameInput.h>

// This should be handled by GameInput.h, but we'll borrow values from XINPUT.
#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  (7849.0f / 32768.0f)
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE (8689.0f / 32768.0f)

#endif

namespace
{
    HWND hwnd;

    bool s_Buttons[2][(uint8_t)Darius::InputManager::DigitalInput::NumDigitalInputs];
    float s_HoldDuration[(uint8_t)Darius::InputManager::DigitalInput::NumDigitalInputs] = {0.0f};
    float s_Analogs[(uint8_t)Darius::InputManager::DigitalInput::NumDigitalInputs];
    float s_AnalogsTC[(uint8_t)Darius::InputManager::DigitalInput::NumDigitalInputs];

#ifdef USE_KEYBOARD_MOUSE

    IDirectInput8A* s_DI;
    IDirectInputDevice8A* s_Keyboard;
    IDirectInputDevice8A* s_Mouse;

    DIMOUSESTATE2 s_MouseState;
    unsigned char s_Keybuffer[256];
    unsigned char s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::NumKeys]; // map DigitalInput enum to DX key codes 

#endif

    inline float FilterAnalogInput(int val, int deadZone)
    {
        if(val < 0)
        {
            if(val > -deadZone)
                return 0.0f;
            else
                return (val + deadZone) / (32768.0f - deadZone);
        }
        else
        {
            if(val < deadZone)
                return 0.0f;
            else
                return (val - deadZone) / (32767.0f - deadZone);
        }
    }

#ifdef USE_KEYBOARD_MOUSE
    void KbmBuildKeyMapping()
    {
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyEscape] = 1;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key1] = 2;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key2] = 3;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key3] = 4;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key4] = 5;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key5] = 6;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key6] = 7;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key7] = 8;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key8] = 9;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key9] = 10;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::Key0] = 11;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyMinus] = 12;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyEquals] = 13;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyBack] = 14;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyTab] = 15;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyQ] = 16;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyW] = 17;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyE] = 18;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyR] = 19;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyT] = 20;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyY] = 21;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyU] = 22;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyI] = 23;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyO] = 24;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyP] = 25;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyLBracket] = 26;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyRBracket] = 27;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyReturn] = 28;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyLControl] = 29;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyA] = 30;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyS] = 31;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyD] = 32;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF] = 33;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyG] = 34;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyH] = 35;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyJ] = 36;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyK] = 37;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyL] = 38;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeySemicolon] = 39;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyApostrophe] = 40;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyGrave] = 41;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyLshift] = 42;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyBackslash] = 43;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyZ] = 44;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyX] = 45;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyC] = 46;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyV] = 47;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyB] = 48;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyN] = 49;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyM] = 50;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyComma] = 51;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyPeriod] = 52;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeySlash] = 53;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyRshift] = 54;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyMultiply] = 55;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyLAlt] = 56;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeySpace] = 57;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyCapital] = 58;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF1] = 59;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF2] = 60;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF3] = 61;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF4] = 62;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF5] = 63;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF6] = 64;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF7] = 65;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF8] = 66;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF9] = 67;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF10] = 68;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumlock] = 69;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyScroll] = 70;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad7] = 71;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad8] = 72;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad9] = 73;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeySubtract] = 74;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad4] = 75;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad5] = 76;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad6] = 77;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyAdd] = 78;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad1] = 79;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad2] = 80;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad3] = 81;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpad0] = 82;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyDecimal] = 83;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF11] = 87;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyF12] = 88;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyNumpadenter] = 156;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyRControl] = 157;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyDivide] = 181;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeySysrq] = 183;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyRAlt] = 184;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyPause] = 197;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyHome] = 199;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyUp] = 200;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyPgup] = 201;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyLeft] = 203;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyRight] = 205;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyEnd] = 207;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyDown] = 208;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyPgdn] = 209;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyInsert] = 210;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyDelete] = 211;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyLwin] = 219;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyRwin] = 220;
        s_DXKeyMapping[(uint8_t)Darius::InputManager::DigitalInput::KeyApps] = 221;
    }

    void KbmZeroInputs()
    {
        memset(&s_MouseState, 0, sizeof(DIMOUSESTATE2));
        memset(s_Keybuffer, 0, sizeof(s_Keybuffer));
    }

    void KbmInitialize()
    {
        KbmBuildKeyMapping();

        if(FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&s_DI, nullptr)))
            D_ASSERT_M(false, "DirectInput8 initialization failed.");

        if(FAILED(s_DI->CreateDevice(GUID_SysKeyboard, &s_Keyboard, nullptr)))
            D_ASSERT_M(false, "Keyboard CreateDevice failed.");
        if(FAILED(s_Keyboard->SetDataFormat(&c_dfDIKeyboard)))
            D_ASSERT_M(false, "Keyboard SetDataFormat failed.");
        if(FAILED(s_Keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
            D_ASSERT_M(false, "Keyboard SetCooperativeLevel failed.");

        DIPROPDWORD dipdw;
        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.dwData = 10;
        if(FAILED(s_Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
            D_ASSERT_M(false, "Keyboard set buffer size failed.");

        if(FAILED(s_DI->CreateDevice(GUID_SysMouse, &s_Mouse, nullptr)))
            D_ASSERT_M(false, "Mouse CreateDevice failed.");
        if(FAILED(s_Mouse->SetDataFormat(&c_dfDIMouse2)))
            D_ASSERT_M(false, "Mouse SetDataFormat failed.");
        if(FAILED(s_Mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
            D_ASSERT_M(false, "Mouse SetCooperativeLevel failed.");

        KbmZeroInputs();
    }

    void KbmShutdown()
    {
        if(s_Keyboard)
        {
            s_Keyboard->Unacquire();
            s_Keyboard->Release();
            s_Keyboard = nullptr;
        }
        if(s_Mouse)
        {
            s_Mouse->Unacquire();
            s_Mouse->Release();
            s_Mouse = nullptr;
        }
        if(s_DI)
        {
            s_DI->Release();
            s_DI = nullptr;
        }
    }

    void KbmUpdate()
    {
        HWND foreground = GetForegroundWindow();
        bool visible = IsWindowVisible(foreground) != 0;

        if(foreground != hwnd // wouldn't be able to acquire
            || !visible)
        {
            KbmZeroInputs();
        }
        else
        {
            s_Mouse->Acquire();
            s_Mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &s_MouseState);
            s_Keyboard->Acquire();
            s_Keyboard->GetDeviceState(sizeof(s_Keybuffer), s_Keybuffer);
        }
    }

#endif

}
namespace Darius::InputManager
{

#ifdef _D_EDITOR
    bool OptionsDrawer(D_SERIALIZATION::Json&)
    {
        return false;
    }
#endif


    void Initialize(HWND window, D_SERIALIZATION::Json const&)
    {
        ZeroMemory(s_Buttons, sizeof(s_Buttons));
        ZeroMemory(s_Analogs, sizeof(s_Analogs));

        hwnd = window;

#ifdef USE_KEYBOARD_MOUSE
        KbmInitialize();
#endif

    }

    void Shutdown()
    {
#ifdef USE_KEYBOARD_MOUSE
        KbmShutdown();
#endif
    }

    void Update(float frameDelta)
    {
        memcpy(s_Buttons[1], s_Buttons[0], sizeof(s_Buttons[0]));
        memset(s_Buttons[0], 0, sizeof(s_Buttons[0]));
        memset(s_Analogs, 0, sizeof(s_Analogs));

#ifdef _GAMING_DESKTOP

#define SET_BUTTON_VALUE(InputEnum, GameInputMask) \
        s_Buttons[0][InputEnum] = !!(newInputState.Gamepad.wButtons & GameInputMask);

        XINPUT_STATE newInputState;
        if(ERROR_SUCCESS == XInputGetState(0, &newInputState))
        {
            SET_BUTTON_VALUE((uint8_t)DigitalInput::DPadUp, XINPUT_GAMEPAD_DPAD_UP);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::DPadDown, XINPUT_GAMEPAD_DPAD_DOWN);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::DPadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::DPadRight, XINPUT_GAMEPAD_DPAD_RIGHT);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::StartButton, XINPUT_GAMEPAD_START);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::BackButton, XINPUT_GAMEPAD_BACK);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::LThumbClick, XINPUT_GAMEPAD_LEFT_THUMB);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::RThumbClick, XINPUT_GAMEPAD_RIGHT_THUMB);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::LShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::RShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::AButton, XINPUT_GAMEPAD_A);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::BButton, XINPUT_GAMEPAD_B);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::XButton, XINPUT_GAMEPAD_X);
            SET_BUTTON_VALUE((uint8_t)DigitalInput::YButton, XINPUT_GAMEPAD_Y);

            s_Analogs[(uint8_t)AnalogInput::LeftTrigger] = newInputState.Gamepad.bLeftTrigger / 255.0f;
            s_Analogs[(uint8_t)AnalogInput::RightTrigger] = newInputState.Gamepad.bRightTrigger / 255.0f;
            s_Analogs[(uint8_t)AnalogInput::LeftStickX] = FilterAnalogInput(newInputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            s_Analogs[(uint8_t)AnalogInput::LeftStickY] = FilterAnalogInput(newInputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            s_Analogs[(uint8_t)AnalogInput::RightStickX] = FilterAnalogInput(newInputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            s_Analogs[(uint8_t)AnalogInput::RightStickY] = FilterAnalogInput(newInputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        }
#else
        IGameInputReading* pGIR = nullptr;
        if(s_pGameInput != nullptr)
            s_pGameInput->GetCurrentReading(GameInputKindGamepad, nullptr, &pGIR);
        bool IsGamepadPresent = (pGIR != nullptr);

        if(IsGamepadPresent)
        {
            GameInputGamepadState newInputState;
            pGIR->GetGamepadState(&newInputState);
            pGIR->Release();

#define SET_BUTTON_VALUE(InputEnum, GameInputMask) \
        s_Buttons[0][InputEnum] = !!(newInputState.buttons & GameInputMask);

            SET_BUTTON_VALUE(kDPadUp, GameInputGamepadDPadUp);
            SET_BUTTON_VALUE(kDPadDown, GameInputGamepadDPadDown);
            SET_BUTTON_VALUE(kDPadLeft, GameInputGamepadDPadLeft);
            SET_BUTTON_VALUE(kDPadRight, GameInputGamepadDPadRight);
            SET_BUTTON_VALUE(kStartButton, GameInputGamepadMenu);
            SET_BUTTON_VALUE(kBackButton, GameInputGamepadView);
            SET_BUTTON_VALUE(kLThumbClick, GameInputGamepadLeftThumbstick);
            SET_BUTTON_VALUE(kRThumbClick, GameInputGamepadRightThumbstick);
            SET_BUTTON_VALUE(kLShoulder, GameInputGamepadLeftShoulder);
            SET_BUTTON_VALUE(kRShoulder, GameInputGamepadRightShoulder);
            SET_BUTTON_VALUE(kAButton, GameInputGamepadA);
            SET_BUTTON_VALUE(kBButton, GameInputGamepadB);
            SET_BUTTON_VALUE(kXButton, GameInputGamepadX);
            SET_BUTTON_VALUE(kYButton, GameInputGamepadY);

            s_Analogs[kAnalogLeftTrigger] = newInputState.leftTrigger;
            s_Analogs[kAnalogRightTrigger] = newInputState.rightTrigger;
            s_Analogs[kAnalogLeftStickX] = FilterAnalogInput(newInputState.leftThumbstickX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            s_Analogs[kAnalogLeftStickY] = FilterAnalogInput(newInputState.leftThumbstickY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            s_Analogs[kAnalogRightStickX] = FilterAnalogInput(newInputState.rightThumbstickX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            s_Analogs[kAnalogRightStickY] = FilterAnalogInput(newInputState.rightThumbstickY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        }
#endif

#ifdef USE_KEYBOARD_MOUSE
        KbmUpdate();

        for(uint32_t i = 0; i < (uint8_t)DigitalInput::NumKeys; ++i)
        {
            s_Buttons[0][i] = (s_Keybuffer[s_DXKeyMapping[i]] & 0x80) != 0;
        }

        for(uint8_t i = 0; i < 8; ++i)
        {
            if(s_MouseState.rgbButtons[i] > 0) s_Buttons[0][(uint8_t)DigitalInput::Mouse0 + i] = true;
        }

        s_Analogs[(uint8_t)AnalogInput::MouseX] = (float)s_MouseState.lX * .0018f;
        s_Analogs[(uint8_t)AnalogInput::MouseY] = (float)s_MouseState.lY * -.0018f;

        if(s_MouseState.lZ > 0)
            s_Analogs[(uint8_t)AnalogInput::MouseScroll] = 1.0f;
        else if(s_MouseState.lZ < 0)
            s_Analogs[(uint8_t)AnalogInput::MouseScroll] = -1.0f;
#endif

        // Update time duration for buttons pressed
        for(uint32_t i = 0; i < (uint8_t)DigitalInput::NumDigitalInputs; ++i)
        {
            if(s_Buttons[0][i])
            {
                if(!s_Buttons[1][i])
                    s_HoldDuration[i] = 0.0f;
                else
                    s_HoldDuration[i] += frameDelta;
            }
        }

        for(uint32_t i = 0; i < (uint8_t)AnalogInput::NumAnalogInputs; ++i)
        {
            s_AnalogsTC[i] = s_Analogs[i] * frameDelta;
        }

    }

    bool SetExclusiveCursor(bool exclusive)
    {
        DWORD flags = exclusive ? (DISCL_FOREGROUND | DISCL_EXCLUSIVE) : (DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
        return SUCCEEDED(s_Mouse->SetCooperativeLevel(hwnd, flags));
    }

    bool IsAnyPressed()
    {
        return s_Buttons[0] != 0;
    }

    bool IsPressed(DigitalInput di)
    {
        return s_Buttons[0][(uint8_t)di];
    }

    bool IsFirstPressed(DigitalInput di)
    {
        return s_Buttons[0][(uint8_t)di] && !s_Buttons[1][(uint8_t)di];
    }

    bool IsReleased(DigitalInput di)
    {
        return !s_Buttons[0][(uint8_t)di];
    }

    bool IsFirstReleased(DigitalInput di)
    {
        return !s_Buttons[0][(uint8_t)di] && s_Buttons[1][(uint8_t)di];
    }

    float GetDurationPressed(DigitalInput di)
    {
        return s_HoldDuration[(uint8_t)di];
    }

    float GetAnalogInput(AnalogInput ai)
    {
        return s_Analogs[(uint8_t)ai];
    }

    float GetTimeCorrectedAnalogInput(AnalogInput ai)
    {
        return s_AnalogsTC[(uint8_t)ai];
    }
}