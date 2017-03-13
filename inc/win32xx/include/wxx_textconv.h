// Win32++   Version 8.4
// Release Date: 10th March 2017
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2017  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// wxx_textconv.h
//  Definitions of the CAtoA, CAtoW, CWtoA, CWtoW,
//	  CAtoBSTR and CWtoBSTR classes

#ifndef _WIN32XX_TEXTCONV_H_
#define _WIN32XX_TEXTCONV_H_

#include <vector>

namespace Win32xx
{
	////////////////////////////////////////////////////////
	// Classes and functions (typedefs) for text conversions
	//
	//  This section defines the following text conversions:
	//  AtoBSTR		ANSI  to BSTR
	//  AtoOLE		ANSI  to OLE
	//	AtoT		ANSI  to TCHAR
	//	AtoW		ANSI  to WCHAR
	//  OLEtoA		OLE   to ANSI
	//  OLEtoT		OLE   to TCHAR
	//  OLEtoW		OLE   to WCHAR
	//  TtoA		TCHAR to ANSI
	//  TtoBSTR		TCHAR to BSTR
	//  TtoOLE      TCHAR to OLE
	//  TtoW		TCHAR to WCHAR
	//  WtoA		WCHAR to ANSI
	//  WtoBSTR		WCHAR to BSTR
	//  WtoOLE		WCHAR to OLE
	//  WtoT		WCHAR to TCHAR

	// About different character and string types:
	// ------------------------------------------
	// char (or CHAR) character types are ANSI (8 bits).
	// wchar_t (or WCHAR) character types are Unicode (16 bits).
	// TCHAR characters are Unicode if the _UNICODE macro is defined, otherwise they are ANSI.
	// BSTR (Basic String) is a type of string used in Visual Basic and COM programming.
	// OLE is the same as WCHAR. It is used in Visual Basic and COM programming.


	// Forward declarations of our classes. They are defined later.
	class CAtoA;
	class CAtoW;
	class CWtoA;
	class CWtoW;
	class CAtoBSTR;
	class CWtoBSTR;

	// typedefs for the well known text conversions
	typedef CAtoW AtoW;
	typedef CWtoA WtoA;
	typedef CWtoBSTR WtoBSTR;
	typedef CAtoBSTR AtoBSTR;
	typedef CWtoA BSTRtoA;
	typedef CWtoW BSTRtoW;

#ifdef _UNICODE
	typedef CAtoW AtoT;
	typedef CWtoA TtoA;
	typedef CWtoW TtoW;
	typedef CWtoW WtoT;
	typedef CWtoBSTR TtoBSTR;
	typedef BSTRtoW BSTRtoT;
#else
	typedef CAtoA AtoT;
	typedef CAtoA TtoA;
	typedef CAtoW TtoW;
	typedef CWtoA WtoT;
	typedef CAtoBSTR TtoBSTR;
	typedef BSTRtoA BSTRtoT;
#endif

	typedef AtoW  AtoOLE;
	typedef TtoW  TtoOLE;
	typedef CWtoW WtoOLE;
	typedef WtoA  OLEtoA;
	typedef WtoT  OLEtoT;
	typedef CWtoW OLEtoW;

	class CAtoW
	{
	public:
		CAtoW(LPCSTR pStr, UINT codePage = CP_ACP) : m_pStr(pStr)
		{
			if (pStr)
			{
				// Resize the vector and assign null WCHAR to each element
				int length = MultiByteToWideChar(codePage, 0, pStr, -1, NULL, 0) + 1;
				m_vWideArray.assign(length, L'\0');

				// Fill our vector with the converted WCHAR array
				MultiByteToWideChar(codePage, 0, pStr, -1, &m_vWideArray[0], length);
			}
		}
		~CAtoW() {}
		operator LPCWSTR() { return m_pStr? &m_vWideArray[0] : NULL; }
		operator LPOLESTR() { return m_pStr? (LPOLESTR)&m_vWideArray[0] : (LPOLESTR)NULL; }

	private:
		CAtoW(const CAtoW&);
		CAtoW& operator= (const CAtoW&);
		std::vector<wchar_t> m_vWideArray;
		LPCSTR m_pStr;
	};

	class CWtoA
	{
	public:
		CWtoA(LPCWSTR pWStr, UINT codePage = CP_ACP) : m_pWStr(pWStr)
		// Usage:
		//   CWtoA ansiString(L"Some Text");
		//   CWtoA utf8String(L"Some Text", CP_UTF8);
		//
		// or
		//   SetWindowTextA( WtoA(L"Some Text") ); The ANSI version of SetWindowText
		{
			// Resize the vector and assign null char to each element
			int length = WideCharToMultiByte(codePage, 0, pWStr, -1, NULL, 0, NULL, NULL) + 1;
			m_vAnsiArray.assign(length, '\0');

			// Fill our vector with the converted char array
			WideCharToMultiByte(codePage, 0, pWStr, -1, &m_vAnsiArray[0], length, NULL,NULL);
		}

		~CWtoA() 
		{
			m_pWStr = 0;
		}
		operator LPCSTR() { return m_pWStr? &m_vAnsiArray[0] : NULL; }

	private:
		CWtoA(const CWtoA&);
		CWtoA& operator= (const CWtoA&);
		std::vector<char> m_vAnsiArray;
		LPCWSTR m_pWStr;
	};

	class CWtoW
	{
	public:
		CWtoW(LPCWSTR pWStr) : m_pWStr(pWStr) {}
		operator LPCWSTR() { return const_cast<LPWSTR>(m_pWStr); }
		operator LPOLESTR() { return const_cast<LPOLESTR>(m_pWStr); }

	private:
		CWtoW(const CWtoW&);
		CWtoW& operator= (const CWtoW&);

		LPCWSTR m_pWStr;
	};

	class CAtoA
	{
	public:
		CAtoA(LPCSTR pStr) : m_pStr(pStr) {}
		operator LPCSTR() { return (LPSTR)m_pStr; }

	private:
		CAtoA(const CAtoA&);
		CAtoA& operator= (const CAtoA&);

		LPCSTR m_pStr;
	};

	class CWtoBSTR
	{
	public:
		CWtoBSTR(LPCWSTR pWStr) { m_bstrString = ::SysAllocString(pWStr); }
		~CWtoBSTR() { ::SysFreeString(m_bstrString); }
		operator BSTR() { return m_bstrString;}

	private:
		CWtoBSTR(const CWtoBSTR&);
		CWtoBSTR& operator= (const CWtoBSTR&);
		BSTR m_bstrString;
	};

	class CAtoBSTR
	{
	public:
		CAtoBSTR(LPCSTR pStr) { m_bstrString = ::SysAllocString(AtoW(pStr)); }
		~CAtoBSTR() { ::SysFreeString(m_bstrString); }
		operator BSTR() { return m_bstrString;}

	private:
		CAtoBSTR(const CAtoBSTR&);
		CAtoBSTR& operator= (const CAtoBSTR&);
		BSTR m_bstrString;
	};
	


	////////////////////////////////////////
	// Global Functions
	//
	
	inline void Trace(LPCSTR str)
	// Trace sends a string to the debug/output pane, or an external debugger
	{
		OutputDebugString(AtoT(str));
	}

	inline void Trace(LPCWSTR str)
	// Trace sends a string to the debug/output pane, or an external debugger
	{
		OutputDebugString(WtoT(str));
	}
	
	
} // namespace Win32xx

#endif // _WIN32XX_TEXTCONV_H_
