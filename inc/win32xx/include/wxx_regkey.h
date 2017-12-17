// Win32++   Version 8.5
// Release Date: 1st December 2017
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2017  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// wxx_regkey.h
//  Declaration of the CRegKey class

#ifndef _WIN32XX_REGKEY_H_
#define _WIN32XX_REGKEY_H_

namespace Win32xx
{

    // The CRegKey class manages access to the system registry. It provides a
    // means of creating, querying, modifying and deleting registry entries.
    class CRegKey
    {
    public:
        CRegKey();
        CRegKey(HKEY hKey);
        CRegKey(const CRegKey&);
        ~CRegKey();
        operator HKEY() const { return m_hKey; }
        CRegKey& operator =(CRegKey& key);

        void Attach(HKEY hKey);
        LONG Close();
        LONG Create(HKEY hKeyParent, LPCTSTR lpszKeyName, LPTSTR lpszClass = REG_NONE,
                    DWORD dwOptions = REG_OPTION_NON_VOLATILE,
                    REGSAM samDesired = KEY_READ | KEY_WRITE,
                    LPSECURITY_ATTRIBUTES lpSecAttr = NULL,
                    LPDWORD lpdwDisposition = NULL);
        LONG DeleteSubKey(LPCTSTR lpszSubKey) const;
        LONG DeleteValue(LPCTSTR lpszSubKey) const;
        HKEY Detach();
        LONG EnumKey(DWORD iIndex, LPTSTR pszName, LPDWORD pnNameLength, FILETIME* pftLastWriteTime = NULL) const;
        LONG Flush() const;
        HKEY GetKey()  const { return m_hKey; }
        LONG NotifyChangeKeyValue(BOOL WatchSubtree, DWORD dwNotifyFilter, HANDLE hEvent, BOOL IsAsync = TRUE) const;
        LONG Open(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired = KEY_READ | KEY_WRITE);
        LONG QueryBinaryValue(LPCTSTR pszValueName, void* pValue, ULONG* pnBytes) const;
        LONG QueryDWORDValue(LPCTSTR pszValueName, DWORD& dwValue) const;
        LONG QueryGUIDValue(LPCTSTR pszValueName, GUID& guidValue) const;
        LONG QueryMultiStringValue(LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars) const;
        LONG QueryStringValue(LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars) const;
        LONG QueryValue(LPCTSTR pszValueName, DWORD* pdwType, void* pData, ULONG* pnBytes) const;
        LONG RecurseDeleteKey(LPCTSTR lpszKey) const;
        LONG SetBinaryValue(LPCTSTR pszValueName, const void* pValue, ULONG nBytes) const;
        LONG SetDWORDValue(LPCTSTR pszValueName, DWORD dwValue) const;
        LONG SetGUIDValue(LPCTSTR pszValueName, REFGUID guidValue) const;
        LONG SetKeySecurity(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd) const;
        LONG SetMultiStringValue(LPCTSTR pszValueName, LPCTSTR pszValue) const;
        LONG SetStringValue(LPCTSTR pszValueName, LPCTSTR pszValue, DWORD dwType = REG_SZ) const;
        LONG SetValue(LPCTSTR pszValueName, DWORD dwType, const void* pValue, ULONG nBytes) const;

#ifdef REG_QWORD
        LONG QueryQWORDValue(LPCTSTR pszValueName, ULONGLONG& qwValue) const;
        LONG SetQWORDValue(LPCTSTR pszValueName, ULONGLONG qwValue) const;
#endif

#if (WINVER >= 0x0600)
        LONG SetKeyValue(LPCTSTR lpszKeyName, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL) const;
#endif

    private:
        HKEY m_hKey;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    inline CRegKey::CRegKey() : m_hKey(0)
    {
    }


    inline CRegKey::CRegKey(HKEY hKey) : m_hKey(0)
    {
        Attach(hKey);
    }


    inline CRegKey::CRegKey(const CRegKey& Key)
    {
        m_hKey = Key.m_hKey;
    }


    inline CRegKey::~CRegKey()
    {
        Close();
    }


    inline CRegKey& CRegKey::operator =(CRegKey& key)
    {
        Close();
        Attach(key.m_hKey);
        return *this;
    }


    // Attaches a KEY handle to this CRegKey object.
    inline void CRegKey::Attach(HKEY hKey)
    {
        assert(m_hKey == NULL);
        m_hKey = hKey;
    }


    // Closes the registry key.
    inline LONG CRegKey::Close()
    {
        LONG lRes = ERROR_SUCCESS;

        if (m_hKey != NULL)
        {
            lRes = ::RegCloseKey(m_hKey);
            m_hKey = NULL;
        }

        return lRes;
    }


    // Creates the specified registry key.
    inline LONG CRegKey::Create(HKEY hKeyParent, LPCTSTR lpszKeyName, LPTSTR lpszClass, DWORD dwOptions,
                   REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecAttr, LPDWORD lpdwDisposition)
    {
        HKEY hKey = 0;
        LONG lRes =  ::RegCreateKeyEx(hKeyParent, lpszKeyName, 0, lpszClass, dwOptions, samDesired, lpSecAttr, &hKey, lpdwDisposition);

        // RegCreateKeyEx opens existing keys, so close it now.
        ::RegCloseKey(hKey);

        return lRes;
    }


    // Removes the specified key from the registry.
    inline LONG CRegKey::DeleteSubKey(LPCTSTR lpszSubKey) const
    {
        assert(m_hKey);
        return ::RegDeleteKey(m_hKey, lpszSubKey);
    }


    // Removes a named value from the specified registry key.
    inline LONG CRegKey::DeleteValue(LPCTSTR lpszSubKey ) const
    {
        assert(m_hKey);
        return ::RegDeleteValue(m_hKey, lpszSubKey);
    }


    // Detaches the key handle from this CRegKey object.
    inline HKEY CRegKey::Detach()
    {
        assert(m_hKey);
        HKEY hKey = m_hKey;
        m_hKey = 0;
        return hKey;
    }


    // Enumerates subkeys of the specified open registry key.
    inline LONG CRegKey::EnumKey(DWORD iIndex, LPTSTR pszName, LPDWORD pnNameLength, FILETIME* pftLastWriteTime) const
    {
        assert(m_hKey);
        return ::RegEnumKeyEx(m_hKey, iIndex, pszName, pnNameLength, 0, 0, 0, pftLastWriteTime);
    }


    // Writes all the attributes of the specified open registry key into the registry.
    inline LONG CRegKey::Flush() const
    {
        assert(m_hKey);
        return ::RegFlushKey(m_hKey);
    }


    // Notifies the caller about changes to the attributes or contents of the registry key.
    inline LONG CRegKey::NotifyChangeKeyValue(BOOL WatchSubtree, DWORD dwNotifyFilter, HANDLE hEvent, BOOL IsAsync) const
    {
        assert(m_hKey);
        return ::RegNotifyChangeKeyValue(m_hKey, WatchSubtree, dwNotifyFilter, hEvent, IsAsync);
    }


    // Opens the specified registry key and assigns it to this CRegKey object.
    inline LONG CRegKey::Open(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired)
    {
        assert(hKeyParent);
        Close();
        return ::RegOpenKeyEx(hKeyParent, lpszKeyName, 0, samDesired, &m_hKey);
    }


    // Retrieves the binary data for the specified value name.
    inline LONG CRegKey::QueryBinaryValue(LPCTSTR pszValueName, void* pValue, ULONG* pnBytes) const
    {
        assert(m_hKey);
        DWORD dwType = REG_BINARY;
        return ::RegQueryValueEx(m_hKey, pszValueName, 0, &dwType, static_cast<LPBYTE>(pValue), pnBytes);
    }


    // Retrieves the DWORD data for the specified value name.
    inline LONG CRegKey::QueryDWORDValue(LPCTSTR pszValueName, DWORD& dwValue) const
    {
        assert(m_hKey);
        DWORD dwType = REG_DWORD;
        DWORD nBytes = sizeof(DWORD);
        return ::RegQueryValueEx(m_hKey, pszValueName, 0, &dwType, reinterpret_cast<LPBYTE>(&dwValue), &nBytes);
    }


    // Retrieves the GUID data for the specified value name.
    inline LONG CRegKey::QueryGUIDValue(LPCTSTR pszValueName, GUID& guidValue) const
    {
        assert(m_hKey);

        TCHAR szGUID[64];
        guidValue = GUID_NULL;
        ULONG nCount = 64;
        LONG lRes = QueryStringValue(pszValueName, szGUID, &nCount);

        if (lRes == ERROR_SUCCESS)
        {
            HRESULT hr = ::CLSIDFromString(TtoOLE(szGUID), &guidValue);

            if (FAILED(hr))
                lRes = ERROR_INVALID_DATA;
        }

        return lRes;
    }


    // Retrieves the multistring data for the specified value name.
    inline LONG CRegKey::QueryMultiStringValue(LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars) const
    {
        assert(m_hKey);
        DWORD dwType = REG_MULTI_SZ;
        return ::RegQueryValueEx(m_hKey, pszValueName, 0, &dwType, reinterpret_cast<LPBYTE>(pszValue), pnChars);
    }


    // Retrieves the string data for the specified value name.
    inline LONG CRegKey::QueryStringValue(LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars) const
    {
        assert(m_hKey);
        DWORD dwType = REG_SZ;
        return ::RegQueryValueEx(m_hKey, pszValueName, 0, &dwType, reinterpret_cast<LPBYTE>(pszValue), pnChars);
    }


    // Retrieves the data for the specified value name.
    inline LONG CRegKey::QueryValue(LPCTSTR pszValueName, DWORD* pdwType, void* pData, ULONG* pnBytes) const
    {
        assert(m_hKey);
        return ::RegQueryValueEx(m_hKey, pszValueName, 0, pdwType, static_cast<LPBYTE>(pData), pnBytes);
    }


    // Removes the specified key and any subkeys from the registry.
    inline LONG CRegKey::RecurseDeleteKey(LPCTSTR lpszKey) const
    {
        assert(m_hKey);
        assert(lpszKey);

        CRegKey Key;
        LONG lRes = Key.Open(m_hKey, lpszKey, KEY_READ | KEY_WRITE);
        if (lRes != ERROR_SUCCESS)
            return lRes;

        FILETIME time;
        DWORD dwSize = MAX_PATH;
        TCHAR szSubKey[MAX_PATH];
        while (ERROR_SUCCESS == ::RegEnumKeyEx(Key, 0, szSubKey, &dwSize, NULL, NULL, NULL, &time))
        {
            lRes = Key.RecurseDeleteKey(szSubKey);
            if (lRes != ERROR_SUCCESS)
                return lRes;
            dwSize = MAX_PATH;
        }

        Key.Close();
        return DeleteSubKey(lpszKey);
    }


    // Sets the binary value of the registry key.
    inline LONG CRegKey::SetBinaryValue(LPCTSTR pszValueName, const void* pValue, ULONG nBytes) const
    {
        assert(m_hKey);
        return ::RegSetValueEx(m_hKey, pszValueName, 0, REG_BINARY, static_cast<const BYTE*>(pValue), nBytes);
    }


    // Sets the DWORD value of the registry key.
    inline LONG CRegKey::SetDWORDValue(LPCTSTR pszValueName, DWORD dwValue) const
    {
        assert(m_hKey);
        return ::RegSetValueEx(m_hKey, pszValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwValue), sizeof(DWORD));
    }


    // Sets the GUID value of the registry key.
    inline LONG CRegKey::SetGUIDValue(LPCTSTR pszValueName, REFGUID guidValue) const
    {
        assert(m_hKey);
        OLECHAR szGUID[64];
        ::StringFromGUID2(guidValue, szGUID, 64);

        return SetStringValue(pszValueName, OLEtoT(szGUID));
    }

#if (WINVER >= 0x0600)

    // Stores data in a specified value field of a specified key.
    inline LONG CRegKey::SetKeyValue(LPCTSTR lpszKeyName, LPCTSTR lpszValue, LPCTSTR lpszValueName) const
    {
        assert(m_hKey);
        return ::RegSetKeyValue(m_hKey, lpszKeyName, lpszValueName, REG_SZ, lpszValue, lstrlen(lpszValue)*sizeof(TCHAR) );
    }

#endif

    // Sets the security of the registry key.
    inline LONG CRegKey::SetKeySecurity(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd) const
    {
        assert(m_hKey);
        return ::RegSetKeySecurity(m_hKey, si, psd);
    }


    // Sets the multistring value of the registry key.
    inline LONG CRegKey::SetMultiStringValue(LPCTSTR pszValueName, LPCTSTR pszValue) const
    {
        assert(m_hKey);
        assert(pszValue);

        // pszValue contains one or more null terminated strings.
        // Calculate the length of all strings including the terminating characters.
        ULONG nBytes = 0;
        LPCTSTR pszTemp = pszValue;
        ULONG nLength = lstrlen(pszTemp) +1; // +1 to include the terminating character

        while (nLength != 1)
        {
            pszTemp += nLength;
            nBytes += nLength*sizeof(TCHAR);
            nLength = lstrlen(pszTemp) +1;
        }

        return ::RegSetValueEx(m_hKey, pszValueName, 0, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(pszValue), nBytes);
    }


    // Sets the string value of the registry key.
    inline LONG CRegKey::SetStringValue(LPCTSTR pszValueName, LPCTSTR pszValue, DWORD dwType) const
    {
        assert(m_hKey);
        return ::RegSetValueEx(m_hKey, pszValueName, 0, dwType, reinterpret_cast<const BYTE*>(pszValue), lstrlen(pszValue)*sizeof(TCHAR));
    }


    // Sets the value of the registry key.
    inline LONG CRegKey::SetValue(LPCTSTR pszValueName, DWORD dwType, const void* pValue, ULONG nBytes) const
    {
        assert(m_hKey);
        return ::RegSetValueEx(m_hKey, pszValueName, 0, dwType, reinterpret_cast<const BYTE*>(pValue), nBytes);
    }

#ifdef REG_QWORD

    // Retrieves the QWORD data for a specified value name.
    inline LONG CRegKey::QueryQWORDValue(LPCTSTR pszValueName, ULONGLONG& qwValue) const
    {
        assert(m_hKey);
        DWORD dwType = REG_QWORD;
        DWORD nBytes = sizeof(ULONGLONG);
        return ::RegQueryValueEx(m_hKey, pszValueName, 0, &dwType, reinterpret_cast<LPBYTE>(&qwValue), &nBytes);
    }


    // Sets the QWORD value of the registry key.
    inline LONG CRegKey::SetQWORDValue(LPCTSTR pszValueName, ULONGLONG qwValue) const
    {
        assert(m_hKey);
        return ::RegSetValueEx(m_hKey, pszValueName, 0, REG_QWORD, reinterpret_cast<const BYTE*>(&qwValue), sizeof(ULONGLONG) );
    }

#endif

}


#endif // defined _WIN32XX_REGKEY_H_
