// Win32++   Version 10.0.0
// Release Date: 9th September 2024
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2024  David Nash
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
    ///////////////////////////////////////////////////////////
    // The CRegKey class manages access to the system registry.
    // It provides a means of creating, querying, modifying and
    // deleting registry entries.
    class CRegKey
    {
    public:
        CRegKey();
        CRegKey(HKEY key);
        CRegKey(const CRegKey&);
        virtual ~CRegKey();
        operator HKEY() const { return m_key; }
        CRegKey& operator =(CRegKey& key);

        void Attach(HKEY key);
        LONG Close();
        LONG Create(HKEY keyParent, LPCTSTR keyName, LPTSTR className = REG_NONE,
                    DWORD options = REG_OPTION_NON_VOLATILE,
                    REGSAM samDesired = KEY_READ | KEY_WRITE,
                    LPSECURITY_ATTRIBUTES secAttr = nullptr,
                    LPDWORD disposition = nullptr);
        LONG DeleteSubKey(LPCTSTR subKey) const;
        LONG DeleteValue(LPCTSTR subKey) const;
        HKEY Detach();
        LONG EnumKey(DWORD index, LPTSTR name, LPDWORD nameLength, FILETIME* lastWriteTime = nullptr) const;
        LONG Flush() const;
        HKEY GetKey()  const { return m_key; }
        LONG NotifyChangeKeyValue(BOOL watchSubtree, DWORD notifyFilter, HANDLE event, BOOL isAsync = TRUE) const;
        LONG Open(HKEY keyParent, LPCTSTR keyName, REGSAM samDesired = KEY_READ | KEY_WRITE);
        LONG QueryBinaryValue(LPCTSTR valueName, void* value, ULONG* bytes) const;
        LONG QueryBoolValue(LPCTSTR valueName, bool& value) const;
        LONG QueryDWORDValue(LPCTSTR valueName, DWORD& value) const;
        LONG QueryGUIDValue(LPCTSTR valueName, GUID& value) const;
        LONG QueryMultiStringValue(LPCTSTR valueName, LPTSTR value, ULONG* chars) const;
        LONG QueryQWORDValue(LPCTSTR valueName, ULONGLONG& value) const;
        LONG QueryStringValue(LPCTSTR valueName, LPTSTR value, ULONG* chars) const;
        LONG QueryValue(LPCTSTR valueName, DWORD* type, void* data, ULONG* bytes) const;
        LONG RecurseDeleteKey(LPCTSTR keyName) const;
        LONG SetBinaryValue(LPCTSTR valueName, const void* value, ULONG bytes) const;
        LONG SetBoolValue(LPCTSTR valueName, bool& value) const;
        LONG SetDWORDValue(LPCTSTR valueName, DWORD value) const;
        LONG SetGUIDValue(LPCTSTR valueName, REFGUID value) const;
        LONG SetKeySecurity(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd) const;
        LONG SetMultiStringValue(LPCTSTR valueName, LPCTSTR value) const;
        LONG SetQWORDValue(LPCTSTR valueName, ULONGLONG value) const;
        LONG SetStringValue(LPCTSTR valueName, LPCTSTR value) const;
        LONG SetValue(LPCTSTR valueName, DWORD type, const void* value, ULONG bytes) const;

    private:
        LONG RecurseDeleteAllKeys(LPCTSTR keyName) const;
        HKEY m_key;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    inline CRegKey::CRegKey() : m_key(nullptr)
    {
    }

    inline CRegKey::CRegKey(HKEY key) : m_key(nullptr)
    {
        Attach(key);
    }

    inline CRegKey::CRegKey(const CRegKey& key)
    {
        m_key = key.m_key;
    }

    inline CRegKey::~CRegKey()
    {
        Close();
    }

    inline CRegKey& CRegKey::operator =(CRegKey& key)
    {
        Close();
        Attach(key.m_key);
        return *this;
    }

    // Attaches a KEY handle to this CRegKey object.
    inline void CRegKey::Attach(HKEY key)
    {
        assert(m_key == nullptr);
        m_key = key;
    }

    // Closes the registry key.
    inline LONG CRegKey::Close()
    {
        LONG lRes = ERROR_SUCCESS;

        if (m_key != nullptr)
        {
            lRes = ::RegCloseKey(m_key);
            m_key = nullptr;
        }

        return lRes;
    }

    // Creates the specified registry key.
    inline LONG CRegKey::Create(HKEY keyParent, LPCTSTR keyName, LPTSTR className, DWORD options,
                   REGSAM samDesired, LPSECURITY_ATTRIBUTES secAttr, LPDWORD disposition)
    {
        HKEY key = nullptr;
        LONG result =  ::RegCreateKeyEx(keyParent, keyName, 0, className, options, samDesired, secAttr, &key, disposition);

        // RegCreateKeyEx opens existing keys, so close it now.
        ::RegCloseKey(key);

        return result;
    }

    // Removes the specified key from the registry.
    inline LONG CRegKey::DeleteSubKey(LPCTSTR subKey) const
    {
        assert(m_key);
        return ::RegDeleteKey(m_key, subKey);
    }

    // Removes a named value from the specified registry key.
    inline LONG CRegKey::DeleteValue(LPCTSTR subKey ) const
    {
        assert(m_key);
        return ::RegDeleteValue(m_key, subKey);
    }

    // Detaches the key handle from this CRegKey object.
    inline HKEY CRegKey::Detach()
    {
        assert(m_key);
        HKEY key = m_key;
        m_key = nullptr;
        return key;
    }

    // Enumerates subkeys of the specified open registry key.
    inline LONG CRegKey::EnumKey(DWORD index, LPTSTR name, LPDWORD nameLength, FILETIME* lastWriteTime) const
    {
        assert(m_key);
        return ::RegEnumKeyEx(m_key, index, name, nameLength, 0, 0, 0, lastWriteTime);
    }

    // Writes all the attributes of the specified open registry key into the registry.
    inline LONG CRegKey::Flush() const
    {
        assert(m_key);
        return ::RegFlushKey(m_key);
    }

    // Notifies the caller about changes to the attributes or contents of the registry key.
    inline LONG CRegKey::NotifyChangeKeyValue(BOOL watchSubtree, DWORD notifyFilter, HANDLE event, BOOL isAsync) const
    {
        assert(m_key);
        return ::RegNotifyChangeKeyValue(m_key, watchSubtree, notifyFilter, event, isAsync);
    }

    // Opens the specified registry key and assigns it to this CRegKey object.
    inline LONG CRegKey::Open(HKEY keyParent, LPCTSTR keyName, REGSAM samDesired)
    {
        assert(keyParent);
        Close();
        return ::RegOpenKeyEx(keyParent, keyName, 0, samDesired, &m_key);
    }

    // Retrieves the binary data for the specified value name.
    inline LONG CRegKey::QueryBinaryValue(LPCTSTR valueName, void* value, ULONG* bytes) const
    {
        assert(m_key);
        LONG result = ERROR_CANTREAD;
        DWORD type = 0;
        if (ERROR_SUCCESS == ::RegQueryValueEx(m_key, valueName, 0, &type, nullptr, nullptr))
        {
            if (type == REG_BINARY)
                result = ::RegQueryValueEx(m_key, valueName, 0, &type, static_cast<LPBYTE>(value), bytes);
        }

        return result;
    }

    // Retrieves the bool data for the specified value name.
    inline LONG CRegKey::QueryBoolValue(LPCTSTR valueName, bool& value) const
    {
        assert(m_key);
        ULONG boolSize = sizeof(bool);
        return QueryBinaryValue(valueName, &value, &boolSize);
    }

    // Retrieves the DWORD data for the specified value name.
    inline LONG CRegKey::QueryDWORDValue(LPCTSTR valueName, DWORD& value) const
    {
        assert(m_key);
        LONG result = ERROR_CANTREAD;
        DWORD type = 0;
        DWORD bytes = sizeof(DWORD);
        if (ERROR_SUCCESS == ::RegQueryValueEx(m_key, valueName, 0, &type, nullptr, nullptr))
        {
            if (type == REG_DWORD)
                result = ::RegQueryValueEx(m_key, valueName, 0, &type, reinterpret_cast<LPBYTE>(&value), &bytes);
        }

        return result;
    }

    // Retrieves the GUID data for the specified value name.
    inline LONG CRegKey::QueryGUIDValue(LPCTSTR valueName, GUID& value) const
    {
        assert(m_key);

        TCHAR szGUID[64];
        value = GUID_NULL;
        ULONG count = 64;
        LONG result = QueryStringValue(valueName, szGUID, &count);

        if (result == ERROR_SUCCESS)
        {
            HRESULT hr = ::CLSIDFromString(TtoOLE(szGUID), &value);

            if (FAILED(hr))
                result = ERROR_INVALID_DATA;
        }

        return result;
    }

    // Retrieves the multistring data for the specified value name.
    inline LONG CRegKey::QueryMultiStringValue(LPCTSTR valueName, LPTSTR value, ULONG* chars) const
    {
        assert(m_key);
        LONG result = ERROR_CANTREAD;
        DWORD type = 0;
        if (ERROR_SUCCESS == ::RegQueryValueEx(m_key, valueName, 0, &type, nullptr, nullptr))
        {
            if (type == REG_MULTI_SZ)
                result = ::RegQueryValueEx(m_key, valueName, 0, &type, reinterpret_cast<LPBYTE>(value), chars);
        }

        return result;
    }

    // Retrieves the QWORD data for a specified value name.
    inline LONG CRegKey::QueryQWORDValue(LPCTSTR valueName, ULONGLONG& value) const
    {
        assert(m_key);
        DWORD bytes = sizeof(ULONGLONG);
        LONG result = ERROR_CANTREAD;
        DWORD type = 0;
        if (ERROR_SUCCESS == ::RegQueryValueEx(m_key, valueName, 0, &type, nullptr, nullptr))
        {
            if (type == REG_QWORD)
                result = ::RegQueryValueEx(m_key, valueName, 0, &type, reinterpret_cast<LPBYTE>(&value), &bytes);
        }

        return result;
    }

    // Retrieves the string data for the specified value name.
    inline LONG CRegKey::QueryStringValue(LPCTSTR valueName, LPTSTR value, ULONG* chars) const
    {
        assert(m_key);
        LONG result = ERROR_CANTREAD;
        DWORD type = 0;
        if (ERROR_SUCCESS == ::RegQueryValueEx(m_key, valueName, 0, &type, nullptr, nullptr))
        {
            if (type == REG_SZ)
                result = ::RegQueryValueEx(m_key, valueName, 0, &type, reinterpret_cast<LPBYTE>(value), chars);
        }

        return result;
    }

    // Retrieves the data for the specified value name.
    inline LONG CRegKey::QueryValue(LPCTSTR valueName, DWORD* type, void* data, ULONG* bytes) const
    {
        assert(m_key);
        return ::RegQueryValueEx(m_key, valueName, 0, type, static_cast<LPBYTE>(data), bytes);
    }

    // Private recursive function called by RecurseDeleteKey.
    inline LONG CRegKey::RecurseDeleteAllKeys(LPCTSTR keyName) const
    {
        assert(m_key);
        assert(keyName);

        CRegKey key;
        LONG result = key.Open(m_key, keyName, KEY_READ | KEY_WRITE);
        if (result != ERROR_SUCCESS)
            return result;

        FILETIME time;
        DWORD size = MAX_PATH;
        TCHAR subKey[MAX_PATH];
        while (ERROR_SUCCESS == ::RegEnumKeyEx(key, 0, subKey, &size, nullptr, nullptr, nullptr, &time))
        {
            result = key.RecurseDeleteAllKeys(subKey);
            if (result != ERROR_SUCCESS)
                return result;
            size = MAX_PATH;
        }

        key.Close();
        return DeleteSubKey(keyName);
    }

    // Removes the specified key and any subkeys from the registry.
    inline LONG CRegKey::RecurseDeleteKey(LPCTSTR keyName) const
    {
        assert(m_key);

        CRegKey key;
        LONG result = key.Open(m_key, keyName, KEY_READ | KEY_WRITE);
        key.Close();
        RecurseDeleteAllKeys(keyName);
        return result;
    }

    // Sets the binary value of the registry key.
    inline LONG CRegKey::SetBinaryValue(LPCTSTR valueName, const void* value, ULONG bytes) const
    {
        assert(m_key);
        return ::RegSetValueEx(m_key, valueName, 0, REG_BINARY, static_cast<const BYTE*>(value), bytes);
    }

    // Sets the bool value of the registry key.
    inline LONG CRegKey::SetBoolValue(LPCTSTR valueName, bool& value) const
    {
        assert(m_key);
        ULONG boolSize = sizeof(bool);
        return SetBinaryValue(valueName, &value, boolSize);
    }

    // Sets the DWORD value of the registry key.
    inline LONG CRegKey::SetDWORDValue(LPCTSTR valueName, DWORD value) const
    {
        assert(m_key);
        return ::RegSetValueEx(m_key, valueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD));
    }

    // Sets the GUID value of the registry key.
    inline LONG CRegKey::SetGUIDValue(LPCTSTR valueName, REFGUID value) const
    {
        assert(m_key);
        OLECHAR szGUID[64];
        if (::StringFromGUID2(value, szGUID, 64) == 0)
            return ERROR_INSUFFICIENT_BUFFER;
        else
            return SetStringValue(valueName, OLEtoT(szGUID));
    }

    // Sets the security of the registry key.
    inline LONG CRegKey::SetKeySecurity(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd) const
    {
        assert(m_key);
        return ::RegSetKeySecurity(m_key, si, psd);
    }

    // Sets the multistring value of the registry key.
    // The value string should be double null terminated.
    inline LONG CRegKey::SetMultiStringValue(LPCTSTR valueName, LPCTSTR value) const
    {
        assert(m_key);
        assert(value);

        // pszValue contains one or more null terminated strings.
        // Calculate the length of all strings including the terminating characters.
        ULONG bytes = sizeof(TCHAR);
        LPCTSTR temp = value;
        int length = lstrlen(temp) +1; // +1 to include the terminating character

        while (length != 1)
        {
            temp += length;
            bytes += length * sizeof(TCHAR);
            length = lstrlen(temp) +1;
        }

        return ::RegSetValueEx(m_key, valueName, 0, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(value), bytes);
    }

    // Sets the string value of the registry key.
    inline LONG CRegKey::SetStringValue(LPCTSTR valueName, LPCTSTR value) const
    {
        assert(m_key);
        return ::RegSetValueEx(m_key, valueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(value), (lstrlen(value)+1)*sizeof(TCHAR));
    }

    // Sets the QWORD value of the registry key.
    inline LONG CRegKey::SetQWORDValue(LPCTSTR valueName, ULONGLONG value) const
    {
        assert(m_key);
        return ::RegSetValueEx(m_key, valueName, 0, REG_QWORD, reinterpret_cast<const BYTE*>(&value), sizeof(ULONGLONG) );
    }

    // Sets the value of the registry key.
    inline LONG CRegKey::SetValue(LPCTSTR valueName, DWORD type, const void* value, ULONG bytes) const
    {
        assert(m_key);
        return ::RegSetValueEx(m_key, valueName, 0, type, reinterpret_cast<const BYTE*>(value), bytes);
    }

}


#endif // defined _WIN32XX_REGKEY_H_
