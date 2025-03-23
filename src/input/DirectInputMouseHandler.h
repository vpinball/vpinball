#pragma once

#include "core/pininput.h"

class DirectInputMouseHandler : public InputHandler
{
public:
   DirectInputMouseHandler(PinInput& pininput, HWND focusWnd)
      : m_pininput(pininput)
      , m_focusHWnd(focusWnd)
   {
      HRESULT hr;
      #ifdef USE_DINPUT8
         PLOGI << "DirectInput 8 mouse input handler registered";
         hr = DirectInput8Create(g_pvp->theInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, nullptr);
      #else
         PLOGI << "DirectInput mouse input handler registered";
         hr = DirectInputCreate(g_pvp->theInstance, DIRECTINPUT_VERSION, &m_pDI, nullptr);
      #endif
      if (SUCCEEDED(m_pDI->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
      {
         hr = m_pMouse->SetDataFormat(&c_dfDIMouse2);
         hr = m_pMouse->SetCooperativeLevel(m_focusHWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
      }
   }

   ~DirectInputMouseHandler() final
   {
      // We unacquire the device one last time just in case the app tried to exit while the device is still acquired.
      if (m_pMouse)
         m_pMouse->Unacquire();
      SAFE_RELEASE(m_pMouse);
      SAFE_RELEASE(m_pDI);
   }

   void Update() final
   {
      if (m_pMouse == nullptr || m_focusHWnd != GetForegroundWindow())
         return;

      HRESULT hr = m_pMouse->Acquire();
      if (hr == S_OK || hr == S_FALSE)
      {
         DIMOUSESTATE2 mouseState;
         hr = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &mouseState);
         if (hr == S_OK || hr == DI_BUFFEROVERFLOW)
         {
            for (DWORD i = 0; i < 3; i++)
            {
               if (m_oldMouseButtonState[i] != mouseState.rgbButtons[i])
               {
                  static constexpr int LeftMouseButtonID = 25;
                  static constexpr int RightMouseButtonID = 26;
                  static constexpr int MiddleMouseButtonID = 27;
                  const int buttonId =
                     i == 0 ? LeftMouseButtonID : 
                     i == 1 ? RightMouseButtonID : 
                              MiddleMouseButtonID;
                  m_pininput.PushMouseEvent(buttonId, (mouseState.rgbButtons[i] & 0x80) != 0);
                  m_oldMouseButtonState[i] = mouseState.rgbButtons[i];
               }
            }
         }
      }
   }

private:
   PinInput& m_pininput;
   const HWND m_focusHWnd;

   #ifdef USE_DINPUT8
      LPDIRECTINPUT8 m_pDI = nullptr;
      LPDIRECTINPUTDEVICE8 m_pMouse = nullptr;
   #else
      LPDIRECTINPUT m_pDI = nullptr;
      LPDIRECTINPUTDEVICE m_pMouse = nullptr;
   #endif

   BYTE m_oldMouseButtonState[3] { 0 };
};
