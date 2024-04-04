/*
 * Portions of this code was derived from ReactOS ATL:
 *
 * https://github.com/reactos/reactos/blob/master/sdk/lib/atl/atldef.h
 */

#pragma once

#define _ATL_PACKING 8

#define ATLASSERT(expr) _ASSERTE(expr)

#define ATL_NO_VTABLE

#define offsetofclass(base, derived) (reinterpret_cast<DWORD_PTR>(static_cast<base *>(reinterpret_cast<derived *>(_ATL_PACKING))) - _ATL_PACKING)

#define _ATL_IIDOF(x)   IID_##x