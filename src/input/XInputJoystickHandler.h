#pragma once

#include "pininput.h"

class XInputJoystickHandler final : public InputHandler
{
public:
   XInputJoystickHandler(PinInput& pininput, HWND focusWnd)
      : m_pininput(pininput)
      , m_focusHWnd(focusWnd)
   {
      PLOGI << "XInput joystick handler registered";
      for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
      {
         XINPUT_STATE state = {};
         DWORD hr = XInputGetState(i, &state);
         if (hr == ERROR_SUCCESS)
         {
            Device joy;
            joy.id = i;
            m_devices.push_back(joy);
            m_pininput.SetupJoyMapping(GetJoyId(i), PinInput::InputLayout::Generic);
         }
      }
   }

   ~XInputJoystickHandler() override
   {
      for (const Device& joy : m_devices)
         m_pininput.UnmapJoy(GetJoyId(joy.id));
   }

   static constexpr uint64_t GetJoyId(const int index) { return static_cast<uint64_t>(0x300000000) | static_cast<uint64_t>(index); }

   void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration) override
   {
      m_rumbleOffTime = ms_duration + msec();
      m_rumbleRunning = true;
      XINPUT_VIBRATION vibration = {};
      // On both PS4 and X360:
      // The left motor is the low - frequency rumble motor. (explosions, etc)
      // The right motor is the high - frequency rumble motor. (subtle stuff)
      // The two motors are not the same, and they create different vibration effects.
      vibration.wLeftMotorSpeed = (WORD)(saturate(lowFrequencySpeed) * 65535.f);
      vibration.wRightMotorSpeed = (WORD)(saturate(highFrequencySpeed) * 65535.f);
      for (const Device& joy : m_devices)
         XInputSetState(joy.id, &vibration);
   }

   void Update() override
   {
      if (m_focusHWnd != GetForegroundWindow())
         return;

      using mappingElement = struct
      {
         WORD xi;
         DWORD di;
      };
      static constexpr mappingElement mappingTable[] = {
         { XINPUT_GAMEPAD_A, 0 },
         { XINPUT_GAMEPAD_B, 1 },
         { XINPUT_GAMEPAD_X, 2 },
         { XINPUT_GAMEPAD_Y, 3 },
         { XINPUT_GAMEPAD_LEFT_SHOULDER, 4 },
         { XINPUT_GAMEPAD_RIGHT_SHOULDER, 5 },
         { XINPUT_GAMEPAD_BACK, 6 },
         { XINPUT_GAMEPAD_START, 7 },
         { XINPUT_GAMEPAD_LEFT_THUMB, 8 },
         { XINPUT_GAMEPAD_RIGHT_THUMB, 9 },
         { XINPUT_GAMEPAD_DPAD_LEFT, 10 },
         { XINPUT_GAMEPAD_DPAD_RIGHT, 11 },
         { XINPUT_GAMEPAD_DPAD_UP, 12 },
         { XINPUT_GAMEPAD_DPAD_DOWN, 13 },
         { 0, 0 } };

      bool stopRumble = false;
      if (m_rumbleRunning)
      {
         const DWORD now = msec();
         if (m_rumbleOffTime <= now || m_rumbleOffTime - now > 65535)
         {
            m_rumbleRunning = false;
            stopRumble = true;
         }
      }

      for (Device joy : m_devices)
      {
         XINPUT_STATE state = {};
         DWORD hr = XInputGetState(joy.id, &state);
         if (hr == ERROR_SUCCESS)
         {
            if (stopRumble)
            {
               XINPUT_VIBRATION vibration { 0, 0 };
               XInputSetState(joy.id, &vibration);
            }

            int i = 0;
            while (mappingTable[i].xi != 0)
            {
               if ((joy.state.Gamepad.wButtons & mappingTable[i].xi) != (state.Gamepad.wButtons & mappingTable[i].xi))
                  m_pininput.PushJoystickButtonEvent(GetJoyId(joy.id), mappingTable[i].di, (state.Gamepad.wButtons & mappingTable[i].xi) > 0);
               i++;
            }
            // NOTE - this is a hard-coded assumption that JOYRANGE is -65536..+65536
            if (joy.state.Gamepad.sThumbLX != state.Gamepad.sThumbLX)
               m_pininput.PushJoystickAxisEvent(GetJoyId(joy.id), 1, state.Gamepad.sThumbLX * -2);
            if (joy.state.Gamepad.sThumbLY != state.Gamepad.sThumbLY)
               m_pininput.PushJoystickAxisEvent(GetJoyId(joy.id), 2, state.Gamepad.sThumbLY * -2);
            if (joy.state.Gamepad.bLeftTrigger != state.Gamepad.bLeftTrigger)
               m_pininput.PushJoystickAxisEvent(GetJoyId(joy.id), 3, state.Gamepad.bLeftTrigger * 512);
            if (joy.state.Gamepad.sThumbRX != state.Gamepad.sThumbRX)
               m_pininput.PushJoystickAxisEvent(GetJoyId(joy.id), 4, state.Gamepad.sThumbRX * -2);
            if (joy.state.Gamepad.sThumbRY != state.Gamepad.sThumbRY)
               m_pininput.PushJoystickAxisEvent(GetJoyId(joy.id), 5, state.Gamepad.sThumbRY * -2);
            if (joy.state.Gamepad.bRightTrigger != state.Gamepad.bRightTrigger)
               m_pininput.PushJoystickAxisEvent(GetJoyId(joy.id), 6, state.Gamepad.bRightTrigger * 512);
            memcpy(&joy.state, &state, sizeof(XINPUT_STATE));
         }
      }
   }

private:
   PinInput& m_pininput;
   const HWND m_focusHWnd;

   struct Device
   {
      int id;
      XINPUT_STATE state { 0 };
   };
   vector<Device> m_devices;

   DWORD m_rumbleOffTime = 0;
   bool m_rumbleRunning = false;
};
