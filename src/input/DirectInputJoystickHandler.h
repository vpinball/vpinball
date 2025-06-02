#pragma once

#include "pininput.h"

// from dinput.h, modernized to please clang
#undef DIJOFS_X
#undef DIJOFS_Y
#undef DIJOFS_Z
#undef DIJOFS_RX
#undef DIJOFS_RY
#undef DIJOFS_RZ
#undef DIJOFS_SLIDER
#undef DIJOFS_POV
#undef DIJOFS_BUTTON
#define DIJOFS_X offsetof(DIJOYSTATE, lX)
#define DIJOFS_Y offsetof(DIJOYSTATE, lY)
#define DIJOFS_Z offsetof(DIJOYSTATE, lZ)
#define DIJOFS_RX offsetof(DIJOYSTATE, lRx)
#define DIJOFS_RY offsetof(DIJOYSTATE, lRy)
#define DIJOFS_RZ offsetof(DIJOYSTATE, lRz)
#define DIJOFS_SLIDER(n) (offsetof(DIJOYSTATE, rglSlider) + (n) * sizeof(LONG))
#define DIJOFS_POV(n) (offsetof(DIJOYSTATE, rgdwPOV) + (n) * sizeof(DWORD))
#define DIJOFS_BUTTON(n) (offsetof(DIJOYSTATE, rgbButtons) + (n))
// end

class DirectInputJoystickHandler final : public InputHandler
{
public:
   DirectInputJoystickHandler(PinInput& pininput, HWND focusWnd)
      : m_pininput(pininput)
      , m_focusHWnd(focusWnd)
   {
      HRESULT hr;
      #ifdef USE_DINPUT8
         PLOGI << "DirectInput 8 joystick input handler registered";
         hr = DirectInput8Create(g_pvp->theInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, nullptr);
         m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoystickCallback, this, DIEDFL_ATTACHEDONLY);
      #else
         PLOGI << "DirectInput joystick input handler registered";
         hr = DirectInputCreate(g_pvp->theInstance, DIRECTINPUT_VERSION, &m_pDI, nullptr);
         m_pDI->EnumDevices(DIDEVTYPE_JOYSTICK, EnumJoystickCallback, this, DIEDFL_ATTACHEDONLY);
      #endif
   }

   ~DirectInputJoystickHandler() override
   {
      for (auto joystick : m_joysticks)
      {
         // We unacquire the device one last time just in case the app tried to exit while the device is still acquired.
         joystick->Unacquire();
         SAFE_RELEASE(joystick);
      }
      m_joysticks.clear();
      SAFE_RELEASE(m_pDI);
   }

   static constexpr uint64_t GetJoyId(const int index) { return static_cast<uint64_t>(0x100000000) | static_cast<uint64_t>(index); }

   int GetNJoysticks() const { return static_cast<int>(m_joysticks.size()); }
   #ifdef USE_DINPUT8
      LPDIRECTINPUTDEVICE8 GetJoystick(int index) const { return m_joysticks[index]; }
   #else
      LPDIRECTINPUTDEVICE GetJoystick(int index) const { return m_joysticks[index]; }
   #endif

   void Update() override
   {
      int index = 0;
      for (auto joystick : m_joysticks)
      {
         HRESULT hr = joystick->Acquire();
         if (hr == S_OK || hr == S_FALSE)
         {
            DWORD dwElements = 64;
            DIDEVICEOBJECTDATA didod[64];
            hr = joystick->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);
            if ((hr == S_OK || hr == DI_BUFFEROVERFLOW) && (m_focusHWnd == GetForegroundWindow()))
            {
               uint64_t joyId = GetJoyId(index);
               for (DWORD i = 0; i < dwElements; i++)
               {
                  if ((didod[i].dwOfs >= DIJOFS_BUTTON0) && (didod[i].dwOfs <= DIJOFS_BUTTON31))
                     m_pininput.PushJoystickButtonEvent(joyId, didod[i].dwOfs - DIJOFS_BUTTON0, (didod[i].dwData & 0x80) != 0);
                  else
                  {
                     switch (didod[i].dwOfs)
                     {
                     case DIJOFS_X: m_pininput.PushJoystickAxisEvent(joyId, 1, didod[i].dwData); break;
                     case DIJOFS_Y: m_pininput.PushJoystickAxisEvent(joyId, 2, didod[i].dwData); break;
                     case DIJOFS_Z: m_pininput.PushJoystickAxisEvent(joyId, 3, didod[i].dwData); break;
                     case DIJOFS_RX: m_pininput.PushJoystickAxisEvent(joyId, 4, didod[i].dwData); break;
                     case DIJOFS_RY: m_pininput.PushJoystickAxisEvent(joyId, 5, didod[i].dwData); break;
                     case DIJOFS_RZ: m_pininput.PushJoystickAxisEvent(joyId, 6, didod[i].dwData); break;
                     case DIJOFS_SLIDER(0): m_pininput.PushJoystickAxisEvent(joyId, 7, didod[i].dwData); break;
                     case DIJOFS_SLIDER(1): m_pininput.PushJoystickAxisEvent(joyId, 8, didod[i].dwData); break;
                     }
                  }
               }
            }
         }
         index++;
      }
   }

private:
   // Callback function for enumerating objects (axes, buttons, POVs) on a joystick. This function enables user interface elements for objects that are found to exist, and scales axes min/max values.
   static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
   {
      #ifdef USE_DINPUT8
         auto joystick = (LPDIRECTINPUTDEVICE8)pContext;
      #else
         auto joystick = (LPDIRECTINPUTDEVICE)pContext;
      #endif
      // For axes that are returned, set the DIPROP_RANGE property for the enumerated axis in order to scale min/max values.
      if (pdidoi->dwType & DIDFT_AXIS)
      {
         DIPROPRANGE diprg;
         diprg.diph.dwSize = sizeof(DIPROPRANGE);
         diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
         diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
         diprg.diph.dwHow = DIPH_BYID;
         diprg.lMin = -JOYRANGEMX;
         diprg.lMax =  JOYRANGEMX;
         if (FAILED(joystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
            return DIENUM_STOP;

         // set DEADBAND to Zero
         DIPROPDWORD dipdw;
         dipdw.diph.dwSize = sizeof(DIPROPDWORD);
         dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
         dipdw.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
         dipdw.diph.dwHow = DIPH_BYID;
         dipdw.dwData = 0; // no dead zone at joystick level
         if (FAILED(joystick->SetProperty(DIPROP_DEADZONE, &dipdw.diph)))
            return DIENUM_STOP;
      }
      return DIENUM_CONTINUE;
   }

   // Callback for enumerating joysticks (gamepads)
   static BOOL CALLBACK EnumJoystickCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
   {
      DirectInputJoystickHandler* const me = (DirectInputJoystickHandler*)pvRef;

      DIPROPSTRING dstr;
      dstr.diph.dwSize = sizeof(DIPROPSTRING);
      dstr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      dstr.diph.dwObj = 0;
      dstr.diph.dwHow = DIPH_DEVICE;

      HRESULT hr;
      #ifdef USE_DINPUT8
         LPDIRECTINPUTDEVICE8 joystick;
      #else
         LPDIRECTINPUTDEVICE joystick;
      #endif
      hr = me->m_pDI->CreateDevice(lpddi->guidInstance, &joystick, nullptr);
      if (FAILED(hr))
         return DIENUM_CONTINUE;

      // Check if device is disabled (somewhat overkill, since we read the settings each time, but neglictible impact and cleaner code)
      DIDEVICEINSTANCE deviceInfo;
      deviceInfo.dwSize = sizeof(DIDEVICEINSTANCE);
      joystick->GetDeviceInfo(&deviceInfo);
      string joystickName(deviceInfo.tszInstanceName);
      bool inputDeviceEnabled = true;
      static const string kDefaultName("None"s);
      for (int i = 0; i < PININ_JOYMXCNT; i++)
      {
         const string deviceName = "Device" + std::to_string(i) + "_Name";
         const string name = g_pvp->m_settings.LoadValueWithDefault(Settings::ControllerDevices, deviceName, kDefaultName);
         if (name == joystickName)
         {
            const string deviceState = "Device" + std::to_string(i) + "_State";
            inputDeviceEnabled = g_pvp->m_settings.LoadValueWithDefault(Settings::ControllerDevices, deviceState, true);
            break;
         }
      }
      if (!inputDeviceEnabled)
      {
         joystick->Release();
         return DIENUM_CONTINUE;
      }

      // Legacy identification to apply fixed button/axis mappings. This only works under DirectInput and for a bunch of hardware => Remove ?
      PinInput::InputLayout joystickType = PinInput::InputLayout::Generic;
      hr = joystick->GetProperty(DIPROP_PRODUCTNAME, &dstr.diph);
      if (hr == S_OK)
      {
         if (wcscmp(dstr.wsz, L"PinballWizard") == 0)
            joystickType = PinInput::InputLayout::PBWizard;
         else if (wcscmp(dstr.wsz, L"UltraCade Pinball") == 0)
            joystickType = PinInput::InputLayout::UltraCade;
         else if (wcscmp(dstr.wsz, L"Microsoft SideWinder Freestyle Pro (USB)") == 0)
            joystickType = PinInput::InputLayout::Sidewinder;
         else if (wcscmp(dstr.wsz, L"VirtuaPin Controller") == 0)
            joystickType = PinInput::InputLayout::VirtuaPin;
         else if (wcscmp(dstr.wsz, L"Pinscape Controller") == 0 || wcscmp(dstr.wsz, L"PinscapePico") == 0)
         {
            joystickType = PinInput::InputLayout::Generic;
            me->m_pininput.m_linearPlunger = true;
            PLOGI << "Pinscape device detected, 'linear' plunger acquisition enabled";
         }
         PLOGI << "Joystick detected: " << dstr.wsz << ", using input layout #" << static_cast<int>(joystickType);
      }
      hr = joystick->SetDataFormat(&c_dfDIJoystick);

      // joystick input foreground or background focus
      hr = joystick->SetCooperativeLevel(me->m_focusHWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

      DIPROPDWORD dipdw;
      dipdw.diph.dwSize = sizeof(DIPROPDWORD);
      dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      dipdw.diph.dwObj = 0;
      dipdw.diph.dwHow = DIPH_DEVICE;
      dipdw.dwData = 64;
      hr = joystick->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

      // Enumerate the joystick objects. The callback function enabled user interface elements for objects that are found, and sets the min/max values property for discovered axes.
      hr = joystick->EnumObjects(EnumObjectsCallback, (VOID*)joystick, DIDFT_ALL);

      me->m_pininput.SetupJoyMapping(GetJoyId(static_cast<int>(me->m_joysticks.size())), joystickType);
      me->m_joysticks.push_back(joystick);

      return DIENUM_CONTINUE;
   }

private:
   PinInput& m_pininput;
   const HWND m_focusHWnd;

   #ifdef USE_DINPUT8
      LPDIRECTINPUT8 m_pDI = nullptr;
      vector<LPDIRECTINPUTDEVICE8> m_joysticks;
   #else
      LPDIRECTINPUT m_pDI = nullptr;
      vector<LPDIRECTINPUTDEVICE> m_joysticks;
   #endif
};
