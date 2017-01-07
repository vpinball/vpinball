#include "StdAfx.h"

#define VP_REGKEY_GENERAL "Software\\Visual Pinball\\"
#define VP_REGKEY "Software\\Visual Pinball\\VP10\\"

HRESULT GetRegString(const char *szKey, const char *szValue, void *szbuffer, DWORD size)
{
   DWORD type = REG_NONE;
   const HRESULT hr = GetRegValue(szKey, szValue, &type, szbuffer, size);

   if (type != REG_SZ)
      return E_FAIL;

   return hr;
}

HRESULT GetRegStringAsFloat(const char *szKey, const char *szValue, float *pfloat)
{
   DWORD type = REG_NONE;
   char szbuffer[16];
   const HRESULT hr = GetRegValue(szKey, szValue, &type, szbuffer, 16);

   if (type != REG_SZ)
      return E_FAIL;

   const int len = lstrlen(szbuffer);
   if (len == 0)
      return E_FAIL;

   //todo: convert decimal character in string to match regional setting of current machine
   if (szbuffer[0] == '-')
   {
      if (len < 2)
         return E_FAIL;
      *pfloat = (float)atof(&szbuffer[1]);
      *pfloat *= -1.0f;
   }
   else
      *pfloat = (float)atof(&szbuffer[0]);

   return hr;
}

HRESULT GetRegInt(const char *szKey, const char *szValue, int *pint)
{
   DWORD type = REG_NONE;
   const HRESULT hr = GetRegValue(szKey, szValue, &type, (void *)pint, 4);

   if (type != REG_DWORD)
      return E_FAIL;

   return hr;
}

HRESULT GetRegValue(const char *szKey, const char *szValue, DWORD *ptype, void *pvalue, DWORD size)
{
   char szPath[1024];
   if (strcmp(szKey, "Controller") == 0)
      lstrcpy(szPath, VP_REGKEY_GENERAL);
   else
      lstrcpy(szPath, VP_REGKEY);
   lstrcat(szPath, szKey);

   HKEY hk;
   DWORD RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hk);

   if (RetVal == ERROR_SUCCESS)
   {
      DWORD type = REG_NONE;

      RetVal = RegQueryValueEx(hk, szValue, NULL, &type, (BYTE *)pvalue, &size);

      *ptype = type;

      RegCloseKey(hk);
   }

   return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;
}


int GetRegIntWithDefault(const char *szKey, const char *szValue, int def)
{
   int val;
   HRESULT hr = GetRegInt(szKey, szValue, &val);
   return SUCCEEDED(hr) ? val : def;
}

float GetRegStringAsFloatWithDefault(const char *szKey, const char *szValue, float def)
{
   float val;
   HRESULT hr = GetRegStringAsFloat(szKey, szValue, &val);
   return SUCCEEDED(hr) ? val : def;
}

bool GetRegBoolWithDefault(const char *szKey, const char *szValue, bool def)
{
   return !!GetRegIntWithDefault(szKey, szValue, def);
}


HRESULT SetRegValue(const char *szKey, const char *szValue, DWORD type, const void *pvalue, DWORD size)
{
   char szPath[1024];
   if (strcmp(szKey, "Controller") == 0)
      lstrcpy(szPath, VP_REGKEY_GENERAL);
   else
      lstrcpy(szPath, VP_REGKEY);
   lstrcat(szPath, szKey);

   HKEY hk;
   //RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hk);
   DWORD RetVal = RegCreateKeyEx(HKEY_CURRENT_USER, szPath, 0, NULL,
      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk, NULL);

   if (RetVal == ERROR_SUCCESS)
   {
      RetVal = RegSetValueEx(hk, szValue, 0, type, (BYTE *)pvalue, size);

      RegCloseKey(hk);
   }

   return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

HRESULT SetRegValueBool(const char *szKey, const char *szValue, bool val)
{
   DWORD dwval = val ? 1 : 0;
   return SetRegValue(szKey, szValue, REG_DWORD, &dwval, sizeof(DWORD));
}

HRESULT SetRegValueInt(const char *szKey, const char *szValue, int val)
{
   return SetRegValue(szKey, szValue, REG_DWORD, &val, sizeof(DWORD));
}

HRESULT SetRegValueFloat(const char *szKey, const char *szValue, float val)
{
   char buf[16];
   sprintf_s(buf, 16, "%f", val);
   return SetRegValue(szKey, szValue, REG_SZ, buf, lstrlen(buf));
}

HRESULT SetRegValueString(const char *szKey, const char *szValue, const char *val)
{
   return SetRegValue(szKey, szValue, REG_SZ, val, lstrlen(val));
}
