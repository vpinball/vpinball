// This file is designed to be included in D3D9-dependent code instead of d3d9.h, while adding minimal amount of junk
#pragma once
 
#include <BaseTyps.h>
#include <BaseTsd.h>
 
// stdlib.h
#ifndef _INC_STDLIB
#define _INC_STDLIB
#endif
 
// Objbase.h
#ifndef _OBJBASE_H_
#define _OBJBASE_H_
#endif
 
// Rpc.h
#ifndef __RPC_H__
#define __RPC_H__
#endif
 
// RpcNdr.h
#ifndef __RPCNDR_H__
#define __RPCNDR_H__
#endif
 
// Windows.h
#ifndef _WINDOWS_
#define _WINDOWS_
#endif
 
// WinNT.h
#ifndef _WINNT_
typedef void *HANDLE;
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
 
#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif
 
#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef LONG HRESULT;
#endif
 
typedef const wchar_t *LPCWSTR;
#define DECLSPEC_NOVTABLE   __declspec(novtable)
#endif
 
#ifndef NT_INCLUDED
#define NT_INCLUDED
#endif
 
// windef.h
#ifndef _WINDEF_
#define _X86_
#include <windef.h>
#endif
 
// WinNT.h
#ifndef _WINNT_
#define _WINNT_
 
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;
 
typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;
 
typedef struct _LUID {
    DWORD LowPart;
    LONG HighPart;
} LUID, *PLUID;
#endif
 
// Unknwn.h
#ifndef __unknwn_h__
interface IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject) = 0;
       
    virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;
       
    virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
};
#endif
 
// WinGDI.h
#ifndef _WINGDI_
typedef struct _RGNDATA RGNDATA;
typedef struct tagPALETTEENTRY PALETTEENTRY;
#endif
 
// WinError.h
#ifndef _WINERROR_
#define _WINERROR_
 
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
#define FAILED(hr) ((HRESULT)(hr) < 0)
#define S_OK                                   ((HRESULT)0x00000000L)
#endif
 
// d3d9.h
#include <d3d9.h>