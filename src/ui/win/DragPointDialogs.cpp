// license:GPLv3+

#include "core/stdafx.h"

#include "DragPointDialogs.h"

namespace VPX::WinUI
{

RotatePointsDialog::RotatePointsDialog(ISelect *psel)
{
   DialogBoxParam(
      g_pvp->theInstance,
      MAKEINTRESOURCE(IDD_ROTATE),
      g_pvp->GetHwnd(),
      RotateProc,
      (size_t)psel);
}

int RotatePointsDialog::m_applyCount = 0;

INT_PTR CALLBACK RotatePointsDialog::RotateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ISelect *psel;

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      m_applyCount = 0;
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      const float angle = psel->GetRotate();

      SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_SETCHECK, BST_CHECKED, 0);

      SetDlgItemText(hwndDlg, IDC_ROTATEBY, f2sz(angle).c_str());
      const Vertex2D v = psel->GetCenter();
      SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(v.x).c_str());
      SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(v.y).c_str());
   }
   return TRUE;
   break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      switch (LOWORD(wParam))
      {
      case IDC_CHECK_ROTATE_CENTER:
      {
         switch (HIWORD(wParam))
         {
            case BN_CLICKED:
            {
               if (SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_GETCHECK, 0, 0) != BST_CHECKED)
               {
                  SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(g_pvp->m_mouseCursorPosition.x).c_str());
                  SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(g_pvp->m_mouseCursorPosition.y).c_str());
               }
               else
               {
                  const Vertex2D v = psel->GetCenter();
                  SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(v.x).c_str());
                  SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(v.y).c_str());
               }
               break;
            }
            default:
               break;
          }
      }
      default:
         break;
      }


      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
         {
            if (m_applyCount == 0)
            {
               char szT[256];
               GetDlgItemText(hwndDlg, IDC_ROTATEBY, szT, 255);
               const float f = sz2f(szT);

               const bool useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);
               GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
               Vertex2D v;
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
               v.y = sz2f(szT);

               psel->Rotate(f, v, useElementCenter);
            }
            EndDialog(hwndDlg, TRUE);
            break;
         }
         case IDC_ROTATE_APPLY_BUTTON:
         {
            m_applyCount++;
            char szT[256];
            GetDlgItemText(hwndDlg, IDC_ROTATEBY, szT, 255);
            const float f = sz2f(szT);

            const bool useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);
            GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
            Vertex2D v;
            v.x = sz2f(szT);
            GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
            v.y = sz2f(szT);

            psel->Rotate(f, v, useElementCenter);
            psel->GetPTable()->SetDirtyDraw();
            break;
         }
         case IDC_ROTATE_UNDO_BUTTON:
         {
            if (m_applyCount > 0)
            {
               m_applyCount--;
               psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            break;
         }
         case IDCANCEL:
            if (m_applyCount > 0)
            {
               for (int i = 0; i < m_applyCount; i++)
                  psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            EndDialog(hwndDlg, FALSE);
            break;
         }
         break;
      }
      break;
   }

   return FALSE;
}


ScalePointsDialog::ScalePointsDialog(ISelect *psel)
{
   DialogBoxParam(
      g_pvp->theInstance,
      MAKEINTRESOURCE(IDD_SCALE),
      g_pvp->GetHwnd(),
      ScaleProc,
      (size_t)psel);
}

int ScalePointsDialog::m_applyCount = 0;

INT_PTR CALLBACK  ScalePointsDialog::ScaleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ISelect *psel;

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      m_applyCount = 0;
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

      Vertex2D v = psel->GetScale();

      SetDlgItemText(hwndDlg, IDC_SCALEFACTOR, f2sz(v.x).c_str());
      SetDlgItemText(hwndDlg, IDC_SCALEY, f2sz(v.y).c_str());
      v = psel->GetCenter();

      SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_SETCHECK, BST_CHECKED, 0);

      SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(v.x).c_str());
      SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(v.y).c_str());

      SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_SETCHECK, TRUE, 0);

      EnableWindow(GetDlgItem(hwndDlg, IDC_SCALEY), FALSE);
      EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_SCALEY), FALSE);
   }
   return TRUE;
   break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      switch (LOWORD(wParam))
      {
         case IDC_CHECK_SCALE_CENTER:
         {
            switch (HIWORD(wParam))
            {
            case BN_CLICKED:
            {
               if (SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_GETCHECK, 0, 0) != BST_CHECKED)
               {
                  SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(g_pvp->m_mouseCursorPosition.x).c_str());
                  SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(g_pvp->m_mouseCursorPosition.y).c_str());
               }
               else
               {
                  const Vertex2D v = psel->GetCenter();
                  SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(v.x).c_str());
                  SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(v.y).c_str());
               }
               break;
            }
            default:
               break;
            }
         }
      default:
         break;
      }

      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
         {
            if (m_applyCount == 0)
            {
               char szT[256];
               GetDlgItemText(hwndDlg, IDC_SCALEFACTOR, szT, 255);
               const float fx = sz2f(szT);
               const size_t checked = SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_GETCHECK, 0, 0);
               float fy;
               if (checked)
               {
                  fy = fx;
               }
               else
               {
                  GetDlgItemText(hwndDlg, IDC_SCALEY, szT, 255);
                  fy = sz2f(szT);
               }

               GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
               Vertex2D v;
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
               v.y = sz2f(szT);

               const bool useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);
               //pihdp->ScalePoints(fx, fy, &v);
               psel->Scale(fx, fy, v, useElementCenter);
            }
            EndDialog(hwndDlg, TRUE);
            break;
         }
         case IDC_SCALE_APPLY_BUTTON:
         {
            m_applyCount++;
            char szT[256];
            GetDlgItemText(hwndDlg, IDC_SCALEFACTOR, szT, 255);
            const float fx = sz2f(szT);
            const size_t checked = SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_GETCHECK, 0, 0);
            float fy;
            if (checked)
            {
               fy = fx;
            }
            else
            {
               GetDlgItemText(hwndDlg, IDC_SCALEY, szT, 255);
               fy = sz2f(szT);
            }

            GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
            Vertex2D v;
            v.x = sz2f(szT);
            GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
            v.y = sz2f(szT);

            const bool useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);

            //pihdp->ScalePoints(fx, fy, &v);
            psel->Scale(fx, fy, v, useElementCenter);
            psel->GetPTable()->SetDirtyDraw();
            break;
         }
         case IDC_SCALE_UNDO_BUTTON:
         {
            if (m_applyCount > 0)
            {
               m_applyCount--;
               psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            break;
         }
         case IDCANCEL:
            if (m_applyCount > 0)
            {
               for (int i = 0; i < m_applyCount; i++)
                  psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            EndDialog(hwndDlg, FALSE);
            break;

         case IDC_SQUARE:
         {
            const size_t checked = SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_GETCHECK, 0, 0);
            EnableWindow(GetDlgItem(hwndDlg, IDC_SCALEY), checked != BST_CHECKED);
            EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_SCALEY), checked != BST_CHECKED);
         }
         break;
         }
         break;
      }
      break;
   }

   return FALSE;
}


TranslatePointsDialog::TranslatePointsDialog(ISelect *psel)
{
   DialogBoxParam(
      g_pvp->theInstance,
      MAKEINTRESOURCE(IDD_TRANSLATE),
      g_pvp->GetHwnd(),
      TranslateProc,
      (size_t)psel);
}

int TranslatePointsDialog::m_applyCount = 0;

INT_PTR CALLBACK TranslatePointsDialog::TranslateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ISelect *psel;

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      m_applyCount = 0;
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      SetDlgItemText(hwndDlg, IDC_OFFSETX, f2sz(0.f).c_str());
      SetDlgItemText(hwndDlg, IDC_OFFSETY, f2sz(0.f).c_str());
   }
   return TRUE;
   break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
         {
            if (m_applyCount == 0)
            {
               Vertex2D v;
               char szT[256];
               GetDlgItemText(hwndDlg, IDC_OFFSETX, szT, 255);
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_OFFSETY, szT, 255);
               v.y = sz2f(szT);
               psel->Translate(v);
            }
            EndDialog(hwndDlg, TRUE);
            break;
         }
         case IDC_TRANSLATE_APPLY_BUTTON:
         {
            m_applyCount++;
            Vertex2D v;
            char szT[256];
            GetDlgItemText(hwndDlg, IDC_OFFSETX, szT, 255);
            v.x = sz2f(szT);
            GetDlgItemText(hwndDlg, IDC_OFFSETY, szT, 255);
            v.y = sz2f(szT);
            psel->Translate(v);
            psel->GetPTable()->SetDirtyDraw();
            break;
         }
         case IDC_TRANSLATE_UNDO_BUTTON:
         {
            if (m_applyCount > 0)
            {
               m_applyCount--;
               psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            break;
         }
         case IDCANCEL:
            if (m_applyCount > 0)
            {
               for (int i = 0; i < m_applyCount; i++)
                  psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            EndDialog(hwndDlg, FALSE);
            break;
         }
         break;
      }
      break;
   }

   return FALSE;
}


}