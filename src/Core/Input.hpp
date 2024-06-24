#pragma once

#include <Core/Serialization/Json.hpp>
#include <Utils/Common.hpp>

#ifndef D_INPUT
#define D_INPUT Darius::InputManager
#endif // !D_INPUT

namespace Darius::InputManager
{
	void Initialize(HWND window, D_SERIALIZATION::Json const& settings);
	void Shutdown();
#ifdef _D_EDITOR
	bool OptionsDrawer(D_SERIALIZATION::Json&);
#endif

    void Initialize();
    void Shutdown();
    void Update(float frameDelta);

    enum class DigitalInput : uint8_t
    {
        // keyboard
        // kKey must start at zero, see s_DXKeyMapping
        KeyEscape = 0,
        Key1,
        Key2,
        Key3,
        Key4,
        Key5,
        Key6,
        Key7,
        Key8,
        Key9,
        Key0,
        KeyMinus,
        KeyEquals,
        KeyBack,
        KeyTab,
        KeyQ,
        KeyW,
        KeyE,
        KeyR,
        KeyT,
        KeyY,
        KeyU,
        KeyI,
        KeyO,
        KeyP,
        KeyLBracket,
        KeyRBracket,
        KeyReturn,
        KeyLControl,
        KeyA,
        KeyS,
        KeyD,
        KeyF,
        KeyG,
        KeyH,
        KeyJ,
        KeyK,
        KeyL,
        KeySemicolon,
        KeyApostrophe,
        KeyGrave,
        KeyLshift,
        KeyBackslash,
        KeyZ,
        KeyX,
        KeyC,
        KeyV,
        KeyB,
        KeyN,
        KeyM,
        KeyComma,
        KeyPeriod,
        KeySlash,
        KeyRshift,
        KeyMultiply,
        KeyLAlt,
        KeySpace,
        KeyCapital,
        KeyF1,
        KeyF2,
        KeyF3,
        KeyF4,
        KeyF5,
        KeyF6,
        KeyF7,
        KeyF8,
        KeyF9,
        KeyF10,
        KeyNumlock,
        KeyScroll,
        KeyNumpad7,
        KeyNumpad8,
        KeyNumpad9,
        KeySubtract,
        KeyNumpad4,
        KeyNumpad5,
        KeyNumpad6,
        KeyAdd,
        KeyNumpad1,
        KeyNumpad2,
        KeyNumpad3,
        KeyNumpad0,
        KeyDecimal,
        KeyF11,
        KeyF12,
        KeyNumpadenter,
        KeyRControl,
        KeyDivide,
        KeySysrq,
        KeyRAlt,
        KeyPause,
        KeyHome,
        KeyUp,
        KeyPgup,
        KeyLeft,
        KeyRight,
        KeyEnd,
        KeyDown,
        KeyPgdn,
        KeyInsert,
        KeyDelete,
        KeyLwin,
        KeyRwin,
        KeyApps,

        NumKeys,

        // gamepad
        DPadUp = NumKeys,
        DPadDown,
        DPadLeft,
        DPadRight,
        StartButton,
        BackButton,
        LThumbClick,
        RThumbClick,
        LShoulder,
        RShoulder,
        AButton,
        BButton,
        XButton,
        YButton,

        // mouse
        Mouse0,
        Mouse1,
        Mouse2,
        Mouse3,
        Mouse4,
        Mouse5,
        Mouse6,
        Mouse7,

        NumDigitalInputs
    };

    enum class AnalogInput : uint8_t
    {
        // gamepad
        LeftTrigger,
        RightTrigger,
        LeftStickX,
        LeftStickY,
        RightStickX,
        RightStickY,

        // mouse
        MouseX,
        MouseY,
        MouseScroll,

        NumAnalogInputs
    };

    bool IsAnyPressed();

    bool IsPressed(DigitalInput di);
    bool IsFirstPressed(DigitalInput di);
    bool IsReleased(DigitalInput di);
    bool IsFirstReleased(DigitalInput di);

    float GetDurationPressed(DigitalInput di);

    float GetAnalogInput(AnalogInput ai);
    float GetTimeCorrectedAnalogInput(AnalogInput ai);

    bool ClipCursorToRect(RECT const* rect);
    bool GetCursorClipRect(RECT* rect);
    void SetCursorVisible(bool visible);
}