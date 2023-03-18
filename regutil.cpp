#include "stdafx.h"

#define VP_REGKEY_GENERAL "Software\\Visual Pinball\\"
#define VP_REGKEY "Software\\Visual Pinball\\VP10\\"

#ifndef ENABLE_INI
//#define ENABLE_INI
#endif

#ifdef ENABLE_INI
//!! when to save registry? on dialog exits? on player start/end? on table load/unload? UI stuff?

#include "tinyxml2/tinyxml2.h"
#include <fstream>

static tinyxml2::XMLDocument xmlDoc;
static tinyxml2::XMLElement *xmlNode[RegName::Num] = {};
static string xmlContent;

// if ini does not exist yet, loop over reg values of each subkey and fill all in
static void InitXMLnodeFromRegistry(tinyxml2::XMLElement *const node, const string &szPath)
{
   HKEY hk;
   LONG res = RegOpenKeyEx(HKEY_CURRENT_USER, szPath.c_str(), 0, KEY_READ, &hk);
   if (res != ERROR_SUCCESS)
      return;

   for (DWORD Index = 0; ; ++Index)
   {
      DWORD dwSize = MAX_PATH;
      TCHAR szName[MAX_PATH];
      res = RegEnumValue(hk, Index, szName, &dwSize, nullptr, nullptr, nullptr, nullptr);
      if (res != ERROR_SUCCESS)
         break;

      if (dwSize == 0 || szName[0] == '\0')
         continue;
      // detect whitespace and skip, as no whitespace allowed in XML tags
      bool whitespace = false;
      unsigned int i = 0;
      while (szName[i])
         if (isspace(szName[i]))
         {
            whitespace = true;
            break;
         }
         else
            ++i;
      if (whitespace)
         continue;

      dwSize = MAXSTRING;
      BYTE pvalue[MAXSTRING];
      DWORD type = REG_NONE;
      res = RegQueryValueEx(hk, szName, nullptr, &type, pvalue, &dwSize);
      if (res != ERROR_SUCCESS)
         continue;

      // old Win32xx and Win32xx 9+ docker keys
      if (strcmp((char*)pvalue, "Dock Windows") == 0) // should not happen, as a folder, not value.. BUT also should save these somehow and restore for Win32++, or not ?
         continue;
      if (strcmp((char*)pvalue, "Dock Settings") == 0) // should not happen, as a folder, not value.. BUT also should save these somehow and restore for Win32++, or not ?
         continue;

      //

      char *copy;
      if (type == REG_SZ)
      {
         const size_t size = strlen((char*)pvalue);
         copy = new char[size + 1];
         memcpy(copy, pvalue, size);
         copy[size] = '\0';
      }
      else if (type == REG_DWORD)
      {
         const string tmp = std::to_string(*(DWORD *)pvalue);
         const size_t len = tmp.length() + 1;
         copy = new char[len];
         strcpy_s(copy, len, tmp.c_str());
      }
      else
      {
         copy = nullptr;
         assert(!"Bad RegKey");
      }

      auto vnode = node->FirstChildElement(szName);
      if (vnode == nullptr)
      {
         vnode = xmlDoc.NewElement(szName);
         node->InsertEndChild(vnode);
      }
      vnode->SetText(copy);
   }

   RegCloseKey(hk);
}

void SaveXMLregistry(const string &path)
{
   tinyxml2::XMLPrinter prn;
   xmlDoc.Print(&prn);

   std::ofstream myFile(path + "VPinballX.ini");
   myFile << prn.CStr();
   myFile.close();
}

void InitXMLregistry(const string &path)
{
   std::stringstream buffer;
   std::ifstream myFile(path + "VPinballX.ini");
   if (myFile.is_open() && myFile.good())
   {
      buffer << myFile.rdbuf();
      myFile.close();
   }

   xmlContent = buffer.str();
   if (xmlContent.empty())
      xmlDoc.InsertEndChild(xmlDoc.NewDeclaration());
   else
   {
      auto error = xmlDoc.Parse(xmlContent.c_str());
      int zz = 1;
   }

   xmlNode[RegName::Controller] = xmlDoc.FirstChildElement(regKey[RegName::Controller].c_str());
   if (xmlNode[RegName::Controller] == nullptr)
   {
      xmlNode[RegName::Controller] = xmlDoc.NewElement(regKey[RegName::Controller].c_str());
      xmlDoc.InsertEndChild(xmlNode[RegName::Controller]);
   }

   auto root = xmlDoc.FirstChildElement("VP10");
   if (root == nullptr)
   {
      root = xmlDoc.NewElement("VP10");
      xmlDoc.InsertEndChild(root);
   }
   auto defProps = root->FirstChildElement("DefaultProps");
   if (defProps == nullptr)
   {
      defProps = xmlDoc.NewElement("DefaultProps");
      root->InsertEndChild(defProps);
   }
   for (unsigned int i = RegName::Controller + 1; i < RegName::Num; ++i)
   {
      auto parent = root;
      auto key = regKey[i];
      if (key.find("\\") != string::npos)
      {
         key = key.substr(key.find("\\") + 1);
         parent = defProps;
      }
      xmlNode[i] = parent->FirstChildElement(key.c_str());
      if (xmlNode[i] == nullptr)
      {
         xmlNode[i] = xmlDoc.NewElement(key.c_str());
         parent->InsertEndChild(xmlNode[i]);
      }
   }

   // load or init registry values for each folder
   for (unsigned int i = 0; i < RegName::Num; ++i)
   {
      string regpath(i == 0 ? VP_REGKEY_GENERAL : VP_REGKEY);
      regpath += regKey[i];
      auto node = xmlNode[i];
      if (node->FirstChildElement() == nullptr)
         InitXMLnodeFromRegistry(node, regpath); // does not exist in XML yet? -> load from registry
   }
}
#else
void InitXMLregistry(const string &path) {}
void SaveXMLregistry(const string &path) {}
#endif

static HRESULT LoadValue(const string &szKey, const string &szValue, DWORD &type, void *pvalue, DWORD size);

HRESULT LoadValue(const string& szKey, const string& szValue, string& buffer)
{
   DWORD type = REG_SZ;
   char szbuffer[MAXSTRING];
   szbuffer[0] = '\0';
   const HRESULT hr = LoadValue(szKey, szValue, type, szbuffer, MAXSTRING);
   buffer = szbuffer;

   return (type != REG_SZ) ? E_FAIL : hr;
}

HRESULT LoadValue(const string& szKey, const string& szValue, void* const szbuffer, const DWORD size)
{
   if (size > 0) // clear string in case of reg value being set, but being null string which results in szbuffer being kept as-is
      ((char*)szbuffer)[0] = '\0';

   DWORD type = REG_SZ;
   const HRESULT hr = LoadValue(szKey, szValue, type, szbuffer, size);

   return (type != REG_SZ) ? E_FAIL : hr;
}

HRESULT LoadValue(const string &szKey, const string &szValue, float &pfloat)
{
   DWORD type = REG_SZ;
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

HRESULT LoadValue(const string &szKey, const string &szValue, int &pint)
{
   DWORD type = REG_DWORD;
   const HRESULT hr = LoadValue(szKey, szValue, type, (void *)&pint, 4);

   return (type != REG_DWORD) ? E_FAIL : hr;
}

HRESULT LoadValue(const string &szKey, const string &szValue, unsigned int &pint)
{
   DWORD type = REG_DWORD;
   const HRESULT hr = LoadValue(szKey, szValue, type, (void *)&pint, 4);

   return (type != REG_DWORD) ? E_FAIL : hr;
}

static HRESULT LoadValue(const string &szKey, const string &szValue, DWORD &type, void *pvalue, DWORD size)
{
   if (size == 0)
   {
      type = REG_NONE;
      return E_FAIL;
   }

#ifdef ENABLE_INI
   tinyxml2::XMLElement* node = nullptr;
   for (unsigned int i = 0; i < RegName::Num; ++i)
      if (szKey == regKey[i])
      {
         node = xmlNode[i];
         break;
      }

   if (node == nullptr)
   {
      assert(!"Bad RegKey");
      return E_FAIL;
   }

   node = node->FirstChildElement(szValue.c_str());
   if (node)
   {
      const char *const value = node->GetText();
      if (type == REG_SZ)
      {
         const DWORD len = (DWORD)strlen(value) + 1;
         const DWORD len_min = min(len, size) - 1;
         memcpy(pvalue, value, len_min);
         ((char*)pvalue)[len_min] = '\0';
         return S_OK;
      }
      else if (type == REG_DWORD)
      {
         *((DWORD*)pvalue) = (DWORD)atoll(value);
         return S_OK;
      }
      else
      {
         assert(!"Bad Type");
         type = REG_NONE;
         return E_FAIL;
      }
   }
   else
   {
      type = REG_NONE;
      return E_FAIL;
   }
#else
   string szPath(szKey == regKey[RegName::Controller] ? VP_REGKEY_GENERAL : VP_REGKEY);
   szPath += szKey;

   type = REG_NONE;

   HKEY hk;
   DWORD RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath.c_str(), 0, KEY_READ, &hk);

   if (RetVal == ERROR_SUCCESS)
   {
      RetVal = RegQueryValueEx(hk, szValue.c_str(), nullptr, &type, (BYTE *)pvalue, &size);

      RegCloseKey(hk);
   }

   return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;
#endif
}


int LoadValueIntWithDefault(const string &szKey, const string &szValue, const int def)
{
   int val;
   const HRESULT hr = LoadValue(szKey, szValue, val);
   return SUCCEEDED(hr) ? val : def;
}

float LoadValueFloatWithDefault(const string &szKey, const string &szValue, const float def)
{
   float val;
   const HRESULT hr = LoadValue(szKey, szValue, val);
   return SUCCEEDED(hr) ? val : def;
}

bool LoadValueBoolWithDefault(const string &szKey, const string &szValue, const bool def)
{
   return !!LoadValueIntWithDefault(szKey, szValue, def);
}

//

static HRESULT SaveValue(const string &szKey, const string &szValue, const DWORD type, const void *pvalue, const DWORD size)
{
   if (szValue.empty() || size == 0)
      return E_FAIL;

#ifdef ENABLE_INI
   tinyxml2::XMLElement *node = nullptr;
   for (unsigned int i = 0; i < RegName::Num; ++i)
      if (szKey == regKey[i])
      {
         node = xmlNode[i];
         break;
      }

   if (node == nullptr)
   {
      assert(!"Bad RegKey");
      return E_FAIL;
   }

   // detect whitespace and skip, as no whitespace allowed in XML tags
   for (size_t i = 0; i < szValue.length(); ++i)
      if (isspace(szValue[i]))
         return E_FAIL;

   char *copy;
   if (type == REG_SZ)
   {
      copy = new char[size+1];
      memcpy(copy, pvalue, size);
      copy[size] = '\0';
   }
   else if (type == REG_DWORD)
   {
      const string tmp = std::to_string(*(DWORD *)pvalue);
      const size_t len = tmp.length()+1;
      copy = new char[len];
      strcpy_s(copy, len, tmp.c_str());
   }
   else
   {
      assert(!"Bad RegKey");
      return E_FAIL;
   }

   auto vnode = node->FirstChildElement(szValue.c_str());
   if (vnode == nullptr)
   {
      vnode = xmlDoc.NewElement(szValue.c_str());
      node->InsertEndChild(vnode);
   }
   vnode->SetText(copy);
#endif

   string szPath(szKey == regKey[RegName::Controller] ? VP_REGKEY_GENERAL : VP_REGKEY);
   szPath += szKey;

   HKEY hk;
   //RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath.c_str(), 0, KEY_ALL_ACCESS, &hk);
   DWORD RetVal = RegCreateKeyEx(HKEY_CURRENT_USER, szPath.c_str(), 0, nullptr,
      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hk, nullptr);

   if (RetVal == ERROR_SUCCESS)
   {
      RetVal = RegSetValueEx(hk, szValue.c_str(), 0, type, (BYTE *)pvalue, size);

      RegCloseKey(hk);
   }

   return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

HRESULT SaveValueBool(const string &szKey, const string &szValue, const bool val)
{
   const DWORD dwval = val ? 1 : 0;
   return SaveValue(szKey, szValue, REG_DWORD, &dwval, sizeof(DWORD));
}

HRESULT SaveValueInt(const string &szKey, const string &szValue, const int val)
{
   return SaveValue(szKey, szValue, REG_DWORD, &val, sizeof(DWORD));
}

HRESULT SaveValueFloat(const string &szKey, const string &szValue, const float val)
{
   char buf[16];
   sprintf_s(buf, sizeof(buf), "%f", val);
   return SaveValue(szKey, szValue, REG_SZ, buf, lstrlen(buf));
}

HRESULT SaveValue(const string &szKey, const string &szValue, const char *val)
{
   return SaveValue(szKey, szValue, REG_SZ, val, lstrlen(val));
}

HRESULT SaveValue(const string &szKey, const string &szValue, const string& val)
{
   return SaveValue(szKey, szValue, REG_SZ, val.c_str(), (DWORD)val.length());
}

HRESULT DeleteValue(const string &szKey, const string &szValue)
{
   string szPath(szKey == regKey[RegName::Controller] ? VP_REGKEY_GENERAL : VP_REGKEY);
   szPath += szKey;

   HKEY hk;
   DWORD RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath.c_str(), 0, KEY_ALL_ACCESS, &hk);

   if (RetVal == ERROR_SUCCESS)
   {
      RetVal = RegDeleteValue(hk, szValue.c_str());
      RegCloseKey(hk);
   }
   else
      return S_OK; // It is a success if you want to delete something that doesn't exist.

   return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

static HRESULT RegDelnodeRecurse(const HKEY hKeyRoot, char lpSubKey[MAX_PATH * 2])
{
   // First, see if we can delete the key without having
   // to recurse.

   LONG lResult = RegDeleteKey(hKeyRoot, lpSubKey);

   if (lResult == ERROR_SUCCESS || lResult == ERROR_FILE_NOT_FOUND)
      return S_OK;

   HKEY hKey;
   lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

   if (lResult != ERROR_SUCCESS)
   {
      if (lResult == ERROR_FILE_NOT_FOUND)
      {
         ShowError("Key not found.");
         return S_OK;
      }
      else
      {
         ShowError("Error opening key.");
         return E_FAIL;
      }
   }

   // Check for an ending slash and add one if it is missing.

   LPTSTR lpEnd = lpSubKey + lstrlen(lpSubKey);

   if (*(lpEnd - 1) != TEXT('\\'))
   {
      *lpEnd = TEXT('\\');
      lpEnd++;
      *lpEnd = TEXT('\0');
   }

   // Enumerate the keys

   DWORD dwSize = MAX_PATH;
   TCHAR szName[MAX_PATH];
   lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, nullptr, nullptr, nullptr, nullptr);

   if (lResult == ERROR_SUCCESS)
   {
      do
      {
         *lpEnd = TEXT('\0');
         strcat_s(lpSubKey, MAX_PATH * 2, szName);

         if (!RegDelnodeRecurse(hKeyRoot, lpSubKey))
            break;

         dwSize = MAX_PATH;
         lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, nullptr, nullptr, nullptr, nullptr);
      } while (lResult == ERROR_SUCCESS);
   }

   lpEnd--;
   *lpEnd = TEXT('\0');

   RegCloseKey(hKey);

   // Try again to delete the key.

   lResult = RegDeleteKey(hKeyRoot, lpSubKey);

   return (lResult == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

HRESULT DeleteSubKey(const string &szKey)
{
   string szPath(szKey == regKey[RegName::Controller] ? VP_REGKEY_GENERAL : VP_REGKEY);
   szPath += szKey;

   char szDelKey[MAX_PATH * 2];
   strcpy_s(szDelKey, MAX_PATH * 2, szPath.c_str());

   return RegDelnodeRecurse(HKEY_CURRENT_USER, szDelKey);
}
