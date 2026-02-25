#pragma once

class IScriptable
{
public:
   IScriptable() { m_wzName[0] = '\0'; }

   STDMETHOD(get_Name)(BSTR *pVal) = 0; // fails for Decals, returns m_wzName or something custom for everything else
   virtual const WCHAR *get_Name() const = 0; // dto (and returns "Decal" for Decals, so always non-nullptr returned), but without going through BSTR conversion (necessary for COM)
   virtual IDispatch *GetDispatch() = 0;
   virtual const IDispatch *GetDispatch() const = 0;
   virtual ISelect *GetISelect() = 0;
   virtual const ISelect *GetISelect() const = 0;

   vector<wstring> GetMethodNames();
   vector<wstring> GetEventNames();

   WCHAR m_wzName[MAXNAMEBUFFER];
};
