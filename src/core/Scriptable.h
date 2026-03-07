#pragma once

class IScriptable
{
public:
   IScriptable() = default;
   virtual ~IScriptable() = default;

   const wstring& get_Name() const { return m_wzName; }
   STDMETHOD(get_Name)(BSTR *pVal) { *pVal = SysAllocStringLen(m_wzName.c_str(), static_cast<UINT>(m_wzName.length())); return S_OK; }
   
   virtual IDispatch *GetDispatch() = 0;
   virtual const IDispatch *GetDispatch() const = 0;
   
   vector<wstring> GetMethodNames();
   vector<wstring> GetEventNames();

   wstring m_wzName;
};
