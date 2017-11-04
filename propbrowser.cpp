#include "StdAfx.h"

#define DISPID_FAKE_NAME 30000

enum
{
   eNotControl,
   eEdit,
   eButton,
   eCombo,
   eColor,
   eFont,
   eSlider
};

#define EXPANDO_EXPAND			WM_USER+100
#define EXPANDO_COLLAPSE		WM_USER+101

#define EXPANDO_X_OFFSET 0 /*3*/
#define EXPANDO_Y_OFFSET 30

#define EXPANDOHEIGHT 30

LRESULT CALLBACK SBFrameProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ColorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FontProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ExpandoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

SmartBrowser::SmartBrowser()
{
   m_hwndFrame = NULL;
   m_olddialog = -1;
   m_pvsel = NULL;
   m_maxdialogwidth = 20;
   m_szHeaderCollection[0] = 0;
   InitializeCriticalSection(&m_hPropertyLock);
}

SmartBrowser::~SmartBrowser()
{
   if (m_pvsel)
   {
      delete m_pvsel;
      m_pvsel = NULL;
   }

   DestroyWindow(m_hwndFrame);
   DeleteObject(m_hfontHeader);

   FreePropPanes();
   DeleteCriticalSection(&m_hPropertyLock);
}

void SmartBrowser::Init(HWND hwndParent)
{
   WNDCLASSEX wcex;
   ZeroMemory(&wcex, sizeof(WNDCLASSEX));
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style = CS_DBLCLKS;//CS_NOCLOSE | CS_OWNDC;
   wcex.lpfnWndProc = (WNDPROC)SBFrameProc;
   wcex.hInstance = g_hinst;
   wcex.hIcon = NULL;
   wcex.lpszClassName = "Properties";
   wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcex.lpszMenuName = NULL;
   wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
   RegisterClassEx(&wcex);

   if (g_pvp->m_fPropertiesFloating)
   {
      RECT rectParent;
      GetWindowRect(hwndParent, &rectParent);
      int x = rectParent.right - eSmartBrowserWidth - 20;
      int y = 40;
      int height = (rectParent.bottom - rectParent.top) - 100;

      m_hwndFrame = CreateWindowEx(0, "Properties", "Properties",
         WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_CAPTION,
         x, y, eSmartBrowserWidth, height, hwndParent, NULL, g_hinst, this);
   }
   else
   {
      m_hwndFrame = CreateWindowEx(0, "Properties", "Properties",
         WS_CHILD | WS_BORDER,
         10, 0, 150, 500, hwndParent, NULL, g_hinst, this);
   }

   m_hfontHeader = CreateFont(-18, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
      ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

   m_hfontHeader2 = CreateFont(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
      ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

   // Register expando window
   wcex.style = CS_DBLCLKS | CS_NOCLOSE; //| CS_OWNDC;
   wcex.lpfnWndProc = (WNDPROC)ExpandoProc;
   wcex.lpszClassName = "ExpandoControl";
   wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcex.lpszMenuName = NULL;
   wcex.hbrBackground = NULL;
   RegisterClassEx(&wcex);

   // Register custom controls
   wcex.style = CS_DBLCLKS | CS_NOCLOSE; //| CS_OWNDC;
   wcex.lpfnWndProc = (WNDPROC)ColorProc;
   wcex.lpszClassName = "ColorControl";
   wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcex.lpszMenuName = NULL;
   wcex.hbrBackground = NULL;
   RegisterClassEx(&wcex);

   // Register custom controls
   wcex.lpfnWndProc = (WNDPROC)FontProc;
   wcex.lpszClassName = "FontControl";
   RegisterClassEx(&wcex);
   m_szHeader[0] = '\0';
}

void SmartBrowser::FreePropPanes()
{
   for (int i = 0; i < m_vproppane.Size(); i++)
      delete m_vproppane.ElementAt(i);

   m_vproppane.RemoveAllElements();
}

void SmartBrowser::RemoveSelection(void)
{
   if (m_pvsel)
   {
      delete m_pvsel;
      m_pvsel = NULL;
   }
}

void SmartBrowser::CreateFromDispatch(HWND hwndParent, VectorProtected<ISelect> *pvsel)
{
   //int resourceid;
   ISelect *pisel = NULL;
   FreePropPanes();
   char colName[64] = { 0 };
   Collection *col = NULL;

   EnterCriticalSection(&m_hPropertyLock);
   if (pvsel != NULL)
   {
      ItemTypeEnum maintype = pvsel->ElementAt(0)->GetItemType();
      bool fSame = true;

      // See if all items in multi-select are of the same type.
      // If not, we can't edit their collective properties
      for (int i = 1; i < pvsel->Size(); i++)
      {
         ISelect *pisel3 = pvsel->ElementAt(i);
         if (pisel3)
         {
            if (pisel3->GetItemType() != maintype)
            {
               PropertyPane *pproppane = new PropertyPane(IDD_PROPMULTI, NULL);
               m_vproppane.AddElement(pproppane);
               //resourceid = IDD_PROPMULTI;
               fSame = false;
            }
            bool endLoop = false;
            for (int t = 0; t < g_pvp->m_ptableActive->m_vcollection.Size() && !endLoop; t++)
            {
               Collection *pcol = g_pvp->m_ptableActive->m_vcollection.ElementAt(t);
               if (pcol)
               {
                  for (int k = 0; k < pcol->m_visel.Size(); k++)
                  {
                     ISelect *pisel2 = pcol->m_visel.ElementAt(k);
                     if ((col == NULL) && (pisel2!=NULL) && (pisel2 == pisel3))
                        col = pcol;
                     else if ((col != NULL) && (col == pcol) && (pisel2 != NULL) && (pisel2 == pisel3))
                     {
                        // user selected one or more elements from a collection and some elements aren't in the collection
                        endLoop = true;
                        break;
                     }
                  }
               }
            }
            if (!endLoop && col != NULL)
            {
               WideCharToMultiByte(CP_ACP, 0, col->m_wzName, -1, colName, 64, NULL, NULL);
            }
         }
      }

      if (fSame)
      {
         pisel = pvsel->ElementAt(0);
         if (pisel)
            pisel->GetDialogPanes(&m_vproppane);
      }
   }

   const int propID = (m_vproppane.Size() > 0) ? m_vproppane.ElementAt(0)->dialogid : -1;

   // Optimized for selecting the same object
   // Have to check resourceid too, since there can be more than one dialog view of the same object (table/backdrop)
   if (pvsel && m_pvsel && (pvsel->Size() == m_pvsel->Size()) /*m_pisel == pisel*/ && (m_olddialog == propID))
   {
      bool fSame = fTrue;
      for (int i = 0; i < pvsel->Size(); i++)
      {
         if (pvsel->ElementAt(i) != m_pvsel->ElementAt(i))
         {
            fSame = false;
            break;
         }
      }
      if (fSame)
      {
         LeaveCriticalSection(&m_hPropertyLock);
         return;
      }
   }

   m_olddialog = propID;

   if (m_vhwndExpand.size() > 0)
   {
      // Stop focus from going to no-man's land if focus is in dialog
      HWND hwndFocus = GetFocus();
      do
      {
         if (hwndFocus == m_hwndFrame)
            SetFocus(hwndParent);
      } while ((hwndFocus = GetParent(hwndFocus)) != NULL);

      for (unsigned i = 0; i < m_vhwndExpand.size(); i++)
         DestroyWindow(m_vhwndExpand[i]);

      m_vhwndExpand.clear();
      m_vhwndDialog.clear(); // Dialog windows will have been destroyed along with their parent expando window
   }

   if (m_pvsel)
   {
      delete m_pvsel;
      m_pvsel = NULL;
   }

   if (pvsel)
   {
      m_pvsel = new VectorProtected<ISelect>();
      pvsel->Clone(m_pvsel);
   }

   if (m_vproppane.Size() == 0)
   {
      m_szHeader[0] = '\0';
      InvalidateRect(m_hwndFrame, NULL, fTrue);
      LeaveCriticalSection(&m_hPropertyLock);
      return;
   }

   if (pisel)
   {
      CComBSTR bstr;
      pisel->GetTypeName(&bstr);

      char szTemp[64];

      WideCharToMultiByte(CP_ACP, 0, bstr, -1, szTemp, 64, NULL, NULL);

      m_szHeaderCollection[0] = 0;
      //char szNum[64];
      if (pvsel->Size() > 1)
      {
         if (col)
         {
            sprintf_s(m_szHeaderCollection, "%s", colName);
         }
         sprintf_s(m_szHeader, "%s(%d)", szTemp, pvsel->Size());
      }
      else
         lstrcpy(m_szHeader, szTemp);
   }
   else
      m_szHeader[0] = '\0';

   InvalidateRect(m_hwndFrame, NULL, fTrue);

   for (int i = 0; i < m_vproppane.Size(); i++)
   {
      PropertyPane * const pproppane = m_vproppane.ElementAt(i);
      ExpandoInfo * const pexinfo = new ExpandoInfo();
      pexinfo->m_id = i;
      pexinfo->m_fExpanded = fFalse;
      pexinfo->m_psb = this;

      LocalString ls(pproppane->titlestringid);
      char *szCaption = ls.m_szbuffer;
      pexinfo->m_fHasCaption = (pproppane->titlestringid != 0);
      HWND hwndExpand = NULL;
      if (g_pvp->m_fPropertiesFloating)
      {
         hwndExpand = CreateWindowEx(WS_EX_TOOLWINDOW, "ExpandoControl", szCaption,
            WS_CHILD /*| WS_VISIBLE *//*| WS_BORDER*/,
            2, EXPANDO_Y_OFFSET, eSmartBrowserWidth - 5, 300, m_hwndFrame, NULL, g_hinst, pexinfo);
      }
      else
      {
         hwndExpand = CreateWindowEx(WS_EX_TOOLWINDOW, "ExpandoControl", szCaption,
            WS_CHILD /*| WS_VISIBLE *//*| WS_BORDER*/,
            2, EXPANDO_Y_OFFSET, 150 - 5, 300, m_hwndFrame, NULL, g_hinst, pexinfo);
      }

      m_vhwndExpand.push_back(hwndExpand);

      HWND hwndDialog;
      if (pproppane->dialogid != 0)
         hwndDialog = CreateDialogParam(g_hinst, MAKEINTRESOURCE(pproppane->dialogid),
         hwndExpand, PropertyProc, (size_t)this);
      else
         hwndDialog = CreateDialogIndirectParam(g_hinst, pproppane->ptemplate,
         hwndExpand, PropertyProc, (size_t)this);

      m_vproppane.ElementAt(i)->dialogHwnd = hwndDialog;
      m_vhwndDialog.push_back(hwndDialog);

      RECT rcDialog;
      GetWindowRect(hwndDialog, &rcDialog);
      pexinfo->m_dialogheight = rcDialog.bottom - rcDialog.top;

      // A little hack - if we have multi-select, we know the Name property
      // can never be use.  Disable it to make that easy to understand for the
      // user.
      if (m_pvsel && m_pvsel->Size() > 1)
      {
         HWND hwndName = GetDlgItem(hwndDialog, DISPID_FAKE_NAME);
         if (hwndName)
            EnableWindow(hwndName, FALSE);
      }
   }

   LayoutExpandoWidth();

   for (unsigned i = 0; i < m_vhwndExpand.size(); i++)
   {
      SendMessage(m_vhwndExpand[i], EXPANDO_EXPAND, 0, 0);
      ShowWindow(m_vhwndExpand[i], SW_SHOWNOACTIVATE);
   }

   if (pisel && pisel->GetItemType() == eItemTable)
   {
      const int data = GetBaseISel()->GetPTable()->GetDetailLevel();

      SendMessage(hwndParent, TBM_SETRANGE, fTrue, MAKELONG(0, 10));
      SendMessage(hwndParent, TBM_SETTICFREQ, 1, 0);
      SendMessage(hwndParent, TBM_SETLINESIZE, 0, 1);
      SendMessage(hwndParent, TBM_SETPAGESIZE, 0, 1);
      SendMessage(hwndParent, TBM_SETTHUMBLENGTH, 5, 0);
      SendMessage(hwndParent, TBM_SETPOS, TRUE, data);
   }

   // expand bottom last
   //for (int i=0;i<m_vhwndExpand.Size();i++) SendMessage(m_vhwndExpand.ElementAt(i), EXPANDO_EXPAND, 1, 0); 

   //expand top last
   for (int i = (int)m_vhwndExpand.size() - 1; i >= 0; --i)
      SendMessage(m_vhwndExpand[i], EXPANDO_EXPAND, 1, 0);

   if (m_pvsel)
   {
      for (int i = 0; i < m_pvsel->Size(); i++)
      {
         ISelect *pisel2 = m_pvsel->ElementAt(i);
         if ( pisel2 )
            pisel2->UpdatePropertyPanes();
      }
   }
   LeaveCriticalSection(&m_hPropertyLock);

}

BOOL CALLBACK EnumChildInitList(HWND hwnd, LPARAM lParam)
{
   SmartBrowser *const psb = (SmartBrowser *)lParam;

   if (psb == NULL)
      return FALSE;

   char szName[256];
   GetClassName(hwnd, szName, 256);

   int type = eNotControl;

   if (!strcmp(szName, "ComboBox"))
      type = eCombo;

   if (type == eNotControl)
      return TRUE;

   int dispid = GetDlgCtrlID(hwnd);

   //case eCombo:
   {
      CALPOLESTR     castr;
      CADWORD        cadw;
      IPerPropertyBrowsing *pippb;
      char szT[512];

      if (psb->GetBaseIDisp() == NULL)
         return FALSE;

      psb->GetBaseIDisp()->QueryInterface(IID_IPerPropertyBrowsing, (void **)&pippb);

      BOOL fGotStrings = fFalse;

      if (pippb)
      {
         HRESULT hr = pippb->GetPredefinedStrings(dispid, &castr, &cadw);

         if (hr == S_OK)
         {
            fGotStrings = fTrue;

            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0); // So we know later whether to set the property as a string or a number from itemdata

            SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

            for (ULONG i = 0; i < castr.cElems; i++)
            {
               WideCharToMultiByte(CP_ACP, 0, castr.pElems[i], -1, szT, 512, NULL, NULL);
               const size_t index = SendMessage(hwnd, CB_ADDSTRING, 0, (size_t)szT);
               SendMessage(hwnd, CB_SETITEMDATA, index, (DWORD)cadw.pElems[i]);
            }

            CoTaskMemFree((void *)cadw.pElems);

            for (ULONG i = 0; i < castr.cElems; i++)
               CoTaskMemFree((void *)castr.pElems[i]);

            CoTaskMemFree((void *)castr.pElems);
         }

         pippb->Release();
      }

      if (!fGotStrings)
      {
         SetWindowLongPtr(hwnd, GWLP_USERDATA, 1); // So we know later whether to set the property as a string or a number from itemdata

         ITypeInfo *piti;
         psb->GetBaseIDisp()->GetTypeInfo(0, 0x409, &piti);

         TYPEATTR *pta;
         piti->GetTypeAttr(&pta);

         const int cfunc = pta->cFuncs;

         for (int i = 0; i < cfunc; i++)
         {
            FUNCDESC *pfd;
            piti->GetFuncDesc(i, &pfd);

            if (pfd->memid == dispid && pfd->invkind == INVOKE_PROPERTYGET)
            {
               // We found the function that this
               // dialog control references
               // Figure out what type the get function returns -
               // Probably an enum (since that's all this supports for now)
               // Then get the TypeInfo for the enum and loop
               // through the names and values, and add them to
               // the combo box
               ITypeInfo *pitiEnum;
               piti->GetRefTypeInfo(pfd->elemdescFunc.tdesc.hreftype, &pitiEnum);
               if (pitiEnum)
               {
                  TYPEATTR *ptaEnum;
                  pitiEnum->GetTypeAttr(&ptaEnum);

                  const int cenum = ptaEnum->cVars;

                  for (int l = 0; l < cenum; l++)
                  {
                     VARDESC *pvd;
                     pitiEnum->GetVarDesc(l, &pvd);

                     // Get Name
                     {
                        BSTR * const rgstr = (BSTR *)CoTaskMemAlloc(6 * sizeof(BSTR *));

                        unsigned int cnames;
                        /*const HRESULT hr =*/ pitiEnum->GetNames(pvd->memid, rgstr, 6, &cnames);

                        // Add enum string to combo control
                        WideCharToMultiByte(CP_ACP, 0, rgstr[0], -1, szT, 512, NULL, NULL);
                        const size_t index = SendMessage(hwnd, CB_ADDSTRING, 0, (size_t)szT);
                        SendMessage(hwnd, CB_SETITEMDATA, index, V_I4(pvd->lpvarValue));

                        for (unsigned int i2 = 0; i2 < cnames; i2++)
                           SysFreeString(rgstr[i2]);

                        CoTaskMemFree(rgstr);
                     }

                     pitiEnum->ReleaseVarDesc(pvd);
                  }

                  piti->ReleaseTypeAttr(ptaEnum);

                  pitiEnum->Release();
               }
            }

            piti->ReleaseFuncDesc(pfd);
         }

         piti->ReleaseTypeAttr(pta);

         piti->Release();
      }
   }

   return TRUE;
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
   SmartBrowser * const psb = (SmartBrowser *)lParam;

   psb->GetControlValue(hwnd);

   return TRUE;
}

void SmartBrowser::PopulateDropdowns()
{
   for (unsigned i = 0; i < m_vhwndDialog.size(); i++)
      EnumChildWindows(m_vhwndDialog[i], EnumChildInitList, (size_t)this);
}

void SmartBrowser::RefreshProperties()
{
   for (unsigned i = 0; i < m_vhwndDialog.size(); i++)
      EnumChildWindows(m_vhwndDialog[i], EnumChildProc, (size_t)this);
}

void SmartBrowser::SetVisible(BOOL fVisible)
{
   ShowWindow(m_hwndFrame, fVisible ? SW_SHOW : SW_HIDE);
}

BOOL SmartBrowser::GetVisible()
{
   return IsWindowVisible(m_hwndFrame);
}

void SmartBrowser::DrawHeader(HDC hdc)
{
   char szText[256];
   HFONT hfontOld;

   SetTextAlign(hdc, TA_CENTER);

   SetBkMode(hdc, TRANSPARENT);
   if (m_szHeaderCollection[0] == 0)
   {
      hfontOld = (HFONT)SelectObject(hdc, m_hfontHeader);
      strcpy_s(szText, sizeof(szText), m_szHeader);
      ExtTextOut(hdc, m_maxdialogwidth >> 1, 0, 0, NULL, szText, lstrlen(szText), NULL);
   }
   else
   {
      hfontOld = (HFONT)SelectObject(hdc, m_hfontHeader2);
      strcpy_s(szText, sizeof(szText), m_szHeaderCollection);
      ExtTextOut(hdc, m_maxdialogwidth >> 1, 0, 0, NULL, szText, lstrlen(szText), NULL);
      strcpy_s(szText, sizeof(szText), m_szHeader);
      ExtTextOut(hdc, m_maxdialogwidth >> 1, 14, 0, NULL, szText, lstrlen(szText), NULL);
   }

   SelectObject(hdc, hfontOld);
}

void SmartBrowser::GetControlValue(HWND hwndControl)
{
   bool fNinch = false;
   char szName[256];
   GetClassName(hwndControl, szName, 256);
   IDispatch * const pdisp = GetBaseIDisp();
   int type = eNotControl;

   if (pdisp == NULL)
      return;

   if (!strcmp(szName, "Edit"))
   {
      type = eEdit;

      HWND hwndParent = GetParent(hwndControl);
      char szParentName[256];
      GetClassName(hwndParent, szParentName, 256);
      if (!strcmp(szParentName, "ComboBox"))
         type = eNotControl; // Ignore edit boxes which are part of a combo-box
   }
   else if (!strcmp(szName, "Button"))
      type = eButton;
   else if (!strcmp(szName, "ComboBox"))
      type = eCombo;
   else if (!strcmp(szName, "ColorControl"))
      type = eColor;
   else if (!strcmp(szName, "FontControl"))
      type = eFont;
   else if (!strcmp(szName, "msctls_trackbar32") || !strcmp(szName, "IDC_ALPHA_SLIDER") || !strcmp(szName, "IDC_DAYNIGHT_SLIDER"))
      type = eSlider;

   if (type == eNotControl)
      return;

   DISPPARAMS dispparams = {
      NULL,
      NULL,
      0,
      0
   };

   // Get value of first object in multi-select
   // If there is only one object in list, we just end up using that value

   DISPID dispid = GetDlgCtrlID(hwndControl);

   // We use a fake name id in the property browser since the official OLE
   // name id is greater than 0xffff, which doesn't work as a
   // dialog control id on Win9x.
   if (dispid == DISPID_FAKE_NAME)
      dispid = 0x80010000;

   CComVariant var, varResult;
   HRESULT hr = pdisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dispparams, &var, NULL, NULL);

   // Check each selection in a multiple selection and see if everything
   // has the same value.  If not, set a ninched state to the control
   for (int i = 1; i < m_pvsel->Size(); i++)
   {
      CComVariant varCheck;

      hr = m_pvsel->ElementAt(i)->GetDispatch()->Invoke(
         dispid, IID_NULL,
         LOCALE_USER_DEFAULT,
         DISPATCH_PROPERTYGET,
         &dispparams, &varCheck, NULL, NULL);

      HRESULT hrEqual;
      if (var.vt == 19)
      {
         // Special case OLE_COLOR because the built-in variant
         // comparer doesn't know how to deal with it
         hrEqual = (var.lVal == varCheck.lVal) ? VARCMP_EQ : VARCMP_LT;
      }
      else
      {
         hrEqual = VarCmp(&var, &varCheck, 0x409, 0);
      }

      if (hrEqual != VARCMP_EQ)
      {
         fNinch = true;
         break;
      }
   }

   switch (type)
   {
   case eEdit:
   {
      // If the control has focus, note the selection status.  If all
      // of the text is currently selected, note this so that we can
      // re-select the text when we're done.  This is important when
      // tabbing between controls in a property sheet - making a change
      // in one control and then tabbing to the next triggers a refresh
      // of the whole property page, including the next control we're
      // tabbing into.  The value refresh that we do here cancels the
      // selection status.  So the effect is that we break the standard
      // Windows behavior of selecting the text in the new control we're
      // tabbing into.  We can fix this by explicitly re-selecting the
      // text when we're done if it was selected to begin with.
      bool reSel = false;
      if (GetFocus() == hwndControl)
      {
         DWORD a, b;
         SendMessage(hwndControl, EM_GETSEL, (WPARAM)&a, (WPARAM)&b);
         LRESULT len = SendMessage(hwndControl, WM_GETTEXTLENGTH, 0, 0);
         if (a == 0 && b == len)
            reSel = true;
      }

      if (!fNinch)
      {
         if (SUCCEEDED(VariantChangeType(&varResult, &var, 0, VT_BSTR)))
         {
            WCHAR *wzT;
            wzT = V_BSTR(&varResult);

            char szT[512 + 1];

            WideCharToMultiByte(CP_ACP, 0, wzT, -1, szT, 512, NULL, NULL);

            SetWindowText(hwndControl, szT);
            VariantClear(&varResult);
         }
      }
      else
         SetWindowText(hwndControl, "");

      // re-select the text if it was selected on entry
      if (reSel)
         SendMessage(hwndControl, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
   }
   break;

   case eButton:
   {
      if (!fNinch)
      {
         if (SUCCEEDED(VariantChangeType(&varResult, &var, 0, VT_BOOL)))
         {
            BOOL fCheck = V_BOOL(&varResult);

            SendMessage(hwndControl, BM_SETCHECK, fCheck ? BST_CHECKED : BST_UNCHECKED, 0);
            VariantClear(&varResult);
         }

      }
      else
         SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);
   }
   break;

   case eColor:
   {
      if (SUCCEEDED(VariantChangeType(&varResult, &var, 0, VT_I4)))
      {
         SendMessage(hwndControl, CHANGE_COLOR, (!fNinch) ? 0 : 1, V_I4(&varResult));
         VariantClear(&varResult);
      }
   }
   break;

   case eFont:
   {
      SendMessage(hwndControl, CHANGE_FONT, (!fNinch) ? 0 : 1, (size_t)V_DISPATCH(&var));
      break;
   }

   case eCombo:
   {
      if (!fNinch)
      {
         char szT[512];

         const LONG_PTR style = GetWindowLongPtr(hwndControl, GWL_STYLE);

         if (SUCCEEDED(VariantChangeType(&varResult, &var, 0, VT_BSTR)))
         {
            WCHAR *wzT;
            wzT = V_BSTR(&varResult);

            WideCharToMultiByte(CP_ACP, 0, wzT, -1, szT, 512, NULL, NULL);
         }
         VariantClear(&varResult);


         if (style & CBS_DROPDOWNLIST)
         {
            // Entry must be on the list - search for existing match

            const size_t index = SendMessage(hwndControl, CB_FINDSTRINGEXACT, ~0u, (LPARAM)szT);

            if (index == CB_ERR)
            {
               // No string - look for numerical match in itemdata
               if (SUCCEEDED(VariantChangeType(&varResult, &var, 0, VT_INT)))
               {
                  const size_t data = V_INT(&varResult);
                  const size_t citems = SendMessage(hwndControl, CB_GETCOUNT, 0, 0);

                  for (size_t i = 0; i < citems; i++)
                  {
                     const size_t matchdata = SendMessage(hwndControl, CB_GETITEMDATA, i, 0);

                     if (data == matchdata)
                     {
                        SendMessage(hwndControl, CB_SETCURSEL, i, 0);
                        break;
                     }
                  }
                  VariantClear(&varResult);
               }

            }
            else
               SendMessage(hwndControl, CB_SETCURSEL, index, 0);
         }
         else
            SendMessage(hwndControl, WM_SETTEXT, 0, (LPARAM)szT);
      } // !fNinch
   }
   break;

   case eSlider:
   {
      if (SUCCEEDED(VariantChangeType(&varResult, &var, 0, VT_I4)))
      {
         if (dispid == IDC_ALPHA_SLIDER)
         {
            int data = V_INT(&varResult);
            // get the range value
            //int range = (data & 0x0000FF00) >> 8;
            // mask off the range from the data
            data &= 0x000000FF;

            SendMessage(hwndControl, TBM_SETRANGE, fTrue, MAKELONG(0, 10));
            SendMessage(hwndControl, TBM_SETTICFREQ, 1, 0);
            SendMessage(hwndControl, TBM_SETLINESIZE, 0, 1);
            SendMessage(hwndControl, TBM_SETPAGESIZE, 0, 1);
            SendMessage(hwndControl, TBM_SETTHUMBLENGTH, 5, 0);
            SendMessage(hwndControl, TBM_SETPOS, TRUE, data);
         }
         else if (dispid == IDC_DAYNIGHT_SLIDER)
         {
            int data = V_INT(&varResult);
            // get the range value
            //int range = (data & 0x0000FF00) >> 8;
            // mask off the range from the data
            data &= 0x000000FF;

            SendMessage(hwndControl, TBM_SETRANGE, fTrue, MAKELONG(2, 100));
            SendMessage(hwndControl, TBM_SETTICFREQ, 10, 0);
            SendMessage(hwndControl, TBM_SETLINESIZE, 0, 1);
            SendMessage(hwndControl, TBM_SETPAGESIZE, 0, 1);
            SendMessage(hwndControl, TBM_SETTHUMBLENGTH, 5, 0);
            SendMessage(hwndControl, TBM_SETPOS, TRUE, data);
         }
         else
         {
            int data = V_INT(&varResult);
            // get the range value
            //int range = (data & 0x0000FF00) >> 8;
            //if (range == 0) range = 100;
            // mask off the range from the data
            data &= 0x000000FF;

            // sliders hold a value between 0 and 100 in steps of 1
            SendMessage(hwndControl, TBM_SETRANGE, fTrue, MAKELONG(0, 100)); //0 - 100 range
            SendMessage(hwndControl, TBM_SETTICFREQ, 10, 0); //tic mark frequency
            SendMessage(hwndControl, TBM_SETLINESIZE, 0, 1); //number of positions to move per keypress
            SendMessage(hwndControl, TBM_SETPAGESIZE, 0, 1); //number of positions to move per click
            SendMessage(hwndControl, TBM_SETTHUMBLENGTH, 0, 0); //ignored
            SendMessage(hwndControl, TBM_SETPOS, (!fNinch) ? 1 : 0, data);
         }
      }
      VariantClear(&varResult);
   }
   break;
   }
}

void SmartBrowser::SetProperty(int dispid, VARIANT *pvar, BOOL fPutRef)
{
   if (m_pvsel == NULL)
      return;

   DISPID mydispid = DISPID_PROPERTYPUT;
   DISPPARAMS disp;
   disp.cNamedArgs = 1;
   disp.rgdispidNamedArgs = &mydispid;//NULL;
   disp.cArgs = 1;
   disp.rgvarg = pvar;

   for (int i = 0; i < m_pvsel->Size(); i++)
   {
      /*const HRESULT hr =*/ m_pvsel->ElementAt(i)->GetDispatch()->Invoke(dispid,
         IID_NULL,
         LOCALE_USER_DEFAULT,
         fPutRef ? DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT,
         &disp,
         NULL, NULL, NULL);
   }
}

void SmartBrowser::LayoutExpandoWidth()
{
   int maxwidth = 20; // Just in case we have a weird situation where there are no dialogs
   for (unsigned i = 0; i < m_vhwndDialog.size(); i++)
   {
      HWND hwnd = m_vhwndDialog[i];
      RECT rc;
      GetWindowRect(hwnd, &rc);
      maxwidth = max(maxwidth, (rc.right - rc.left));
   }

   for (unsigned i = 0; i < m_vhwndExpand.size(); i++)
   {
      HWND hwndExpand = m_vhwndExpand[i];
      SetWindowPos(hwndExpand, NULL, 0, 0, maxwidth, 20, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);
   }

   m_maxdialogwidth = maxwidth + EXPANDO_X_OFFSET * 2 + 1;

   SendMessage(g_pvp->m_hwnd, WM_SIZE, 0, 0);
}

void SmartBrowser::RelayoutExpandos()
{
   int totalheight = 0;
   for (unsigned i = 0; i < m_vhwndExpand.size(); i++)
   {
      HWND hwndExpand = m_vhwndExpand[i];
      ExpandoInfo *pexinfo = (ExpandoInfo *)GetWindowLongPtr(hwndExpand, GWLP_USERDATA);
      if (pexinfo && pexinfo->m_fExpanded)
         totalheight += pexinfo->m_dialogheight;
      totalheight += EXPANDOHEIGHT;
   }

   RECT rcFrame;
   GetWindowRect(m_hwndFrame, &rcFrame);
   const int maxheight = (rcFrame.bottom - rcFrame.top) - EXPANDO_Y_OFFSET;
   while (totalheight > maxheight)
   {
      // The total height of our expandos is taller than our window
      // We have to shrink some
      int indexBest = -1;
      int prioBest = 0xffff;
      int cnt = 0;

      // Loop through all expandos.  Find the one which is currently expanded
      // and is the lowest on the priority list (the one that was opened the
      // longest time ago)
      // If nothing has ever been expanded by the user, higher panels will
      // get priority over lower panels
      for (unsigned i = 0; i < m_vhwndExpand.size(); i++)
      {
         const int titleid = m_vproppane.ElementAt(i)->titlestringid;
         if (titleid != NULL)
         {
            HWND hwndExpand = m_vhwndExpand[i];
            RECT rc;
            GetWindowRect(hwndExpand, &rc);
            if ((rc.bottom - rc.top) > EXPANDOHEIGHT)
            {
               const int prio = FindIndexOf(m_vproppriority, titleid);
               cnt++;
               if (prio != -1 && prio <= prioBest) //
               {
                  prioBest = prio;
                  indexBest = i;
               }
            }
         }
      }

      if (indexBest != -1 && cnt > 1) //oldest  only the one we expanded
      {
         RECT rc;
         HWND hwndExpand = m_vhwndExpand[indexBest];
         GetWindowRect(hwndExpand, &rc);
         SendMessage(hwndExpand, EXPANDO_COLLAPSE, 0, 0);
         totalheight -= (rc.bottom - rc.top) - EXPANDOHEIGHT;
      }
      else break; // Ugh?  No expandos? Or one panel is larger than maximum.
   }

   totalheight = 0;
   for (unsigned i = 0; i < m_vhwndExpand.size(); i++)
   {
      HWND hwndExpand = m_vhwndExpand[i];
      SetWindowPos(hwndExpand, NULL, EXPANDO_X_OFFSET, totalheight + EXPANDO_Y_OFFSET, 0, 0, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
      RECT rc;
      GetWindowRect(hwndExpand, &rc);
      totalheight += (rc.bottom - rc.top);
   }
}

void SmartBrowser::ResetPriority(int expandoid)
{
   // base prioritys on the title of the property pane
   const int titleid = m_vproppane.ElementAt(expandoid)->titlestringid;
   RemoveFromVector(m_vproppriority, titleid); // Remove this element if it currently exists in our current priority chain
   m_vproppriority.push_back(titleid); // Add it back at the end (top) of the chain
}


INT_PTR CALLBACK PropertyProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   //HRESULT hr;

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      EnumChildWindows(hwndDlg, EnumChildInitList, lParam);

      EnumChildWindows(hwndDlg, EnumChildProc, lParam);

      HWND hwndExpando = GetParent(hwndDlg);
      const int textlength = GetWindowTextLength(hwndExpando);
      const int titleheight = (textlength > 0) ? EXPANDOHEIGHT : 0;

      SetWindowPos(hwndDlg, NULL,
         0, titleheight, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);

      return FALSE;//TRUE;
   }
   break;

   case WM_DESTROY:
   {
      return FALSE;
      break;

   }

   case GET_COLOR_TABLE:
   {
      SmartBrowser * const psb = (SmartBrowser *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      *((unsigned long **)lParam) = psb->GetBaseISel()->GetPTable()->m_rgcolorcustom;
      return TRUE;
   }
   break;

   // a slider has been changed
   case WM_HSCROLL:
   {
      if (lParam != NULL)									// must be scrollbar message
      {
         int nScrollCode = (int)LOWORD(wParam);			// scroll bar value

         if ((nScrollCode == SB_PAGELEFT) ||
            (nScrollCode == SB_LINELEFT) ||
            (nScrollCode == SB_LINERIGHT) ||
            (nScrollCode == SB_LEFT) ||
            (nScrollCode == SB_RIGHT) ||
            (nScrollCode == SB_PAGERIGHT) ||
            (nScrollCode == SB_THUMBPOSITION) ||		// updates for mouse scrollwheel
            (nScrollCode == SB_THUMBTRACK))			// update as long as button is held down
         {
            SmartBrowser *psb = (SmartBrowser *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
            if (psb != NULL)
            {
               size_t nPos = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);

               int dispid = GetDlgCtrlID((HWND)lParam);

               CComVariant var(nPos);

               psb->SetProperty(dispid, &var, FALSE);

               psb->GetControlValue((HWND)lParam);

               psb->RefreshProperties();
            }
         }
      }
   }
   break;

   case WM_COMMAND:
   {
      const int code = HIWORD(wParam);
      int dispid = LOWORD(wParam);

      if (dispid == DISPID_FAKE_NAME)
         dispid = 0x80010000;

      SmartBrowser * const psb = (SmartBrowser *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      if (psb==NULL)
      {
         return FALSE;
      }

      //IDispatch *pdisp = psb->m_pdisp;
      switch (code)
      {
      case EN_KILLFOCUS:
      case CBN_KILLFOCUS:
      {
         HWND hwndEdit;
         size_t fChanged;

         if (code == EN_KILLFOCUS)
         {
            hwndEdit = (HWND)lParam;
            fChanged = SendMessage(hwndEdit, EM_GETMODIFY, 0, 0);
         }
         else
         {
            POINT pt;
            pt.x = 1;
            pt.y = 1;
            hwndEdit = ChildWindowFromPoint((HWND)lParam, pt);
            fChanged = GetWindowLongPtr(hwndEdit, GWLP_USERDATA);
            if (fChanged)
               SetWindowLongPtr(hwndEdit, GWLP_USERDATA, 0);
         }

         if (fChanged)//SendMessage(hwndEdit, EM_GETMODIFY, 0, 0))
         {
            char szText[1024];

            GetWindowText(hwndEdit, szText, 1024);

            CComVariant var(szText);

            psb->SetProperty(dispid, &var, fFalse);

            psb->GetControlValue((HWND)lParam); // If the new value was not valid, re-fill the control with the real value

            psb->RefreshProperties();
         }
      }
      break;

      case CBN_EDITCHANGE:
      {
         POINT pt;
         pt.x = 1;
         pt.y = 1;
         HWND hwndEdit = ChildWindowFromPoint((HWND)lParam, pt);
         SetWindowLongPtr(hwndEdit, GWLP_USERDATA, 1);
      }
      break;

      case BN_CLICKED:
      {
         if (dispid == IDOK)
         {
            //(The user pressed enter)
            //!! BUG!!!!!!
            // If the object has a boolean at dispid 1 (IDOK),
            // This will override the correct behavior
            PostMessage(hwndDlg, WM_NEXTDLGCTL, 0, 0L);
            return TRUE;
         }
         else if (dispid == IDC_LOAD_MESH_BUTTON)
         {
            for (int i = 0; i < psb->m_pvsel->Size(); i++)
               psb->m_pvsel->ElementAt(i)->LoadMesh();
            psb->RefreshProperties();
         }
         else if (dispid == IDC_EXPORT_MESH_BUTTON)
         {
            for (int i = 0; i < psb->m_pvsel->Size(); i++)
               psb->m_pvsel->ElementAt(i)->ExportMesh();
            psb->RefreshProperties();
         }
         else if (dispid == IDC_IMPORT_PHYSICS_BUTTON)
         {
            psb->GetBaseISel()->GetPTable()->ImportPhysics();
         }
         else if (dispid == IDC_EXPORT_PHYSICS_BUTTON)
         {
            psb->GetBaseISel()->GetPTable()->ExportPhysics();
         }
         else
         {
            const size_t state = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);

            const BOOL fChecked = (state & BST_CHECKED) != 0;

            CComVariant var(fChecked);

            psb->SetProperty(dispid, &var, fFalse);
            psb->GetControlValue((HWND)lParam);
         }
         EnterCriticalSection(&psb->m_hPropertyLock);
         for (int i = 0; i < psb->m_pvsel->Size(); i++)
            psb->m_pvsel->ElementAt(i)->UpdatePropertyPanes();
         LeaveCriticalSection(&psb->m_hPropertyLock);

      }
      break;

      case CBN_SELENDOK://CBN_SELCHANGE:
      {
         const size_t sel = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
         const size_t cookie = SendMessage((HWND)lParam, CB_GETITEMDATA, sel, 0);

         const size_t proptype = GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);

         if (proptype == 0)
         {
            IPerPropertyBrowsing *pippb;
            psb->GetBaseIDisp()->QueryInterface(IID_IPerPropertyBrowsing, (void **)&pippb);

            CComVariant var;
            pippb->GetPredefinedValue(dispid, (DWORD)cookie, &var);

            pippb->Release();

            psb->SetProperty(dispid, &var, fFalse);
         }
         else
         {
            // enum value
            CComVariant var((int)cookie);
            psb->SetProperty(dispid, &var, fFalse);
         }

         //SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)"Foo"/*szT*/);
         psb->GetControlValue((HWND)lParam);

         EnterCriticalSection(&psb->m_hPropertyLock);
         for (int i = 0; i < psb->m_pvsel->Size(); i++)
            psb->m_pvsel->ElementAt(i)->UpdatePropertyPanes();
         LeaveCriticalSection(&psb->m_hPropertyLock);
      }
      break;

      case COLOR_CHANGED:
      {
         const size_t color = GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);/*SendMessage((HWND)lParam, WM_GETTEXT, 0, 0);*/

         CComVariant var(color);

         psb->SetProperty(dispid, &var, fFalse);

         psb->GetControlValue((HWND)lParam);
      }
      break;

      case FONT_CHANGED:
      {
         IFontDisp * const pifd = (IFontDisp*)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
         // Addred because the object will release the old one (really this one), before addreffing it again
         pifd->AddRef();
         CComVariant var(pifd);
         psb->SetProperty(dispid, &var, fTrue);
         pifd->Release();
      }
      }
   }
   break;
   }

   return FALSE;
}

LRESULT CALLBACK SBFrameProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_CREATE:
   {
      CREATESTRUCT * const pcs = (CREATESTRUCT *)lParam;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (size_t)pcs->lpCreateParams);
   }
   break;

   case WM_CLOSE:
   {
      if (g_pvp->m_fPropertiesFloating)
      {
         return TRUE;
      }
      break;
   }
   case WM_PAINT:
   {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      SmartBrowser * const psb = (SmartBrowser *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      psb->DrawHeader(hdc);

      EndPaint(hwnd, &ps);
   }
   return 0;
   break;
   }

   return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ColorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_CREATE:
   {
      RECT rc;
      GetClientRect(hwnd, &rc);

      /*const HWND hwndButton =*/ CreateWindow("BUTTON", "Color", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hwnd, NULL, g_hinst, 0);
   }
   break;

   case CHANGE_COLOR:
   {
      size_t color = lParam;
      if (wParam == 1)
         color |= 0x80000000;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, color);
      InvalidateRect(hwnd, NULL, fFalse);
   }
   break;

   case WM_GETTEXT:
   {
      return GetWindowLongPtr(hwnd, GWLP_USERDATA);
   }
   break;

   /*case WM_PAINT:
   {
   HDC hdc;
   PAINTSTRUCT ps;
   hdc = BeginPaint(hwnd,&ps);

   //SmartBrowser *psb = (SmartBrowser *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
   //psb->DrawHeader(hdc);

   HBRUSH hbrush = CreateSolidBrush(RGB(255,0,0));

   HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc, hbrush);

   PatBlt(hdc, 0, 0, 60, 60, PATCOPY);

   SelectObject(hdc, hbrushOld);

   EndPaint(hwnd,&ps);
   }
   return 0;
   break;*/

   case WM_DRAWITEM:
   {
      DRAWITEMSTRUCT * const pdis = (DRAWITEMSTRUCT *)lParam;
      HDC hdc = pdis->hDC;
      int offset = 0;

      DWORD state = DFCS_BUTTONPUSH;
      if (pdis->itemState & ODS_SELECTED)
      {
         state |= DFCS_PUSHED;
         offset = 1;
      }

      DrawFrameControl(hdc, &pdis->rcItem, DFC_BUTTON, state);

      const size_t color = GetWindowLongPtr(hwnd, GWLP_USERDATA);

      if (!(color & 0x80000000)) // normal color, not ninched
      {
         const COLORREF oldcolor = (COLORREF)color & 0xffffff; // have to AND it to get rid of ninch bit
         HBRUSH hbrush = CreateSolidBrush(oldcolor);

         HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc, hbrush);

         PatBlt(hdc, 6 + offset, 6 + offset, pdis->rcItem.right - pdis->rcItem.left - 12, pdis->rcItem.bottom - pdis->rcItem.top - 12, PATCOPY);

         SelectObject(hdc, hbrushOld);

         DeleteObject(hbrush);
      }
   }
   break;

   case WM_COMMAND:
   {
      const int code = HIWORD(wParam);
      switch (code)
      {
      case BN_CLICKED:
      {
         HWND hwndDlg = GetParent(hwnd);
         /*SmartBrowser * const psb =*/ (SmartBrowser *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
         CHOOSECOLOR cc;
         cc.lStructSize = sizeof(CHOOSECOLOR);
         cc.hwndOwner = hwnd;
         cc.hInstance = NULL;
         cc.rgbResult = (COLORREF)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         SendMessage(hwndDlg, GET_COLOR_TABLE, 0, (size_t)&cc.lpCustColors);
         //cc.lpCustColors = (unsigned long *)SendMessage(hwndDlg, GET_COLOR_TABLE, 0, 0);//psb->m_pisel->GetPTable()->m_rgcolorcustom;//cr;
         cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
         cc.lCustData = NULL;
         cc.lpfnHook = NULL;
         cc.lpTemplateName = NULL;
         if (ChooseColor(&cc))
         {
            const int id = GetDlgCtrlID(hwnd);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, cc.rgbResult);
            SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(id, COLOR_CHANGED), (LPARAM)hwnd);
            InvalidateRect(hwnd, NULL, fFalse);
         }
         break;
      }
      }
   }
   }

   return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK FontProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_CREATE:
   {
      RECT rc;
      GetClientRect(hwnd, &rc);
      HWND hwndButton = CreateWindow("BUTTON", "", BS_LEFT | WS_VISIBLE | WS_CHILD, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hwnd, NULL, g_hinst, 0);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);

      HFONT hfontButton = CreateFont(-10, 0, 0, 0, /*FW_NORMAL*/ FW_MEDIUM, FALSE, FALSE, FALSE,
         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
         ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

      SendMessage(hwndButton, WM_SETFONT, (size_t)hfontButton, 0);
   }
   break;

   case WM_DESTROY:
   {
      POINT pt;
      pt.x = 1;
      pt.y = 1;
      HWND hwndButton = ChildWindowFromPoint(hwnd, pt);

      HFONT hfontButton = (HFONT)SendMessage(hwndButton, WM_GETFONT, 0, 0);

      SendMessage(hwndButton, WM_SETFONT, NULL, 0); // Just in case the button needs to use a font before it is destroyed

      if (hfontButton)
         DeleteObject(hfontButton);
   }
   break;

   case CHANGE_FONT:
   {
      SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

      POINT pt;
      pt.x = 1;
      pt.y = 1;
      HWND hwndButton = ChildWindowFromPoint(hwnd, pt);

      IFontDisp * const pifd = (IFontDisp *)lParam;
      IFont *pif;
      pifd->QueryInterface(IID_IFont, (void **)&pif);
      CComBSTR bstrName;
      /*const HRESULT hr =*/ pif->get_Name(&bstrName);
      pif->Release();

      if (wParam != 1) // non-niched value
      {
         char szT[64];
         WideCharToMultiByte(CP_ACP, 0, bstrName, -1, szT, 64, NULL, NULL);
         SetWindowText(hwndButton, szT);
      }
      else
         SetWindowText(hwndButton, "");
   }
   break;

   case WM_GETTEXT:
   {
      return GetWindowLongPtr(hwnd, GWLP_USERDATA);
   }
   break;

   case WM_COMMAND:
   {
      const int code = HIWORD(wParam);
      switch (code)
      {
      case BN_CLICKED:
      {
         CHOOSEFONT cf;
         LOGFONT lf;
         char szstyle[256];

         // Set up logfont to be like our current font
         IFontDisp * const pifd = (IFontDisp *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         IFont *pif;
         pifd->QueryInterface(IID_IFont, (void **)&pif);

         HFONT hfont;
         pif->get_hFont(&hfont);
         GetObject(hfont, sizeof(LOGFONT), &lf);

         pif->Release();

         ZeroMemory(&cf, sizeof(CHOOSEFONT));

         cf.lStructSize = sizeof(CHOOSEFONT);
         cf.hwndOwner = hwnd;
         cf.lpLogFont = &lf;
         cf.lpszStyle = (char *)&szstyle;
         //cf.iPointSize = 120;
         cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
         if (ChooseFont(&cf))
         {
            IFontDisp * const pifd2 = (IFontDisp *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            IFont *pif2;
            pifd2->QueryInterface(IID_IFont, (void **)&pif2);

            CY cy;
            cy.int64 = (cf.iPointSize * 10000 / 10);
            /*HRESULT hr =*/ pif2->put_Size(cy);

            WCHAR wzT[64];
            MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, -1, wzT, 64);
            CComBSTR bstr(wzT);
            /*hr =*/ pif2->put_Name(bstr);

            pif2->put_Weight((short)lf.lfWeight);

            pif2->put_Italic(lf.lfItalic);

            SendMessage(hwnd, CHANGE_FONT, 0, (size_t)pifd2);

            HWND hwndDlg = GetParent(hwnd);
            const int id = GetDlgCtrlID(hwnd);
            SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(id, FONT_CHANGED), (LPARAM)hwnd);

            pif2->Release();
         }
         break;
      }
      }
   }
   }

   return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ExpandoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ExpandoInfo *pexinfo;

   switch (uMsg)
   {
   case WM_CREATE:
   {
      CREATESTRUCT * const pcs = (CREATESTRUCT *)lParam;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (size_t)pcs->lpCreateParams);
      break;
   }

   case WM_DESTROY:
   {
      pexinfo = (ExpandoInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      delete pexinfo;
      break;
   }

   case WM_LBUTTONUP:
   {
      pexinfo = (ExpandoInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

      if (pexinfo->m_fHasCaption) // Null title means not an expando
      {
         //const int xPos = LOWORD(lParam); 
         const int yPos = HIWORD(lParam);
         if (yPos < 16)//wParam == HTCAPTION)
            SendMessage(hwnd, pexinfo->m_fExpanded ? EXPANDO_COLLAPSE : EXPANDO_EXPAND, 1, 0);
      }
   }
   break;

   case EXPANDO_EXPAND:
   {
      pexinfo = (ExpandoInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      pexinfo->m_fExpanded = fTrue;

      int titleheight;
      if (pexinfo->m_fHasCaption) // Null title means not an expando
      {
         titleheight = EXPANDOHEIGHT;
         pexinfo->m_psb->ResetPriority(pexinfo->m_id);
      }
      else
         titleheight = 0;

      SetWindowPos(hwnd, NULL, 0, 0, pexinfo->m_psb->m_maxdialogwidth, titleheight + pexinfo->m_dialogheight, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);

      InvalidateRect(hwnd, NULL, fFalse);

      if (wParam == 1)
         pexinfo->m_psb->RelayoutExpandos();
   }
   break;

   case EXPANDO_COLLAPSE:
   {
      pexinfo = (ExpandoInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      pexinfo->m_fExpanded = fFalse;

      SetWindowPos(hwnd, NULL, 0, 0, pexinfo->m_psb->m_maxdialogwidth, EXPANDOHEIGHT, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);

      InvalidateRect(hwnd, NULL, fFalse);

      if (wParam == 1)
         pexinfo->m_psb->RelayoutExpandos();
   }
   break;

   case WM_SETCURSOR:
   {
      POINT ptCursor;
      GetCursorPos(&ptCursor);
      ScreenToClient(hwnd, &ptCursor);

      if (ptCursor.y < EXPANDOHEIGHT)
      {
         HCURSOR hcursor = LoadCursor(NULL, MAKEINTRESOURCE(32649));

         SetCursor(hcursor);
         return TRUE;
      }
   }
   break;

   case WM_PAINT:
   {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      pexinfo = (ExpandoInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

      //int textlength = GetWindowTextLength(hwnd);

      RECT rc;
      if (pexinfo->m_fHasCaption) // Null title means not an expando
      {
         RECT rcCur;
         GetWindowRect(hwnd, &rcCur);

         rc.left = 0;
         rc.top = 0;
         rc.right = rcCur.right - rcCur.left;
         rc.bottom = EXPANDOHEIGHT;

         DrawCaption(hwnd, hdc, &rc, DC_SMALLCAP | DC_TEXT | 0x0020 | DC_ACTIVE);

         HBITMAP hbmchevron = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_CHEVRON), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
         HDC hdcbmp = CreateCompatibleDC(hdc);
         HBITMAP hbmOld = (HBITMAP)SelectObject(hdcbmp, hbmchevron);
         const int offsetx = (rcCur.bottom - rcCur.top > EXPANDOHEIGHT) ? 18 : 0;
         BitBlt(hdc, rc.right - 20, 0, 18, 18, hdcbmp, offsetx, 0, SRCPAINT);
         SelectObject(hdcbmp, hbmOld);
         DeleteDC(hdcbmp);
         DeleteObject(hbmchevron);
      }

      EndPaint(hwnd, &ps);
   }
   break;
   }

   return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
