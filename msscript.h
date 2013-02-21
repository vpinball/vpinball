//=--------------------------------------------------------------------------=
// MSScript.h
//=--------------------------------------------------------------------------=
// (C) Copyright 1996 Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
//
// Declarations for Microsoft-provided script engines that support ActiveX Scripting.
//

#pragma once

#include "windows.h"
#include "ole2.h"


#ifndef MSSCRIPT_H
#define MSSCRIPT_H

// The GUID used to identify the coclass of the VB Script engine
// 	{B54F3741-5B07-11cf-A4B0-00AA004A55E8}
#define szCLSID_VBScript "{B54F3741-5B07-11cf-A4B0-00AA004A55E8}"
DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf, 0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8);

// The GUID used to identify the coclass of the JavaScript engine
// 	{f414c260-6ac0-11cf-b6d1-00aa00bbbb58}
#define szCLSID_JScript "{f414c260-6ac0-11cf-b6d1-00aa00bbbb58}"
DEFINE_GUID(CLSID_JScript, 0xf414c260, 0x6ac0, 0x11cf, 0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58);

#endif // MSSCRIPT_H

// End of file
