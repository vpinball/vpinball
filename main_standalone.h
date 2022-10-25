#ifndef __MAIN_STANDALONE_H__
#define __MAIN_STANDALONE_H__

#if defined(__clang__)
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wmissing-exception-spec"
#endif

#define ONLY_USE_BASS

extern "C" {
#include <atlbase.h>
}

#include <oleauto.h>
#include "winbase.h"

#undef wcsncpy
#undef strncpy
#include <wchar.h>

#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_WCHAR L'/'

#include <vector>
#include <string>
#include <algorithm>

#include <limits.h>

using std::string;
using std::wstring;
using std::vector;

#ifdef NULL
#undef NULL
#define NULL 0
#endif

#define _T(x) x
#define AtoT(x) x
#define CString string

#define fopen_s(pFile, filename, mode) \
  ((*(pFile)) = fopen((filename), (mode))) == NULL
#define fprintf_s fprintf
#define fscanf_s fscanf
#define fread_s(buffer, bufferSize, elementSize, count, stream) fread(buffer, bufferSize, count, stream)
#define localtime_s(x, y) localtime_r(y, x)
#define gmtime_s(x, y) gmtime_r(y, x)
#undef lstrlen
#define lstrlen strlen
#undef lstrcmpi
#define lstrcmpi strcasecmp
#define _stricmp strcasecmp
#undef lstrcmp
#define lstrcmp strcmp
#define strcpy_s(A, B, C) strncpy(A, C, B)
#define strnlen_s strnlen
#define sprintf_s snprintf
#define _snprintf_s snprintf
#define swprintf_s swprintf
#define StrStrI strstr

#define STRNCPY_S3(a, b, c) strncpy(a, b, c)
#define STRNCPY_S4(a, b, c, d) strncpy(a, c, d)
#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define strncpy_s(...) \
  GET_MACRO(__VA_ARGS__, STRNCPY_S4, STRNCPY_S3)(__VA_ARGS__)

#undef GetCurrentDirectory
#define GetCurrentDirectory GetCurrentDirectoryA

#undef GetModuleFileName
#define GetModuleFileName GetModuleFileNameA

#undef MessageBox
#define MessageBox MessageBoxA

#undef OutputDebugString
#define OutputDebugString OutputDebugStringA

#undef CharLowerBuff
#define CharLowerBuff CharLowerBuffA

#define _ASSERTE(A)
#define ATLASSERT(expr) _ASSERTE(expr)

#define DUMP_FID(A)                                                       \
  printf("%c%c%c%c\n", ((A)&0xFF), ((A >> 8) & 0xFF), ((A >> 16) & 0xFF), \
         ((A >> 24) & 0xFF));

#define ATL_NO_VTABLE

struct _ATL_CONNMAP_ENTRY
{
    DWORD_PTR dwOffset;
};

#define _ATL_PACKING 8

#define offsetofclass(base, derived) (reinterpret_cast<DWORD_PTR>(static_cast<base *>(reinterpret_cast<derived *>(_ATL_PACKING))) - _ATL_PACKING)

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

#define _ATL_SIMPLEMAPENTRY ((_ATL_CREATORARGFUNC *)1)
#define _ATL_IIDOF(x)   IID_##x

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

#define DECLARE_PROTECT_FINAL_CONSTRUCT()
#define DECLARE_REGISTRY_RESOURCEID(VAL)

#define DIK_ESCAPE 0x01
#define DIK_1 0x02
#define DIK_2 0x03
#define DIK_3 0x04
#define DIK_4 0x05
#define DIK_5 0x06
#define DIK_6 0x07
#define DIK_7 0x08
#define DIK_8 0x09
#define DIK_9 0x0A
#define DIK_0 0x0B
#define DIK_MINUS 0x0C /* - on main keyboard */
#define DIK_EQUALS 0x0D
#define DIK_BACK 0x0E /* backspace */
#define DIK_TAB 0x0F
#define DIK_Q 0x10
#define DIK_W 0x11
#define DIK_E 0x12
#define DIK_R 0x13
#define DIK_T 0x14
#define DIK_Y 0x15
#define DIK_U 0x16
#define DIK_I 0x17
#define DIK_O 0x18
#define DIK_P 0x19
#define DIK_LBRACKET 0x1A
#define DIK_RBRACKET 0x1B
#define DIK_RETURN 0x1C /* Enter on main keyboard */
#define DIK_LCONTROL 0x1D
#define DIK_A 0x1E
#define DIK_S 0x1F
#define DIK_D 0x20
#define DIK_F 0x21
#define DIK_G 0x22
#define DIK_H 0x23
#define DIK_J 0x24
#define DIK_K 0x25
#define DIK_L 0x26
#define DIK_SEMICOLON 0x27
#define DIK_APOSTROPHE 0x28
#define DIK_GRAVE 0x29 /* accent grave */
#define DIK_LSHIFT 0x2A
#define DIK_BACKSLASH 0x2B
#define DIK_Z 0x2C
#define DIK_X 0x2D
#define DIK_C 0x2E
#define DIK_V 0x2F
#define DIK_B 0x30
#define DIK_N 0x31
#define DIK_M 0x32
#define DIK_COMMA 0x33
#define DIK_PERIOD 0x34 /* . on main keyboard */
#define DIK_SLASH 0x35  /* / on main keyboard */
#define DIK_RSHIFT 0x36
#define DIK_MULTIPLY 0x37 /* * on numeric keypad */
#define DIK_LMENU 0x38    /* left Alt */
#define DIK_SPACE 0x39
#define DIK_CAPITAL 0x3A
#define DIK_F1 0x3B
#define DIK_F2 0x3C
#define DIK_F3 0x3D
#define DIK_F4 0x3E
#define DIK_F5 0x3F
#define DIK_F6 0x40
#define DIK_F7 0x41
#define DIK_F8 0x42
#define DIK_F9 0x43
#define DIK_F10 0x44
#define DIK_NUMLOCK 0x45
#define DIK_SCROLL 0x46 /* Scroll Lock */
#define DIK_NUMPAD7 0x47
#define DIK_NUMPAD8 0x48
#define DIK_NUMPAD9 0x49
#define DIK_SUBTRACT 0x4A /* - on numeric keypad */
#define DIK_NUMPAD4 0x4B
#define DIK_NUMPAD5 0x4C
#define DIK_NUMPAD6 0x4D
#define DIK_ADD 0x4E /* + on numeric keypad */
#define DIK_NUMPAD1 0x4F
#define DIK_NUMPAD2 0x50
#define DIK_NUMPAD3 0x51
#define DIK_NUMPAD0 0x52
#define DIK_DECIMAL 0x53 /* . on numeric keypad */
#define DIK_F11 0x57
#define DIK_F12 0x58
#define DIK_F13 0x64         /*                     (NEC PC98) */
#define DIK_F14 0x65         /*                     (NEC PC98) */
#define DIK_F15 0x66         /*                     (NEC PC98) */
#define DIK_NUMPADENTER 0x9C /* Enter on numeric keypad */
#define DIK_RCONTROL 0x9D
#define DIK_DIVIDE 0xB5    /* / on numeric keypad */
#define DIK_RMENU 0xB8     /* right Alt */
#define DIK_HOME 0xC7      /* Home on arrow keypad */
#define DIK_UP 0xC8        /* UpArrow on arrow keypad */
#define DIK_PRIOR 0xC9     /* PgUp on arrow keypad */
#define DIK_LEFT 0xCB      /* LeftArrow on arrow keypad */
#define DIK_RIGHT 0xCD     /* RightArrow on arrow keypad */
#define DIK_END 0xCF       /* End on arrow keypad */
#define DIK_DOWN 0xD0      /* DownArrow on arrow keypad */
#define DIK_NEXT 0xD1      /* PgDn on arrow keypad */
#define DIK_INSERT 0xD2    /* Insert on arrow keypad */
#define DIK_DELETE 0xD3    /* Delete on arrow keypad */
#define DIK_LWIN 0xDB      /* Left Windows key */
#define DIK_RWIN 0xDC      /* Right Windows key */
#define DIK_APPS 0xDD      /* AppMenu key */
#define DIK_LALT DIK_LMENU /* left Alt */
#define DIK_RALT DIK_RMENU /* right Alt */

#define DIJOFS_BUTTON0 DIJOFS_BUTTON(0)
#define DIJOFS_BUTTON1 DIJOFS_BUTTON(1)
#define DIJOFS_BUTTON2 DIJOFS_BUTTON(2)
#define DIJOFS_BUTTON3 DIJOFS_BUTTON(3)
#define DIJOFS_BUTTON4 DIJOFS_BUTTON(4)
#define DIJOFS_BUTTON5 DIJOFS_BUTTON(5)
#define DIJOFS_BUTTON6 DIJOFS_BUTTON(6)
#define DIJOFS_BUTTON7 DIJOFS_BUTTON(7)
#define DIJOFS_BUTTON8 DIJOFS_BUTTON(8)
#define DIJOFS_BUTTON9 DIJOFS_BUTTON(9)
#define DIJOFS_BUTTON10 DIJOFS_BUTTON(10)
#define DIJOFS_BUTTON11 DIJOFS_BUTTON(11)
#define DIJOFS_BUTTON12 DIJOFS_BUTTON(12)
#define DIJOFS_BUTTON13 DIJOFS_BUTTON(13)
#define DIJOFS_BUTTON14 DIJOFS_BUTTON(14)
#define DIJOFS_BUTTON15 DIJOFS_BUTTON(15)
#define DIJOFS_BUTTON16 DIJOFS_BUTTON(16)
#define DIJOFS_BUTTON17 DIJOFS_BUTTON(17)
#define DIJOFS_BUTTON18 DIJOFS_BUTTON(18)
#define DIJOFS_BUTTON19 DIJOFS_BUTTON(19)
#define DIJOFS_BUTTON20 DIJOFS_BUTTON(20)
#define DIJOFS_BUTTON21 DIJOFS_BUTTON(21)
#define DIJOFS_BUTTON22 DIJOFS_BUTTON(22)
#define DIJOFS_BUTTON23 DIJOFS_BUTTON(23)
#define DIJOFS_BUTTON24 DIJOFS_BUTTON(24)
#define DIJOFS_BUTTON25 DIJOFS_BUTTON(25)
#define DIJOFS_BUTTON26 DIJOFS_BUTTON(26)
#define DIJOFS_BUTTON27 DIJOFS_BUTTON(27)
#define DIJOFS_BUTTON28 DIJOFS_BUTTON(28)
#define DIJOFS_BUTTON29 DIJOFS_BUTTON(29)
#define DIJOFS_BUTTON30 DIJOFS_BUTTON(30)
#define DIJOFS_BUTTON31 DIJOFS_BUTTON(31)

#define DSBPLAY_LOOPING 0x00000001
#define DSBVOLUME_MIN -10000

typedef LPSTR LPTSTR;
typedef LPCSTR LPCTSTR;
typedef DWORD FOURCC;
typedef ULONG_PTR HCRYPTHASH;
typedef ULONG_PTR HCRYPTKEY;
typedef ULONG_PTR HCRYPTPROV;
typedef DWORD D3DCOLOR;
typedef float D3DVALUE;

typedef UINT_PTR(CALLBACK* __CDHOOKPROC)(HWND, UINT, WPARAM, LPARAM);
typedef __CDHOOKPROC LPFRHOOKPROC;

typedef struct _LOGFONT {
  LONG lfHeight;
  LONG lfWidth;
  LONG lfEscapement;
  LONG lfOrientation;
  LONG lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  CHAR lfFaceName[LF_FACESIZE];
} LOGFONT;

typedef struct _WNDCLASS {
  UINT style;
  WNDPROC lpfnWndProc;
  int cbClsExtra;
  int cbWndExtra;
  HINSTANCE hInstance;
  HICON hIcon;
  HCURSOR hCursor;
  HBRUSH hbrBackground;
  LPCSTR lpszMenuName;
  LPCSTR lpszClassName;
} WNDCLASS;

typedef struct _CREATESTRUCT {
  LPVOID lpCreateParams;
  HINSTANCE hInstance;
  HMENU hMenu;
  HWND hwndParent;
  int cy;
  int cx;
  int y;
  int x;
  LONG style;
  LPCSTR lpszName;
  LPCSTR lpszClass;
  DWORD dwExStyle;
} CREATESTRUCT;

typedef struct _FINDREPLACE {
  DWORD lStructSize;
  HWND hwndOwner;
  HINSTANCE hInstance;
  DWORD Flags;
  LPSTR lpstrFindWhat;
  LPSTR lpstrReplaceWith;
  WORD wFindWhatLen;
  WORD wReplaceWithLen;
  LPARAM lCustData;
  LPFRHOOKPROC lpfnHook;
  LPCSTR lpTemplateName;
} FINDREPLACE;

typedef struct _MONITORINFOEX {
  DWORD cbSize;
  RECT rcMonitor;
  RECT rcWork;
  DWORD dwFlags;
} MONITORINFOEX;

typedef struct _DIDEVICEOBJECTDATA {
  DWORD dwOfs;
  DWORD dwData;
  DWORD dwTimeStamp;
  DWORD dwSequence;
  UINT_PTR uAppData;
} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;

typedef struct _DIJOYSTATE {
  LONG lX;
  LONG lY;
  LONG lZ;
  LONG lRx;
  LONG lRy;
  LONG lRz;
  LONG rglSlider[2];
  DWORD rgdwPOV[4];
  BYTE rgbButtons[32];
} DIJOYSTATE, *LPDIJOYSTATE;

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

template <class Base>
class CComObject : public Base {
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject) { return this->_InternalQueryInterface(iid, ppvObject); }

   STDMETHOD_(ULONG, AddRef)() { return this->InternalAddRef(); }

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
      HRESULT hResult;

      newInstance = new CComObject<Base>();

      *pp = newInstance;

      return S_OK;
   }
};

class CComMultiThreadModel {};

class CComSingleThreadModel {};

class CComObjectRootBase
{
public:
   LONG m_dwRef;

   CComObjectRootBase()
   {
      m_dwRef = 0;
   }

   ~CComObjectRootBase()
   {
   }

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
      return ++m_dwRef; // FIXME ThreadModel::Increment(&m_dwRef);
   }

   ULONG InternalRelease()
   {
      ATLASSERT(m_dwRef > 0);
      return --m_dwRef; // FIXME ThreadModel::Decrement(&m_dwRef);
   }

   void Lock()
   {
   }

   void Unlock()
   {
   }
};

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

typedef CComSingleThreadModel CComObjectThreadModel;
typedef CComObjectRootEx<CComObjectThreadModel> CComObjectRoot;

template <class T>
class CComPtr {
 public:
  T* p;

 public:
  CComPtr() { p = NULL; }

  CComPtr(T* lp) {
    p = lp;
    if (p != NULL) p->AddRef();
  }

  CComPtr(const CComPtr<T>& lp) {
    p = lp.p;
    if (p != NULL) p->AddRef();
  }

  ~CComPtr() {
    if (p != NULL) p->Release();
  }

  T* operator=(T* lp) {
    T* pOld = p;

    p = lp;
    if (p != NULL) p->AddRef();

    if (pOld != NULL) pOld->Release();

    return *this;
  }

  T* operator=(const CComPtr<T>& lp) {
    T* pOld = p;

    p = lp.p;
    if (p != NULL) p->AddRef();

    if (pOld != NULL) pOld->Release();

    return *this;
  }

  // We cannot enable this until gcc starts supporting __uuidof
  // See CORE-12710
#if 0
    template <typename Q>
    T* operator=(const CComPtr<Q>& lp)
    {
        T* pOld = p;

        if (!lp.p || FAILED(lp.p->QueryInterface(__uuidof(T), (void**)(IUnknown**)&p)))
            p = NULL;

        if (pOld != NULL)
            pOld->Release();

        return *this;
    }
#endif

  void Release() {
    if (p != NULL) {
      p->Release();
      p = NULL;
    }
  }

  void Attach(T* lp) {
    if (p != NULL) p->Release();
    p = lp;
  }

  T* Detach() {
    T* saveP;

    saveP = p;
    p = NULL;
    return saveP;
  }

  T** operator&() {
    ATLASSERT(p == NULL);
    return &p;
  }

  operator T*() { return p; }

  T* operator->() {
    ATLASSERT(p != NULL);
    return p;
  }
};

class CComBSTR {
 public:
  BSTR m_str;

 public:
  CComBSTR() : m_str(NULL) {}

  CComBSTR(LPCOLESTR pSrc) {
    if (pSrc == NULL)
      m_str = NULL;
    else
      m_str = ::SysAllocString(pSrc);
  }

  CComBSTR(int length) {
    if (length == 0)
      m_str = NULL;
    else
      m_str = ::SysAllocStringLen(NULL, length);
  }

  CComBSTR(int length, LPCOLESTR pSrc) {
    if (length == 0)
      m_str = NULL;
    else
      m_str = ::SysAllocStringLen(pSrc, length);
  }

  CComBSTR(PCSTR pSrc) {
    if (pSrc) {
      int len = MultiByteToWideChar(CP_THREAD_ACP, 0, pSrc, -1, NULL, 0);
      m_str = ::SysAllocStringLen(NULL, len - 1);
      if (m_str) {
        int res = MultiByteToWideChar(CP_THREAD_ACP, 0, pSrc, -1, m_str, len);
        ATLASSERT(res == len);
        if (res != len) {
          ::SysFreeString(m_str);
          m_str = NULL;
        }
      }
    } else {
      m_str = NULL;
    }
  }

  CComBSTR(const CComBSTR& other) { m_str = other.Copy(); }

  CComBSTR(REFGUID guid) {
    OLECHAR szGuid[40];
    ::StringFromGUID2(guid, szGuid, 40);
    m_str = ::SysAllocString(szGuid);
  }

  ~CComBSTR() {
    ::SysFreeString(m_str);
    m_str = NULL;
  }

  operator BSTR() const { return m_str; }

  BSTR* operator&() { return &m_str; }

  CComBSTR& operator=(const CComBSTR& other) {
    ::SysFreeString(m_str);
    m_str = other.Copy();
    return *this;
  }

  void Attach(BSTR bstr) {
    ::SysFreeString(m_str);
    m_str = bstr;
  }

  BSTR Detach() {
    BSTR str = m_str;
    m_str = NULL;
    return str;
  }

  BSTR Copy() const {
    if (!m_str) return NULL;
    return ::SysAllocStringLen(m_str, ::SysStringLen(m_str));
  }

  HRESULT CopyTo(BSTR* other) const {
    if (!other) return E_POINTER;
    *other = Copy();
    return S_OK;
  }

  /*bool LoadString(HMODULE module, DWORD uID)
  {
      ::SysFreeString(m_str);
      m_str = NULL;
      const wchar_t *ptr = NULL;
      int len = ::LoadStringW(module, uID, (PWSTR)&ptr, 0);
      if (len)
          m_str = ::SysAllocStringLen(ptr, len);
      return m_str != NULL;
  }*/

  unsigned int Length() const { return ::SysStringLen(m_str); }

  unsigned int ByteLength() const { return ::SysStringByteLen(m_str); }
};

class CComVariant : public tagVARIANT {
 public:
  CComVariant() { ::VariantInit(this); }

  CComVariant(const CComVariant& other) {
    V_VT(this) = VT_EMPTY;
    Copy(&other);
  }

  ~CComVariant() { Clear(); }

  CComVariant(LPCOLESTR lpStr) {
    V_VT(this) = VT_BSTR;
    V_BSTR(this) = ::SysAllocString(lpStr);
  }

  CComVariant(LPCSTR lpStr) {
    V_VT(this) = VT_BSTR;
    CComBSTR str(lpStr);
    V_BSTR(this) = str.Detach();
  }

  CComVariant(bool value) {
    V_VT(this) = VT_BOOL;
    V_BOOL(this) = value ? VARIANT_TRUE : VARIANT_FALSE;
  }

  CComVariant(char value) {
    V_VT(this) = VT_I1;
    V_I1(this) = value;
  }

  CComVariant(BYTE value) {
    V_VT(this) = VT_UI1;
    V_UI1(this) = value;
  }

  CComVariant(short value) {
    V_VT(this) = VT_I2;
    V_I2(this) = value;
  }

  CComVariant(unsigned short value) {
    V_VT(this) = VT_UI2;
    V_UI2(this) = value;
  }

  CComVariant(int value, VARENUM type = VT_I4) {
    if (type == VT_I4 || type == VT_INT) {
      V_VT(this) = type;
      V_I4(this) = value;
    } else {
      V_VT(this) = VT_ERROR;
      V_ERROR(this) = E_INVALIDARG;
    }
  }

  CComVariant(unsigned int value, VARENUM type = VT_UI4) {
    if (type == VT_UI4 || type == VT_UINT) {
      V_VT(this) = type;
      V_UI4(this) = value;
    } else {
      V_VT(this) = VT_ERROR;
      V_ERROR(this) = E_INVALIDARG;
    }
  }

  CComVariant(long value, VARENUM type = VT_I4) {
    if (type == VT_I4 || type == VT_ERROR) {
      V_VT(this) = type;
      V_I4(this) = value;
    } else {
      V_VT(this) = VT_ERROR;
      V_ERROR(this) = E_INVALIDARG;
    }
  }

  CComVariant(unsigned long value) {
    V_VT(this) = VT_UI4;
    V_UI4(this) = value;
  }

  CComVariant(float value) {
    V_VT(this) = VT_R4;
    V_R4(this) = value;
  }

  CComVariant(double value, VARENUM type = VT_R8) {
    if (type == VT_R8 || type == VT_DATE) {
      V_VT(this) = type;
      V_R8(this) = value;
    } else {
      V_VT(this) = VT_ERROR;
      V_ERROR(this) = E_INVALIDARG;
    }
  }

  CComVariant(const LONGLONG& value) {
    V_VT(this) = VT_I8;
    V_I8(this) = value;
  }

  CComVariant(const ULONGLONG& value) {
    V_VT(this) = VT_UI8;
    V_UI8(this) = value;
  }

  CComVariant(const CY& value) {
    V_VT(this) = VT_CY;
    V_I8(this) = value.int64;
  }

  CComVariant(IDispatch* value) {
    V_VT(this) = VT_DISPATCH;
    V_DISPATCH(this) = value;

    if (value != NULL)
      value->AddRef();
  }

  HRESULT Clear() { return ::VariantClear(this); }

  HRESULT Copy(_In_ const VARIANT* src) {
    return ::VariantCopy(this, const_cast<VARIANT*>(src));
  }

  HRESULT ChangeType(_In_ VARTYPE newType,
                     _In_opt_ const LPVARIANT src = NULL) {
    const LPVARIANT lpSrc = src ? src : this;
    return ::VariantChangeType(this, lpSrc, 0, newType);
  }

  HRESULT Detach(VARIANT* pDest) {
    if (pDest == NULL)
       return E_POINTER;

    HRESULT hr = ::VariantClear(pDest);
    if (SUCCEEDED(hr))
    {
      ::VariantCopy(const_cast<VARIANT*>(pDest), this);
      hr = S_OK;
    }
    return hr;
  }
};

template <typename T>
struct _ATL_AutomationType {};

#define DEFINE_AUTOMATION_TYPE_FUNCTION(ctype, typewrapper, oleautomationtype) \
  template <>                                                                  \
  struct _ATL_AutomationType<ctype> {                                          \
    typedef typewrapper _typewrapper;                                          \
    enum { type = oleautomationtype };                                         \
    static void* GetT(const ctype& t) throw() { return (void*)&t; }            \
  };

DEFINE_AUTOMATION_TYPE_FUNCTION(VARIANT, CComVariant, VT_VARIANT)

class CComSafeArrayBound : public SAFEARRAYBOUND
{
  public:
	CComSafeArrayBound(ULONG ulCount = 0, LONG lLowerBound = 0) throw()
	{
		cElements = ulCount;
		lLbound = lLowerBound;
	}
	CComSafeArrayBound& operator=(const CComSafeArrayBound& bound) throw()
	{
		cElements = bound.cElements;
		lLbound = bound.lLbound;
		return *this;
	}
	CComSafeArrayBound& operator=(ULONG ulCount) throw() 
	{
		cElements = ulCount;
		lLbound = 0;
		return *this;
	}
	ULONG GetCount() const throw()
	{
		return cElements;
	}
	ULONG SetCount(ULONG ulCount) throw()
	{
		cElements = ulCount;
		return cElements;
	}
	LONG GetLowerBound() const throw()
	{
		return lLbound;
	}
	LONG SetLowerBound(LONG lLowerBound) throw()
	{
		lLbound = lLowerBound;
		return lLbound;
	}
	LONG GetUpperBound() const throw()
	{
		return lLbound + cElements - 1;
	}
};

template <typename T, VARTYPE _vartype = _ATL_AutomationType<T>::type>
class CComSafeArray {
 public:
  CComSafeArray() throw() : m_psa(NULL) {}
  CComSafeArray(ULONG ulCount, LONG lLBound = 0) : m_psa(NULL) {
    CComSafeArrayBound bound(ulCount, lLBound);
    HRESULT hRes = Create(&bound);
  }
  ~CComSafeArray() {
    Destroy();
  }

  HRESULT Create(const SAFEARRAYBOUND *pBound, UINT uDims = 1)
	{
		if(m_psa != NULL)
			return E_FAIL;

		if(pBound == NULL || uDims == 0)
			return E_INVALIDARG;

		HRESULT hRes = S_OK;
		m_psa = SafeArrayCreate(_vartype, uDims, const_cast<LPSAFEARRAYBOUND>(pBound));
		if (NULL == m_psa)
			hRes = E_OUTOFMEMORY;
		else
			hRes = Lock();
		return hRes;
	}

  HRESULT Destroy()
	{
		HRESULT hRes = S_OK;
		if (m_psa != NULL)
		{
			hRes = Unlock();
			if (SUCCEEDED(hRes))
			{
				hRes = SafeArrayDestroy(m_psa);
				if (SUCCEEDED(hRes))
					m_psa = NULL;
			}
		}
		return hRes;
	}

  const typename _ATL_AutomationType<T>::_typewrapper& operator[](
      LONG nIndex) const {
    return GetAt(nIndex);
  }

  typename _ATL_AutomationType<T>::_typewrapper& operator[](LONG nIndex) {
    return GetAt(nIndex);
  }

  typename _ATL_AutomationType<T>::_typewrapper& GetAt(LONG lIndex) {
    LONG lLBound = GetLowerBound();

    return ((CComVariant*)m_psa->pvData)[lIndex - lLBound];
  }

  LONG GetLowerBound(UINT uDim = 0) const {
    LONG lLBound = 0;
    HRESULT hRes = SafeArrayGetLBound(m_psa, uDim + 1, &lLBound);
    return lLBound;
  }

  LONG GetUpperBound(UINT uDim = 0) const {
    LONG lUBound = 0;
    HRESULT hRes = SafeArrayGetUBound(m_psa, uDim + 1, &lUBound);
    return lUBound;
  }

  LPSAFEARRAY Detach() {
    Unlock();
    LPSAFEARRAY pTemp = m_psa;
    m_psa = NULL;
    return pTemp;
  }

 protected:
  HRESULT Lock() {
    return SafeArrayLock(m_psa);
  }
  HRESULT Unlock() {
    return SafeArrayUnlock(m_psa);
  }

 public:
  LPSAFEARRAY m_psa;
};

#include "helpers.h"

#include "def.h"

class IProcessDebugManager {};

class CAtlModule {
 public:
  static GUID m_libid;
};

class CComTypeInfoHolder {
public:
  HRESULT GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
  {
    return S_OK;
  }
};

template <const CLSID* pcoclsid, const IID* psrcid,
          const GUID* plibid = &CAtlModule::m_libid, WORD wMajor = 1,
          WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
class ATL_NO_VTABLE IProvideClassInfo2Impl : public IProvideClassInfo2
{
public:
  STDMETHOD(GetClassInfo)(ITypeInfo** pptinfo)
  {
    return S_OK;
  }
  
  STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID* pGUID)
  {
    return E_FAIL;
  }
};

template <class T>
class _CopyInterface
{
};

class CComEnumConnectionPoints
{
};

template <class Base, const IID *piid, class T, class Copy>
class CComEnumImpl : public Base
{
};

template <class Base, const IID* piid, class T, class Copy, class ThreadModel = CComObjectThreadModel>
class ATL_NO_VTABLE CComEnum :
  public CComEnumImpl<Base, piid, T, Copy>,
  public CComObjectRootEx< ThreadModel >
{
public:
  typedef CComEnum<Base, piid, T, Copy > _CComEnum;
  typedef CComEnumImpl<Base, piid, T, Copy > _CComEnumBase;
  BEGIN_COM_MAP(_CComEnum)
    COM_INTERFACE_ENTRY_IID(*piid, _CComEnumBase)
  END_COM_MAP()
};

template <class T>
class ATL_NO_VTABLE IConnectionPointContainerImpl : public IConnectionPointContainer {
        typedef const _ATL_CONNMAP_ENTRY * (*handlerFunctionType)(int *);
        typedef CComEnum<IEnumConnectionPoints, &IID_IEnumConnectionPoints, IConnectionPoint *, _CopyInterface<IConnectionPoint> >
                        CComEnumConnectionPoints;

  STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints** ppEnum)
  {
    return S_OK;
  }

  STDMETHOD(FindConnectionPoint)(REFIID riid, IConnectionPoint** ppCP)
  {
    return S_OK;
  }
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

  STDMETHOD(_LocCPQueryInterface)(REFIID riid, void **ppvObject)
  {
    return E_NOINTERFACE;
  }
  STDMETHOD(Advise)(IUnknown* pUnkSink, DWORD* pdwCookie) {
    return S_OK;
  }
};

template <class T, const IID* piid /*= &__uuidof(T)*/,
          const GUID* plibid = &CAtlModule::m_libid, WORD wMajor = 1,
          WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
class IDispatchImpl : public T {
    CComPtr<ITypeInfo> m_pTypeInfo;

  STDMETHOD(EnsureTILoaded)(LCID lcid) {
    return S_OK;
  }

  STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) {
    return S_OK;
  }

  STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) {
    return S_OK;
  }

  STDMETHOD(GetIDsOfNames)
  (REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,
   DISPID* rgDispId) {
        return S_OK;
  }

  STDMETHOD(Invoke)
  (DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags,
   DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo,
   UINT* puArgErr) {
        return S_OK;
  }
};

class CException : public std::exception {
 public:
  LPCTSTR GetText() const { return ""; };
};

class CWinApp {};

class CDC {};

class CWnd {
 public:
  virtual void PreRegisterClass(WNDCLASS& wc){};
  virtual void PreCreate(CREATESTRUCT& cs) {}
  virtual int OnCreate(CREATESTRUCT& cs) { return 0; };
  virtual void OnClose(){};
  virtual LRESULT OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam) {
    return 0;
  };
  virtual BOOL OnEraseBkgnd(CDC& dc) { return false; };
  virtual void OnInitialUpdate(){};
  virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
  };
  virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) { return false; };
  virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) { return 0; };
  virtual void Destroy() { };

  BOOL Create(CWnd* pParentWnd) { return true; }
  int MessageBox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) {
    return ::MessageBox(0L, lpText, lpCaption, uType);
  }
  HWND GetHwnd() { return 0L; }
  
};

class CMenu {};
class CDialog {
 public:
  CDialog(int id){};

 protected:
  virtual BOOL OnInitDialog() { return true; };
  virtual void OnOK(){};
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) { return false; };
  virtual void OnClose(){};
  virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
  };
  virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) { return 0; };
};

class CDocker : public CWnd {};
class CMDIDockFrame : public CDocker {};
class CMDIChild : public CWnd {};
class CDockProperty {};
class CDockNotes {};
class CDockLayers {};
class CDockToolbar {};
class CComboBox {};
class CEdit {};
class CButton {};
class CColorDialog {};
class CResizer {};
class CRect {};
class CProgressBar {};
class CStatic {};

class SearchSelectDialog {};
class LayersListDialog {};
class ImageDialog {};
class SoundDialog {};
class AudioOptionsDialog {};
class VideoOptionsDialog {};
class VROptionsDialog {};
class EditorOptionsDialog {};
class CollectionManagerDialog {};
class PhysicsOptionsDialog {};
class TableInfoDialog {};
class DimensionDialog {};
class MaterialDialog {};
class AboutDialog {};
class ToolbarDialog {};
class PropertyDialog {};
class NotesDialog {};

class ColorButton {};

class SCNotification {};

#undef _WIN64
#include "pole.h"
#define _WIN64

class PoleStream : public IStream {
 public:
  POLE::Stream* m_pStream = nullptr;

  PoleStream(POLE::Storage* pStorage, const char* szPath) {
    m_pStream = new POLE::Stream(pStorage, szPath);
  }

  HRESULT Read(void* pv, ULONG cb, ULONG* pcbRead) {
    *pcbRead = m_pStream->read((unsigned char*)pv, cb);
    return S_OK;
  }

  HRESULT Write(const void* pv, ULONG cb, ULONG* pcbWritten) { return E_FAIL; }

  HRESULT Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
               ULARGE_INTEGER* plibNewPosition) {
    if (dwOrigin == STREAM_SEEK_SET) {
      m_pStream->seek(dlibMove.QuadPart);
    } else if (dwOrigin == STREAM_SEEK_CUR) {
      m_pStream->seek(m_pStream->tell() + dlibMove.QuadPart);
    }
    return S_OK;
  }

  HRESULT Stat(STATSTG* pstatstg, DWORD grfStatFlag) {
    pstatstg->cbSize.LowPart = m_pStream->size();
    return S_OK;
  }

  ULONG STDMETHODCALLTYPE Release() {
    if (m_pStream) {
      delete m_pStream;
      m_pStream = nullptr;
    }
    delete this;
    return S_OK;
  }

  HRESULT SetSize(union _ULARGE_INTEGER) { return S_OK; }

  HRESULT CopyTo(struct IStream*, union _ULARGE_INTEGER, union _ULARGE_INTEGER*,
                 union _ULARGE_INTEGER*) {
    return S_OK;
  }

  HRESULT Commit(ULONG) { return S_OK; }

  HRESULT Revert() { return S_OK; }

  HRESULT LockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, ULONG) {
    return S_OK;
  }

  HRESULT UnlockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, ULONG) {
    return S_OK;
  }

  HRESULT Clone(struct IStream**) { return S_OK; }

  HRESULT QueryInterface(REFIID, void**) { return S_OK; }
  ULONG AddRef() { return 0L; }
};

class PoleStorage : public IStorage {
 public:
  POLE::Storage* m_pStorage = nullptr;
  string m_szName;
  string m_szPath;

  void SetSize(const unsigned int i){};

  HRESULT OpenStream(LPCOLESTR pwcsName, void* reserved1, DWORD grfMode,
                     DWORD reserved2, IStream** ppstm) {
    char szName[1024];
    WideCharToMultiByteNull(CP_ACP, 0, pwcsName, -1, szName, sizeof(szName),
                            nullptr, nullptr);

    string path = m_szPath;
    if (path.length() > 0) {
      path.append("/");
    }
    path.append(szName);

    if (m_pStorage->exists(path.c_str())) {
      *ppstm = new PoleStream(m_pStorage, path.c_str());
      return S_OK;
    }

    return E_FAIL;
  }

  HRESULT CreateStream(LPCOLESTR pwcsName, DWORD grfMode, DWORD reserved1,
                       DWORD reserved2, IStream** ppstm) {
    return -1L;
  }

  HRESULT OpenStorage(LPCOLESTR pwcsName, IStorage* pstgPriority, DWORD grfMode,
                      SNB snbExclude, DWORD reserved, IStorage** ppstg) {
    PoleStorage* pStorage = nullptr;

    char szName[1024];
    WideCharToMultiByteNull(CP_ACP, 0, pwcsName, -1, szName, sizeof(szName),
                            nullptr, nullptr);

    if (m_pStorage == nullptr) {
      m_szName = string(szName);
      m_pStorage = new POLE::Storage(m_szName.c_str());
      pStorage = this;
    } else {
      string path = m_szPath;
      if (path.length() > 0) {
        path.append("/");
      }
      path.append(szName);

      if (m_pStorage->exists(path.c_str())) {
        pStorage = new PoleStorage();
        pStorage->m_szName = m_szName;
        pStorage->m_szPath = path;
        pStorage->m_pStorage = new POLE::Storage(m_szName.c_str());
      }
    }

    if (pStorage) {
      *ppstg = pStorage;

      if (pStorage->m_pStorage->open()) {
        if (pStorage->m_pStorage->result() == POLE::Storage::Ok) {
          return S_OK;
        }
      }
    }

    return E_FAIL;
  }

  HRESULT CreateStorage(LPCOLESTR pwcsName, DWORD grfMode, DWORD dwStgFmt,
                        DWORD reserved2, IStorage** ppstg) {
    return -1L;
  }

  ULONG STDMETHODCALLTYPE Release() {
    if (m_pStorage) {
      delete m_pStorage;
      m_pStorage = nullptr;
    }
    delete this;
    return S_OK;
  }

  HRESULT CopyTo(ULONG, const struct _GUID*, WCHAR**,
                 struct IStorage* pstgNew) {
    return S_OK;
  }

  HRESULT MoveElementTo(const WCHAR*, struct IStorage*, const WCHAR*, ULONG) {
    return S_OK;
  }

  HRESULT Commit(ULONG) { return S_OK; }

  HRESULT Revert() { return S_OK; }

  HRESULT EnumElements(ULONG, void*, ULONG, struct IEnumSTATSTG**) {
    return S_OK;
  }

  HRESULT DestroyElement(const WCHAR*) { return S_OK; }

  HRESULT RenameElement(const WCHAR*, const WCHAR*) { return S_OK; }

  HRESULT SetElementTimes(const WCHAR*, const struct _FILETIME*,
                          const struct _FILETIME*, const struct _FILETIME*) {
    return S_OK;
  }

  HRESULT SetClass(const struct _GUID&) { return S_OK; }

  HRESULT SetStateBits(ULONG, ULONG) { return S_OK; }

  HRESULT Stat(struct tagSTATSTG*, ULONG) { return S_OK; }

  HRESULT QueryInterface(REFIID, void**) { return S_OK; }
  ULONG AddRef() { return 0L; }
};

HRESULT StgOpenStorage(const OLECHAR* pwcsName, IStorage* pstgPriority,
                       DWORD grfMode, SNB snbExclude, DWORD reserved,
                       IStorage** ppstgOpen);

inline int GetWinVersion() { return 2601; }

#include "math/math.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "math/bbox.h"

#include "dispid.h"

#include "variant.h"
#include "vector.h"
#include "vectorsort.h"
#include "vpinball_standalone.h"
#include "regutil.h"

#include "idebug.h"

#include "eventproxy.h"

#include "audioplayer.h"
#include "media/fileio.h"
#include "pinundo.h"
#include "iselect.h"

#include "ieditable.h"
#include "codeview.h"

#include "media/lzwreader.h"
#include "media/lzwwriter.h"

#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#undef PlaySound

#define DSBCAPS_STATIC              0x00000002
#define DSBCAPS_CTRLVOLUME          0x00000080
#define DSBCAPS_CTRLFREQUENCY       0x00000020
#define DSBCAPS_CTRLPAN             0x00000040
#define DSBCAPS_CTRL3D              0x00000010

typedef struct _DSBUFFERDESC
{
    DWORD		dwSize;
    DWORD		dwFlags;
    DWORD		dwBufferBytes;
    DWORD		dwReserved;
    LPWAVEFORMATEX	lpwfxFormat;
    GUID		guid3DAlgorithm;
} DSBUFFERDESC,*LPDSBUFFERDESC;

#define IID_IDirectSound3DBuffer GUID_NULL

#define DSBSTATUS_PLAYING           0x00000001
#define DSBPAN_RIGHT                 10000
#define DS3D_IMMEDIATE              0x00000000

typedef BOOL (CALLBACK *LPDSENUMCALLBACKA)(LPGUID,LPCSTR,LPCSTR,LPVOID);

HRESULT WINAPI DirectSoundEnumerate(
    LPDSENUMCALLBACKA lpDSEnumCallback,
    LPVOID lpContext);

typedef struct IDirectSoundBuffer {
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject) { return 0L; };
  HRESULT GetStatus(DWORD* status) { return 0L; }
  HRESULT Lock(DWORD writecursor,
        DWORD writebytes, void **lplpaudioptr1, DWORD *audiobytes1, void **lplpaudioptr2,
        DWORD *audiobytes2, DWORD flags) { return 0L; };
  HRESULT Unlock(void *p1, DWORD x1, void *p2, DWORD x2) { return 0L; };
  HRESULT SetVolume(LONG vol) { return 0L; }
  HRESULT SetPan(LONG pan) { return 0L; }
  HRESULT SetCurrentPosition(DWORD newpos) { return 0L; }
  HRESULT Play(DWORD reserved1, DWORD reserved2, DWORD flags) { return 0L; }
  HRESULT SetFrequency(DWORD freq) { return 0L; }
  HRESULT GetFrequency(DWORD *freq) { return 0L; }
  HRESULT Stop() { return 0L; };
  ULONG Release() { return 0L; }
} * LPDIRECTSOUNDBUFFER, **LPLPDIRECTSOUNDBUFFER;

typedef struct IDirectSound3DBuffer {
  HRESULT SetMinDistance(D3DVALUE fMinDistance, DWORD dwApply) { return 0L; }
  HRESULT SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply) { return 0L; }
  ULONG Release() { return 0L; }
} * LPDIRECTSOUND3DBUFFER;

typedef struct IDirectSound {
  HRESULT CreateSoundBuffer(const DSBUFFERDESC *dsbd, IDirectSoundBuffer **ppdsb, IUnknown *lpunk) { return 0L; }
  HRESULT DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER psb, LPLPDIRECTSOUNDBUFFER ppdsb) { return 0L; };
  ULONG Release() { return 0L; }
} * LPDIRECTSOUND;

typedef struct IDirectSound3DListener {
  ULONG Release() { return 0L; }
} * LPDIRECTSOUND3DLISTENER;

#include "pinsound.h"
#include "pinbinary.h"

#include "extern.h"

#include "vpinball_h.h"
#include "pintable.h"

#include "mesh.h"
#include "pin/collide.h"
#include "pin3d.h"

#include "sur.h"
#include "paintsur.h"
#include "hitsur.h"
#include "hitrectsur.h"

#include "ballex.h"

#include "pin/collideex.h"
#include "pin/ball.h"
#include "pin/hittimer.h"
#include "pin/hitable.h"
#include "pin/hitflipper.h"
#include "pin/hitplunger.h"
#include "pin/player.h"

#include "color.h"

#include "dragpoint.h"
#include "timer.h"
#include "flipper.h"
#include "plunger.h"
#include "textbox.h"
#include "surface.h"
#include "dispreel.h"
#include "lightseq.h"
#include "bumper.h"
#include "trigger.h"
#include "light.h"
#include "kicker.h"
#include "decal.h"
#include "primitive.h"
#include "hittarget.h"
#include "gate.h"
#include "spinner.h"
#include "ramp.h"
#include "flasher.h"
#include "rubber.h"
#include "mixer.h"
#include "hid.h"
#include "plumb.h"

#include "kdtree.h"

#include "wintimer.h"

#include "trace.h"

inline void ShowError(const char* const sz) { std::cout << sz << std::endl; }

inline void ShowError(const string& sz) { std::cout << sz << std::endl; }

#include "editablereg.h"
#include "typedefs3D.h"

__forceinline float getBGxmult() { return g_pplayer->m_AAfactor; }

__forceinline float getBGymult() { return g_pplayer->m_AAfactor; }

inline void* _aligned_malloc(size_t size, size_t align) {
  void* result;
#ifdef _MSC_VER
  result = _aligned_malloc(size, align);
#else
  if (posix_memalign(&result, align, size)) result = 0;
#endif
  return result;
}

inline void _aligned_free(void* ptr) {
#ifdef _MSC_VER
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}

#include "inc/vpinmame/vpinmame.h"
#include "inc/vpinmame/vpinmame_c.h"

static CMDIChild* g_pActiveMDIChild = nullptr;

inline CMDIChild* GetActiveMDIChild() { return g_pActiveMDIChild; }

inline void AddMDIChild(CMDIChild* pChild) { g_pActiveMDIChild = pChild; }

extern "C" HRESULT vpinball_create_object(const WCHAR *progid, IClassFactory* cf, IUnknown* obj);

#endif
