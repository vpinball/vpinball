#include "stdafx.h"
#include "Intshcut.h"

unsigned long long tinymt64state[2] = { 'T', 'M' };

float sz2f(char *sz)
{
   WCHAR wzT[256];
   MultiByteToWideChar(CP_ACP, 0, sz, -1, wzT, 256);

   CComVariant var = wzT;

   if (SUCCEEDED(VariantChangeType(&var, &var, 0, VT_R4)))
   {
      float result = V_R4(&var);
      VariantClear(&var);
      return result;
   }
   return 0.0f;
}

void f2sz(const float f, char *sz)
{
   CComVariant var = f;

   if (SUCCEEDED(VariantChangeType(&var, &var, 0, VT_BSTR)))
   {
      WCHAR *wzT;
      wzT = V_BSTR(&var);

      WideCharToMultiByte(CP_ACP, 0, wzT, -1, sz, 256, NULL, NULL);
      VariantClear(&var);
   }
   else
      sprintf_s(sz, 255, "0.0");

}

void WideStrCopy(WCHAR *wzin, WCHAR *wzout)
{
   while (*wzin) { *wzout++ = *wzin++; }
   *wzout = 0;
}

void WideStrNCopy(WCHAR *wzin, WCHAR *wzout, const DWORD wzoutMaxLen)
{
   DWORD i = 0;
   while (*wzin && (i < wzoutMaxLen - 1)) { *wzout++ = *wzin++; i++; }
   *wzout = 0;
}

void WideStrCat(WCHAR *wzin, WCHAR *wzout)
{
   wzout += lstrlenW(wzout);
   while (*wzin) { *wzout++ = *wzin++; }
   *wzout = 0;
}

int WideStrCmp(WCHAR *wz1, WCHAR *wz2)
{
   while (*wz1 != L'\0')
   {
      if (*wz1 != *wz2)
      {
         if (*wz1 > *wz2)
         {
            return 1; // If *wz2 == 0, then wz1 will return as higher, which is correct
         }
         else if (*wz1 < *wz2)
         {
            return -1;
         }
      }
      wz1++;
      wz2++;
   }
   if (*wz2 != L'\0')
   {
      return -1; // wz2 is longer - and therefore higher
   }
   return 0;
}

int WzSzStrCmp(WCHAR *wz1, char *sz2)
{
   while (*wz1 != L'\0')
   {
      if (*wz1++ != *sz2++)
      {
         return 1;
      }
   }
   if (*sz2 != L'\0')
   {
      return 1;
   }
   return 0;
}

int WzSzStrnCmp(WCHAR *wz1, char *sz2, int count)
{
   int i = 0;

   while (*wz1 != L'\0' && i < count)
   {
      if (*wz1++ != *sz2++)
      {
         return 1;
      }
      i++;
   }
   if (*sz2 != L'\0')
   {
      return 1;
   }
   return 0;
}

LocalString::LocalString(const int resid)
{
   if (resid > 0)
      /*const int cchar =*/ LoadString(g_hinst, resid, m_szbuffer, 256);
   else
      m_szbuffer[0] = 0;
}

LocalStringW::LocalStringW(int resid)
{
   if (resid > 0)
      LoadStringW(g_hinst, resid, this->str, 256);
   else
      str[0] = 0;
}

WCHAR *MakeWide(char *sz)
{
   const int len = lstrlen(sz);
   WCHAR * const wzT = new WCHAR[len + 1];
   MultiByteToWideChar(CP_ACP, 0, sz, -1, wzT, len + 1);

   return wzT;
}

char *MakeChar(WCHAR *wz)
{
   const int len = lstrlenW(wz);
   char * const szT = new char[len + 1];
   WideCharToMultiByte(CP_ACP, 0, wz, -1, szT, len + 1, NULL, NULL);

   return szT;
}

HRESULT OpenURL(char *szURL)
{
   IUniformResourceLocator* pURL;

   HRESULT hres = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**)&pURL);
   if (!SUCCEEDED(hres))
   {
      return hres;
   }

   hres = pURL->SetURL(szURL, IURL_SETURL_FL_GUESS_PROTOCOL);

   if (!SUCCEEDED(hres))
   {
      pURL->Release();
      return hres;
   }

   //Open the URL by calling InvokeCommand
   URLINVOKECOMMANDINFO ivci;
   ivci.dwcbSize = sizeof(URLINVOKECOMMANDINFO);
   ivci.dwFlags = IURL_INVOKECOMMAND_FL_ALLOW_UI;
   ivci.hwndParent = g_pvp->m_hwnd;
   ivci.pcszVerb = "open";
   hres = pURL->InvokeCommand(&ivci);
   pURL->Release();
   return (hres);
}
