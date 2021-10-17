#include "StdAfx.h"

#define VP_REGKEY_GENERAL "Software\\Visual Pinball\\"
#define VP_REGKEY "Software\\Visual Pinball\\VP10\\"

#ifdef ENABLE_INI
 //!! TODO implement reading/writing to ini file instead of registry
#else
static HRESULT LoadValue(const std::string &szKey, const std::string &szValue, DWORD &type, void *pvalue, DWORD size);

HRESULT LoadValue(const std::string& szKey, const std::string& szValue, std::string& buffer)
{
   DWORD type = REG_NONE;
   char szbuffer[MAXSTRING];
   szbuffer[0] = '\0';
   const HRESULT hr = LoadValue(szKey, szValue, type, szbuffer, MAXSTRING);
   buffer = szbuffer;

   return (type != REG_SZ) ? E_FAIL : hr;
}

HRESULT LoadValue(const std::string& szKey, const std::string& szValue, void* const szbuffer, const DWORD size)
{
   if (size > 0) // clear string in case of reg value being set, but being null string which results in szbuffer being kept as-is
      ((char*)szbuffer)[0] = '\0';

   DWORD type = REG_NONE;
   const HRESULT hr = LoadValue(szKey, szValue, type, szbuffer, size);

   return (type != REG_SZ) ? E_FAIL : hr;
}

HRESULT LoadValue(const std::string &szKey, const std::string &szValue, float &pfloat)
{
   DWORD type = REG_NONE;
   char szbuffer[16];
   const HRESULT hr = LoadValue(szKey, szValue, type, szbuffer, 16);

   if (type != REG_SZ)
      return E_FAIL;

   const int len = lstrlen(szbuffer);
   if (len == 0)
      return E_FAIL;

   char* const fo = strchr(szbuffer, ',');
   if (fo != nullptr)
      *fo = '.';

   if (szbuffer[0] == '-')
   {
      if (len < 2)
         return E_FAIL;
      pfloat = (float)atof(&szbuffer[1]);
      pfloat = -pfloat;
   }
   else
      pfloat = (float)atof(szbuffer);

   return hr;
}

HRESULT LoadValue(const std::string &szKey, const std::string &szValue, int &pint)
{
   DWORD type = REG_NONE;
   const HRESULT hr = LoadValue(szKey, szValue, type, (void *)&pint, 4);

   return (type != REG_DWORD) ? E_FAIL : hr;
}

HRESULT LoadValue(const std::string &szKey, const std::string &szValue, unsigned int &pint)
{
   DWORD type = REG_NONE;
   const HRESULT hr = LoadValue(szKey, szValue, type, (void *)&pint, 4);

   return (type != REG_DWORD) ? E_FAIL : hr;
}

static HRESULT LoadValue(const std::string &szKey, const std::string &szValue, DWORD &type, void *pvalue, DWORD size)
{
   char szPath[MAXSTRING];
   if(szKey=="Controller")
      lstrcpy(szPath, VP_REGKEY_GENERAL);
   else
      lstrcpy(szPath, VP_REGKEY);
   lstrcat(szPath, szKey.c_str());

   HKEY hk;
   DWORD RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hk);

   if (RetVal == ERROR_SUCCESS)
   {
      type = REG_NONE;

      RetVal = RegQueryValueEx(hk, szValue.c_str(), nullptr, &type, (BYTE *)pvalue, &size);

      RegCloseKey(hk);
   }

   return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;
}


int LoadValueIntWithDefault(const std::string &szKey, const std::string &szValue, const int def)
{
   int val;
   const HRESULT hr = LoadValue(szKey, szValue, val);
   return SUCCEEDED(hr) ? val : def;
}

float LoadValueFloatWithDefault(const std::string &szKey, const std::string &szValue, const float def)
{
   float val;
   const HRESULT hr = LoadValue(szKey, szValue, val);
   return SUCCEEDED(hr) ? val : def;
}

bool LoadValueBoolWithDefault(const std::string &szKey, const std::string &szValue, const bool def)
{
   return !!LoadValueIntWithDefault(szKey, szValue, def);
}

//

static HRESULT SaveValue(const std::string &szKey, const std::string &szValue, const DWORD type, const void *pvalue, const DWORD size)
{
   char szPath[MAXSTRING];
   if(szKey=="Controller")
      lstrcpy(szPath, VP_REGKEY_GENERAL);
   else
      lstrcpy(szPath, VP_REGKEY);
   lstrcat(szPath, szKey.c_str());

   HKEY hk;
   //RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hk);
   DWORD RetVal = RegCreateKeyEx(HKEY_CURRENT_USER, szPath, 0, nullptr,
      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hk, nullptr);

   if (RetVal == ERROR_SUCCESS)
   {
      RetVal = RegSetValueEx(hk, szValue.c_str(), 0, type, (BYTE *)pvalue, size);

      RegCloseKey(hk);
   }

   return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

HRESULT SaveValueBool(const std::string &szKey, const std::string &szValue, const bool val)
{
   const DWORD dwval = val ? 1 : 0;
   return SaveValue(szKey, szValue, REG_DWORD, &dwval, sizeof(DWORD));
}

HRESULT SaveValueInt(const std::string &szKey, const std::string &szValue, const int val)
{
   return SaveValue(szKey, szValue, REG_DWORD, &val, sizeof(DWORD));
}

HRESULT SaveValueFloat(const std::string &szKey, const std::string &szValue, const float val)
{
   char buf[16];
   sprintf_s(buf, 16, "%f", val);
   return SaveValue(szKey, szValue, REG_SZ, buf, lstrlen(buf));
}

HRESULT SaveValue(const std::string &szKey, const std::string &szValue, const char *val)
{
   return SaveValue(szKey, szValue, REG_SZ, val, lstrlen(val));
}

HRESULT SaveValue(const std::string &szKey, const std::string &szValue, const string& val)
{
   return SaveValue(szKey, szValue, REG_SZ, val.c_str(), (DWORD)val.length());
}

HRESULT DeleteValue(const std::string &szKey, const std::string &szValue)
{
   char szPath[MAXSTRING];
   if(szKey=="Controller")
      lstrcpy(szPath, VP_REGKEY_GENERAL);
   else
      lstrcpy(szPath, VP_REGKEY);
   lstrcat(szPath, szKey.c_str());

   HKEY hk;
   DWORD RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hk);

   if (RetVal == ERROR_SUCCESS)
   {
      RetVal = RegDeleteValue(hk, szValue.c_str());
      RegCloseKey(hk);
   }
   else {
      return ERROR_SUCCESS; // It is a success if you want to delete something that doesn't exist.
   }

   return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;
}
BOOL RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey)
{
   LPTSTR lpEnd;
   LONG lResult;
   DWORD dwSize;
   TCHAR szName[MAX_PATH];
   HKEY hKey;
   FILETIME ftWrite;

   // First, see if we can delete the key without having
   // to recurse.

   lResult = RegDeleteKey(hKeyRoot, lpSubKey);

   if (lResult == ERROR_SUCCESS)
      return TRUE;

   lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

   if (lResult != ERROR_SUCCESS)
   {
      if (lResult == ERROR_FILE_NOT_FOUND)
      {
         printf("Key not found.\n");
         return TRUE;
      }
      else
      {
         printf("Error opening key.\n");
         return FALSE;
      }
   }

   // Check for an ending slash and add one if it is missing.

   lpEnd = lpSubKey + lstrlen(lpSubKey);

   if (*(lpEnd - 1) != TEXT('\\'))
   {
      *lpEnd = TEXT('\\');
      lpEnd++;
      *lpEnd = TEXT('\0');
   }

   // Enumerate the keys

   dwSize = MAX_PATH;
   lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);

   if (lResult == ERROR_SUCCESS)
   {
      do
      {

         *lpEnd = TEXT('\0');
         strcat_s(lpSubKey, MAX_PATH * 2, szName);

         if (!RegDelnodeRecurse(hKeyRoot, lpSubKey))
         {
            break;
         }

         dwSize = MAX_PATH;

         lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);

      } while (lResult == ERROR_SUCCESS);
   }

   lpEnd--;
   *lpEnd = TEXT('\0');

   RegCloseKey(hKey);

   // Try again to delete the key.

   lResult = RegDeleteKey(hKeyRoot, lpSubKey);

   if (lResult == ERROR_SUCCESS)
      return TRUE;

   return FALSE;
}

HRESULT DeleteSubKey(const std::string &szKey)
{
   char szDelKey[MAX_PATH * 2];

   char szPath[MAXSTRING];
   if (szKey == "Controller")
      lstrcpy(szPath, VP_REGKEY_GENERAL);
   else
      lstrcpy(szPath, VP_REGKEY);
   lstrcat(szPath, szKey.c_str());

   strcpy_s(szDelKey, MAX_PATH * 2, szPath);
   return RegDelnodeRecurse(HKEY_CURRENT_USER, szDelKey) == ERROR_SUCCESS;
}

#endif
