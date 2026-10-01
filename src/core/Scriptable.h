#pragma once

class IScriptable
{
public:
   IScriptable() = default;
   virtual ~IScriptable() = default;

   const string& get_Name() const { return m_name; }
   STDMETHOD(get_Name)(BSTR *pVal) { *pVal = MakeWideBSTR(m_name); return S_OK; }

   virtual IDispatch *GetIDispatch() = 0;
   virtual const IDispatch *GetIDispatch() const = 0;

   vector<wstring> GetMethodNames();
   vector<wstring> GetEventNames();

   string m_name; // Script name, UTF-8 (UTF-16 in table files), at most MAXNAMEBUFFER - 1 UTF-16 units
};
