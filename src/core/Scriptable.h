#pragma once

class IScriptable
{
public:
   IScriptable() { }

   STDMETHOD(get_Name)(BSTR *pVal) = 0; // returns m_wzName
   virtual const wstring& get_Name() const = 0; // dto (always non-nullptr returned), but without going through BSTR conversion (necessary for COM)
   virtual IDispatch *GetDispatch() = 0;
   virtual const IDispatch *GetDispatch() const = 0;
   virtual ISelect *GetISelect() = 0;
   virtual const ISelect *GetISelect() const = 0;

   vector<wstring> GetMethodNames();
   vector<wstring> GetEventNames();

   wstring m_wzName;
};
