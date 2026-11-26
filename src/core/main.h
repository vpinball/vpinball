// license:GPLv3+

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
   #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
   #define NOMINMAX
#endif

#ifdef __STANDALONE__
   #define __WINE_WINCON_H
#endif

#include <windows.h>

#ifdef __STANDALONE__
   #ifdef GetClassInfo
   #undef GetClassInfo
   #endif
#endif

#ifndef __STANDALONE__
   #define RPC_NO_WINDOWS_H
   #define COM_NO_WINDOWS_H
   #include <objbase.h>
#endif

#if defined(ENABLE_DX9)
 #ifdef _DEBUG
  #define D3D_DEBUG_INFO
 #endif
 #include "minid3d9.h"
#endif

#include <atlbase.h>

#ifndef __STANDALONE__
   #include <atlctl.h>
#endif

#ifdef _MSC_VER
   #define PATH_SEPARATOR_CHAR '\\'
   #define PATH_SEPARATOR_WCHAR L'\\'
#else
   #define PATH_SEPARATOR_CHAR '/'
   #define PATH_SEPARATOR_WCHAR L'/'
#endif

#include <oleauto.h>

#ifndef __STANDALONE__
   #include <intrin.h>
#endif
#if defined(_M_IX86) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
   #define ENABLE_SSE_OPTIMIZATIONS
   #include <xmmintrin.h>
#elif (defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && (!defined(__ARM_ARCH) || __ARM_ARCH >= 7) && (!defined(_MSC_VER) || defined(__clang__)) //!! disable sse2neon if MSVC&non-clang
   #define ENABLE_SSE_OPTIMIZATIONS
   #include "sse2neon.h"
#endif
#ifdef __SSSE3__
 #include <tmmintrin.h>
#endif

#include <vector>
using std::vector;

#include <format>
#include <algorithm>

#include <string>
using namespace std::string_literals;
using namespace std::string_view_literals;
using std::string;
using std::wstring;


#ifndef __STANDALONE__
// commdlg/dlgs/cderr: required by win32xx headers (wxx_appcore.h uses PRINTDLG, wxx_commondlg.h
// uses OPENFILENAME/pshHelp/CDERR_* ...) which never include them, and by the ui/win file
// dialogs. Kept here as the framework's expected pre-include rather than repeating it ahead of
// every wxx header.
#include <commdlg.h>
#include <dlgs.h>
#include <cderr.h>

#else
   #ifndef _MSC_VER // MSVC provides the secure CRT functions (and deprecates the plain ones)
      #define fopen_s(pFile, filename, mode) (((*(pFile)) = fopen((filename), (mode))) == nullptr)
      #define fprintf_s fprintf
      #define fscanf_s fscanf
      #define sscanf_s sscanf
   #endif

   #ifndef _WIN32
      #define localtime_s(x, y) localtime_r(y, x)
      #define gmtime_s(x, y) gmtime_r(y, x)
      // asctime_r needs at least 26 bytes and takes no size, so that argument is dropped
      #define asctime_s(x, size, y) asctime_r(y, x)
      #define strnlen_s strnlen
      #define _aligned_malloc(size, align) aligned_alloc(align, size)
      #define _aligned_free free
   #endif

   #ifndef _ASSERTE
      #define _ASSERTE(expr) ((void)0)
   #endif

   #ifndef VPX_HAS_WIN32_SDK // Otherwise from wincrypt.h
      typedef ULONG_PTR HCRYPTKEY;
   #endif

#include <atldef.h>
#include <atlcom.h>
#include <atlcomcli.h>
#include <atlsafe.h>

   #ifdef _MSC_VER
      // Microsoft's ATL serves the type information from the registered type library, which this build has none of: VBScript's
      // TypeName then returns "Object" for every scriptable class. Like wine's ATL (libwinevbs, MinGW build), the object is its own
      // ITypeInfo, which only answers GetDocumentation(MEMBERID_NIL) with the class name (the IDL generated GetDocumentation)
      template <class T, const IID *piid, const GUID *plibid, WORD wMajor = 1, WORD wMinor = 0>
      class StandaloneDispatchImpl : public T, public ITypeInfo
      {
      public:
         STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) override
         {
            if (pctinfo == nullptr)
               return E_POINTER;
            *pctinfo = 1;
            return S_OK;
         }
         STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) override
         {
            if (ppTInfo == nullptr)
               return E_POINTER;
            *ppTInfo = static_cast<ITypeInfo *>(this);
            (*ppTInfo)->AddRef();
            return S_OK;
         }

         STDMETHOD(GetTypeAttr)(TYPEATTR **ppTypeAttr) override { return E_NOTIMPL; }
         STDMETHOD(GetTypeComp)(ITypeComp **ppTComp) override { return E_NOTIMPL; }
         STDMETHOD(GetFuncDesc)(UINT index, FUNCDESC **ppFuncDesc) override { return E_NOTIMPL; }
         STDMETHOD(GetVarDesc)(UINT index, VARDESC **ppVarDesc) override { return E_NOTIMPL; }
         STDMETHOD(GetNames)(MEMBERID memid, BSTR *rgBstrNames, UINT cMaxNames, UINT *pcNames) override { return E_NOTIMPL; }
         STDMETHOD(GetRefTypeOfImplType)(UINT index, HREFTYPE *pRefType) override { return E_NOTIMPL; }
         STDMETHOD(GetImplTypeFlags)(UINT index, INT *pImplTypeFlags) override { return E_NOTIMPL; }
         STDMETHOD(GetIDsOfNames)(LPOLESTR *rgszNames, UINT cNames, MEMBERID *pMemId) override { return E_NOTIMPL; }
         STDMETHOD(Invoke)(PVOID pvInstance, MEMBERID memid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override { return E_NOTIMPL; }
         STDMETHOD(GetDocumentation)(MEMBERID memid, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) override { return E_NOTIMPL; }
         STDMETHOD(GetDllEntry)(MEMBERID memid, INVOKEKIND invKind, BSTR *pBstrDllName, BSTR *pBstrName, WORD *pwOrdinal) override { return E_NOTIMPL; }
         STDMETHOD(GetRefTypeInfo)(HREFTYPE hRefType, ITypeInfo **ppTInfo) override { return E_NOTIMPL; }
         STDMETHOD(AddressOfMember)(MEMBERID memid, INVOKEKIND invKind, PVOID *ppv) override { return E_NOTIMPL; }
         STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, PVOID *ppvObj) override { return E_NOTIMPL; }
         STDMETHOD(GetMops)(MEMBERID memid, BSTR *pBstrMops) override { return E_NOTIMPL; }
         STDMETHOD(GetContainingTypeLib)(ITypeLib **ppTLib, UINT *pIndex) override { return E_NOTIMPL; }
         STDMETHOD_(void, ReleaseTypeAttr)(TYPEATTR *pTypeAttr) override { }
         STDMETHOD_(void, ReleaseFuncDesc)(FUNCDESC *pFuncDesc) override { }
         STDMETHOD_(void, ReleaseVarDesc)(VARDESC *pVarDesc) override { }
      };

      // VBScript's TypeName asks IProvideClassInfo first: as in wine's ATL, serve the IDispatch's type information
      template <const CLSID *pcoclsid, const IID *psrcid, const GUID *plibid, WORD wMajor = 1, WORD wMinor = 0>
      class ATL_NO_VTABLE StandaloneProvideClassInfo2Impl : public IProvideClassInfo2
      {
      public:
         STDMETHOD(GetClassInfo)(ITypeInfo **pptinfo) override
         {
            if (pptinfo == nullptr)
               return E_POINTER;
            *pptinfo = nullptr;
            IDispatch *disp = nullptr;
            if (FAILED(QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&disp))) || disp == nullptr)
               return E_NOTIMPL;
            const HRESULT hr = disp->GetTypeInfo(0, 0, pptinfo);
            disp->Release();
            return hr;
         }
         STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID *pGUID) override { return E_FAIL; }
      };

      // The scriptable classes name them unqualified (ATL's are reached through its 'using namespace ATL')
      #define IDispatchImpl StandaloneDispatchImpl
      #define IProvideClassInfo2Impl StandaloneProvideClassInfo2Impl
   #endif
#endif


// TODO Custom defines to be removed from PCH to get back to a cleaner codebase with includes corresponding to the actual dependencies of each file (beside std/OS/...)

#ifndef __STANDALONE__
#include "vpinball.h"
#else
#include "core/generated/vpinball_standalone_i.h"
#endif

#include "utils/Logger.h"
#include "core/def.h"
#include "core/dispid.h"
#include "core/Settings.h"
#include "core/SettingsService.h"
#include "core/extern.h"
#include "core/player.h"
