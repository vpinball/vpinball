#pragma once

#include "pininput.h"

class DirectInputKeyboardHandler final : public InputHandler
{
public:
   DirectInputKeyboardHandler(PinInput& pininput, HWND focusWnd)
      : m_pininput(pininput)
      , m_focusHWnd(focusWnd)
   {
      HRESULT hr;
      #ifdef USE_DINPUT8
         PLOGI << "DirectInput 8 keyboard input handler registered";
         hr = DirectInput8Create(g_pvp->theInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, nullptr);
      #else
         PLOGI << "DirectInput keyboard input handler registered";
         hr = DirectInputCreate(g_pvp->theInstance, DIRECTINPUT_VERSION, &m_pDI, nullptr);
      #endif

      hr = m_pDI->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, nullptr); //Standard Keyboard device
      hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
      hr = m_pKeyboard->SetCooperativeLevel(m_focusHWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND); //!! exclusive necessary??

      DIPROPDWORD dipdw;
      dipdw.diph.dwSize = sizeof(DIPROPDWORD);
      dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      dipdw.diph.dwObj = 0;
      dipdw.diph.dwHow = DIPH_DEVICE;
      dipdw.dwData = 64;
      hr = m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
   }

   ~DirectInputKeyboardHandler() override
   {
      // We unacquire the device one last time just in case the app tried to exit while the device is still acquired.
      if (m_pKeyboard)
         m_pKeyboard->Unacquire();
      SAFE_RELEASE(m_pKeyboard);
      SAFE_RELEASE(m_pDI);
   }

   void Update() override
   {
      if (m_pKeyboard == nullptr || m_focusHWnd != GetForegroundWindow())
         return;

      HRESULT hr = m_pKeyboard->Acquire();
      if (hr == S_OK || hr == S_FALSE)
      {
         DWORD dwElements = 64;
         DIDEVICEOBJECTDATA didod[64];
         hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);
         if (hr == S_OK || hr == DI_BUFFEROVERFLOW)
         {
            for (DWORD i = 0; i < dwElements; i++)
               m_pininput.PushKeyboardEvent(didod[i].dwOfs, (didod[i].dwData & 0x80) != 0);
         }
      }
   }

private:
   PinInput& m_pininput;
   const HWND m_focusHWnd;

   #ifdef USE_DINPUT8
      LPDIRECTINPUT8 m_pDI = nullptr;
      LPDIRECTINPUTDEVICE8 m_pKeyboard = nullptr;
   #else
      LPDIRECTINPUT m_pDI = nullptr;
      LPDIRECTINPUTDEVICE m_pKeyboard = nullptr;
   #endif
};
