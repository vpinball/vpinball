/*
 * Portions of this code was derived from ReactOS ATL:
 *
 * https://github.com/reactos/reactos/blob/master/sdk/lib/atl/atlcom.h
 */

#pragma once

class CComObjectRootBase {
public:
  LONG m_dwRef;

  CComObjectRootBase()
  {
    m_dwRef = 0;
  }

  ~CComObjectRootBase() { }

  static HRESULT WINAPI InternalQueryInterface(void* pThis, const _ATL_INTMAP_ENTRY* pEntries, REFIID iid, void** ppvObject)
  {
    return AtlInternalQueryInterface(pThis, pEntries, iid, ppvObject);
  }
};

template <class ThreadModel>
class CComObjectRootEx : public CComObjectRootBase {
public:
  ~CComObjectRootEx() { }

  ULONG InternalAddRef()
  {
    ATLASSERT(m_dwRef >= 0);
      
    m_dwRef++;

    return m_dwRef;
  }

  ULONG InternalRelease()
  {
    ATLASSERT(m_dwRef > 0);

    m_dwRef--;

    return m_dwRef;
  }

  void Lock() { }
  void Unlock() { }
};

template <class Base>
class CComObject : public Base {
public:
  STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject) 
  { 
    return this->_InternalQueryInterface(iid, ppvObject); 
  }

  STDMETHOD_(ULONG, AddRef)() 
  { 
    return this->InternalAddRef(); 
  }

  STDMETHOD_(ULONG, Release)()
  {
    ULONG newRefCount;

    newRefCount = this->InternalRelease();

    if (newRefCount == 0)
      delete this;
    
    return newRefCount;
  }

  static HRESULT CreateInstance(CComObject<Base>** pp)
  {
    CComObject<Base>* newInstance;

    newInstance = new CComObject<Base>();

    *pp = newInstance;

    return S_OK;
  }
};

typedef CComSingleThreadModel CComObjectThreadModel;
typedef CComObjectRootEx<CComObjectThreadModel> CComObjectRoot;

template <class T, const CLSID* pclsid = &CLSID_NULL>
class CComCoClass {
  template <class Q>
    static HRESULT CreateInstance(IUnknown* punkOuter, Q** pp)
    {
      return T::_CreatorClass::CreateInstance(punkOuter, __uuidof(Q), (void**) pp);
    }
    template <class Q>
    static HRESULT CreateInstance(Q** pp)
    {
      return T::_CreatorClass::CreateInstance(NULL, __uuidof(Q), (void**) pp);
    }
};

#define BEGIN_COM_MAP(x)                                                        \
public:                                                                            \
    typedef x _ComMapClass;                                                        \
    HRESULT _InternalQueryInterface(REFIID iid, void **ppvObject)                \
    {                                                                            \
        return this->InternalQueryInterface(this, _GetEntries(), iid, ppvObject);        \
    }                                                                            \
    const static _ATL_INTMAP_ENTRY *WINAPI _GetEntries()                    \
    {                                                                            \
        static const _ATL_INTMAP_ENTRY _entries[] = {

#define END_COM_MAP()                                                            \
            {NULL, 0, 0}                                                        \
        };                                                                        \
        return _entries;                                                        \
    }                                                                            \
    virtual ULONG STDMETHODCALLTYPE AddRef() = 0;                                \
    virtual ULONG STDMETHODCALLTYPE Release() = 0;                                \
    STDMETHOD(QueryInterface)(REFIID, void **) = 0;

#define COM_INTERFACE_ENTRY(x)                                                  \
    {&_ATL_IIDOF(x),                                                            \
    offsetofclass(x, _ComMapClass),                                             \
    _ATL_SIMPLEMAPENTRY},

#define COM_INTERFACE_ENTRY_IID(iid, x)                                            \
    {&iid, offsetofclass(x, _ComMapClass), _ATL_SIMPLEMAPENTRY},

#define COM_INTERFACE_ENTRY_IMPL(x)\
  COM_INTERFACE_ENTRY_IID(_ATL_IIDOF(x), x##Impl<_ComMapClass>)

template <class T>
class _CopyInterface { };

template <class Base, const IID *piid, class T, class Copy>
class CComEnumImpl : public Base { };

class CComDynamicUnkArray {
public:
  int m_nSize;
  IUnknown **m_ppUnk;
public:
  CComDynamicUnkArray()
  {
    m_nSize = 0;
    m_ppUnk = NULL;
  }

  ~CComDynamicUnkArray()
  {
    free(m_ppUnk);
  }

  IUnknown **begin()
  {
    return m_ppUnk;
  }

  IUnknown **end()
  {
    return &m_ppUnk[m_nSize];
  }
};

template <class Base, const IID* piid, class T, class Copy, class ThreadModel = CComObjectThreadModel>
class ATL_NO_VTABLE CComEnum :
  public CComEnumImpl<Base, piid, T, Copy>,
  public CComObjectRootEx< ThreadModel > {
public:
  typedef CComEnum<Base, piid, T, Copy > _CComEnum;
  typedef CComEnumImpl<Base, piid, T, Copy > _CComEnumBase;
  BEGIN_COM_MAP(_CComEnum)
    COM_INTERFACE_ENTRY_IID(*piid, _CComEnumBase)
  END_COM_MAP()
};

struct _ATL_CONNMAP_ENTRY
{
  DWORD_PTR dwOffset;
};

template <const IID* piid>
class _ICPLocator {
public:
  STDMETHOD(_LocCPQueryInterface)(REFIID riid, void **ppvObject) = 0;
  virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
  virtual ULONG STDMETHODCALLTYPE Release() = 0;
};

template <class T, const IID* piid, class CDV = CComDynamicUnkArray>
class IConnectionPointImpl : public _ICPLocator<piid> {
public:
  CDV m_vec;

  STDMETHOD(_LocCPQueryInterface)(REFIID riid, void **ppvObject) { return E_NOINTERFACE; }
};

template <class T>
class ATL_NO_VTABLE IConnectionPointContainerImpl : public IConnectionPointContainer {
        typedef const _ATL_CONNMAP_ENTRY * (*handlerFunctionType)(int *);
        typedef CComEnum<IEnumConnectionPoints, &IID_IEnumConnectionPoints, IConnectionPoint *, _CopyInterface<IConnectionPoint> >
                        CComEnumConnectionPoints;

  STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints** ppEnum) { return E_NOTIMPL; }
  STDMETHOD(FindConnectionPoint)(REFIID riid, IConnectionPoint** ppCP) { return E_NOTIMPL; }
};

#define BEGIN_CONNECTION_POINT_MAP(x)                                            \
    typedef x _atl_conn_classtype;                                                \
    static const _ATL_CONNMAP_ENTRY *GetConnMap(int *pnEntries) {            \
    static const _ATL_CONNMAP_ENTRY _entries[] = {

#define END_CONNECTION_POINT_MAP()                                                \
    {(DWORD_PTR)-1} };                                                            \
    if (pnEntries)                                                                \
        *pnEntries = sizeof(_entries) / sizeof(_ATL_CONNMAP_ENTRY) - 1;    \
    return _entries;}

#define CONNECTION_POINT_ENTRY(iid)                                                \
    {offsetofclass(_ICPLocator<&iid>, _atl_conn_classtype) -                \
    offsetofclass(IConnectionPointContainerImpl<_atl_conn_classtype>, _atl_conn_classtype)},

#define DECLARE_PROTECT_FINAL_CONSTRUCT()                                        \
    void InternalFinalConstructAddRef()                                            \
    {                                                                            \
        InternalAddRef();                                                        \
    }                                                                            \
    void InternalFinalConstructRelease()                                        \
    {                                                                            \
        InternalRelease();                                                        \
    }

#define DECLARE_REGISTRY_RESOURCEID(x)                                            \
    static HRESULT WINAPI UpdateRegistry(BOOL bRegister)                        \
    {                                                                            \
        return E_NOTIMPL;                                                         \
    }

class CComTypeInfoHolder {
public:
  HRESULT GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) { return E_NOTIMPL; }
};

template <const CLSID* pcoclsid, const IID* psrcid,
          const GUID* plibid = &CAtlModule::m_libid, WORD wMajor = 1,
          WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
class ATL_NO_VTABLE IProvideClassInfo2Impl : public IProvideClassInfo2 {
public:
  STDMETHOD(GetClassInfo)(ITypeInfo** pptinfo) { return E_NOTIMPL; }
  STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID* pGUID) { return E_FAIL; }
};

template <class T, const IID* piid /*= &__uuidof(T)*/,
          const GUID* plibid = &CAtlModule::m_libid, WORD wMajor = 1,
          WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
class IDispatchImpl : public T, public ITypeInfo {
public:
  virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
  virtual ULONG STDMETHODCALLTYPE Release() = 0;

  STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) { return E_NOTIMPL; }

  STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) {
    *ppTInfo = (ITypeInfo*)this;
    this->AddRef();
    return S_OK;
  }
    
  STDMETHOD(GetTypeAttr)(TYPEATTR **ppTypeAttr) { return E_NOTIMPL; }
  STDMETHOD(GetTypeComp)(ITypeComp **ppTComp) { return E_NOTIMPL; }
  STDMETHOD(GetFuncDesc)(UINT index, FUNCDESC **ppFuncDesc) { return E_NOTIMPL; }
  STDMETHOD(GetVarDesc)(UINT index, VARDESC **ppVarDesc) { return E_NOTIMPL; }
  STDMETHOD(GetNames)(MEMBERID memid,BSTR *rgBstrNames,UINT cMaxNames,UINT *pcNames) { return E_NOTIMPL; }
  STDMETHOD(GetRefTypeOfImplType)(UINT index,HREFTYPE *pRefType) { return E_NOTIMPL; }
  STDMETHOD(GetImplTypeFlags)(UINT index,INT *pImplTypeFlags) { return E_NOTIMPL; }
  STDMETHOD(GetIDsOfNames)(LPOLESTR *rgszNames, UINT cNames,MEMBERID *pMemId) { return E_NOTIMPL; }
  STDMETHOD(Invoke)(PVOID pvInstance, MEMBERID memid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) { return E_NOTIMPL; }
  STDMETHOD(GetDocumentation)(MEMBERID memid, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) { return E_NOTIMPL; }
  STDMETHOD(GetDllEntry)(MEMBERID memid, INVOKEKIND invKind, BSTR *pBstrDllName, BSTR *pBstrName, WORD *pwOrdinal) { return E_NOTIMPL; }
  STDMETHOD(GetRefTypeInfo)(HREFTYPE hRefType, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
  STDMETHOD(AddressOfMember)(MEMBERID memid, INVOKEKIND invKind, PVOID *ppv) { return E_NOTIMPL; }
  STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, PVOID *ppvObj) { return E_NOTIMPL; }
  STDMETHOD(GetMops)(MEMBERID memid, BSTR *pBstrMops) { return E_NOTIMPL; };
  STDMETHOD(GetContainingTypeLib)(ITypeLib **ppTLib, UINT *pIndex) { return E_NOTIMPL; }
  void STDMETHODCALLTYPE ReleaseTypeAttr(TYPEATTR *pTypeAttr) { }
  void STDMETHODCALLTYPE ReleaseFuncDesc(FUNCDESC *pFuncDesc) { }
  void STDMETHODCALLTYPE ReleaseVarDesc(VARDESC *pVarDesc) { }
};