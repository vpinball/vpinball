// Win32++   Version 10.3.0
// Release Date: 4th September 2026
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2026  David Nash
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

#ifndef WIN32XX_REGKEY_H_
#define WIN32XX_REGKEY_H_

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
        ~CRegKey();

        CRegKey(const CRegKey&) = delete;
        CRegKey& operator=(const CRegKey&) = delete;
        CRegKey(CRegKey&& other) noexcept : m_key(other.Detach()) {}
        CRegKey& operator=(CRegKey&& other) noexcept;

        [[nodiscard]] operator HKEY() const { return m_key; }
        [[nodiscard]] HKEY Detach();
        [[nodiscard]] HKEY GetKey() const { return m_key; }

        [[nodiscard]] LONG Create(HKEY keyParent, LPCTSTR keyName,
            LPCTSTR className = nullptr,
            DWORD options = REG_OPTION_NON_VOLATILE,
            REGSAM samDesired = KEY_READ | KEY_WRITE,
            LPSECURITY_ATTRIBUTES secAttr = nullptr,
            LPDWORD disposition = nullptr);
        [[nodiscard]] LONG Open(HKEY keyParent, LPCTSTR keyName,
            REGSAM samDesired = KEY_READ | KEY_WRITE);

        void Attach(HKEY key);
        LONG Close();
        LONG Flush() const;

        LONG DeleteSubKey(LPCTSTR subKey) const;
        LONG DeleteValue(LPCTSTR subKey) const;
        LONG RecurseDeleteKey(LPCTSTR keyName) const;

        LONG EnumKey(DWORD index, LPTSTR name, LPDWORD nameLength,
            FILETIME* lastWriteTime = nullptr) const;
        LONG NotifyChangeKeyValue(BOOL watchSubtree, DWORD notifyFilter,
            HANDLE event, BOOL isAsync = TRUE) const;

        LONG QueryBinaryValue(LPCTSTR valueName, void* value, ULONG* bytes) const;
        LONG QueryBoolValue(LPCTSTR valueName, bool& value) const;
        LONG QueryBOOLValue(LPCTSTR valueName, BOOL& value) const;
        LONG QueryDWORDValue(LPCTSTR valueName, DWORD& value) const;
        LONG QueryGUIDValue(LPCTSTR valueName, GUID& value) const;
        LONG QueryMultiStringValue(LPCTSTR valueName, LPTSTR value, ULONG* chars) const;
        LONG QueryMultiStringValue(LPCTSTR valueName, std::vector<CString>& value) const;
        LONG QueryQWORDValue(LPCTSTR valueName, ULONGLONG& value) const;
        LONG QueryStringValue(LPCTSTR valueName, LPTSTR value, ULONG* chars) const;
        LONG QueryStringValue(LPCTSTR valueName, CString& value) const;
        LONG QueryValue(LPCTSTR valueName, DWORD* type, void* data, ULONG* bytes) const;

        LONG SetBinaryValue(LPCTSTR valueName, const void* value, ULONG bytes) const;
        LONG SetBoolValue(LPCTSTR valueName, bool value) const;
        LONG SetBOOLValue(LPCTSTR valueName, BOOL value) const;
        LONG SetDWORDValue(LPCTSTR valueName, DWORD value) const;
        LONG SetGUIDValue(LPCTSTR valueName, REFGUID value) const;
        LONG SetKeySecurity(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd) const;
        LONG SetMultiStringValue(LPCTSTR valueName, const std::vector<CString>& value) const;
        LONG SetMultiStringValue(LPCTSTR valueName, LPCTSTR value) const;
        LONG SetQWORDValue(LPCTSTR valueName, ULONGLONG value) const;
        LONG SetStringValue(LPCTSTR valueName, const CString& value) const;
        LONG SetStringValue(LPCTSTR valueName, LPCTSTR value) const;
        LONG SetValue(LPCTSTR valueName, DWORD type, const void* value, ULONG bytes) const;

    private:
        HKEY m_key;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////
    // Definitions for the CRegKey class.
    //

    inline CRegKey::CRegKey() : m_key(nullptr)
    {
    }

    inline CRegKey::CRegKey(HKEY key) : m_key(nullptr)
    {
        Attach(key);
    }

    inline CRegKey::~CRegKey()
    {
        Close();
    }

    inline CRegKey& CRegKey::operator=(CRegKey&& other) noexcept
    {
        if (this != &other)
        {
            Close();
            m_key = std::exchange(other.m_key, nullptr);
        }
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
    inline LONG CRegKey::Create(HKEY keyParent, LPCTSTR keyName,
        LPCTSTR className, DWORD options, REGSAM samDesired,
        LPSECURITY_ATTRIBUTES secAttr, LPDWORD disposition)
    {
        HKEY key = nullptr;
        LONG result = ::RegCreateKeyEx(keyParent, keyName, 0,
            const_cast<LPTSTR>(className),
            options, samDesired, secAttr, &key, disposition);

        if (result == ERROR_SUCCESS)
        {
            Close();
            m_key = key;
        }

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
        HKEY key = m_key;
        m_key = nullptr;
        return key;
    }

    // Enumerates sub-keys of the specified open registry key.
    inline LONG CRegKey::EnumKey(DWORD index, LPTSTR name, LPDWORD nameLength,
        FILETIME* lastWriteTime) const
    {
        assert(m_key);
        return ::RegEnumKeyEx(m_key, index, name, nameLength, nullptr, nullptr,
            nullptr, lastWriteTime);
    }

    // Writes all the attributes of the specified open registry key into the registry.
    inline LONG CRegKey::Flush() const
    {
        assert(m_key);
        return ::RegFlushKey(m_key);
    }

    // Notifies the caller about changes to the attributes or contents of the
    // registry key.
    inline LONG CRegKey::NotifyChangeKeyValue(BOOL watchSubtree,
        DWORD notifyFilter, HANDLE event, BOOL isAsync) const
    {
        assert(m_key);
        return ::RegNotifyChangeKeyValue(m_key, watchSubtree, notifyFilter,
            event, isAsync);
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
        assert(bytes);

        DWORD type = 0;
        LONG result = ::RegQueryValueEx(m_key, valueName, nullptr, &type,
           static_cast<LPBYTE>(value), bytes);

        if (result == ERROR_SUCCESS && type != REG_BINARY)
            return ERROR_INVALID_DATA;

        return result;
    }

    // Retrieves the bool data for the specified value name.
    inline LONG CRegKey::QueryBoolValue(LPCTSTR valueName, bool& value) const
    {
        assert(m_key);

        DWORD dwValue = 0;
        LONG result = QueryDWORDValue(valueName, dwValue);
        if (result == ERROR_SUCCESS)
            value = (dwValue != 0);

        return result;
    }

    // Retrieves the BOOL data for the specified value name.
    inline LONG CRegKey::QueryBOOLValue(LPCTSTR valueName, BOOL& value) const
    {
        assert(m_key);

        DWORD dwValue = 0;
        LONG result = QueryDWORDValue(valueName, dwValue);
        if (result == ERROR_SUCCESS)
            value = dwValue ? TRUE : FALSE;

        return result;
    }

    // Retrieves the DWORD data for the specified value name.
    inline LONG CRegKey::QueryDWORDValue(LPCTSTR valueName, DWORD& value) const
    {
        assert(m_key);

        DWORD type = 0;
        DWORD bytes = sizeof(DWORD);

        LONG result = ::RegQueryValueEx(m_key, valueName, nullptr, &type,
            reinterpret_cast<LPBYTE>(&value), &bytes);

        if (result == ERROR_SUCCESS && type != REG_DWORD)
            return ERROR_INVALID_DATA;

        return result;
    }

    // Retrieves and parses a GUID value for the specified value name.
    inline LONG CRegKey::QueryGUIDValue(LPCTSTR valueName, GUID& value) const
    {
        assert(m_key);
        value = GUID_NULL;

        CString guid;
        LONG result = QueryStringValue(valueName, guid);
        if (result != ERROR_SUCCESS)
            return result;

        HRESULT hr = ::CLSIDFromString(TtoOLE(guid), &value);
        if (FAILED(hr))
            return ERROR_INVALID_DATA;

        return ERROR_SUCCESS;
    }

    // Retrieves the multi-string data for the specified value name.
    inline LONG CRegKey::QueryMultiStringValue(LPCTSTR valueName, std::vector<CString>& value) const
    {
        assert(m_key);

        DWORD bytes = 0;
        DWORD type = 0;
        value.clear();
        LONG result = ::RegQueryValueEx(m_key, valueName, nullptr, &type, nullptr, &bytes);
        if (result == ERROR_SUCCESS && type == REG_MULTI_SZ)
        {
            int chars = bytes / sizeof(TCHAR);
            CString multiString;
            result = ::RegQueryValueEx(m_key, valueName, nullptr, &type,
                reinterpret_cast<LPBYTE>(multiString.GetBuffer(chars)), &bytes);

            multiString.ReleaseBuffer(chars);
            int totalLength = multiString.GetLength();
            LPCTSTR pBuffer = multiString.c_str();
            int currentPos = 0;

            while (currentPos < totalLength)
            {
                // Check for the double-null termination
                if (pBuffer[currentPos] == _T('\0'))
                    break;

                LPCTSTR pCurrentStart = pBuffer + currentPos;
                CString subString(pCurrentStart);
                value.push_back(subString);
                currentPos += subString.GetLength() + 1;
            }
        }

        return result;
    }

    // Retrieves the multi-string data for the specified value name.
    inline LONG CRegKey::QueryMultiStringValue(LPCTSTR valueName, LPTSTR value,
        ULONG* chars) const
    {
        assert(m_key);
        assert(chars);

        DWORD bytes = (*chars) * sizeof(TCHAR);
        DWORD type = 0;

        LONG result = ::RegQueryValueEx(m_key, valueName, nullptr, &type,
            reinterpret_cast<LPBYTE>(value), &bytes);

        *chars = bytes / sizeof(TCHAR);

        if (result == ERROR_SUCCESS && type != REG_MULTI_SZ)
            return ERROR_INVALID_DATA;

        return result;
    }

    // Retrieves a 64-bit integer QWORD data for the specified value name.
    inline LONG CRegKey::QueryQWORDValue(LPCTSTR valueName, ULONGLONG& value) const
    {
        assert(m_key);

        DWORD type = 0;
        DWORD bytes = sizeof(ULONGLONG);

        LONG result = ::RegQueryValueEx(m_key, valueName, nullptr, &type,
            reinterpret_cast<LPBYTE>(&value), &bytes);

        if (result == ERROR_SUCCESS && type != REG_QWORD)
            return ERROR_INVALID_DATA;

        return result;
    }

    // Retrieves the string data for the specified value name.
    inline LONG CRegKey::QueryStringValue(LPCTSTR valueName, CString& value) const
    {
        assert(m_key);

        DWORD bytes = 0;
        DWORD type = 0;

        LONG result = ::RegQueryValueEx(m_key, valueName, nullptr, &type, nullptr, &bytes);
        int chars = bytes / sizeof(TCHAR);

        if (result == ERROR_SUCCESS && type != REG_SZ && type != REG_EXPAND_SZ)
            return ERROR_INVALID_DATA;

        LPBYTE buffer = reinterpret_cast<LPBYTE>(value.GetBuffer(chars));
        result = ::RegQueryValueEx(m_key, valueName, nullptr, &type, buffer, &bytes);
        value.ReleaseBuffer();

        if (result == ERROR_SUCCESS && type != REG_SZ && type != REG_EXPAND_SZ)
            return ERROR_INVALID_DATA;

        return result;
    }

    // Retrieves the string data for the specified value name.
    inline LONG CRegKey::QueryStringValue(LPCTSTR valueName, LPTSTR value,
        ULONG* chars) const
    {
        assert(m_key);
        assert(chars);

        DWORD bytes = (*chars) * sizeof(TCHAR);
        DWORD type = 0;

        LONG result = ::RegQueryValueEx(m_key, valueName, nullptr, &type, reinterpret_cast<LPBYTE>(value), &bytes);
        *chars = bytes / sizeof(TCHAR);

        if (result == ERROR_SUCCESS && type != REG_SZ && type != REG_EXPAND_SZ)
            return ERROR_INVALID_DATA;

        return result;
    }

    // Low-level query pass configuration
    inline LONG CRegKey::QueryValue(LPCTSTR valueName, DWORD* type, void* data, ULONG* bytes) const
    {
        assert(m_key);
        return ::RegQueryValueEx(m_key, valueName, nullptr, type, static_cast<LPBYTE>(data), bytes);
    }

    // Deletes a subkey recursively.
    inline LONG CRegKey::RecurseDeleteKey(LPCTSTR keyName) const
    {
        assert(m_key);
        assert(keyName);

        typedef LONG(WINAPI* PFN_SHDeleteKey)(HKEY, LPCTSTR);
        LONG result = ERROR_PROC_NOT_FOUND;
        HMODULE hShlwapi = ::LoadLibrary(_T("shlwapi.dll"));
        if (hShlwapi != nullptr)
        {

#ifdef UNICODE
            PFN_SHDeleteKey pfnSHDeleteKey = reinterpret_cast<PFN_SHDeleteKey>(
                reinterpret_cast<void*>(::GetProcAddress(hShlwapi, "SHDeleteKeyW")));
#else
            PFN_SHDeleteKey pfnSHDeleteKey = reinterpret_cast<PFN_SHDeleteKey>(
                reinterpret_cast<void*>(::GetProcAddress(hShlwapi, "SHDeleteKeyA")));
#endif

            if (pfnSHDeleteKey != nullptr)
                result = pfnSHDeleteKey(m_key, keyName);

            ::FreeLibrary(hShlwapi);
        }

        return result;
    }

    // Sets the binary value of the registry key.
    inline LONG CRegKey::SetBinaryValue(LPCTSTR valueName, const void* value,
        ULONG bytes) const
    {
        assert(m_key);
        return ::RegSetValueEx(m_key, valueName, 0, REG_BINARY,
            static_cast<const BYTE*>(value), bytes);
    }

    // Sets a C++ bool value in the registry as a 4-byte configuration token.
    inline LONG CRegKey::SetBoolValue(LPCTSTR valueName, bool value) const
    {
        assert(m_key);
        return SetDWORDValue(valueName, value ? 1 : 0);
    }

    // Sets a Win32 BOOL value in the registry.
    inline LONG CRegKey::SetBOOLValue(LPCTSTR valueName, BOOL value) const
    {
        assert(m_key);
        return SetDWORDValue(valueName, value ? 1 : 0);
    }

    // Sets the DWORD value of the registry key.
    inline LONG CRegKey::SetDWORDValue(LPCTSTR valueName, DWORD value) const
    {
        assert(m_key);
        return ::RegSetValueEx(m_key, valueName, 0, REG_DWORD,
            reinterpret_cast<const BYTE*>(&value), sizeof(DWORD));
    }

    // Sets the GUID value of the registry key.
    inline LONG CRegKey::SetGUIDValue(LPCTSTR valueName, REFGUID value) const
    {
        assert(m_key);
        OLECHAR szGUID[64];
        if (::StringFromGUID2(value, szGUID, 64) == 0)
            return ERROR_INSUFFICIENT_BUFFER;
        else
        {
            CString guid(OLEtoT(szGUID).c_str());
            return SetStringValue(valueName, guid);
        }
    }

    // Sets the security of the registry key.
    inline LONG CRegKey::SetKeySecurity(SECURITY_INFORMATION si,
        PSECURITY_DESCRIPTOR psd) const
    {
        assert(m_key);
        return ::RegSetKeySecurity(m_key, si, psd);
    }

    // Sets the multistring value of the registry key. The vector of strings is
    // converted to a multi-string with embedded null characters and double
    // null terminated.
    inline LONG CRegKey::SetMultiStringValue(LPCTSTR valueName, const std::vector<CString>& value) const
    {
        assert(m_key);
        assert(!value.empty());

        if (value.empty())
            return ERROR_INVALID_DATA;

        // Use a vector of characters as our raw data buffer.
        std::vector<TCHAR> buffer;

        // Flatten the strings into the buffer sequentially.
        for (const auto& str : value)
        {
            buffer.insert(buffer.end(), str.c_str(), str.c_str() + str.GetLength());
            buffer.push_back(_T('\0'));
        }

        // Insert the final, second terminating null character
        buffer.push_back(_T('\0'));

        // buffer.size() tracks every single character accurately, including embedded nulls
        ULONG bytes = static_cast<ULONG>(buffer.size() * sizeof(TCHAR));
        return ::RegSetValueEx(m_key, valueName, 0, REG_MULTI_SZ,
            reinterpret_cast<const BYTE*>(buffer.data()), bytes);
    }

    // Sets the multistring value of the registry key. The value string can have
    // embedded null characters and should be double null terminated.
    inline LONG CRegKey::SetMultiStringValue(LPCTSTR valueName, LPCTSTR value) const
    {
        assert(m_key);
        assert(value);

        // Specify a reasonable size limit of 64K for a registry multi-string.
        constexpr size_t MAX_SAFE_CHARS = 65536;

        LPCTSTR temp = value;

        // Determine the length of the multi-string.
        while (*temp != _T('\0'))
        {
            while (*temp != _T('\0'))
            {
                temp++;
                if (static_cast<size_t>(temp - value) >= MAX_SAFE_CHARS)
                    return ERROR_INVALID_DATA;
            }

            temp++;
        }

        // Determine the total number of characters, including the double null terminator.
        size_t totalChars = static_cast<size_t>((temp - value) + 1);
        ULONG bytes = static_cast<ULONG>(totalChars * sizeof(TCHAR));

        return ::RegSetValueEx(m_key, valueName, 0, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(value), bytes);
    }

    // Sets the string value of the registry key.
    inline LONG CRegKey::SetStringValue(LPCTSTR valueName, const CString& value) const
    {
        assert(m_key);
        assert(value);
        return ::RegSetValueEx(m_key, valueName, 0, REG_SZ,
            reinterpret_cast<const BYTE*>(value.c_str()),
            (value.GetLength() + 1) * sizeof(TCHAR));
    }

    // Sets the string value of the registry key.
    inline LONG CRegKey::SetStringValue(LPCTSTR valueName, LPCTSTR value) const
    {
        assert(m_key);
        assert(value);

        if (m_key == nullptr || value == nullptr)
            return ERROR_INVALID_PARAMETER;

        const DWORD cbData = static_cast<DWORD>((_tcslen(value) + 1) * sizeof(TCHAR));
        return ::RegSetValueEx(m_key, valueName, 0, REG_SZ,
            reinterpret_cast<const BYTE*>(value), cbData);
    }

    // Sets the QWORD value of the registry key.
    inline LONG CRegKey::SetQWORDValue(LPCTSTR valueName, ULONGLONG value) const
    {
        assert(m_key);
        return ::RegSetValueEx(m_key, valueName, 0, REG_QWORD,
            reinterpret_cast<const BYTE*>(&value), sizeof(ULONGLONG));
    }

    // Sets the value of the registry key.
    inline LONG CRegKey::SetValue(LPCTSTR valueName, DWORD type,
        const void* value, ULONG bytes) const
    {
        assert(m_key);
        return ::RegSetValueEx(m_key, valueName, 0, type,
            reinterpret_cast<const BYTE*>(value), bytes);
    }

}


#endif // defined WIN32XX_REGKEY_H_
