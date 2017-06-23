//---------------------------------------------------------------------------
// Helpers.h
//---------------------------------------------------------------------------
// General OLE Automation helper functions.
//---------------------------------------------------------------------------
/*	Copyright (C) Microsoft Corporation, 1999.  All rights reserved.


   This source code is intended only as a supplement to Microsoft
   Development Tools and/or on-line documentation.  See these other
   materials for detailed information regarding Microsoft code samples.
   */

#pragma once

//---------------------------------------------------------------------------
// Allocates a temporary buffer that will disappear when it goes out of scope
// NOTE: Be careful of that-- make sure you use the string in the same or
// nested scope in which you created this buffer.  People should not use this
// class directly; use the macro(s) below.
//---------------------------------------------------------------------------
class TempBuffer
{
public:
   TempBuffer(const ULONG cb)
   {
      m_fAlloc = (cb > 256);
      if (m_fAlloc)
         m_pbBuf = new char[cb];
      else
         m_pbBuf = m_szBufT;
   }
   ~TempBuffer()
   {
      if (m_pbBuf && m_fAlloc) delete[] m_pbBuf;
   }
   char *GetBuffer() const
   {
      return m_pbBuf;
   }

private:
   char *m_pbBuf;
   char  m_szBufT[256];  // We'll use this temp buffer for small cases.
   bool  m_fAlloc;
};


//---------------------------------------------------------------------------
// String helpers.
//---------------------------------------------------------------------------
// Given and ANSI String, copy it into a wide buffer.
// NOTE: Be careful about scoping when using this macro!
//
// How to use the below two macros:
//
//  ...
//  LPSTR pszA;
//  pszA = MyGetpszAnsiingRoutine();
//  MAKE_WIDEPTR_FROMANSI(pwsz, pszA);
//  MyUseWideStringRoutine(pwsz);
//  ...
//
// Similarily for MAKE_ANSIPTR_FROMWIDE().  Note that the first param does
// not have to be declared, and no clean up must be done.
//---------------------------------------------------------------------------
#define UNICODE_FROM_ANSI(pwszUnicode, pszAnsi, cb) \
    MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, pwszUnicode, cb);

#define MAKE_WIDEPTR_FROMANSI(ptrname, pszAnsi) \
    char *__psz##ptrname = pszAnsi?pszAnsi:""; \
    long __l##ptrname = (lstrlen(__psz##ptrname) + 1) * (long)sizeof(WCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    MultiByteToWideChar(CP_ACP, 0, __psz##ptrname, -1, (LPWSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname.GetBuffer()

#define MAKE_ANSIPTR_FROMWIDE(ptrname, pwszUnicode) \
    WCHAR *__pwsz##ptrname = pwszUnicode?pwszUnicode:L""; \
    long __l##ptrname = (lstrlenW(__pwsz##ptrname) + 1) * (long)sizeof(char); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    WideCharToMultiByte(CP_ACP, 0, __pwsz##ptrname, -1, (LPSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname, NULL, NULL); \
    LPSTR ptrname = (LPSTR)__TempBuffer##ptrname.GetBuffer()

//--- EOF -------------------------------------------------------------------
