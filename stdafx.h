// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_)
#define AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _CRT_SECURE_NO_DEPRECATE 1

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
class CExeModule : public CComModule
{
public:
	LONG Unlock();
	DWORD dwThreadID;
	HANDLE hEventShutdown;
	void MonitorShutdown();
	bool StartMonitor();
	bool bActivity;
};
extern CExeModule _Module;

#include <Windows.h>

#include "Richedit.h"
#include "RichOle.h"
#include "Tom.h"

#include <atlcom.h>

#ifdef VBA
#define APC_IMPORT_MIDL
#endif

#include <atlbase.h>
#include <atlhost.h>
//extern CComModule _Module;
#ifdef VBA
#include <apcCpp.h>
using namespace MSAPC;
#endif

//#include "evalkey.h"

#include "math.h"

#include "commctrl.h"

#include "vp-sup\inc\activdbg.h"

#include <atlctl.h>
#include <atlhost.h>

#define DIRECTINPUT_VERSION 0x0700
#define D3D_OVERLOADS 1
#include "dinput.h"
#include "ddraw.h"
#include "d3d.h"
#include "dsound.h"
#include <ocidl.h>	// Added by ClassView

#include "stdio.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED)
