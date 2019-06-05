#include "StdAfx.h"

#define VP_REGKEY_GENERAL "Software\\Visual Pinball\\"
#define VP_REGKEY "Software\\Visual Pinball\\VP10\\"

#ifdef ENABLE_INI
 //!! TODO implement reading/writing to ini file instead of registry
#else
HRESULT LoadValue(const char *szKey, const char *szValue, DWORD *ptype, void *pvalue, DWORD size);


HRESULT LoadValueString(const char * const szKey, const char * const szValue, void *const szbuffer, const DWORD size)
{
   if (size > 0) // clear string in case of reg value being set, but being null string which results in szbuffer being kept as-is
      ((char*)szbuffer)[0] = 0;

   DWORD type = REG_NONE;
   const HRESULT hr = LoadValue(szKey, szValue, &type, szbuffer, size);

   return (type != REG_SZ) ? E_FAIL : hr;
}

HRESULT LoadValueFloat(const char *szKey, const char *szValue, float *pfloat)
{
   DWORD type = REG_NONE;
   char szbuffer[16];
   const HRESULT hr = LoadValue(szKey, szValue, &type, szbuffer, 16);

   if (type != REG_SZ)
      return E_FAIL;

   const int len = lstrlen(szbuffer);
   if (len == 0)
      return E_FAIL;

   char* const fo = strchr(szbuffer, ',');
   if (fo != NULL)
      *fo = '.';

   if (szbuffer[0] == '-')
   {
      if (len < 2)
         return E_FAIL;
      *pfloat = (float)atof(&szbuffer[1]);
      *pfloat = -*pfloat;
   }
   else
      *pfloat = (float)atof(&szbuffer[0]);

   return hr;
}

HRESULT LoadValueInt(const char *szKey, const char *szValue, int *pint)
{
   DWORD type = REG_NONE;
   const HRESULT hr = LoadValue(szKey, szValue, &type, (void *)pint, 4);

   if (type != REG_DWORD)
      return E_FAIL;

   return hr;
}

static HRESULT LoadValue(const char *szKey, const char *szValue, DWORD *ptype, void *pvalue, DWORD size)
{
   char szPath[MAXSTRING];
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


int LoadValueIntWithDefault(const char *szKey, const char *szValue, const int def)
{
   int val;
   const HRESULT hr = LoadValueInt(szKey, szValue, &val);
   return SUCCEEDED(hr) ? val : def;
}

float LoadValueFloatWithDefault(const char *szKey, const char *szValue, const float def)
{
   float val;
   const HRESULT hr = LoadValueFloat(szKey, szValue, &val);
   return SUCCEEDED(hr) ? val : def;
}

bool LoadValueBoolWithDefault(const char *szKey, const char *szValue, const bool def)
{
   return !!LoadValueIntWithDefault(szKey, szValue, def);
}


static HRESULT SaveValue(const char *szKey, const char *szValue, DWORD type, const void *pvalue, const DWORD size)
{
   char szPath[MAXSTRING];
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

HRESULT SaveValueBool(const char *szKey, const char *szValue, const bool val)
{
   const DWORD dwval = val ? 1 : 0;
   return SaveValue(szKey, szValue, REG_DWORD, &dwval, sizeof(DWORD));
}

HRESULT SaveValueInt(const char *szKey, const char *szValue, const int val)
{
   return SaveValue(szKey, szValue, REG_DWORD, &val, sizeof(DWORD));
}

HRESULT SaveValueFloat(const char *szKey, const char *szValue, const float val)
{
   char buf[16];
   sprintf_s(buf, 16, "%f", val);
   return SaveValue(szKey, szValue, REG_SZ, buf, lstrlen(buf));
}

HRESULT SaveValueString(const char *szKey, const char *szValue, const char *val)
{
   return SaveValue(szKey, szValue, REG_SZ, val, lstrlen(val));
}

HRESULT DeleteValue(const char *szKey, const char *szValue)
{
   char szPath[MAXSTRING];
   if (strcmp(szKey, "Controller") == 0)
      lstrcpy(szPath, VP_REGKEY_GENERAL);
   else
      lstrcpy(szPath, VP_REGKEY);
   lstrcat(szPath, szKey);

   HKEY hk;
   DWORD RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hk);

   if (RetVal == ERROR_SUCCESS)
   {
      RetVal = RegDeleteValue(hk, szValue);
      RegCloseKey(hk);
   }
   else {
      return ERROR_SUCCESS; // It is a success if you want to delete something that doesn't exist.
   }

   return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;
}
#endif
