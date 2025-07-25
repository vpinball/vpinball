#pragma once

#include "pininput.h"

class Win32InputKeyboardHandler final : public InputHandler
{
public:
   Win32InputKeyboardHandler(PinInput& pininput, HWND focusWnd)
      : m_pininput(pininput)
      , m_focusHWnd(focusWnd)
   {
      PLOGI << "Win32 Keyboard handler registered";
   }
   ~Win32InputKeyboardHandler() override = default;

   void Update(const HWND foregroundWindow) override
   {
      if (m_focusHWnd != foregroundWindow)
         return;

      for (unsigned int i = 0; i < eCKeys; ++i)
      {
         const auto rgk = static_cast<unsigned int>(g_pplayer->m_rgKeys[i]);
         const unsigned int vk = get_vk(rgk);
         if (vk == ~0u)
            continue;
         const SHORT keyState = GetAsyncKeyState(vk);
         const bool keyDown = !!((1 << 16) & keyState);
         if (m_oldKeyStates[i] == keyDown)
            continue;
         m_oldKeyStates[i] = keyDown;
         m_pininput.PushActionEvent(static_cast<EnumAssignKeys>(i), keyDown);
      }
   }

private:
   PinInput& m_pininput;
   const HWND m_focusHWnd;
   bool m_oldKeyStates[eCKeys] = { false };
};
