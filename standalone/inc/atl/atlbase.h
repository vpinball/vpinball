/*
 * Portions of this code was derived from ReactOS ATL:
 *
 * https://github.com/reactos/reactos/blob/master/sdk/lib/atl/atlbase.h
 */

#pragma once

#define _ATL_SIMPLEMAPENTRY ((_ATL_CREATORARGFUNC *)1)

class CAtlModule {
public:
  static GUID m_libid;
};

class CComMultiThreadModel { };
class CComSingleThreadModel { };