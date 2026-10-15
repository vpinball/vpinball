// license:GPLv3+

#include "core/stdafx.h"
#include "ui/win/WinApp.h"

#include "core/extern.h"
#include "core/player.h"
#include "ui/win/WinEditor.h"


BOOL WinApp::OnIdle(LONG)
{
   if (g_pplayer)
      g_pplayer->m_pluginManager.ProcessAsyncCallbacks();
   return FALSE;
}

static bool IsRealDialogWindow(HWND hWnd)
{
   if (hWnd == nullptr)
      return false;

   WCHAR className[256];
   if (::GetClassNameW(hWnd, className, _countof(className)) == 0)
      return false;

   WNDCLASSEXW wc = { sizeof(wc) };
   HINSTANCE hInst = reinterpret_cast<HINSTANCE>(::GetWindowLongPtrW(hWnd, GWLP_HINSTANCE));

   // GetClassInfoEx wants the instance the class was registered against;
   // try the window's own instance first, then fall back to NULL (system classes).
   if (!::GetClassInfoExW(hInst, className, &wc) && !::GetClassInfoExW(nullptr, className, &wc))
      return false;

   return wc.cbWndExtra >= DLGWINDOWEXTRA;
}

BOOL WinApp::PreTranslateMessage(MSG &msg)
{
   // The WinEditor frame registers itself as this thread's main window on creation (wxx CFrame::OnCreate)
   const CWnd *const mainWnd = CWnd::GetCWndPtr(GetMainWnd());
   if (mainWnd != nullptr && mainWnd->IsWindow() && msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
   {
      // Always forward F1-F12 to the main VPinball class to open subdialogs from everywhere
      if (const int keyPressed = LOWORD(msg.wParam); (keyPressed >= VK_F1 && keyPressed <= VK_F12))
         return __super::PreTranslateMessage(msg);

      // Leave the Alt combinations to the default pre translation rather than to the edit control filtering below,
      // so that each pane decides for itself: the ones which are meant to reach the frame menu let them through in
      // their PreTranslateMessage, the dialog like ones (notes, property pages) keep them for their own mnemonics
      if (IsAltKeyMessage(msg.message))
         return __super::PreTranslateMessage(msg);

      // Skip accelerators for control of the main editor embedded dialogs (property pane edits, to avoid Delete, Copy/Paste, Undo,... conflicts, support tabbing through pane control)
      if (CWnd focus = mainWnd->GetFocus(); focus != nullptr && mainWnd->IsChild(focus) && focus.GetClassName() == WC_EDIT)
      {
         while (focus != nullptr)
         {
            if (IsRealDialogWindow(focus))
            {
               return focus.IsDialogMessage(msg);
            }
            focus = focus.GetParent();
         }
      }
   }
   return __super::PreTranslateMessage(msg);
}
