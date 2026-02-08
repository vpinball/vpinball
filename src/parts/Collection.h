#pragma once


class Collection :
   public IDispatchImpl<ICollection, &IID_ICollection, &LIBID_VPinballLib>,
   public CComObjectRoot,
   public CComCoClass<Collection, &CLSID_Collection>,
   public EventProxy<Collection, &DIID_ICollectionEvents>,
   public IConnectionPointContainerImpl<Collection>,
   public IProvideClassInfo2Impl<&CLSID_Collection, &DIID_ICollectionEvents, &LIBID_VPinballLib>,
   public IScriptable,
   public ILoadable
{
public:
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) final;
#endif
   Collection();

   // IScriptable
   const WCHAR *get_Name() const final { return m_wzName; }
   STDMETHOD(get_Name)(BSTR *pVal) override { *pVal = SysAllocString(m_wzName); return S_OK; }
   IDispatch *GetDispatch() final { return (IDispatch *)this; }
   const IDispatch *GetDispatch() const final { return (const IDispatch *)this; }

   ISelect *GetISelect() final { return nullptr; }
   const ISelect *GetISelect() const final { return nullptr; }

   //ILoadable
   HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo);
   HRESULT LoadData(IStream *pstm, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   bool LoadToken(const int id, BiffReader * const pbr) final;
   HRESULT InitPostLoad(PinTable * const pt);

   STDMETHOD(get_Count)(LONG __RPC_FAR *plCount) override;
   STDMETHOD(get_Item)(LONG index, IDispatch __RPC_FAR * __RPC_FAR *ppidisp) override;
   STDMETHOD(get__NewEnum)(IUnknown** ppunk) override;

   BEGIN_COM_MAP(Collection)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(ICollection)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(Collection)
      CONNECTION_POINT_ENTRY(DIID_ICollectionEvents)
   END_CONNECTION_POINT_MAP()

   VectorProtected<ISelect> m_visel;

   bool m_fireEvents;
   bool m_stopSingleEvents;
   bool m_groupElements;

private:
   vector<wstring> m_tmp_isel_name;
};


class OMCollectionEnum :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IEnumVARIANT
{
public:
   BEGIN_COM_MAP(OMCollectionEnum)
      COM_INTERFACE_ENTRY(IEnumVARIANT)
   END_COM_MAP()

   OMCollectionEnum() {}
   ~OMCollectionEnum() {}

   STDMETHOD(Init)(Collection *pcol);

   STDMETHOD(Next)(ULONG celt, VARIANT __RPC_FAR *rgVar, ULONG __RPC_FAR *pCeltFetched) override;
   STDMETHOD(Skip)(ULONG celt) override;
   STDMETHOD(Reset)() override;
   STDMETHOD(Clone)(IEnumVARIANT __RPC_FAR *__RPC_FAR *ppEnum) override;

private:
   Collection *m_pcol;
   int m_index;
};
