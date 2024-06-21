#include "stdafx.h"

#include "scilexer.h"

#include <initguid.h>
#include <DbgProp.h>
//#include <Windowsx.h>
// The GUID used to identify the coclass of the VB Script engine
//  {B54F3741-5B07-11cf-A4B0-00AA004A55E8}
#define szCLSID_VBScript "{B54F3741-5B07-11cf-A4B0-00AA004A55E8}"
DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf, 0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8);
//DEFINE_GUID(IID_IActiveScriptParse32, 0xbb1a2ae2, 0xa4f9, 0x11cf, 0x8f, 0x20, 0x0, 0x80, 0x5f, 0x2c, 0xd0, 0x64);
//DEFINE_GUID(IID_IActiveScriptParse64,0xc7ef7658,0xe1ee,0x480e,0x97,0xea,0xd5,0x2c,0xb4,0xd7,0x6d,0x17);
//DEFINE_GUID(IID_IActiveScriptDebug, 0x51973C10, 0xCB0C, 0x11d0, 0xB5, 0xC9, 0x00, 0xA0, 0x24, 0x4A, 0x0E, 0x7A);

//#define RECOLOR_LINE WM_USER+100
#define CONTEXTCOOKIE_NORMAL 1000
#define CONTEXTCOOKIE_DEBUG 1001

static constexpr int LAST_ERROR_WIDGET_HEIGHT = 256;

static UINT g_FindMsgString; // Windows message for the FindText dialog

//Scintillia Lexer parses only lower case unless otherwise told
static constexpr char vbsReservedWords[] =
"and as byref byval case call const "
"continue dim do each else elseif end error exit false for function global "
"goto if in loop me new next not nothing on optional or private public "
"redim rem resume select set sub then to true type while with "
"boolean byte currency date double integer long object single string type "
"variant option explicit randomize";

static const string VBvalidChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"s);

static char CaretTextBuff[MAX_FIND_LENGTH];
static char ConstructTextBuff[MAX_FIND_LENGTH];

INT_PTR CALLBACK CVPrefProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

IScriptable::IScriptable()
{
   m_wzName[0] = '\0';
}

int CodeViewDispatch::SortAgainstValue(const wstring& pv) const
{
   char szName1[MAXSTRING];
   WideCharToMultiByteNull(CP_ACP, 0, pv.c_str(), -1, szName1, MAXSTRING, nullptr, nullptr);
   CharLowerBuff(szName1, lstrlen(szName1));
   char szName2[MAXSTRING];
   WideCharToMultiByteNull(CP_ACP, 0, m_wName.c_str(), -1, szName2, MAXSTRING, nullptr, nullptr);
   CharLowerBuff(szName2, lstrlen(szName2));
   return lstrcmp(szName1, szName2); //WideStrCmp((WCHAR *)pv, m_wzName);
}

void CodeViewer::Init(IScriptableHost *psh)
{
   CComObject<DebuggerModule>::CreateInstance(&m_pdm);
   m_pdm->AddRef();
   m_pdm->Init(this);

   m_psh = psh;

   m_hwndMain = nullptr;
   m_hwndFind = nullptr;
   m_hwndStatus = nullptr;

   szFindString[0] = '\0';
   szReplaceString[0] = '\0';

   g_FindMsgString = RegisterWindowMessage(FINDMSGSTRING);

   m_pScript = nullptr;

   m_visible = false;
   m_minimized = false;

   const HRESULT res = InitializeScriptEngine();
   if (res != S_OK)
   {
      char bla[128];
      sprintf_s(bla, sizeof(bla), "Cannot initialize Script Engine 0x%X", res);
      ShowError(bla);
   }

   m_sdsDirty = eSaveClean;
   m_ignoreDirty = false;

   m_findreplaceold.lStructSize = 0; // So we know nothing has been searched for yet

   m_errorLineNumber = -1;
   m_scriptError = false;
}

CodeViewer::~CodeViewer()
{
   if (g_pvp && g_pvp->m_pcv == this)
      g_pvp->m_pcv = nullptr;

   Destroy();

   for (size_t i = 0; i < m_vcvd.size(); ++i)
      delete m_vcvd[i];

   if (m_haccel)
      DestroyAcceleratorTable(m_haccel);

   m_pdm->Release();
}

//
// UTF-8 conversions/validations:
//

// old ANSI to UTF-8
// allocates new mem block
static char* iso8859_1_to_utf8(const char* str, const size_t length)
{
   char* const utf8 = new char[1 + 2*length]; // worst case

   char* c = utf8;
   for (size_t i = 0; i < length; ++i, ++str)
   {
      if (*str & 0x80)
      {
         *c++ = 0xc0 | (char)((unsigned char)*str >> 6);
         *c++ = 0x80 | (*str & 0x3f);
      }
      //else // check for bogus ASCII control characters
      //if (*str < 9 || (*str > 10 && *str < 13) || (*str > 13 && *str < 32))
      //   *c++ = ' ';
      else
         *c++ = *str;
   }
   *c++ = '\0';

   return utf8;
}

// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static constexpr uint8_t utf8d[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

static uint32_t decode(uint32_t* const state, uint32_t* const codep, const uint32_t byte)
{
   const uint32_t type = utf8d[byte];

   *codep = (*state != UTF8_ACCEPT) ?
      (byte & 0x3fu) | (*codep << 6) :
      (0xff >> type) & (byte);

   *state = utf8d[256 + *state*16 + type];
   return *state;
}

static uint32_t validate_utf8(uint32_t *const state, const char * const str, const size_t length)
{
   for (size_t i = 0; i < length; i++)
   {
      const uint32_t type = utf8d[(uint8_t)str[i]];
      *state = utf8d[256 + (*state) * 16 + type];

      if (*state == UTF8_REJECT)
         return UTF8_REJECT;
   }
   return *state;
}

//
//
//

// strSearchData has to be lower case
template<bool uniqueKey> // otherwise keyName
static int UDKeyIndexHelper(const fi_vector<UserData>& ListIn, const string& strSearchData, int& curPosOut)
{
	const int ListSize = (int)ListIn.size();
	curPosOut = 1u << 30;
	while (!(curPosOut & ListSize) && (curPosOut > 1))
		curPosOut >>= 1;
	int iJumpDelta = curPosOut >> 1;
	--curPosOut; //Zero Base
	while (true)
	{
		const int result = (curPosOut >= ListSize) ? -1 : strSearchData.compare(uniqueKey ? ListIn[curPosOut].m_uniqueKey : lowerCase(ListIn[curPosOut].m_keyName));
		if (iJumpDelta == 0 || result == 0) return result;
		curPosOut = (result < 0) ? (curPosOut - iJumpDelta) : (curPosOut + iJumpDelta);
		iJumpDelta >>= 1;
	}
}

//true:  Returns current Index of strIn in ListIn based on m_uniqueKey, or -1 if not found
//false: Returns current Index of strIn in ListIn based on m_keyName,   or -1 if not found
template <bool uniqueKey> // otherwise keyName
static int UDKeyIndex(const fi_vector<UserData>& ListIn, const string& strIn)
{
	if (strIn.empty() || ListIn.empty()) return -1;

	int iCurPos;
	const int result = UDKeyIndexHelper<uniqueKey>(ListIn, lowerCase(strIn), iCurPos);

	///TODO: needs to consider children?
	return (result == 0) ? iCurPos : -1;
}

/*	FindUD - Now a human Search!
 0 =Found set to point at UD in list.
-1 =Not Found 
 1 =Not Found
-2 =Zero Length string or error
strSearchData has to be lower case */
static int FindUD(const fi_vector<UserData>& ListIn, const string& strSearchData, int& Pos)
{
	if (strSearchData.empty() || ListIn.empty()) return -2;

	Pos = -1;
	const int KeyResult = UDKeyIndexHelper<true>(ListIn, strSearchData, Pos);

	//If it's a top level construct it will have no parents and therefore have a unique key.
	if (KeyResult == 0) return 0;

	//Now see if it's in the Name list
	//Jumpdelta should be initialized to the maximum count of an individual key name
	//But for the moment the biggest is 64 x's in AMH
	Pos += KeyResult; //Start very close to the result of key search
	if (Pos < 0) Pos = 0;
	//Find the start of other instances of strSearchData by crawling up list
	//Usually (but not always) UDKeyIndexHelper<true> returns top of the list so its fast
	const size_t SearchWidth = strSearchData.size();
	do
	{
		--Pos;
	} while (Pos >= 0 && strSearchData.compare(ListIn[Pos].m_uniqueKey.substr(0, SearchWidth)) == 0);
	++Pos;
	// now walk down list of Keynames looking for what we want.
	int result;
	do 
	{
		result = strSearchData.compare(lowerCase(ListIn[Pos].m_keyName)); 
		if (result == 0) break; //Found
		++Pos;
		if (Pos == (int)ListIn.size()) break;

		result = strSearchData.compare(lowerCase(ListIn[Pos].m_keyName).substr(0, SearchWidth));
	} while (result == 0); //EO SubList

	return result;
}

static bool warn_on_dupes = false;

//Assumes case insensitive sorted list
//Returns index or insertion point (-1 == error)
static size_t FindOrInsertUD(fi_vector<UserData>& ListIn, const UserData& udIn)
{
	if (ListIn.empty()) // First in
	{
		ListIn.push_back(udIn);
		return 0;
	}

	int Pos = 0;
	const int KeyFound = udIn.m_uniqueKey.empty() ? -2 : UDKeyIndexHelper<true>(ListIn, udIn.m_uniqueKey, Pos);
	if (KeyFound == 0)
	{
		//Same name, different parents?
		const fi_vector<UserData>::const_iterator iterFound = ListIn.begin() + Pos;
		const int ParentResult = udIn.m_uniqueParent.compare(iterFound->m_uniqueParent);
		if (ParentResult == -1)
			ListIn.insert(iterFound, udIn);
		else if (ParentResult == 1)
		{
			ListIn.insert(iterFound+1, udIn);
			++Pos;
		}
		else
		{
			// detect/warn about duplicate subs/functions (at least rudimentary)
			if (g_pvp && g_pvp->m_pcv &&
			    warn_on_dupes &&
			    (udIn.eTyping == eSub || udIn.eTyping == eFunction) && // only check subs and functions
			    (iterFound->m_lineNum != udIn.m_lineNum)) // use this simple check as dupe test: are the keys on different lines?
			{
				const Sci_Position dwellpos = SendMessage(g_pvp->m_pcv->m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
				SendMessage(g_pvp->m_pcv->m_hwndScintilla, SCI_CALLTIPSHOW, dwellpos,
				           (LPARAM)("Duplicate Definition found: " + iterFound->m_description + " (Line: " + std::to_string(iterFound->m_lineNum) + ")\n                            " + udIn.m_description + " (Line: " + std::to_string(udIn.m_lineNum) + ')').c_str());
				warn_on_dupes = false;
			}

			// assign again, as e.g. line of func/sub/var could have been changed by other updates
			ListIn[Pos] = udIn;
		}
		return Pos;
	}

	if (KeyFound == -1) //insert before, somewhere in the middle
	{
		ListIn.insert(ListIn.begin() + Pos, udIn);
		return Pos;
	}
	else if (KeyFound == 1) //insert above last element - Special case 
	{
		ListIn.insert(ListIn.begin() + (Pos+1), udIn);
		return Pos+1;
	}
	else if ((ListIn.begin() + Pos) == (ListIn.end() - 1))
	{ //insert at end
		ListIn.push_back(udIn);
		return ListIn.size() - 1; //Zero Base
	}
	return -1;
}

// Needs speeding up.
// can potentially return a static variable, i.e. use the pointer before the next call
static const UserData* GetUDfromUniqueKey(const fi_vector<UserData>& ListIn, const string& UniKey)
{
	static UserData retUserData;
	retUserData.eTyping = eUnknown;
	const size_t ListSize = ListIn.size();
	for (size_t i = 0; i < ListSize; ++i)
		if (UniKey == ListIn[i].m_uniqueKey)
		{
			if (ListIn[i].eTyping != eUnknown)
				return &ListIn[i];
			retUserData = ListIn[i];
		}
	return &retUserData;
}

//TODO: Needs speeding up.
static size_t GetUDIdxfromUniqueKey(const fi_vector<UserData>& ListIn, const string& UniKey)
{
	const size_t ListSize = ListIn.size();
	for (size_t i = 0; i < ListSize; ++i)
		if (UniKey == ListIn[i].m_uniqueKey)
			return i;
	return -1;
}

//Finds the closest UD from CurrentLine in ListIn
//On entry CurrentIdx must be set to the UD in the line
static int FindClosestUD(const fi_vector<UserData>& ListIn, const int CurrentLine, const int CurrentIdx)
{
	const string strSearchData = lowerCase(ListIn[CurrentIdx].m_keyName);
	const size_t SearchWidth = strSearchData.size();
	//Find the start of other instances of strIn by crawling up list
	int iNewPos = CurrentIdx;
	do
	{
		--iNewPos;
	} while (iNewPos >= 0 && strSearchData.compare(ListIn[iNewPos].m_uniqueKey.substr(0, SearchWidth)) == 0);
	++iNewPos;
	//Now at top of list
	//find nearest definition above current line
	//int ClosestLineNum = 0;
	int ClosestPos = CurrentIdx;
	int Delta = INT_MIN;
	do
	{
		const int NewLineNum = ListIn[iNewPos].m_lineNum;
		const int NewDelta = NewLineNum - CurrentLine;
		if (NewDelta >= Delta && NewLineNum <= CurrentLine && lowerCase(ListIn[iNewPos].m_keyName).compare(strSearchData) == 0)
		{
			Delta = NewDelta;
			//ClosestLineNum = NewLineNum;
			ClosestPos = iNewPos;
		}
		++iNewPos;
	} while (iNewPos != (int)ListIn.size() && strSearchData.compare(lowerCase(ListIn[iNewPos].m_keyName).substr(0, SearchWidth)) == 0);
	//--iNewPos;
	return ClosestPos;
}

// returns true if inserted, false if already in list
static bool FindOrInsertStringIntoAutolist(vector<string>& ListIn, const string &strIn)
{
	//First in the list
	if (ListIn.empty())
	{
		ListIn.push_back(strIn);
		return true;
	}
	const unsigned int ListSize = (unsigned int)ListIn.size();
	unsigned int iNewPos = 1u << 31;
	while (!(iNewPos & ListSize) && (iNewPos > 1))
		iNewPos >>= 1;
	int iJumpDelta = iNewPos >> 1;
	--iNewPos; //Zero Base
	const string strSearchData = lowerCase(strIn);
	unsigned int iCurPos;
	int result;
	while (true)
	{
		iCurPos = iNewPos;
		result = (iCurPos >= ListSize) ? - 1 : strSearchData.compare(lowerCase(ListIn[iCurPos]));
		if (result == 0) return false; // Already in list
		if (iJumpDelta == 0) break;
		iNewPos = (result < 0) ? (iCurPos - iJumpDelta) : (iCurPos + iJumpDelta);
		iJumpDelta >>= 1;
	}

	const vector<string>::const_iterator i = ListIn.begin() + iCurPos;

	if (result == -1) //insert before, somewhere in the middle
	{
		ListIn.insert(i, strIn);
		return true;
	}

	if (i == (ListIn.end() - 1)) //insert above last element - Special case
	{
		ListIn.push_back(strIn);
		return true;
	}

	if (result == 1)
	{
		ListIn.insert(i+1, strIn);
		return true;
	}

	return false; //Oh pop poop, never should hit here.
}

//
//
//

static void GetRange(const HWND hwndScintilla, const size_t start, const size_t end, char * const text)
{
   Sci_TextRange tr;
   tr.chrg.cpMin = (Sci_PositionCR)start;
   tr.chrg.cpMax = (Sci_PositionCR)end;
   tr.lpstrText = text;
   SendMessage(hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
}

void CodeViewer::GetWordUnderCaret()
{
   const LRESULT CurPos = SendMessage(m_hwndScintilla, SCI_GETCURRENTPOS, 0, 0 );
   m_wordUnderCaret.chrg.cpMin = (Sci_PositionCR)SendMessage(m_hwndScintilla, SCI_WORDSTARTPOSITION, CurPos, TRUE);
   m_wordUnderCaret.chrg.cpMax = (Sci_PositionCR)SendMessage(m_hwndScintilla, SCI_WORDENDPOSITION, CurPos, TRUE);
   if ((m_wordUnderCaret.chrg.cpMax - m_wordUnderCaret.chrg.cpMin) > MAX_FIND_LENGTH) return;

   SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&m_wordUnderCaret);
}

void CodeViewer::SetClean(const SaveDirtyState sds)
{
   if (sds == eSaveClean)
      SendMessage(m_hwndScintilla, SCI_SETSAVEPOINT, 0, 0);
   m_sdsDirty = sds;
   m_psh->SetDirtyScript(sds);
}

void CodeViewer::EndSession()
{
   CleanUpScriptEngine();

   InitializeScriptEngine();
}

HRESULT CodeViewer::AddTemporaryItem(const BSTR bstr, IDispatch * const pdisp)
{
   CodeViewDispatch * const pcvd = new CodeViewDispatch();

   pcvd->m_wName = bstr;
   pcvd->m_pdisp = pdisp;
   pcvd->m_pdisp->QueryInterface(IID_IUnknown, (void **)&pcvd->m_punk);
   pcvd->m_punk->Release();
   pcvd->m_piscript = nullptr;
   pcvd->m_global = false;

   if (m_vcvd.GetSortedIndex(pcvd) != -1 || m_vcvdTemp.GetSortedIndex(pcvd) != -1)
   {
      delete pcvd;
      return E_FAIL; //already exists
   }

   m_vcvdTemp.AddSortedString(pcvd);

   constexpr int flags = SCRIPTITEM_ISSOURCE | SCRIPTITEM_ISVISIBLE;

   /*const HRESULT hr =*/ m_pScript->AddNamedItem(bstr, flags);

   m_pScript->SetScriptState(SCRIPTSTATE_CONNECTED);

   return S_OK;
}

HRESULT CodeViewer::AddItem(IScriptable * const piscript, const bool global)
{
   CodeViewDispatch * const pcvd = new CodeViewDispatch();

   CComBSTR bstr;
   piscript->get_Name(&bstr);

   pcvd->m_wName = bstr;
   pcvd->m_pdisp = piscript->GetDispatch();
   pcvd->m_pdisp->QueryInterface(IID_IUnknown, (void **)&pcvd->m_punk);
   pcvd->m_punk->Release();
   pcvd->m_piscript = piscript;
   pcvd->m_global = global;

   if (m_vcvd.GetSortedIndex(pcvd) != -1)
   {
      delete pcvd;
      return E_FAIL;
   }

   m_vcvd.AddSortedString(pcvd);

   // Add item to dropdown
   char szT[MAXNAMEBUFFER * 2]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByteNull(CP_ACP, 0, pcvd->m_wName.c_str(), -1, szT, sizeof(szT), nullptr, nullptr);
   const size_t index = SendMessage(m_hwndItemList, CB_ADDSTRING, 0, (size_t)szT);
   SendMessage(m_hwndItemList, CB_SETITEMDATA, index, (size_t)piscript);
   //AndyS - WIP insert new item into autocomplete list??
   return S_OK;
}

void CodeViewer::RemoveItem(IScriptable * const piscript)
{
   CComBSTR bstr;
   piscript->get_Name(&bstr);

   const int idx = m_vcvd.GetSortedIndex(bstr);

   if (idx == -1)
      return;

   const CodeViewDispatch * const pcvd = m_vcvd[idx];

   _ASSERTE(pcvd);

   m_vcvd.RemoveElementAt(idx);

   // Remove item from dropdown
   char szT[MAXNAMEBUFFER*2]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, szT, MAXNAMEBUFFER*2, nullptr, nullptr);
   const size_t index = ::SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   ::SendMessage(m_hwndItemList, CB_DELETESTRING, index, 0);

   delete pcvd;
}

void CodeViewer::SelectItem(IScriptable * const piscript)
{
   CComBSTR bstr;
   piscript->get_Name(&bstr);

   char szT[MAXNAMEBUFFER*2]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, szT, MAXNAMEBUFFER*2, nullptr, nullptr);

   const LRESULT index = ::SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   if (index != CB_ERR)
   {
       ::SendMessage(m_hwndItemList, CB_SETCURSEL, index, 0);

       ListEventsFromItem();
   }
}

HRESULT CodeViewer::ReplaceName(IScriptable * const piscript, const WCHAR * const wzNew)
{
   if (m_vcvd.GetSortedIndex(wzNew) != -1)
      return E_FAIL;

   CComBSTR bstr;
   piscript->get_Name(&bstr);

   const int idx = m_vcvd.GetSortedIndex(bstr);
   if (idx == -1)
      return E_FAIL;

   CodeViewDispatch * const pcvd = m_vcvd[idx];

   _ASSERTE(pcvd);

   m_vcvd.RemoveElementAt(idx);

   pcvd->m_wName = wzNew;

   m_vcvd.AddSortedString(pcvd);

   // Remove old name from dropdown and replace it with the new
   char szT[MAXNAMEBUFFER*2]; // Names can only be 32 characters (plus terminator)
   WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, szT, MAXNAMEBUFFER*2, nullptr, nullptr);
   size_t index = ::SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   ::SendMessage(m_hwndItemList, CB_DELETESTRING, index, 0);

   WideCharToMultiByteNull(CP_ACP, 0, wzNew, -1, szT, MAXNAMEBUFFER*2, nullptr, nullptr);
   index = ::SendMessage(m_hwndItemList, CB_ADDSTRING, 0, (size_t)szT);
   ::SendMessage(m_hwndItemList, CB_SETITEMDATA, index, (size_t)piscript);

   ::SendMessage(m_hwndItemList, CB_SETCURSEL, index, 0);
   ListEventsFromItem(); // Just to get us into a good state

   return S_OK;
}

STDMETHODIMP CodeViewer::InitializeScriptEngine()
{
	const HRESULT vbScriptResult = CoCreateInstance(CLSID_VBScript, 0, CLSCTX_ALL/*CLSCTX_INPROC_SERVER*/, IID_IActiveScriptParse, (LPVOID*)&m_pScriptParse); //!! CLSCTX_INPROC_SERVER good enough?!
	if (vbScriptResult != S_OK) return vbScriptResult;

	// This can fail on some systems (I tested with wine 6.9 and this fails)
	// In that case, m_pProcessDebugManager will remain as nullptr
	CoCreateInstance(
		CLSID_ProcessDebugManager,
		0,
		CLSCTX_ALL,
		IID_IProcessDebugManager,
		(LPVOID*)&m_pProcessDebugManager
	);

	// Also check if we have a debugger installed
	// If not, we should abandon the process debug manager and fall back to plain basic errors
	IDebugApplication* debugApp;
	if (SUCCEEDED(GetApplication(&debugApp)))
	{
		debugApp->Release();
	}
	else
	{
		if (m_pProcessDebugManager) 
		{
			m_pProcessDebugManager->Release();
			m_pProcessDebugManager = nullptr;
		}
	}

	m_pScriptParse->QueryInterface(IID_IActiveScript,
		(LPVOID*)&m_pScript);

	m_pScriptParse->QueryInterface(IID_IActiveScriptDebug,
		(LPVOID*)&m_pScriptDebug);

	m_pScriptParse->InitNew();
	m_pScript->SetScriptSite(this);

	IObjectSafety* pios;
	m_pScriptParse->QueryInterface(IID_IObjectSafety, (LPVOID*)&pios);

	if (pios)
	{
		DWORD supported, enabled;
		pios->GetInterfaceSafetyOptions(IID_IActiveScript, &supported, &enabled);

		/*const HRESULT hr =*/ pios->SetInterfaceSafetyOptions(IID_IActiveScript, supported, INTERFACE_USES_SECURITY_MANAGER);

		pios->Release();
	}

	return S_OK;
}

STDMETHODIMP CodeViewer::CleanUpScriptEngine()
{
   if (m_pScript)
   {
      //m_pScript->SetScriptState(SCRIPTSTATE_DISCONNECTED);
      //m_pScript->SetScriptState(SCRIPTSTATE_CLOSED);
      // Cleanly wait for the script to end to allow Exit event, triggered just before closing, to be processed
      SCRIPTSTATE state;
      m_pScript->GetScriptState(&state);
      if (state != SCRIPTSTATE_CLOSED && state != SCRIPTSTATE_UNINITIALIZED)
      {
         PLOGI << "Sending Close to script interpreter #" << m_pScript;
         m_pScript->Close();
         U32 startWaitTick = msec();
         while ((msec() - startWaitTick < 5000) && (state != SCRIPTSTATE_CLOSED))
         {
            Sleep(16);
            m_pScript->GetScriptState(&state);
         }
         if (state != SCRIPTSTATE_CLOSED)
         {
            PLOGE << "Script did not terminated within 5s after request. Forcing close of interpreter #" << m_pScript;
            EXCEPINFO eiInterrupt = {};
            const LocalString ls(IDS_HANG);
            const WCHAR *const wzError = MakeWide(ls.m_szbuffer);
            eiInterrupt.bstrDescription = SysAllocString(wzError);
            //eiInterrupt.scode = E_NOTIMPL;
            eiInterrupt.wCode = 2345;
            delete[] wzError;
            m_pScript->InterruptScriptThread(SCRIPTTHREADID_BASE /*SCRIPTTHREADID_ALL*/, &eiInterrupt, /*SCRIPTINTERRUPT_DEBUG*/ SCRIPTINTERRUPT_RAISEEXCEPTION);
         }
         else
         {
            PLOGI << "Script interpreter state is now closed. Releasing interpreter #" << m_pScript;
         }
      }
      SAFE_RELEASE_NO_RCC(m_pScript);
      SAFE_RELEASE_NO_RCC(m_pScriptParse);
      SAFE_RELEASE(m_pScriptDebug);
      if (m_pProcessDebugManager != nullptr) m_pProcessDebugManager->Release();
   }

   for (size_t i = 0; i < m_vcvdTemp.size(); ++i)
      delete m_vcvdTemp[i];
   m_vcvdTemp.clear();

   return S_OK;
}

void CodeViewer::SetVisible(const bool visible)
{
   if (!visible && !m_minimized)
   {
      const CRect rc = GetWindowRect();
      g_pvp->m_settings.SaveValue(Settings::Editor, "CodeViewPosX"s, (int)rc.left);
      g_pvp->m_settings.SaveValue(Settings::Editor, "CodeViewPosY"s, (int)rc.top);
      const int w = rc.right - rc.left;
      g_pvp->m_settings.SaveValue(Settings::Editor, "CodeViewPosWidth"s, w);
      const int h = rc.bottom - rc.top;
      g_pvp->m_settings.SaveValue(Settings::Editor, "CodeViewPosHeight"s, h);
   }

   if (m_hwndFind && !visible)
   {
      DestroyWindow(m_hwndFind);
      m_hwndFind = nullptr;
   }

   if (IsIconic())
   {
      // SW_RESTORE usually works in all cases, but if the window
      // is maximized, we don't want to restore to a smaller size,
      // so we check IsIconic to only restore in the minimized state.
      ShowWindow(visible ? SW_RESTORE : SW_HIDE);
      m_minimized = false;
   }
   else
      ShowWindow(visible ? SW_SHOW : SW_HIDE);

   if (visible)
   {
	   if (!m_visible)
	   {
		   const int x = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "CodeViewPosX"s, 0);
         const int y = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "CodeViewPosY"s, 0);
         const int w = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "CodeViewPosWidth"s, 640);
         const int h = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "CodeViewPosHeight"s, 490);
         POINT p { x, y };
         if (MonitorFromPoint(p, MONITOR_DEFAULTTONULL) != NULL) // Do not apply if point is offscreen
            SetWindowPos(HWND_TOP, x, y, w, h, SWP_NOMOVE | SWP_NOSIZE);
	   }
	   SetForegroundWindow();
   }
   m_visible = visible;
}

void CodeViewer::SetEnabled(const bool enabled)
{
   ::SendMessage(m_hwndScintilla, SCI_SETREADONLY, !enabled, 0);

   ::EnableWindow(m_hwndItemList, enabled);
   ::EnableWindow(m_hwndEventList, enabled);
}

void CodeViewer::SetCaption(const string& szCaption)
{
   string szT;
   if (!external_script_name.empty())
      szT = "MODIFYING EXTERNAL SCRIPT: " + external_script_name;
   else
   {
      const LocalString ls(IDS_SCRIPT);
      szT = szCaption + ' ' + ls.m_szbuffer;
   }
   SetWindowText(szT.c_str());
}

void CodeViewer::UpdatePrefsfromReg()
{
   m_bgColor = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, "BackGroundColor"s, (int)RGB(255,255,255));
   m_bgSelColor = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, "BackGroundSelectionColor"s, (int)RGB(192,192,192));
   m_displayAutoComplete = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, "DisplayAutoComplete"s, true);
   m_displayAutoCompleteLength = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, "DisplayAutoCompleteAfter"s, 1);
   m_dwellDisplay = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, "DwellDisplay"s, true);
   m_dwellHelp = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, "DwellHelp"s, true);
   m_dwellDisplayTime = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, "DwellDisplayTime"s, 700);
   for (size_t i = 0; i < m_lPrefsList->size(); ++i)
      m_lPrefsList->at(i)->GetPrefsFromReg();
}

void CodeViewer::UpdateRegWithPrefs()
{
   g_pvp->m_settings.SaveValue(Settings::CVEdit, "BackGroundColor"s, (int)m_bgColor);
   g_pvp->m_settings.SaveValue(Settings::CVEdit, "BackGroundSelectionColor"s, (int)m_bgSelColor);
   g_pvp->m_settings.SaveValue(Settings::CVEdit, "DisplayAutoComplete"s, m_displayAutoComplete);
   g_pvp->m_settings.SaveValue(Settings::CVEdit, "DisplayAutoCompleteAfter"s, m_displayAutoCompleteLength);
   g_pvp->m_settings.SaveValue(Settings::CVEdit, "DwellDisplay"s, m_dwellDisplay);
   g_pvp->m_settings.SaveValue(Settings::CVEdit, "DwellHelp"s, m_dwellHelp);
   g_pvp->m_settings.SaveValue(Settings::CVEdit, "DwellDisplayTime"s, m_dwellDisplayTime);
   for (size_t i = 0; i < m_lPrefsList->size(); i++)
      m_lPrefsList->at(i)->SetPrefsToReg();
}

void CodeViewer::InitPreferences()
{
	memset(m_prefCols, 0, sizeof(m_prefCols));

	m_bgColor = RGB(255,255,255);
	m_bgSelColor = RGB(192,192,192);
	m_lPrefsList = new vector<CVPreference*>();

	m_prefEverythingElse = new CVPreference(RGB(0,0,0), true, "EverythingElse",  STYLE_DEFAULT, 0 , IDC_CVP_BUT_COL_EVERYTHINGELSE, IDC_CVP_BUT_FONT_EVERYTHINGELSE);
	m_lPrefsList->push_back(m_prefEverythingElse);
	prefDefault = new CVPreference(RGB(0,0,0), true, "Default", SCE_B_DEFAULT, 0 , 0, 0);
	m_lPrefsList->push_back(prefDefault);
	prefVBS = new CVPreference(RGB(0,0,160), true, "ShowVBS", SCE_B_KEYWORD, IDC_CVP_CHECKBOX_VBS, IDC_CVP_BUT_COL_VBS, IDC_CVP_BUT_FONT_VBS);
	m_lPrefsList->push_back(prefVBS);
	prefComps = new CVPreference(RGB(120,120,0), true, "ShowComponents", SCE_B_KEYWORD3, IDC_CVP_CHKB_COMP, IDC_CVP_BUT_COL_COMPS, IDC_CVP_BUT_FONT_COMPS);
	m_lPrefsList->push_back(prefComps);
	prefSubs = new CVPreference(RGB(120,0,120), true, "ShowSubs", SCE_B_KEYWORD2, IDC_CVP_CHKB_SUBS, IDC_CVP_BUT_COL_SUBS, IDC_CVP_BUT_FONT_SUBS);
	m_lPrefsList->push_back(prefSubs);
	prefComments = new CVPreference(RGB(0,120,0), true, "ShowRemarks", SCE_B_COMMENT, IDC_CVP_CHKB_COMMENTS, IDC_CVP_BUT_COL_COMMENTS, IDC_CVP_BUT_FONT_COMMENTS);
	m_lPrefsList->push_back(prefComments);
	prefLiterals = new CVPreference(RGB(0,120,160), true, "ShowLiterals", SCE_B_STRING, IDC_CVP_CHKB_LITERALS, IDC_CVP_BUT_COL_LITERALS, IDC_CVP_BUT_FONT_LITERALS);
	m_lPrefsList->push_back(prefLiterals);
	prefVPcore = new CVPreference(RGB(200,50,60), true, "ShowVPcore", SCE_B_KEYWORD4, IDC_CVP_CHKB_VPCORE, IDC_CVP_BUT_COL_VPCORE, IDC_CVP_BUT_FONT_VPCORE);
	m_lPrefsList->push_back(prefVPcore);
	for (size_t i = 0; i < m_lPrefsList->size(); ++i)
	{
		CVPreference* const Pref = m_lPrefsList->at(i);
		Pref->SetDefaultFont(m_hwndMain);
	}
	// load prefs from registry
	UpdatePrefsfromReg();
}

int CodeViewer::OnCreate(CREATESTRUCT& cs)
{
   m_haccel = LoadAccelerators(g_pvp->theInstance, MAKEINTRESOURCE(IDR_CODEVIEWACCEL)); // Accelerator keys

   m_hwndMain = GetHwnd();
   SetWindowLongPtr(GWLP_USERDATA, (size_t)this);

   /////////////////// Item / Event Lists //!! ALL THIS STUFF IS NOT RES/DPI INDEPENDENT! also see WM_SIZE handler

   m_hwndItemText = CreateWindowEx(0, "Static", "ObjectsText",
      WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 5, 0, 330, 30, m_hwndMain, nullptr, g_pvp->theInstance, 0);
   ::SetWindowText(m_hwndItemText, "Table component:");
   ::SendMessage(m_hwndItemText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndItemList = CreateWindowEx(0, "ComboBox", "Objects",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL,
      5, 30+2, 330, 400, m_hwndMain, nullptr, g_pvp->theInstance, 0);
   ::SetWindowLongPtr(m_hwndItemList, GWL_ID, IDC_ITEMLIST);
   ::SendMessage(m_hwndItemList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndEventText = CreateWindowEx(0, "Static", "EventsText",
      WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 360 + 5, 0, 330, 30, m_hwndMain, nullptr, g_pvp->theInstance, 0);
   ::SetWindowText(m_hwndEventText, "Create Sub from component:");
   ::SendMessage(m_hwndEventText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndEventList = CreateWindowEx(0, "ComboBox", "Events",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL,
      360 + 5, 30+2, 330, 400, m_hwndMain, nullptr, g_pvp->theInstance, 0);
   ::SetWindowLongPtr(m_hwndEventList, GWL_ID, IDC_EVENTLIST);
   ::SendMessage(m_hwndEventList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndFunctionText = CreateWindowEx(0, "Static", "FunctionsText",
      WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 730 + 5, 0, 330, 30, m_hwndMain, nullptr, g_pvp->theInstance, 0);
   ::SetWindowText(m_hwndFunctionText, "Go to Sub/Function:");
   ::SendMessage(m_hwndFunctionText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndFunctionList = CreateWindowEx(0, "ComboBox", "Functions",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
      730 + 5, 30+2, 330, 400, m_hwndMain, nullptr, g_pvp->theInstance, 0);
   ::SetWindowLongPtr(m_hwndFunctionList, GWL_ID, IDC_FUNCTIONLIST);
   ::SendMessage(m_hwndFunctionList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   //////////////////////// Status Window (& Sizing Box)

   m_hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", m_hwndMain, 1);

   constexpr int foo[4] = { 220, 420, 450, 500 };
   ::SendMessage(m_hwndStatus, SB_SETPARTS, 4, (size_t)foo);

   //////////////////////// Last error widget

   m_hwndLastErrorTextArea = CreateWindowEx(0, "Edit", "",
      WS_CHILD | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE,
      0, 0, 0, 0, m_hwndMain, nullptr, g_pvp->theInstance, 0);
   SendMessage(m_hwndLastErrorTextArea, EM_SETREADONLY, TRUE, 0);
   ::SendMessage(m_hwndLastErrorTextArea, WM_SETFONT, (size_t)GetStockObject(ANSI_FIXED_FONT), 0);
   
   //////////////////////// Scintilla text editor

   m_hwndScintilla = CreateWindowEx(0, "Scintilla", "",
      WS_CHILD | ES_NOHIDESEL | WS_VISIBLE | ES_SUNKEN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN,
      0, 30+2 +40, 0, 0, m_hwndMain, nullptr, g_pvp->theInstance, 0);

	//if still using old dll load VB lexer instead
	//use SCI_SETLEXERLANGUAGE as SCI_GETLEXER doesn't return the correct value with SCI_SETLEXER
	::SendMessage(m_hwndScintilla, SCI_SETLEXERLANGUAGE, 0, (LPARAM)"vpscript");
	const LRESULT lexVersion = SendMessage(m_hwndScintilla, SCI_GETLEXER, 0, 0);
	if (lexVersion != SCLEX_VPSCRIPT)
	{
		::SendMessage(m_hwndScintilla, SCI_SETLEXER, (WPARAM)SCLEX_VBSCRIPT, 0);
	}

	char szValidChars[256] = {};
	::SendMessage(m_hwndScintilla, SCI_GETWORDCHARS, 0, (LPARAM)szValidChars);
	m_validChars = szValidChars;
	m_stopErrorDisplay = false;

	// Create new list of user functions & Collections- filled in ParseForFunction(), first called in LoadFromStream()
	m_wordUnderCaret.lpstrText = CaretTextBuff;
	m_currentConstruct.lpstrText = ConstructTextBuff;

	// parse vb reserved words for auto complete.
	int intWordFinish = -1; //skip space
	char WordChar = vbsReservedWords[0];
	while (WordChar != '\0') //Just make sure with chars, we reached EOL
	{
		string szWord;

		intWordFinish++; //skip space
		WordChar = vbsReservedWords[intWordFinish];
		while (WordChar != '\0' && WordChar != ' ')
		{
			szWord += WordChar;
			intWordFinish++;
			WordChar = vbsReservedWords[intWordFinish];
		}

		UserData VBWord;
		if (!szWord.empty())
		{
			VBWord.m_uniqueKey = VBWord.m_keyName = szWord;
			// Capitalize first letter
			const char fl = VBWord.m_keyName[0];
			if (fl >= 'a' && fl <= 'z') VBWord.m_keyName[0] = fl - ('a' - 'A');
		}
		FindOrInsertUD(m_VBwordsDict, VBWord);
	}

	///// Preferences
	InitPreferences();

   ::SendMessage(m_hwndScintilla, SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT, 0);
   ::SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 0, (LPARAM)vbsReservedWords);
   ::SendMessage(m_hwndScintilla, SCI_SETTABWIDTH, 4, 0);


   // The null visibility policy is like Visual Studio - if a search goes
   // off the screen, the newly selected text is placed in the middle of the
   // screen
   ::SendMessage(m_hwndScintilla, SCI_SETVISIBLEPOLICY, 0, 0);
   //Set up line numbering
   ::SendMessage(m_hwndScintilla, SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
   ::SendMessage(m_hwndScintilla, SCI_SETMARGINSENSITIVEN, 1, 1);
   ::SendMessage(m_hwndScintilla, SCI_SETMARGINWIDTHN, 0, 40);
   //Cursor line dimmed
   ::SendMessage(m_hwndScintilla, SCI_SETCARETLINEVISIBLE, 1, 0);
   ::SendMessage(m_hwndScintilla, SCI_SETCARETLINEBACK, RGB(240, 240, 255), 0);
   //Highlight Errors
   ::SendMessage(m_hwndScintilla, SCI_INDICSETSTYLE, 0, INDIC_ROUNDBOX);
   ::SendMessage(m_hwndScintilla, SCI_SETINDICATORCURRENT, 0, 0);
   ::SendMessage(m_hwndScintilla, SCI_INDICSETFORE, 0, RGB(255, 0, 0));
   ::SendMessage(m_hwndScintilla, SCI_INDICSETALPHA, 0, 90);
   //Set up folding.
   ::SendMessage(m_hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
   ::SendMessage(m_hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"0");
   //Set up folding margin
   ::SendMessage(m_hwndScintilla, SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
   ::SendMessage(m_hwndScintilla, SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);
   ::SendMessage(m_hwndScintilla, SCI_SETMARGINWIDTHN, 1, 20);

   ::SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, m_prefEverythingElse->m_rgb);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, m_bgColor);
   //WIP markers
   ::SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, m_prefEverythingElse->m_rgb);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, m_bgColor);
   ::SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERSUB, m_prefEverythingElse->m_rgb);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, m_bgColor);
   ::SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERTAIL, m_prefEverythingElse->m_rgb);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, m_bgColor);
   ::SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEREND, m_prefEverythingElse->m_rgb);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEREND, m_bgColor);
   ::SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPENMID, m_prefEverythingElse->m_rgb);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPENMID, m_bgColor);
   ::SendMessage(m_hwndScintilla, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERMIDTAIL, m_prefEverythingElse->m_rgb);
   ::SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, m_bgColor);

   ::SendMessage(m_hwndScintilla, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
   ::SendMessage(m_hwndScintilla, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Courier");

   ::SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_PREPROCESSOR, RGB(255, 0, 0));
   ::SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_OPERATOR, RGB(0, 0, 160));
   ::SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_IDENTIFIER, RGB(0, 0, 0));
   ::SendMessage(m_hwndScintilla, SCI_STYLESETFORE, SCE_B_DATE, RGB(0, 0, 0));

   ::SendMessage(m_hwndScintilla, SCI_SETWORDCHARS, 0, (LPARAM)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

   //SendMessage(m_hwndScintilla, SCI_SETMOUSEDOWNCAPTURES ,0,0); //send mouse events through scintilla.
   ::SendMessage(m_hwndScintilla, SCI_AUTOCSETIGNORECASE, TRUE, 0);
   ::SendMessage(m_hwndScintilla, SCI_AUTOCSETCASEINSENSITIVEBEHAVIOUR, SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE,0);

   ::SendMessage(m_hwndScintilla, SCI_AUTOCSETFILLUPS, 0,(LPARAM) "[]{}()");
   ::SendMessage(m_hwndScintilla, SCI_AUTOCSTOPS, 0,(LPARAM) " ");

   //

   ParseVPCore();
   UpdateScinFromPrefs();

   SendMessage(WM_SIZE, 0, 0); // Make our window relay itself out
   return CWnd::OnCreate(cs);
}

void CodeViewer::Destroy()
{
	delete m_prefEverythingElse;
	delete prefDefault;
	delete prefVBS;
	delete prefComps;
	delete prefSubs;
	delete prefComments;
	delete prefLiterals;
	delete prefVPcore;
	if (m_lPrefsList)
	{
		m_lPrefsList->clear();
		delete m_lPrefsList;
	}
	m_componentsDict.clear();
	m_pageConstructsDict.clear();
	m_VBwordsDict.clear();
	m_currentMembers.clear();
	m_VPcoreDict.clear();

	if (m_hwndFind) DestroyWindow(m_hwndFind);

   CWnd::Destroy();
}

bool CodeViewer::PreTranslateMessage(MSG *msg)
{
   if (!IsWindow())
      return FALSE;

   // only pre-translate mouse and keyboard input events
   if (   ((msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) || (msg->message >= WM_MOUSEFIRST && msg->message <= WM_MOUSELAST))
       && TranslateAccelerator(m_hwndMain, m_haccel, msg))
      return TRUE;

   if (::IsDialogMessage(m_hwndMain, msg))
      return TRUE;
   return FALSE;
}

STDMETHODIMP CodeViewer::GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask,
   IUnknown **ppiunkItem, ITypeInfo **ppti)
{
   if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
      *ppiunkItem = 0;
   if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
      *ppti = 0;

   CodeViewDispatch *pcvd = m_vcvd.GetSortedElement((void *)pstrName);

   if (pcvd == nullptr)
   {
      pcvd = m_vcvdTemp.GetSortedElement((void *)pstrName);
      if (pcvd == nullptr)
         return E_FAIL;
   }

   if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
   {
      if ((*ppiunkItem = pcvd->m_punk))
         (*ppiunkItem)->AddRef();
   }

   if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
   {
      IProvideClassInfo* pClassInfo;
      pcvd->m_punk->QueryInterface(IID_IProvideClassInfo,
         (LPVOID*)&pClassInfo);
      if (pClassInfo)
      {
         pClassInfo->GetClassInfo(ppti);
         pClassInfo->Release();
      }
   }

   return S_OK;
}

/**
 * Called on compilation errors. Also called on runtime errors in we couldn't create a "process debug manager" (such
 * as when running on wine), or if no debug application is available (where a "debug application" is something like
 * VS 2010 Isolated Shell).
 *
 * See CodeViewer::OnScriptErrorDebug for runtime errors, when a debug application is available
 */
STDMETHODIMP CodeViewer::OnScriptError(IActiveScriptError *pscripterror)
{
	DWORD dwCookie;
	ULONG nLine;
	LONG nChar;
	pscripterror->GetSourcePosition(&dwCookie, &nLine, &nChar);
	BSTR bstr = 0;
	pscripterror->GetSourceLineText(&bstr);
	EXCEPINFO exception = {};
	pscripterror->GetExceptionInfo(&exception);
	nLine++;

	const char* const szT = MakeChar((exception.bstrDescription) ? exception.bstrDescription : L"");

	PLOGE_(PLOG_NO_DBG_OUT_INSTANCE_ID) << "Script Error at line " << nLine << " : " << szT;

	if (dwCookie == CONTEXTCOOKIE_DEBUG)
	{
		AddToDebugOutput(szT);
		delete[] szT;
		SysFreeString(bstr);
		return S_OK;
	}
	else
		delete[] szT;

	m_scriptError = true;

	if (g_pplayer)
	{
		g_pplayer->LockForegroundWindow(false);
		g_pplayer->EnableWindow(FALSE);
	}

	CComObject<PinTable>* const pt = g_pvp->GetActiveTable();
	if (pt)
	{
		pt->m_pcv->SetVisible(true);
      pt->m_pcv->ShowWindow(SW_RESTORE);
      pt->m_pcv->ColorError(nLine, nChar);
	}

	// Check if this is a compile error or a runtime error
	SCRIPTSTATE state;
	m_pScript->GetScriptState(&state);
	const bool isRuntimeError = (state == SCRIPTSTATE_CONNECTED);

	// Error log content
	std::wstringstream errorStream;
	if (isRuntimeError)
		errorStream << L"Runtime error\r\n";
	else
		errorStream << L"Compile error\r\n";

	errorStream << L"-------------\r\n";
	errorStream << L"Line: " << nLine << "\r\n";
	errorStream << (exception.bstrDescription ? exception.bstrDescription : L"Description unavailable") << "\r\n";
	errorStream << L"\r\n";

	SysFreeString(bstr);
	SysFreeString(exception.bstrSource);
	SysFreeString(exception.bstrDescription);
	SysFreeString(exception.bstrHelpFile);

	g_pvp->EnableWindow(FALSE);

	const wstring errorStr{errorStream.str()};

	// Show the error in the last error log
   if (pt)
   {
      pt->m_pcv->AppendLastErrorTextW(errorStr);
      pt->m_pcv->SetLastErrorVisibility(true);
   }

	// Also pop up a dialog if this is a runtime error
	if (isRuntimeError && !m_suppressErrorDialogs)
	{
		g_pvp->EnableWindow(FALSE);
		ScriptErrorDialog scriptErrorDialog(errorStr);
		scriptErrorDialog.DoModal();
		m_suppressErrorDialogs = scriptErrorDialog.WasSuppressErrorsRequested();
		g_pvp->EnableWindow(TRUE);

		if (pt != nullptr)
         ::SetFocus(pt->m_pcv->m_hwndScintilla);
	}

	g_pvp->EnableWindow(TRUE);

	if (pt != nullptr)
      ::SetFocus(pt->m_pcv->m_hwndScintilla);

	return S_OK;
}

STDMETHODIMP CodeViewer::GetDocumentContextFromPosition(
	DWORD_PTR dwSourceContext,
	ULONG uCharacterOffset,
	ULONG uNumChars,
	IDebugDocumentContext** ppsc
)
{
	return E_NOTIMPL;
}

STDMETHODIMP CodeViewer::GetApplication(
	IDebugApplication** ppda
)
{
	if (m_pProcessDebugManager != nullptr)
	{
		IDebugApplication* app;
		const HRESULT result = m_pProcessDebugManager->GetDefaultApplication(&app);

		// We want to make sure the debug application supports JIT debugging, otherwise we don't seem to get notified
		// of runtime errors at all (neither in OnScriptError or in OnScriptErrorDebug)!
		if (SUCCEEDED(result) && app->FCanJitDebug())
		{
			*ppda = app;
			return S_OK;
		}
		else
			return E_FAIL;
	}
	else
		return E_NOTIMPL;
}

STDMETHODIMP CodeViewer::GetRootApplicationNode(
	IDebugApplicationNode** ppdanRoot
)
{
	IDebugApplication* app;
	const HRESULT result = GetApplication(&app);
	if (SUCCEEDED(result))
		return app->GetRootNode(ppdanRoot);
	else
		return result;
}

/**
 * Called on runtime errors, if debugging is supported, and a debug application is available.
 *
 * See CodeViewer::OnScriptError for compilation errors, and also runtime errors when debugging isn't available.
 */
STDMETHODIMP CodeViewer::OnScriptErrorDebug(
	IActiveScriptErrorDebug* pscripterror,
	BOOL* pfEnterDebugger,
	BOOL* pfCallOnScriptErrorWhenContinuing
)
{
	// TODO: What debuggers even work with VBScript? It might be an idea to offer a "Debug" button (set pfEnterDebugger to
	//       true) if it can pop open some old version of visual studio to debug stuff.
	//
	//       VS 2010 Isolated Shell seems to work, but trying to enter debugging with it complains with an "invalid
	//       license" error. I haven't found anything else to work yet, not even regular VS 2010 (though, it might be
	//       that you need to manually set some registry keys to select the default debugger?)
	//
	//       HKEY_CLASSES_ROOT\CLSID\{834128A2-51F4-11D0-8F20-00805F2CD064}\LocalServer32 seems to be the registry key
	//       to select the default debugger.
	//       (https://stackoverflow.com/questions/2288043/how-do-i-debug-a-stand-alone-vbscript-script#comment36315883_2288064)
	*pfEnterDebugger = FALSE;
	*pfCallOnScriptErrorWhenContinuing = FALSE;

	DWORD dwCookie;
	ULONG nLine;
	LONG nChar;
	pscripterror->GetSourcePosition(&dwCookie, &nLine, &nChar);
	BSTR bstr = 0;
	pscripterror->GetSourceLineText(&bstr);
	EXCEPINFO exception = {};
	pscripterror->GetExceptionInfo(&exception);
	nLine++;

	PLOGE_(PLOG_NO_DBG_OUT_INSTANCE_ID) << "Script Error at line " << nLine << " : " << exception.bstrDescription;

	if (dwCookie == CONTEXTCOOKIE_DEBUG)
	{
		char* szT = MakeChar(exception.bstrDescription);
		AddToDebugOutput(szT);
		delete[] szT;
		SysFreeString(bstr);
		return S_OK;
	}

	m_scriptError = true;

	if (g_pplayer)
	{
		g_pplayer->LockForegroundWindow(false);
		g_pplayer->EnableWindow(FALSE);
	}

	CComObject<PinTable>* const pt = g_pvp->GetActiveTable();
	if (pt)
	{
		pt->m_pcv->SetVisible(true);
      pt->m_pcv->ShowWindow(SW_RESTORE);
      pt->m_pcv->ColorError(nLine, nChar);
	}
	
	// Error log content
	std::wstringstream errorStream;
	errorStream << L"Runtime error\r\n";
	errorStream << L"-------------\r\n";
	errorStream << L"Line: " << nLine << "\r\n";
	errorStream << (exception.bstrDescription ? exception.bstrDescription : L"Description unavailable") << "\r\n";

	// Get stack trace
	IDebugStackFrame* errStackFrame;
	if (pscripterror->GetStackFrame(&errStackFrame) == S_OK)
	{
		errorStream << L"\r\nStack trace (Most recent call first):\r\n";

		IDebugApplicationThread *thread;
		errStackFrame->GetThread(&thread);

      if (thread)
      {
         IEnumDebugStackFrames *stackFramesEnum;
         thread->EnumStackFrames(&stackFramesEnum);

         DebugStackFrameDescriptor stackFrames[128];
         ULONG numStackFrames;
         stackFramesEnum->Next(128, stackFrames, &numStackFrames);

         for (ULONG i = 0; i < numStackFrames; i++)
         {
            // The frame description is the name of the function in this stack frame
            BSTR frameDesc;
            stackFrames[i].pdsf->GetDescriptionString(TRUE, &frameDesc);

            // Fetch local variables and args
            IDebugProperty *debugProp;
            stackFrames[i].pdsf->GetDebugProperty(&debugProp);

            IEnumDebugPropertyInfo *propInfoEnum;
            debugProp->EnumMembers(PROP_INFO_FULLNAME | PROP_INFO_VALUE,
               10, // Radix (for numerical info)
               IID_IDebugPropertyEnumType_LocalsPlusArgs, &propInfoEnum);

            DebugPropertyInfo infos[128];
            ULONG numInfos;
            propInfoEnum->Next(128, infos, &numInfos);

            std::wstringstream stackVariablesStream;
            for (ULONG i2 = 0; i2 < numInfos; i2++)
            {
               stackVariablesStream << infos[i2].m_bstrFullName << L'=' << infos[i2].m_bstrValue;
               // Add a comma if this isn't the last item in the list
               if (i2 != numInfos - 1)
                  stackVariablesStream << L", ";
            }

            propInfoEnum->Release();
            debugProp->Release();
            // End fetch local variables and args

            errorStream << L"    " << frameDesc;

            // If there are any locals/args, add them to the end of the frame description
            if (numInfos > 0)
            {
               errorStream << L" (";
               errorStream << stackVariablesStream.str();
               errorStream << L')';
               PLOGE_(PLOG_NO_DBG_OUT_INSTANCE_ID) << "Stacktrace: " << frameDesc << " (" << stackVariablesStream.str() << ')';
            }
            else
            {
               PLOGE_(PLOG_NO_DBG_OUT_INSTANCE_ID) << "Stacktrace: " << frameDesc;
            }

            errorStream << L"\r\n";

            SysFreeString(frameDesc);
         }

         stackFramesEnum->Release();
         thread->Release();
      }
	}

	errorStream << L"\r\n";

	SysFreeString(bstr);
	SysFreeString(exception.bstrSource);
	SysFreeString(exception.bstrDescription);
	SysFreeString(exception.bstrHelpFile);

	const wstring errorStr{errorStream.str()};

	// Show the error in the last error log of the active table
   if (pt != nullptr)
   {
      pt->m_pcv->AppendLastErrorTextW(errorStr);
      pt->m_pcv->SetLastErrorVisibility(true);
   }

	// Also pop up a dialog
	if (!m_suppressErrorDialogs)
	{
		g_pvp->EnableWindow(FALSE);
		ScriptErrorDialog scriptErrorDialog(errorStr);

		// Since we got a "debug error", we don't need to prompt to install a debugger for more detailed errors
		scriptErrorDialog.HideInstallDebuggerText();

		scriptErrorDialog.DoModal();
		m_suppressErrorDialogs = scriptErrorDialog.WasSuppressErrorsRequested();
		g_pvp->EnableWindow(TRUE);

		if (pt != nullptr)
         ::SetFocus(pt->m_pcv->m_hwndScintilla);
	}

	return S_OK;
}

void CodeViewer::Compile(const bool message)
{
   if (m_pScript)
   {
      const size_t cchar = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);

      char * const szText = new char[cchar + 1];
      WCHAR * const wzText = new WCHAR[cchar + 1];

      SendMessage(m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)szText);
      MultiByteToWideCharNull(CP_UTF8, 0, szText, -1, wzText, (int)cchar+1);

      EXCEPINFO exception = {};
      m_pScript->SetScriptState(SCRIPTSTATE_INITIALIZED);

      /*const HRESULT hr =*/ m_pScript->AddTypeLib(LIBID_VPinballLib, 1, 0, 0);

      for (size_t i = 0; i < m_vcvd.size(); ++i)
      {
         int flags = SCRIPTITEM_ISSOURCE | SCRIPTITEM_ISVISIBLE;
         if (m_vcvd[i]->m_global)
            flags |= SCRIPTITEM_GLOBALMEMBERS;
         m_pScript->AddNamedItem(m_vcvd[i]->m_wName.c_str(), flags);
      }

      if (m_pScriptParse->ParseScriptText(wzText, 0, 0, 0, CONTEXTCOOKIE_NORMAL, 0,
         SCRIPTTEXT_ISVISIBLE, 0, &exception) == S_OK)
         if (message)
            MessageBox("Compilation successful", "Compile", MB_OK);

      m_pScript->SetScriptState(SCRIPTSTATE_INITIALIZED);

      delete[] wzText;
      delete[] szText;
   }
}

void CodeViewer::Start()
{
	//ShowError("CodeViewer::Start"); //debug logging BDS
	if (m_pScript)
	{
		SetLastErrorTextW(L"Starting script\r\n\r\n");
		m_suppressErrorDialogs = false;
		m_pScript->SetScriptState(SCRIPTSTATE_CONNECTED);
	}
}

void CodeViewer::EvaluateScriptStatement(const char * const szScript)
{
   const int scriptlen = lstrlen(szScript);
   WCHAR * const wzScript = new WCHAR[scriptlen + 1];
   MultiByteToWideCharNull(CP_ACP, 0, szScript, -1, wzScript, scriptlen + 1);

   EXCEPINFO exception = {};
   m_pScriptParse->ParseScriptText(wzScript, L"Debug", 0, 0, CONTEXTCOOKIE_DEBUG, 0, 0, nullptr, &exception);

   delete[] wzScript;
}

void CodeViewer::AddToDebugOutput(const char * const szText)
{
   SendMessage(g_pplayer->m_hwndDebugOutput, SCI_ADDTEXT, lstrlen(szText), (LPARAM)szText);
   SendMessage(g_pplayer->m_hwndDebugOutput, SCI_ADDTEXT, 1, (LPARAM)"\n");

   const size_t pos = SendMessage(g_pplayer->m_hwndDebugOutput, SCI_GETCURRENTPOS, 0, 0);
   const size_t line = SendMessage(g_pplayer->m_hwndDebugOutput, SCI_LINEFROMPOSITION, pos, 0);
   SendMessage(g_pplayer->m_hwndDebugOutput, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
}

void CodeViewer::ShowFindDialog()
{
   if (m_hwndFind == nullptr)
   {
      m_wordUnderCaret.lpstrText = szFindString;
      GetWordUnderCaret();

      m_findreplacestruct.lStructSize = sizeof(FINDREPLACE);
      m_findreplacestruct.hwndOwner = m_hwndMain;
      m_findreplacestruct.hInstance = nullptr;
      m_findreplacestruct.Flags = FR_DOWN | FR_HIDEWHOLEWORD;
      m_findreplacestruct.lpstrFindWhat = szFindString;
      m_findreplacestruct.lpstrReplaceWith = nullptr;
      m_findreplacestruct.wFindWhatLen = MAX_FIND_LENGTH-1;
      m_findreplacestruct.wReplaceWithLen = 0;
      m_findreplacestruct.lCustData = 0;
      m_findreplacestruct.lpfnHook = nullptr;
      m_findreplacestruct.lpTemplateName = nullptr;

      m_hwndFind = FindText(&m_findreplacestruct);
   }
}

void CodeViewer::ShowFindReplaceDialog()
{
   if (m_hwndFind == nullptr)
   {
      m_wordUnderCaret.lpstrText = szFindString;
      GetWordUnderCaret();

      m_findreplacestruct.lStructSize = sizeof(FINDREPLACE);
      m_findreplacestruct.hwndOwner = m_hwndMain;
      m_findreplacestruct.hInstance = nullptr;
      m_findreplacestruct.Flags = FR_DOWN | FR_HIDEWHOLEWORD;
      m_findreplacestruct.lpstrFindWhat = szFindString;
      m_findreplacestruct.lpstrReplaceWith = szReplaceString;
      m_findreplacestruct.wFindWhatLen = MAX_FIND_LENGTH-1;
      m_findreplacestruct.wReplaceWithLen = MAX_FIND_LENGTH-1;
      m_findreplacestruct.lCustData = 0;
      m_findreplacestruct.lpfnHook = nullptr;
      m_findreplacestruct.lpTemplateName = nullptr;

      m_hwndFind = ReplaceText(&m_findreplacestruct);
   }
}

void CodeViewer::Find(const FINDREPLACE * const pfr)
{
   if (pfr->lStructSize == 0) // Our built-in signal that we are doing 'find next' and nothing has been searched for yet
      return;

   m_findreplaceold = *pfr;

   const size_t selstart = SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   const size_t selend = SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);

   size_t startChar, stopChar;

   if (pfr->Flags & FR_DOWN)
   {
      const size_t len = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
      startChar = selend;
      stopChar = len;
   }
   else
   {
      startChar = selstart - 1;
      stopChar = 0;
   }

   const int scinfindflags = ((pfr->Flags & FR_WHOLEWORD) ? SCFIND_WHOLEWORD : 0) |
      ((pfr->Flags & FR_MATCHCASE) ? SCFIND_MATCHCASE : 0) /*|
                                                           ((pfr->Flags & 0) ? SCFIND_REGEXP : 0)*/;

   SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0);
   SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);
   SendMessage(m_hwndScintilla, SCI_SETSEARCHFLAGS, scinfindflags, 0);
   LRESULT posFind = SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, lstrlen(pfr->lpstrFindWhat), (LPARAM)pfr->lpstrFindWhat);

   bool wrapped = false;

   if (posFind == -1)
   {
      // Not found, try looping the document
      wrapped = true;
      if (pfr->Flags & FR_DOWN)
      {
         startChar = 0;
         stopChar = selstart;
      }
      else
      {
         const size_t len = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
         startChar = len;
         stopChar = selend;
      }

      SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0);
      SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);

      posFind = SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, lstrlen(pfr->lpstrFindWhat), (LPARAM)pfr->lpstrFindWhat);
   }

   if (posFind != -1)
   {
      const size_t start = SendMessage(m_hwndScintilla, SCI_GETTARGETSTART, 0, 0);
      const size_t end = SendMessage(m_hwndScintilla, SCI_GETTARGETEND, 0, 0);
      const size_t lineStart = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, min(start, end), 0);
      const size_t lineEnd = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, max(start, end), 0);
      for (size_t line = lineStart; line <= lineEnd; ++line)
         SendMessage(m_hwndScintilla, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
      SendMessage(m_hwndScintilla, SCI_SETSEL, start, end);

      if (!wrapped)
         SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)"");
      else
      {
         const LocalString ls(IDS_FINDLOOPED);
         SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)ls.m_szbuffer);
      }
   }
   else
   {
      const LocalString ls(IDS_FINDFAILED);
      const LocalString ls2(IDS_FINDFAILED2);
      const string szT = string(ls.m_szbuffer) + pfr->lpstrFindWhat + ls2.m_szbuffer;
      MessageBeep(MB_ICONEXCLAMATION);
      SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)szT.c_str());
   }
}

void CodeViewer::Replace(const FINDREPLACE * const pfr)
{
   const size_t selstart = SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   const size_t selend = SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);

   const size_t len = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);

   FINDTEXTEX ft;
   ft.chrg.cpMax = (LONG)len;			// search through end of the text
   ft.chrg.cpMin = (LONG)selstart;
   if (!(pfr->Flags & (FR_REPLACE | FR_REPLACEALL)))
      ft.chrg.cpMin = (LONG)selend;
   ft.lpstrText = pfr->lpstrFindWhat;

   LONG cszReplaced = 0;
next:
   const size_t cpMatch = SendMessage(m_hwndScintilla, SCI_FINDTEXT, (WPARAM)(pfr->Flags), (LPARAM)&ft);
   if ((SSIZE_T)cpMatch < 0)
   {
      if (cszReplaced == 0)
      {
         const LocalString ls(IDS_FINDFAILED);
         const LocalString ls2(IDS_FINDFAILED2);
         const string szT = string(ls.m_szbuffer) + ft.lpstrText + ls2.m_szbuffer;
         MessageBeep(MB_ICONEXCLAMATION);
         SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)szT.c_str());
      }
      else
      {
         const LocalString ls(IDS_REPLACEALL);
         const LocalString ls2(IDS_REPLACEALL2);
         char szT[MAX_PATH];
         wsprintfA(szT, "%s %ld %s", ls.m_szbuffer, cszReplaced, ls2.m_szbuffer);
         MessageBeep(MB_ICONEXCLAMATION);
         SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)szT);
      }
      return;
   }

   ft.chrg.cpMin = ft.chrgText.cpMin;
   ft.chrg.cpMax = ft.chrgText.cpMax;
   SendMessage(m_hwndScintilla, SCI_SETSEL, ft.chrgText.cpMin, ft.chrgText.cpMax);
   if (((pfr->Flags & FR_REPLACE) && cszReplaced == 0) || (pfr->Flags & FR_REPLACEALL))
   {
      SendMessage(m_hwndScintilla, SCI_REPLACESEL, true, (LPARAM)pfr->lpstrReplaceWith);
      ft.chrg.cpMin = (LONG)(cpMatch + lstrlen(pfr->lpstrReplaceWith));
      ft.chrg.cpMax = (LONG)len;	// search through end of the text
      cszReplaced++;
      goto next;
   }
}

void CodeViewer::SaveToStream(IStream *pistream, const HCRYPTHASH hcrypthash)
{
   size_t cchar = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   char * szText = new char[cchar + MAXNAMEBUFFER + 1]; //!! MAXNAMEBUFFER ??
   SendMessage(m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)szText);

   // if there was an external vbs loaded, save the script to that file
   // and ask if to save the original script also to the table
   bool save_external_script_to_table = true;
   if (!external_script_name.empty())
   {
      FILE* fScript;
      if ((fopen_s(&fScript, external_script_name.c_str(), "wb") == 0) && fScript)
      {
         fwrite(szText, 1, cchar, fScript);
         fclose(fScript);
      }

      save_external_script_to_table = (MessageBox("Save externally loaded .vbs script also to .vpx table?", "Visual Pinball", MB_YESNO | MB_DEFBUTTON2) == IDYES);

      if (!save_external_script_to_table)
      {
         delete[] szText;
         szText = g_pvp->m_pcv->original_table_script.data();
         cchar = g_pvp->m_pcv->original_table_script.size();
      }
   }

   ULONG writ = 0;
   pistream->Write(&cchar, (ULONG)sizeof(int), &writ);
   pistream->Write(szText, (ULONG)(cchar*sizeof(char)), &writ);

   CryptHashData(hcrypthash, (BYTE *)szText, (DWORD)cchar, 0);

   if (save_external_script_to_table)
      delete[] szText;
}

void CodeViewer::SaveToFile(const string& filename)
{
   FILE * fScript;
   if ((fopen_s(&fScript, filename.c_str(), "wb") == 0) && fScript)
   {
      const size_t cchar = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
      char * const szText = new char[cchar + 1];
      SendMessage(m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)szText);
      fwrite(szText, 1, cchar, fScript);
      fclose(fScript);
      delete[] szText;
   }
}

void CodeViewer::LoadFromStream(IStream *pistream, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey)
{
   m_ignoreDirty = true;

   ULONG read = 0;
   int cchar;
   pistream->Read(&cchar, sizeof(int), &read);

   char * szText = new char[cchar + 1];

   pistream->Read(szText, cchar*(int)sizeof(char), &read);

   CryptHashData(hcrypthash, (BYTE *)szText, cchar, 0);

   // if there is a valid key, then decrypt the script text (now in szText)
   //(must be done after the hash is updated)
   if (hcryptkey != 0)
   {
      // get the size of the data to decrypt
      DWORD cryptlen = cchar*(int)sizeof(char);

      // decrypt the script
      CryptDecrypt(hcryptkey, // key to use
         0,                   // not hashing data at the same time
         TRUE,                // last block (or only block)
         0,                   // no flags
         (BYTE *)szText,      // buffer to decrypt
         &cryptlen);          // size of data to decrypt

      /*const int foo =*/ GetLastError();	// purge any errors

      // update the size of the buffer
      cchar = cryptlen;
   }

   // ensure that the script is null terminated
   szText[cchar] = L'\0';

   // save original script, in case an external vbs is loaded
   original_table_script.resize(cchar);
   memcpy(original_table_script.data(), szText, cchar);

   // check if script is either plain ASCII or UTF-8, or if it contains invalid stuff
   uint32_t state = UTF8_ACCEPT;
   if (validate_utf8(&state, szText, cchar) == UTF8_REJECT) {
      char* const utf8Text = iso8859_1_to_utf8(szText, cchar); // old ANSI characters? -> convert to UTF-8
      delete[] szText;
      szText = utf8Text;
   }

   SendMessage(m_hwndScintilla, SCI_SETCODEPAGE, SC_CP_UTF8, 0); // Set to UTF-8 codepage
   SendMessage(m_hwndScintilla, SCI_SETTEXT, 0, (size_t)szText);
   SendMessage(m_hwndScintilla, SCI_EMPTYUNDOBUFFER, 0, 0);
   delete[] szText;

   m_ignoreDirty = false;
   m_sdsDirty = eSaveClean;

   // Allow updates to take now we know the script size
   UpdateScinFromPrefs();
}

void CodeViewer::LoadFromFile(const string& filename)
{
   FILE * fScript;
   if ((fopen_s(&fScript, filename.c_str(), "rb") == 0) && fScript)
   {
		external_script_name = filename;

		fseek(fScript, 0L, SEEK_END);
		size_t cchar = ftell(fScript);
		fseek(fScript, 0L, SEEK_SET);
		m_ignoreDirty = true;

		char * szText = new char[cchar + 1];

		cchar = fread(szText, 1, cchar, fScript);
		fclose(fScript);

		szText[cchar] = L'\0';

		uint32_t state = UTF8_ACCEPT;
		if (validate_utf8(&state, szText, cchar) == UTF8_REJECT) {
			char* const utf8Text = iso8859_1_to_utf8(szText, cchar); // old ANSI characters? -> convert to UTF-8
			delete[] szText;
			szText = utf8Text;
		}

		SendMessage(m_hwndScintilla, SCI_SETCODEPAGE, SC_CP_UTF8, 0); // Set to UTF-8 codepage
		SendMessage(m_hwndScintilla, SCI_SETTEXT, 0, (size_t)szText);
		SendMessage(m_hwndScintilla, SCI_EMPTYUNDOBUFFER, 0, 0);
		delete[] szText;

		m_ignoreDirty = false;
		m_sdsDirty = eSaveClean;
   }
}


void CodeViewer::ColorLine(const int line)
{
   //!!
}

void CodeViewer::UncolorError()
{
   const size_t startChar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, m_errorLineNumber, 0);
   const size_t length = SendMessage(m_hwndScintilla, SCI_LINELENGTH, m_errorLineNumber, 0);

   SendMessage(m_hwndScintilla, SCI_INDICATORCLEARRANGE, startChar, length);

   m_errorLineNumber = -1;
}

void CodeViewer::ColorError(const int line, const int nchar)
{
   m_errorLineNumber = line - 1;

   const size_t startChar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line - 1, 0);
   const size_t length = SendMessage(m_hwndScintilla, SCI_LINELENGTH, line - 1, 0);

   SendMessage(m_hwndScintilla, SCI_INDICATORFILLRANGE, startChar, length);
   SendMessage(m_hwndScintilla, SCI_GOTOLINE, line, 0);
}

STDMETHODIMP CodeViewer::OnEnterScript()
{
   return S_OK;
}

STDMETHODIMP CodeViewer::OnLeaveScript()
{
   return S_OK;
}

void CodeViewer::TellHostToSelectItem()
{
   const size_t index = SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   IScriptable * const pscript = (IScriptable *)SendMessage(m_hwndItemList, CB_GETITEMDATA, index, 0);

   m_psh->SelectItem(pscript);
}

void CodeViewer::GetParamsFromEvent(const UINT iEvent, char * const szParams)
{
   szParams[0] = '\0';

   const size_t index = SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   IScriptable * const pscript = (IScriptable *)SendMessage(m_hwndItemList, CB_GETITEMDATA, index, 0);
   IDispatch * const pdisp = pscript->GetDispatch();
   IProvideClassInfo* pClassInfo;
   pdisp->QueryInterface(IID_IProvideClassInfo, (void **)&pClassInfo);

   if (pClassInfo)
   {
      ITypeInfo *pti;
      pClassInfo->GetClassInfo(&pti);

      TYPEATTR *pta;
      pti->GetTypeAttr(&pta);

      for (int i = 0; i < pta->cImplTypes; ++i)
      {
         HREFTYPE href;
         pti->GetRefTypeOfImplType(i, &href);

         ITypeInfo *ptiChild;
         pti->GetRefTypeInfo(href, &ptiChild);

         TYPEATTR *ptaChild;
         ptiChild->GetTypeAttr(&ptaChild);

         if (ptaChild->wTypeFlags == 4096) // Events
         {
            FUNCDESC *pfd;
            ptiChild->GetFuncDesc(iEvent, &pfd);

            if (pfd->cParams != 0) // no parameters makes it easy
            {
               // Get parameter names
               BSTR * const rgstr = (BSTR *)CoTaskMemAlloc(6 * sizeof(BSTR));

               unsigned int cnames;
               /*const HRESULT hr =*/ ptiChild->GetNames(pfd->memid, rgstr, 6, &cnames);

               // Add enum string to combo control
               for (unsigned int l = 1; l < cnames; ++l)
               {
                  char szT[512];
                  WideCharToMultiByteNull(CP_ACP, 0, rgstr[l], -1, szT, 512, nullptr, nullptr);
                  if (l > 1)
                  {
                     lstrcat(szParams, ", ");
                  }
                  lstrcat(szParams, szT);
               }

               for (unsigned int l = 0; l < cnames; l++)
                  SysFreeString(rgstr[l]);

               CoTaskMemFree(rgstr);
            }

            ptiChild->ReleaseFuncDesc(pfd);
         }

         ptiChild->ReleaseTypeAttr(ptaChild);

         ptiChild->Release();
      }

      pti->ReleaseTypeAttr(pta);

      pti->Release();
      pClassInfo->Release();
   }
}

static void AddEventToList(const char * const sz, const int index, const int dispid, const LPARAM lparam)
{
   const HWND hwnd = (HWND)lparam;
   const size_t listindex = SendMessage(hwnd, CB_ADDSTRING, 0, (size_t)sz);
   SendMessage(hwnd, CB_SETITEMDATA, listindex, index);
}

void CodeViewer::ListEventsFromItem()
{
   // Clear old events
   SendMessage(m_hwndEventList, CB_RESETCONTENT, 0, 0);

   const size_t index = SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   IScriptable * const pscript = (IScriptable *)SendMessage(m_hwndItemList, CB_GETITEMDATA, index, 0);
   IDispatch * const pdisp = pscript->GetDispatch();

   EnumEventsFromDispatch(pdisp, AddEventToList, (LPARAM)m_hwndEventList);
}

void CodeViewer::FindCodeFromEvent()
{
   bool found = false;

   char szItemName[512]; // Can't be longer than 32 chars, but use this buffer for concatenating
   char szEventName[512];
   size_t index = SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   SendMessage(m_hwndItemList, CB_GETLBTEXT, index, (size_t)szItemName);
   index = SendMessage(m_hwndEventList, CB_GETCURSEL, 0, 0);
   SendMessage(m_hwndEventList, CB_GETLBTEXT, index, (size_t)szEventName);
   const size_t iEventIndex = SendMessage(m_hwndEventList, CB_GETITEMDATA, index, 0);
   lstrcat(szItemName, "_"); // VB Specific event names
   lstrcat(szItemName, szEventName);
   size_t codelen = SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   const size_t stopChar = codelen;
   SendMessage(m_hwndScintilla, SCI_TARGETWHOLEDOCUMENT, 0, 0);
   SendMessage(m_hwndScintilla, SCI_SETSEARCHFLAGS, SCFIND_WHOLEWORD, 0);
   LRESULT posFind;
   while ((posFind = SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, lstrlen(szItemName), (LPARAM)szItemName)) != -1)
   {
      const size_t line = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, posFind, 0);
      // Check for 'sub' and make sure we're not in a comment
      const size_t beginchar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line, 0);
      bool goodMatch = true;

      char szLine[MAX_LINE_LENGTH] = {};
      SOURCE_TEXT_ATTR wzFormat[MAX_LINE_LENGTH];
      WCHAR wzText[MAX_LINE_LENGTH];

      const size_t cchar = SendMessage(m_hwndScintilla, SCI_GETLINE, line, (LPARAM)szLine);
      MultiByteToWideCharNull(CP_ACP, 0, szLine, -1, wzText, MAX_LINE_LENGTH);
      m_pScriptDebug->GetScriptTextAttributes(wzText, (ULONG)cchar, nullptr, 0, wzFormat);

      const size_t inamechar = posFind - beginchar - 1;

      int i;
      for (i = (int)inamechar; i >= 0; i--)
      {
         if (wzFormat[i] == SOURCETEXT_ATTR_KEYWORD)
            break;

         if (!IsWhitespace(szLine[i]) /*&& (wzFormat[i] != 0 || wzFormat[i] != SOURCETEXT_ATTR_COMMENT)*/) //!!?
            goodMatch = false;
      }

      if (i < 2) // Can't fit the word 'sub' in here
      {
         goodMatch = false;
      }
      else
      {
         szLine[i + 1] = '\0';
         if (lstrcmpi(&szLine[i - 2], "sub")) //!! correct like this?
            goodMatch = false;
      }

      if (goodMatch)
      {
         // We found a real sub heading - move the cursor inside of it

         found = true;

         LRESULT ichar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line + 1, 0);
         if (ichar == -1)
         {
            // The function was declared as the last line of the script - rare but possible
            ichar = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line, 0);
         }

         const size_t lineEvent = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, ichar, 0);
         SendMessage(m_hwndScintilla, SCI_ENSUREVISIBLEENFORCEPOLICY, lineEvent, 0);
         SendMessage(m_hwndScintilla, SCI_SETSEL, ichar, ichar);
      }

      if (found)
         break;

      const size_t startChar = posFind + 1;
      SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0);
      SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);
   }

   if (!found)
   {
      char szEnd[2];
      TEXTRANGE tr;
      tr.chrg.cpMax = (LONG)codelen;
      tr.chrg.cpMin = (LONG)codelen - 1;
      tr.lpstrText = szEnd;

      // Make sure there is at least a one space gap between the last function and this new one
      SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (size_t)&tr);

      if (szEnd[0] != '\n')
      {
         SendMessage(m_hwndScintilla, SCI_SETSEL, codelen, codelen);
         SendMessage(m_hwndScintilla, SCI_REPLACESEL, TRUE, (size_t)"\n");
         codelen++;
      }

      if (szEnd[1] != '\n')
      {
         SendMessage(m_hwndScintilla, SCI_SETSEL, codelen, codelen);
         SendMessage(m_hwndScintilla, SCI_REPLACESEL, TRUE, (size_t)"\n");
         codelen++;
      }

      // Add the new function at the end
      SendMessage(m_hwndScintilla, SCI_SETSEL, codelen, codelen);

      char szParams[MAX_LINE_LENGTH];

      GetParamsFromEvent((UINT)iEventIndex, szParams);

      char szNewCode[MAX_LINE_LENGTH];
      lstrcpy(szNewCode, "Sub ");
      lstrcat(szNewCode, szItemName);
      lstrcat(szNewCode, "(");
      lstrcat(szNewCode, szParams);
      lstrcat(szNewCode, ")\n\t");
      const size_t subtitlelen = lstrlen(szNewCode);
      lstrcat(szNewCode, "\nEnd Sub");

      SendMessage(m_hwndScintilla, SCI_REPLACESEL, TRUE, (size_t)szNewCode);

      SendMessage(m_hwndScintilla, SCI_SETSEL, codelen + subtitlelen, codelen + subtitlelen);
   }

   ::SetFocus(m_hwndScintilla);
}

HRESULT STDMETHODCALLTYPE CodeViewer::GetSecurityId(
   BYTE *pbSecurityId,
   DWORD *pcbSecurityId,
   DWORD_PTR dwReserved)
{
   return S_OK;
}

HRESULT STDMETHODCALLTYPE CodeViewer::ProcessUrlAction(
   DWORD dwAction,
   BYTE __RPC_FAR *pPolicy,
   DWORD cbPolicy,
   BYTE __RPC_FAR *pContext,
   DWORD cbContext,
   DWORD dwFlags,
   DWORD dwReserved)
{

   *pPolicy = (dwAction == URLACTION_ACTIVEX_RUN && (g_pvp->m_securitylevel < eSecurityNoControls)) ?
   URLPOLICY_ALLOW : URLPOLICY_DISALLOW;

   return S_OK;
}

DEFINE_GUID(GUID_CUSTOM_CONFIRMOBJECTSAFETY, 0x10200490, 0xfa38, 0x11d0, 0xac, 0x0e, 0x00, 0xa0, 0xc9, 0x0f, 0xff, 0xc0);

HRESULT STDMETHODCALLTYPE CodeViewer::QueryCustomPolicy(
   REFGUID guidKey,
   BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
   DWORD __RPC_FAR *pcbPolicy,
   BYTE __RPC_FAR *pContext,
   DWORD cbContext,
   DWORD dwReserved)
{
   DWORD * const ppolicy = (DWORD *)CoTaskMemAlloc(sizeof(DWORD));
   *ppolicy = URLPOLICY_DISALLOW;

   *ppPolicy = (BYTE *)ppolicy;

   *pcbPolicy = sizeof(DWORD);

   if (InlineIsEqualGUID(guidKey, GUID_CUSTOM_CONFIRMOBJECTSAFETY))
   {
      bool safe = false;
      CONFIRMSAFETY *pcs = (CONFIRMSAFETY *)pContext;

      if (g_pvp->m_securitylevel == eSecurityNone)
         safe = true;

      if (!safe && ((g_pvp->m_securitylevel == eSecurityWarnOnUnsafeType) || (g_pvp->m_securitylevel == eSecurityWarnOnType)))
         safe = FControlAlreadyOkayed(pcs);

      if (!safe && (g_pvp->m_securitylevel <= eSecurityWarnOnUnsafeType))
         safe = FControlMarkedSafe(pcs);

      if (!safe)
      {
         safe = FUserManuallyOkaysControl(pcs);
         if (safe && ((g_pvp->m_securitylevel == eSecurityWarnOnUnsafeType) || (g_pvp->m_securitylevel == eSecurityWarnOnType)))
            AddControlToOkayedList(pcs);
      }

      if (safe)
         *ppolicy = URLPOLICY_ALLOW;
   }

   return S_OK;
}

bool CodeViewer::FControlAlreadyOkayed(const CONFIRMSAFETY *pcs)
{
   if (g_pplayer)
   {
      for (size_t i = 0; i < g_pplayer->m_controlclsidsafe.size(); ++i)
      {
         const CLSID * const pclsid = g_pplayer->m_controlclsidsafe[i];
         if (*pclsid == pcs->clsid)
            return true;
      }
   }

   return false;
}

void CodeViewer::AddControlToOkayedList(const CONFIRMSAFETY *pcs)
{
   if (g_pplayer)
   {
      CLSID * const pclsid = new CLSID();
      *pclsid = pcs->clsid;
      g_pplayer->m_controlclsidsafe.push_back(pclsid);
   }
}

bool CodeViewer::FControlMarkedSafe(const CONFIRMSAFETY *pcs)
{
   bool safe = false;
   IObjectSafety *pios = nullptr;

   if (FAILED(pcs->pUnk->QueryInterface(IID_IObjectSafety, (void **)&pios)))
      goto LError;

   DWORD supported, enabled;
   if (FAILED(pios->GetInterfaceSafetyOptions(IID_IDispatch, &supported, &enabled)))
      goto LError;

   if (!(supported & INTERFACESAFE_FOR_UNTRUSTED_CALLER) || !(supported & INTERFACESAFE_FOR_UNTRUSTED_DATA))
      goto LError;

   if (!(enabled & INTERFACESAFE_FOR_UNTRUSTED_CALLER) || !(enabled & INTERFACESAFE_FOR_UNTRUSTED_DATA))
   {
      if (FAILED(pios->SetInterfaceSafetyOptions(IID_IDispatch, supported, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA)))
         goto LError;
   }

   safe = true;

LError:

   if (pios)
      pios->Release();

   return safe;
}

bool CodeViewer::FUserManuallyOkaysControl(const CONFIRMSAFETY *pcs)
{
   OLECHAR *wzT;
   if (FAILED(OleRegGetUserType(pcs->clsid, USERCLASSTYPE_FULL, &wzT)))
      return false;

   const int len = lstrlenW(wzT) + 1; // include null termination
   char * const szName = new char[len];
   WideCharToMultiByteNull(CP_ACP, 0, wzT, -1, szName, len, nullptr, nullptr);

   const LocalString ls1(IDS_UNSECURECONTROL1);
   const LocalString ls2(IDS_UNSECURECONTROL2);
   const string szT = string(ls1.m_szbuffer) + szName + ls2.m_szbuffer;
   delete[] szName;

   const int ans = MessageBox(szT.c_str(), "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);

   return (ans == IDYES);
}

HRESULT STDMETHODCALLTYPE CodeViewer::QueryService(
   REFGUID guidService,
   REFIID riid,
   void **ppv)
{
   const HRESULT hr = (riid == IID_IInternetHostSecurityManager) ? QueryInterface(riid /*IID_IInternetHostSecurityManager*/, ppv) : E_NOINTERFACE;

   return hr;
}

void CodeViewer::ShowAutoComplete(const SCNotification *pSCN)
{
	if (!pSCN) return;

	const char KeyPressed = pSCN->ch;
	if (KeyPressed != '.')
	{
		m_wordUnderCaret.lpstrText = CaretTextBuff;
		GetWordUnderCaret();
		const size_t intWordLen = strnlen_s(m_wordUnderCaret.lpstrText, sizeof(CaretTextBuff));
		if ((int)intWordLen > m_displayAutoCompleteLength && intWordLen < MAX_FIND_LENGTH)
		{
			const char * McStr = m_autoCompString.c_str();
			SendMessage(m_hwndScintilla, SCI_AUTOCSHOW, intWordLen, (LPARAM)McStr);
		}
	}
	else
	{
		//Get member construct

		const LRESULT ConstructPos = SendMessage(m_hwndScintilla, SCI_GETCURRENTPOS, 0, 0 ) - 2;
		m_currentConstruct.chrg.cpMin = (Sci_PositionCR)SendMessage(m_hwndScintilla, SCI_WORDSTARTPOSITION, ConstructPos, TRUE);
		m_currentConstruct.chrg.cpMax = (Sci_PositionCR)SendMessage(m_hwndScintilla, SCI_WORDENDPOSITION, ConstructPos, TRUE);
		if ((m_currentConstruct.chrg.cpMax - m_currentConstruct.chrg.cpMin) > MAX_FIND_LENGTH) return;
		SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&m_currentConstruct);

		//Check Core dict first
		m_currentConstruct.lpstrText = ConstructTextBuff;
		GetMembers(m_VPcoreDict, m_currentConstruct.lpstrText);

		//Check Table Script
		if (m_currentMembers.empty())
		{
			GetMembers(m_pageConstructsDict, m_currentConstruct.lpstrText);
			//if no construct (no children) exit
			if (m_currentMembers.empty()) return;
		}

		//autocomp string  = members of construct
		m_autoCompMembersString.clear();
		for (fi_vector<UserData>::const_iterator i = m_currentMembers.begin(); i != m_currentMembers.end(); ++i)
		{
			m_autoCompMembersString += i->m_keyName;
			m_autoCompMembersString += ' ';
		}
		//display
		const char * McStr = m_autoCompMembersString.c_str();
		SendMessage(m_hwndScintilla, SCI_AUTOCSHOW, 0, (LPARAM)McStr);
	}
}

void CodeViewer::GetMembers(const fi_vector<UserData>& ListIn, const string& strIn)
{
	m_currentMembers.clear();
	const int idx = UDKeyIndex<false>(ListIn, strIn);
	if (idx != -1)
	{
		const UserData& udParent = ListIn[idx];
		const size_t NumberOfMembers = udParent.m_children.size();
		for (size_t i = 0; i < NumberOfMembers; ++i)
			FindOrInsertUD(m_currentMembers, *GetUDfromUniqueKey(ListIn, udParent.m_children[i]));
	}
}

// if tooltip then show tooltip, otherwise jump to function/sub/variable definition
bool CodeViewer::ShowTooltipOrGoToDefinition(const SCNotification *pSCN, const bool tooltip)
{
	//get word under pointer
	const Sci_Position dwellpos = pSCN ? pSCN->position : SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
	const LRESULT wordstart = SendMessage(m_hwndScintilla, SCI_WORDSTARTPOSITION, dwellpos, TRUE);
	const LRESULT wordfinish = SendMessage(m_hwndScintilla, SCI_WORDENDPOSITION, dwellpos, TRUE);
	const int CurrentLineNo = (int)SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, dwellpos, 0);

	//return if in a comment
	char text[MAX_LINE_LENGTH] = {};
	SendMessage(m_hwndScintilla, SCI_GETLINE, CurrentLineNo, (LPARAM)text);
	if (text[0] != '\0')
	{
		const size_t t = string(text).find_first_of('\'', 0);
		if (t != string::npos)
		{
			const LRESULT linestart = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, CurrentLineNo, 0);
			if (((size_t)(wordstart - linestart)) >= t) return false;
		}
	}

	// is it a valid 'word'
	string Mess;
	const UserData *gotoDefinition = nullptr;
	if ((SendMessage(m_hwndScintilla, SCI_ISRANGEWORD, wordstart, wordfinish)) && ((wordfinish - wordstart) < 255))
	{
		//Retrieve the word
		char szDwellWord[256] = {};
		GetRange(m_hwndScintilla, wordstart, wordfinish, szDwellWord);
		string DwellWord = lowerCase(szDwellWord);
		RemovePadding(DwellWord);
		RemoveNonVBSChars(DwellWord);
		if (DwellWord.empty()) return false;

		// Search for VBS reserved words
		// ToDo: Should be able to get some MS help for better descriptions
		int idx;
		RemovePadding(DwellWord);
		if (FindUD(m_VBwordsDict, DwellWord, idx) == 0)
		{
			Mess = "VBS: " + m_VBwordsDict[idx].m_keyName;
		}
		else //if (MessLen == 0)
		{
			//Has function list been filled?
			m_stopErrorDisplay = true;
			if (m_pageConstructsDict.empty()) ParseForFunction();

			//search subs/funcs
			if (FindUD(m_pageConstructsDict, DwellWord, idx) == 0)
			{
				idx = FindClosestUD(m_pageConstructsDict, CurrentLineNo, idx);
				const UserData* const Word = &m_pageConstructsDict[idx];
				Mess = Word->m_description;
				Mess += " (Line: " + std::to_string(Word->m_lineNum + 1) + ')';
				if ((Word->m_comment.length() > 1) && m_dwellHelp)
				{
					Mess += '\n';
					Mess += m_pageConstructsDict[idx].m_comment;
				}
				gotoDefinition = Word;
			}
			//Search VP core
			else if (FindUD(m_VPcoreDict, DwellWord, idx) == 0)
			{
				idx = FindClosestUD(m_VPcoreDict, CurrentLineNo, idx);
				Mess = m_VPcoreDict[idx].m_description;
				if ((m_VPcoreDict[idx].m_comment.length() > 1) && m_dwellHelp)
				{
					Mess += '\n';
					Mess += m_VPcoreDict[idx].m_comment;
				}
			}
			else if (FindUD(m_componentsDict, DwellWord, idx) == 0)
				Mess = "Component: "s + szDwellWord;
		}
#ifdef _DEBUG
		if (Mess.empty())
		{
			Mess = "Test: "s + szDwellWord;
		}
#endif
	}
	if (!Mess.empty())
	{
		if (tooltip)
			SendMessage(m_hwndScintilla,SCI_CALLTIPSHOW,dwellpos, (LPARAM)Mess.c_str());
		else if (gotoDefinition)
		{
			SendMessage(m_hwndScintilla, SCI_GOTOLINE, gotoDefinition->m_lineNum, 0);
			SendMessage(m_hwndScintilla, SCI_GRABFOCUS, 0, 0);
		}
		else
			return false;
		return true;
	}
	return false;
}

void CodeViewer::MarginClick(const Sci_Position position, const int modifiers)
{
   const size_t lineClick = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, position, 0);
   if ((modifiers & SCMOD_SHIFT) && (modifiers & SCMOD_CTRL))
   {
      //FoldAll();
   }
   else
   {
      const size_t levelClick = SendMessage(m_hwndScintilla, SCI_GETFOLDLEVEL, lineClick, 0);
      if (levelClick & SC_FOLDLEVELHEADERFLAG)
      {
         if (modifiers & SCMOD_SHIFT)
         {
            // Ensure all children visible
            SendMessage(m_hwndScintilla, SCI_SETFOLDEXPANDED, lineClick, 1);
            //Expand(lineClick, true, true, 100, levelClick);
         }
         else if (modifiers & SCMOD_CTRL)
         {
            if (SendMessage(m_hwndScintilla, SCI_GETFOLDEXPANDED, lineClick, 0))
            {
               // Contract this line and all children
               SendMessage(m_hwndScintilla, SCI_SETFOLDEXPANDED, lineClick, 0);
               //Expand(lineClick, false, true, 0, levelClick);
            }
            else
            {
               // Expand this line and all children
               SendMessage(m_hwndScintilla, SCI_SETFOLDEXPANDED, lineClick, 1);
               //Expand(lineClick, true, true, 100, levelClick);
            }
         }
         else
         {
            // Toggle this line
            SendMessage(m_hwndScintilla, SCI_TOGGLEFOLD, lineClick, 0);
         }
      }
   }
}

static void AddComment(const HWND m_hwndScintilla)
{
   constexpr char comment[] = "'";

   const size_t startSel = SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   size_t endSel = SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);

   const size_t selStartLine = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, startSel, 0);
   size_t selEndLine = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, endSel, 0);
   size_t lines = selEndLine - selStartLine + 1;
   const size_t posFromLine = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, selEndLine, 0);

   if (lines > 1 && endSel == posFromLine)
   {
      selEndLine--;
      lines--;
      endSel = SendMessage(m_hwndScintilla, SCI_GETLINEENDPOSITION, selEndLine, 0);
   }
   SendMessage(m_hwndScintilla, SCI_BEGINUNDOACTION, 0, 0);
   if (lines <= 1)
   {
      const size_t lineStart = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, selStartLine, 0);
      SendMessage(m_hwndScintilla, SCI_INSERTTEXT, lineStart, (LPARAM)comment);
   }
   else
   {
      // More than one line selected, so insert middle_comments where needed
      for (size_t i = selStartLine; i < selEndLine + 1; ++i)
      {
         const size_t lineStart = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, i, 0);
         SendMessage(m_hwndScintilla, SCI_INSERTTEXT, lineStart, (LPARAM)comment);
      }
   }
   SendMessage(m_hwndScintilla, SCI_ENDUNDOACTION, 0, 0);
}

static void RemoveComment(const HWND m_hwndScintilla)
{
   //const char * const comment = "\b";
   const size_t startSel = SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   size_t endSel = SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);

   const size_t selStartLine = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, startSel, 0);
   size_t selEndLine = SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, endSel, 0);
   size_t lines = selEndLine - selStartLine + 1;
   const size_t posFromLine = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, selEndLine, 0);

   if (lines > 1 && endSel == posFromLine)
   {
      selEndLine--;
      lines--;
      endSel = SendMessage(m_hwndScintilla, SCI_GETLINEENDPOSITION, selEndLine, 0);
   }

   SendMessage(m_hwndScintilla, SCI_BEGINUNDOACTION, 0, 0);

   for (size_t i = selStartLine; i < selEndLine + 1; ++i)
   {
      const size_t lineStart = SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, i, 0);
      const size_t lineEnd = SendMessage(m_hwndScintilla, SCI_GETLINEENDPOSITION, i, 0);
      if (lineEnd - lineStart < (MAX_LINE_LENGTH-1) )
      {
         char buf[MAX_LINE_LENGTH];
         GetRange(m_hwndScintilla, lineStart, lineEnd, buf);
         const size_t idx = string(buf).find_first_of('\'');
         if (idx == 0)
         {
            SendMessage(m_hwndScintilla, SCI_SETSEL, lineStart, lineStart + 1);
            SendMessage(m_hwndScintilla, SCI_REPLACESEL, 0, (LPARAM)"");
         }
      }
   }
   SendMessage(m_hwndScintilla, SCI_ENDUNDOACTION, 0, 0);
}

// Makes sure what is found has only VBS Chars in..
size_t CodeViewer::SureFind(const string &LineIn, const string &ToFind)
{
	const size_t Pos = LineIn.find(ToFind);
	if (Pos == string::npos)
		return string::npos;

	const char EndChr = LineIn[Pos + ToFind.length()];
	size_t IsValidVBChr = VBvalidChars.find(EndChr);
	if (IsValidVBChr != string::npos) // Extra char on end - not what we want
		return string::npos;

	if (Pos > 0)
	{
		const char StartChr = LineIn[Pos - 1];
		IsValidVBChr = VBvalidChars.find(StartChr);
		if (IsValidVBChr != string::npos)
			return string::npos;
	}
	return Pos;
}

void CodeViewer::PreCreate(CREATESTRUCT& cs)
{
    const int x = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "CodeViewPosX"s, 0);
    const int y = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "CodeViewPosY"s, 0);
    const int w = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "CodeViewPosWidth"s, 640);
    const int h = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "CodeViewPosHeight"s, 490);

    cs.x = x;
    cs.y = y;
    cs.cx = w;
    cs.cy = h;
    cs.style = WS_POPUP | WS_SIZEBOX | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    cs.hInstance = g_pvp->theInstance;
    cs.lpszClass = "CVFrame";
    cs.lpszName = "Script";
}

void CodeViewer::PreRegisterClass(WNDCLASS& wc)
{
    wc.style = CS_DBLCLKS;
    wc.hInstance = g_pvp->theInstance;
    wc.hIcon = LoadIcon(g_pvp->theInstance, MAKEINTRESOURCE(IDI_SCRIPT));
    wc.lpszClassName = "CVFrame";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_SCRIPTMENU);//nullptr;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
}

bool CodeViewer::ParseOKLineLength(const size_t LineLen)
{
	if (LineLen > MAX_LINE_LENGTH)
	{
		MessageBox(("The current maximum script line length is " + std::to_string(MAX_LINE_LENGTH)).c_str(), ("Line too long on line " + std::to_string(LineLen)).c_str(), MB_OK);
		return false;
	}
	if (LineLen < 3) return false;
	return true;
}

//false is a fail/syntax error
bool CodeViewer::ParseStructureName(fi_vector<UserData>& ListIn, const UserData& ud_org, const string& UCline, const string& line, const int Lineno)
{
	const size_t endIdx = SureFind(UCline,"END"s); 
	const size_t exitIdx = SureFind(UCline,"EXIT"s); 

	if (endIdx == string::npos && exitIdx == string::npos)
	{
		UserData ud = ud_org;
		RemoveNonVBSChars(ud.m_keyName);
		if (ud.eTyping == eDim || ud.eTyping == eConst)
		{
			ud.m_uniqueKey = lowerCase(ud.m_keyName) + m_currentParentKey;
			ud.m_uniqueParent = m_currentParentKey;
			FindOrInsertUD(ListIn, ud);
			const size_t iCurParent = GetUDIdxfromUniqueKey(ListIn, m_currentParentKey);
			if (!m_currentParentKey.empty() && !ud.m_uniqueKey.empty() && (iCurParent < ListIn.size()))
			{
				ListIn[iCurParent].m_children.push_back(ud.m_uniqueKey);//add child to parent
			}
			string RemainingLine = line;
			size_t CommPos = UCline.find_first_of(',');
			while (CommPos != string::npos)
			{
				//Insert stuff after comma after cleaning up
				const size_t NewCommPos = RemainingLine.find_first_of(',', CommPos+1);
				//get word @
				string crWord = RemainingLine.substr(CommPos+1, (NewCommPos == string::npos) ? string::npos : (NewCommPos - CommPos)-1);
				RemoveByVal(crWord);
				RemovePadding(crWord);
				RemoveNonVBSChars(crWord);
				if (crWord.size() <= MAX_FIND_LENGTH && !crWord.empty()) 
				{
					ud.m_keyName = crWord;
					ud.m_uniqueKey = lowerCase(ud.m_keyName) + m_currentParentKey;
					ud.m_uniqueParent = m_currentParentKey;
					FindOrInsertUD(ListIn, ud);
					if (!m_currentParentKey.empty() && !ud.m_uniqueKey.empty() && (iCurParent < ListIn.size()))
					{
						ListIn[iCurParent].m_children.push_back(ud.m_uniqueKey);//add child to parent
					}
				}
				RemainingLine = RemainingLine.substr(CommPos+1, string::npos);
				CommPos = RemainingLine.find_first_of(',');
			}
			return false;
		}
		//Its something new and structural and therefore we are now a parent
		if (m_parentLevel == 0) // its a root
		{
			ud.m_uniqueKey = lowerCase(ud.m_keyName);
			ud.m_uniqueParent.clear();
			const size_t iCurParent = FindOrInsertUD(ListIn, ud);
			//if (iCurParent == -1)
			//{
			//	ShowError("Parent == -1");
			//}
			m_currentParentKey = ud.m_uniqueKey;
			++m_parentLevel;
			// get construct autodims
			string RemainingLine = line;
			size_t CommPos = UCline.find_first_of('(');
			while (CommPos != string::npos)
			{
				//Insert stuff after comma after cleaning up
				const size_t NewCommPos = RemainingLine.find_first_of(',', CommPos+1);
				//get word @
				string crWord = RemainingLine.substr(CommPos+1, (NewCommPos == string::npos) ? string::npos : (NewCommPos - CommPos)-1);
				RemoveByVal(crWord);
				RemovePadding(crWord);
				RemoveNonVBSChars(crWord);
				if (crWord.size() <= MAX_FIND_LENGTH && !crWord.empty())
				{
					ud.m_keyName = crWord;
					ud.eTyping = eDim;
					ud.m_uniqueKey = lowerCase(ud.m_keyName) + m_currentParentKey;
					ud.m_uniqueParent = m_currentParentKey;
					FindOrInsertUD(ListIn, ud);
					if (!m_currentParentKey.empty() && !ud.m_uniqueKey.empty() && (iCurParent < ListIn.size()))
					{
						ListIn[iCurParent].m_children.push_back(ud.m_uniqueKey);//add child to parent
					}
				}
				RemainingLine = RemainingLine.substr(CommPos+1, string::npos);
				CommPos = RemainingLine.find_first_of(',');
			}
		}
		else 
		{
			ud.m_uniqueKey = lowerCase(ud.m_keyName) + m_currentParentKey;
			ud.m_uniqueParent = m_currentParentKey;
			FindOrInsertUD(ListIn, ud);
			const int iUDIndx = UDKeyIndex<true>(ListIn, m_currentParentKey);
			if (iUDIndx == -1)
			{
				//m_parentTreeInvalid = true;
				m_parentLevel = 0;
				m_currentParentKey.clear();
				if (!m_stopErrorDisplay)
				{
					m_stopErrorDisplay = true;
					MessageBox("Construct not closed", ("Parse error on line: " + std::to_string(Lineno)).c_str(), MB_OK);
				}
				return true;
			}
			ListIn[iUDIndx].m_children.push_back(ud.m_uniqueKey);//add child to parent
			m_currentParentKey = ud.m_uniqueKey;
			++m_parentLevel;
		}
	}
	else
	{
		if (endIdx != string::npos)
		{
			if (m_parentLevel == -1)
			{
				//m_parentTreeInvalid = true;
				m_parentLevel = 0;
				m_currentParentKey.clear();
				if (!m_stopErrorDisplay)
				{
					m_stopErrorDisplay = true;
					MessageBox("Construct not opened", ("Parse error on line: " + std::to_string(Lineno)).c_str(), MB_OK);
				}

				return true;
			}
			else
			{ 
				if (m_parentLevel > 0)
				{//finished with child ===== END =====
					const int iCurParent = UDKeyIndex<true>(ListIn, m_currentParentKey);
					if (iCurParent != -1)
					{
						const int iGrandParent = UDKeyIndex<true>(ListIn, ListIn[iCurParent].m_uniqueParent);
						if (iGrandParent != -1)
							m_currentParentKey = ListIn[iGrandParent].m_uniqueKey; 
						else
							m_currentParentKey.clear();
						--m_parentLevel;
						return false;
					}
					/// error - end without start
					m_currentParentKey.clear();
					--m_parentLevel;
					return true;
				}
				else
				{	//Error - end without start
					m_parentLevel = 0;
					m_currentParentKey.clear();
					return true;
				}
			}//if (m_parentLevel == -1)
		}//if (endIdx != string::npos)
	}
	return false;
}

string CodeViewer::ParseDelimtByColon(string &wholeline)
{
	string result;
	const size_t idx = wholeline.find(':'); 
	if (idx == string::npos || idx == 0)
	{
		result = wholeline;
		wholeline.clear();
	}
	else
	{
		result = wholeline.substr(0, idx);
		wholeline = wholeline.substr(idx + 1, wholeline.length() - result.length());
	}

	return result;
}

void CodeViewer::ParseFindConstruct(size_t &Pos, const string &UCLineIn, WordType &Type, int &ConstructSize)
{
	if ((Pos = SureFind(UCLineIn, "DIM"s)) != string::npos)
	{
		ConstructSize = 3;
		Type = eDim;
		return;
	}
	if ((Pos = SureFind(UCLineIn, "CONST"s)) != string::npos)
	{
		ConstructSize = 5;
		Type = eConst;
		return;
	}
	if ((Pos = SureFind(UCLineIn, "SUB"s)) != string::npos)
	{
		ConstructSize = 3;
		Type = eSub;
		return;
	}
	if ((Pos = SureFind(UCLineIn, "FUNCTION"s)) != string::npos)
	{
		ConstructSize = 8;
		Type = eFunction;
		return;
	}
	if ((Pos = SureFind(UCLineIn, "CLASS"s)) != string::npos)
	{
		ConstructSize = 5;
		Type = eClass;
		return;
	}
	if ((Pos = SureFind(UCLineIn, "PROPERTY"s)) != string::npos)
	{
		size_t GetLetPos;
		if ((GetLetPos = SureFind(UCLineIn, "GET"s)) != string::npos)
		{
			if (Pos < GetLetPos)
			{
				Pos = GetLetPos;
				ConstructSize = 3;
				Type = ePropGet;
				return;
			}
		}
		if ((GetLetPos = SureFind(UCLineIn, "LET"s)) != string::npos)
		{
			if (Pos < GetLetPos)
			{
				Pos = GetLetPos;
				ConstructSize = 3;
				Type = ePropLet;
				return;
			}
		}
		if ((GetLetPos = SureFind(UCLineIn, "SET"s)) != string::npos)
		{
			if (Pos < GetLetPos)
			{
				Pos = GetLetPos;
				ConstructSize = 3;
				Type = ePropSet;
				return;
			}
		}
		ConstructSize = 8;
		return;
	}

	Pos = string::npos;
}

void CodeViewer::ReadLineToParseBrain(string wholeline, const int linecount, fi_vector<UserData>& ListIn)
{
		const string comment = ParseRemoveVBSLineComments(wholeline);
		while (wholeline.length() > 1)
		{
			string line = ParseDelimtByColon(wholeline);
			RemovePadding(line);
			const string UCline = upperCase(line);
			UserData UD;
			UD.eTyping = eUnknown;
			UD.m_lineNum = linecount;
			UD.m_comment = comment;
			int SearchLength = 0;
			size_t idx = string::npos;
			ParseFindConstruct(idx, UCline, UD.eTyping, SearchLength);
			if (idx != string::npos) // Found something structural
			{
				const size_t doubleQuoteIdx = line.find('\"');
				if ((doubleQuoteIdx != string::npos) && (doubleQuoteIdx < idx)) continue; // in a string literal
				UD.m_description = line;
				UD.m_keyName = ExtractWordOperand(line, idx + SearchLength); // sSubName
				//UserData ud(linecount, line, sSubName, Type);
				if (!ParseStructureName(ListIn, UD, UCline, line, linecount))
				{/*A critical brain error occurred */}
			}// if ( idx != string::npos)
		}// while (wholeline.length > 1)
}

void CodeViewer::RemoveByVal(string &line)
{
	const size_t LL = line.length();
	size_t Pos = SureFind(lowerCase(line), "byval"s);
	if (Pos != string::npos)
	{
		Pos += 5;
		if ((SSIZE_T)(LL-Pos) < 0) return;
		line = line.substr(Pos, (LL-Pos) );
	}
}

void CodeViewer::RemoveNonVBSChars(string &line)
{
	const size_t LL = line.length();
	size_t Pos = line.find_first_of(VBvalidChars);
	if (Pos == string::npos)
	{
		line.clear();
		return;
	}
	if (Pos > 0)
	{
		if ((SSIZE_T)(LL-Pos) < 1) return;
		line = line.substr(Pos, (LL-Pos));
	}

	Pos = line.find_last_of(VBvalidChars);
	if (Pos != string::npos)
	{
		line = line.erase(Pos+1);
	}
}

void CodeViewer::ParseForFunction() // Subs & Collections WIP 
{
	const int scriptLines = (int)SendMessage(m_hwndScintilla, SCI_GETLINECOUNT, 0, 0);

	m_parentLevel = 0; //root
	m_currentParentKey.clear();
	//m_parentTreeInvalid = false;

	m_pageConstructsDict.clear(); //!! it's actually not needed to clear this list, BUT there is no mechanism (it seems) to delete non-existant subs/functions/etc from it, so rather redo it completely

	for (int linecount = 0; linecount < scriptLines; ++linecount)
	{
		// Read line
		const size_t lineLength = SendMessage(m_hwndScintilla, SCI_LINELENGTH, linecount, 0);
		if (!ParseOKLineLength(lineLength)) continue;
		char text[MAX_LINE_LENGTH] = {};
		SendMessage(m_hwndScintilla, SCI_GETLINE, linecount, (LPARAM)text);
		if (text[0] != '\0')
			ReadLineToParseBrain(text, linecount, m_pageConstructsDict);
	}
	SendMessage(m_hwndFunctionList, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
	SendMessage(m_hwndFunctionList, CB_RESETCONTENT, 0, 0);

	//Propagate subs&funcs in menu in order
	for (fi_vector<UserData>::const_iterator i = m_pageConstructsDict.begin(); i != m_pageConstructsDict.end(); ++i) 
	{
		if (i->eTyping < eDim)
		{
			const char *c_str1 = i->m_keyName.c_str();
			SendMessage(m_hwndFunctionList, CB_ADDSTRING, 0, (LPARAM)c_str1);
		}
	}
	SendMessage(m_hwndFunctionList, WM_SETREDRAW, TRUE, 0);

	//Collect Objects/Components from the menu. (cheat!)
	size_t CBCount = SendMessage(m_hwndItemList, CB_GETCOUNT, 0, 0)-1; //Zero Based
	while ((SSIZE_T)CBCount >= 0)
	{
		const LRESULT s = SendMessage(m_hwndItemList, CB_GETLBTEXTLEN, CBCount, 0);
		if (s > 1)
		{
			char * const c_str1 = new char[s + 1];
			SendMessage(m_hwndItemList, CB_GETLBTEXT, CBCount, (LPARAM)c_str1);
			UserData ud;
			ud.m_keyName = c_str1;
			delete[] c_str1;
			ud.m_uniqueKey = lowerCase(ud.m_keyName);
			FindOrInsertUD(m_componentsDict,ud);
		}
		CBCount--;
	}

	//Now merge the lot for Auto complete...
	vector<string> autoCompList;
	for (fi_vector<UserData>::const_iterator i = m_VBwordsDict.begin(); i != m_VBwordsDict.end(); ++i)
	{
		FindOrInsertStringIntoAutolist(autoCompList,i->m_keyName);
	}
	string strVPcoreWords;
	for (fi_vector<UserData>::const_iterator i = m_VPcoreDict.begin(); i != m_VPcoreDict.end(); ++i)
	{
		if (FindOrInsertStringIntoAutolist(autoCompList,i->m_keyName))
		{
			strVPcoreWords += i->m_keyName;
			strVPcoreWords += ' ';
		}
	}
	string strCompOut;
	for (fi_vector<UserData>::const_iterator i = m_componentsDict.begin(); i != m_componentsDict.end(); ++i)
	{
		if (FindOrInsertStringIntoAutolist(autoCompList,i->m_keyName))
		{
			strCompOut += i->m_keyName;
			strCompOut += ' ';
		}
	}
	string sSubFunOut;
	for (fi_vector<UserData>::const_iterator i = m_pageConstructsDict.begin(); i != m_pageConstructsDict.end(); ++i)
	{
		if (FindOrInsertStringIntoAutolist(autoCompList,i->m_keyName))
		{
			sSubFunOut += i->m_keyName;
			sSubFunOut += ' ';
		}
	}
	m_autoCompString.clear();
	for (vector<string>::const_iterator i = autoCompList.begin(); i != autoCompList.end(); ++i)
	{
		m_autoCompString += *i;
		m_autoCompString += ' ';
	}

	//Send the collected func/subs to scintilla for highlighting - always as lowercase as VBS is case insensitive.
	//TODO: Need to comune with scintilla closer (COM pointers??)
	std::transform(sSubFunOut.begin(), sSubFunOut.end(), sSubFunOut.begin(), ::tolower);
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 1, (LPARAM)sSubFunOut.c_str());
	std::transform(strCompOut.begin(), strCompOut.end(), strCompOut.begin(), ::tolower);
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 2, (LPARAM)strCompOut.c_str());
	std::transform(strVPcoreWords.begin(), strVPcoreWords.end(), strVPcoreWords.begin(), ::tolower);
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 3, (LPARAM)strVPcoreWords.c_str());
}

static string GetTextFileFromDirectory(const string& szfilename, const string& dirname)
{
   string szPath;
   if (!dirname.empty())
      szPath = g_pvp->m_szMyPath + dirname;
   // else: use current directory
   return szPath + szfilename;
}

void CodeViewer::ParseVPCore()
{
	FILE* fCore = nullptr;
	for(size_t i = 0; i < std::size(defaultFileNameSearch); ++i)
		if ((fopen_s(&fCore, GetTextFileFromDirectory(defaultFileNameSearch[i] + "core.vbs", defaultPathSearch[i]).c_str(), "r") == 0) && fCore)
			break;

	if(!fCore)
	{
		MessageBox("Couldn't find core.vbs for code completion parsing!", "Script Parser Warning", MB_OK);
		return;
	}

	//initialize Parent child
	m_parentLevel = 0; //root
	m_currentParentKey.clear();
	m_stopErrorDisplay = true;/// WIP BRANDREW (was set to false)
	//m_parentTreeInvalid = false;
	int linecount = 0;
	while (!feof(fCore))
	{
		char text[MAX_LINE_LENGTH] = {};
		fgets(text, MAX_LINE_LENGTH, fCore);
		if (text[0] != '\0')
		{
			string wholeline(text);
			++linecount;
			const size_t lineLength = wholeline.length();
			if (!ParseOKLineLength(lineLength)) continue;
			ReadLineToParseBrain(wholeline, linecount, m_VPcoreDict);
		}
	}
	fclose(fCore);
}

string CodeViewer::ExtractWordOperand(const string &line, const size_t StartPos)
{
	size_t Substart = StartPos;
	const size_t lineLength = line.size();
	char linechar = line[Substart];
	while ((m_validChars.find(linechar) == string::npos) && (Substart < lineLength))
	{
		Substart++;
		linechar = line[Substart];
	}
	//scan for last valid char
	size_t Subfinish = Substart;
	while ((m_validChars.find(linechar) != string::npos) && (Subfinish < lineLength))
	{
		Subfinish++;
		linechar = line[Subfinish];
	}
	return line.substr(Substart,Subfinish-Substart);
}

CodeViewer* CodeViewer::GetCodeViewerPtr()
{
   return (CodeViewer *)GetWindowLongPtr(GWLP_USERDATA);
}

BOOL CodeViewer::ParseClickEvents(const int id, const SCNotification *pSCN)
{
   CodeViewer* const pcv = GetCodeViewerPtr();
   switch (id)
   {
      case ID_COMPILE:
      {
         pcv->Compile(true);
         pcv->EndSession();
         return TRUE;
      }
      case ID_SCRIPT_TOGGLE_LAST_ERROR_VISIBILITY:
      {
         SetLastErrorVisibility(!m_lastErrorWidgetVisible);
         return TRUE;
      }
      case ID_SCRIPT_PREFERENCES:
      {
         DialogBox(g_pvp->theInstance, MAKEINTRESOURCE(IDD_CODEVIEW_PREFS), GetHwnd(), CVPrefProc);
         return TRUE;
      }
      case ID_FIND:
      {
         pcv->ShowFindDialog();
         return TRUE;
      }
      case ID_REPLACE:
      {
         pcv->ShowFindReplaceDialog();
         return TRUE;
      }
      case ID_EDIT_FINDNEXT:
      {
         pcv->Find(&pcv->m_findreplaceold);
         return TRUE;
      }
      case ID_EDIT_UNDO:
      {
         ::SendMessage(pcv->m_hwndScintilla, SCI_UNDO, 0, 0);
         return TRUE;
      }
      case ID_EDIT_COPY:
      {
         ::SendMessage(pcv->m_hwndScintilla, WM_COPY, 0, 0);
         return TRUE;
      }
      case ID_EDIT_CUT:
      {
         ::SendMessage(pcv->m_hwndScintilla, WM_CUT, 0, 0);
         return TRUE;
      }
      case ID_EDIT_PASTE:
      {
         ::SendMessage(pcv->m_hwndScintilla, WM_PASTE, 0, 0);
         return TRUE;
      }
      case ID_ADD_COMMENT:
      {
         AddComment(pcv->m_hwndScintilla);
         return TRUE;
      }
      case ID_REMOVE_COMMENT:
      {
         RemoveComment(pcv->m_hwndScintilla);
         return TRUE;
      }
      case ID_GO_TO_DEFINITION:
      {
         ShowTooltipOrGoToDefinition(pSCN,false);
         return TRUE;
      }
   }
   return FALSE;
}

BOOL CodeViewer::ParseSelChangeEvent(const int id, const SCNotification *pSCN)
{
   CodeViewer* const pcv = GetCodeViewerPtr();
   switch (id)
   {
      case ID_FIND: // accelerator
      {
         pcv->ShowFindDialog();
         return TRUE;
      }
      case ID_SAVE:
      case ID_TABLE_CAMERAMODE:
      case ID_TABLE_LIVEEDIT:
      case ID_TABLE_PLAY:
      {
         pcv->m_psh->DoCodeViewCommand(id);
         return TRUE;
      }
      case ID_EDIT_FINDNEXT:
      {
         pcv->Find(&pcv->m_findreplaceold);
         return TRUE;
      }
      case ID_REPLACE:
      {
         pcv->ShowFindReplaceDialog();
         return TRUE;
      }
      case ID_EDIT_UNDO:
      {
         ::SendMessage(pcv->m_hwndScintilla, SCI_UNDO, 0, 0);
         return TRUE;
      }
      case IDC_ITEMLIST:
      {
         pcv->ListEventsFromItem();
         pcv->TellHostToSelectItem();
         return TRUE;
      }
      case IDC_EVENTLIST:
      {
         pcv->FindCodeFromEvent();
         return TRUE;
      }
      case IDC_FUNCTIONLIST:
      {
         const LRESULT Listindex = ::SendMessage(pcv->m_hwndFunctionList, CB_GETCURSEL, 0, 0);
         if (Listindex != -1)
         {
            char ConstructName[MAX_FIND_LENGTH] = {};
            /*size_t index =*/ SendMessage(pcv->m_hwndFunctionList, CB_GETLBTEXT, Listindex, (LPARAM)ConstructName);
            int idx;
            string s(ConstructName);
            RemovePadding(s);
            FindUD(pcv->m_pageConstructsDict, lowerCase(s), idx);
            if (idx != -1)
            {
               SendMessage(pcv->m_hwndScintilla, SCI_GOTOLINE, pcv->m_pageConstructsDict[idx].m_lineNum, 0);
               SendMessage(pcv->m_hwndScintilla, SCI_GRABFOCUS, 0, 0);
            }
         }
         return TRUE;
      }
      case ID_ADD_COMMENT:
      {
         AddComment(pcv->m_hwndScintilla);
         return TRUE;
      }
      case ID_REMOVE_COMMENT:
      {
         RemoveComment(pcv->m_hwndScintilla);
         return TRUE;
      }
      case ID_GO_TO_DEFINITION:
      {
         ShowTooltipOrGoToDefinition(pSCN, false);
         return TRUE;
      }
      case ID_SHOWAUTOCOMPLETE:
      {
         pcv->ShowAutoComplete(pSCN);
         return TRUE;
      }
   }//switch (id)
   return FALSE;
}

LRESULT CodeViewer::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CodeViewer* const pcv = GetCodeViewerPtr();

   if (uMsg == g_FindMsgString)
   {
      FINDREPLACE * const pfr = (FINDREPLACE *)lParam;
      if (pfr->Flags & FR_DIALOGTERM)
      {
         pcv->m_hwndFind = nullptr;
         const size_t selstart = SendMessage(pcv->m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
         const size_t selend = SendMessage(pcv->m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);
         ::SetFocus(pcv->m_hwndScintilla);
         SendMessage(pcv->m_hwndScintilla, SCI_SETSEL, selstart, selend);
         return 0;
      }
      if (pfr->Flags & FR_FINDNEXT)
         pcv->Find(pfr);
      if ((pfr->Flags & FR_REPLACE) || (pfr->Flags & FR_REPLACEALL))
         pcv->Replace(pfr);
   }

   switch (uMsg)
   {
      case WM_ACTIVATE:
      {
         if (LOWORD(wParam) != WA_INACTIVE)
         {
            g_pvp->m_pcv = pcv;
            pcv->m_stopErrorDisplay = true; ///stop Error reporting WIP
            warn_on_dupes = true;
            pcv->ParseForFunction();
         }
         break;
      }
      case WM_CLOSE:
      {
         pcv->SetVisible(false);
         return TRUE;
      }
      case WM_SYSCOMMAND:
      {
         if (wParam == SC_MINIMIZE && g_pvp != nullptr)
              pcv->m_minimized = true;

         if (wParam == SC_RESTORE && g_pvp != nullptr)
              pcv->m_minimized = false;
         break;
      }
      case WM_SIZE:
      {
         if (pcv && pcv->m_hwndStatus)
         {
            ::SendMessage(pcv->m_hwndStatus, WM_SIZE, wParam, lParam);

            ResizeScintillaAndLastError();
         }
         break;
      }
   }

   return WndProcDefault(uMsg, wParam, lParam);
}

BOOL CodeViewer::OnCommand(WPARAM wparam, LPARAM lparam)
{
   UNREFERENCED_PARAMETER(lparam);

   CodeViewer* const pcv = GetCodeViewerPtr();

   const int code = HIWORD(wparam);
   const int id = LOWORD(wparam);

   switch (code)
   {
      case SCEN_SETFOCUS:
      {
         warn_on_dupes = true;
         pcv->ParseForFunction();
         return TRUE;
      }
      case SCEN_CHANGE:
      {
         //also see SCN_MODIFIED handling which does more finegrained updating calls

         if (pcv->m_errorLineNumber != -1)
            pcv->UncolorError();
         if (!pcv->m_ignoreDirty && (pcv->m_sdsDirty < eSaveDirty))
         {
            pcv->m_sdsDirty = eSaveDirty;
            pcv->m_psh->SetDirtyScript(eSaveDirty);
         }
         return TRUE;
      }
      case CBN_SETFOCUS:
      {
         if (id == IDC_FUNCTIONLIST)
         {
            pcv->m_stopErrorDisplay = true;
            pcv->ParseForFunction();
         }
         return TRUE;
      }
      case CBN_SELCHANGE: // Or accelerator
      {
         const SCNotification *const pscn = (SCNotification *)lparam;
         return ParseSelChangeEvent(id, pscn) ? TRUE : FALSE;
      }
      case BN_CLICKED:
      {
         const SCNotification *const pscn = (SCNotification *)lparam;
         return ParseClickEvents(id, pscn);
      }
   }
   return FALSE;
}

LRESULT CodeViewer::OnNotify(WPARAM wparam, LPARAM lparam)
{
   const NMHDR* const pnmh = (LPNMHDR)lparam;
   const SCNotification* const pscn = (SCNotification*)lparam;

   const HWND hwndRE = pnmh->hwndFrom;
   const int code = pnmh->code;
   CodeViewer* const pcv = GetCodeViewerPtr();
   switch (code)
   {
      case SCN_SAVEPOINTREACHED:
      {
         if (pcv->m_sdsDirty > eSaveClean)
         {
            pcv->m_sdsDirty = eSaveClean;
            pcv->m_psh->SetDirtyScript(eSaveClean);
         }
         break;
      }
      case SCN_DWELLSTART:
      {
         if (pcv->m_dwellDisplay)
            pcv->m_toolTipActive = pcv->ShowTooltipOrGoToDefinition(pscn, true);
         break;
      }
      case SCN_DWELLEND:
      {
         if (pcv->m_toolTipActive)
         {
            SendMessage(pcv->m_hwndScintilla, SCI_CALLTIPCANCEL, 0, 0);
            pcv->m_toolTipActive = false;
         }
         break;
      } 
      case SCN_CHARADDED:
      {
         // Member selection
         if (pcv->m_displayAutoComplete)
            pcv->ShowAutoComplete(pscn);
         break;
      }
      case SCN_UPDATEUI:
      {
         const size_t pos = ::SendMessage(hwndRE, SCI_GETCURRENTPOS, 0, 0);
         const size_t line = ::SendMessage(hwndRE, SCI_LINEFROMPOSITION, pos, 0) + 1;
         const size_t column = ::SendMessage(hwndRE, SCI_GETCOLUMN, pos, 0);

         char szT[256];
         sprintf_s(szT, sizeof(szT), "Line %u, Col %u", (U32)line, (U32)column);
         ::SendMessage(pcv->m_hwndStatus, SB_SETTEXT, 0 | 0, (size_t)szT);
         break;
      }
      case SCN_DOUBLECLICK:
      {
         pcv->m_wordUnderCaret.lpstrText = CaretTextBuff;
         memset(CaretTextBuff, 0, sizeof(CaretTextBuff));
         pcv->GetWordUnderCaret();
         szLower(pcv->m_wordUnderCaret.lpstrText);
         // set back ground colour of all words on display
         ::SendMessage(pcv->m_hwndScintilla, SCI_STYLESETBACK, SCE_B_KEYWORD5, RGB(200, 200, 200));
         ::SendMessage(pcv->m_hwndScintilla, SCI_SETKEYWORDS, 4, (LPARAM)CaretTextBuff);
         break;
      }
      case SCN_MARGINCLICK:
      {
         if (pscn->margin == 1)
            pcv->MarginClick(pscn->position, pscn->modifiers);
         break;
      }
      case SCN_MODIFIED:
      {
         if (pscn->linesAdded != 0) // line(s) of text deleted or added? -> update all (i.e. especially to update line numbers then)
            pcv->ParseForFunction(); //!! too slow for large script tables?!
         break;
      }
   }
   return CWnd::OnNotify(wparam, lparam);
}

INT_PTR CALLBACK CVPrefProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      const HWND hwndParent = GetParent(hwndDlg);
      RECT rcDlg;
      RECT rcMain;
      GetWindowRect(hwndParent, &rcMain);
      GetWindowRect(hwndDlg, &rcDlg);
      SetWindowPos(hwndDlg, nullptr,
         (rcMain.right + rcMain.left) / 2 - (rcDlg.right - rcDlg.left) / 2,
         (rcMain.bottom + rcMain.top) / 2 - (rcDlg.bottom - rcDlg.top) / 2,
         0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
      CodeViewer* const pcv = g_pvp->m_pcv;
		if (pcv->m_lPrefsList)
		{
			for (size_t i = 0; i < pcv->m_lPrefsList->size(); i++)
			{
				pcv->m_lPrefsList->at(i)->GetPrefsFromReg();
				pcv->m_lPrefsList->at(i)->SetCheckBox(hwndDlg);
			}
			pcv->m_bgColor = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, "BackGroundColor"s, (int)RGB(255, 255, 255));
			pcv->m_bgSelColor = g_pvp->m_settings.LoadValueWithDefault(Settings::CVEdit, "BackGroundSelectionColor"s, (int)RGB(192, 192, 192));
			pcv->UpdateScinFromPrefs();
			SNDMSG(GetDlgItem(hwndDlg, IDC_CVP_CHKBOX_SHOWAUTOCOMPLETE), BM_SETCHECK, pcv->m_displayAutoComplete ? BST_CHECKED : BST_UNCHECKED, 0L);
			SNDMSG(GetDlgItem(hwndDlg, IDC_CVP_CHKBOX_DISPLAYDWELL), BM_SETCHECK, pcv->m_dwellDisplay ? BST_CHECKED : BST_UNCHECKED, 0L);
			SNDMSG(GetDlgItem(hwndDlg, IDC_CVP_CHKBOX_HELPWITHDWELL), BM_SETCHECK, pcv->m_dwellHelp ? BST_CHECKED : BST_UNCHECKED, 0L);

			SetDlgItemText(hwndDlg, IDC_CVP_EDIT_AUTOCHARS, std::to_string(pcv->m_displayAutoCompleteLength).c_str());
			SetDlgItemText(hwndDlg, IDC_CVP_EDIT_MOUSEDWELL, std::to_string(pcv->m_dwellDisplayTime).c_str());
		}

		//SetFocus(hwndDlg);
		//#if !(defined(IMSPANISH) | defined(IMGERMAN) | defined(IMFRENCH))
		//      ShowWindow(GetDlgItem(hwndDlg, IDC_TRANSNAME), SW_HIDE);
		//#endif
		//
		//#if !(defined(IMSPANISH))
		//      ShowWindow(GetDlgItem(hwndDlg, IDC_TRANSLATEWEBSITE), SW_HIDE);
		//#endif
	}
   break; //case WM_INITDIALOG:

   case WM_COMMAND:
   {
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
			CodeViewer * const pcv = g_pvp->m_pcv;
			if (pcv)
			{
				const int wParamLowWord = LOWORD(wParam);
				switch (wParamLowWord)
				{
				case IDC_CVP_BUT_CANCEL:
				{
					pcv->UpdatePrefsfromReg();
					pcv->UpdateScinFromPrefs();
					EndDialog(hwndDlg, FALSE);
				}
				break;
				case IDC_CVP_BUT_OK:
				{
					//Save to reg.
					pcv->m_displayAutoCompleteLength = GetDlgItemInt(hwndDlg, IDC_CVP_EDIT_AUTOCHARS, 0, false);
					pcv->m_dwellDisplayTime = GetDlgItemInt(hwndDlg, IDC_CVP_EDIT_MOUSEDWELL, 0, false);
					pcv->UpdateRegWithPrefs();
					pcv->UpdateScinFromPrefs();
					EndDialog(hwndDlg, TRUE);
				}
				break;
				case IDC_CVP_CHKBOX_DISPLAYDWELL:
				{
					pcv->m_dwellDisplay = !!IsDlgButtonChecked(hwndDlg, IDC_CVP_CHKBOX_DISPLAYDWELL);
					g_pvp->m_settings.SaveValue(Settings::CVEdit, "DwellDisplay"s, pcv->m_dwellDisplay);
				}
				break;
				case IDC_CVP_CHKBOX_HELPWITHDWELL:
				{
					pcv->m_dwellHelp = !!IsDlgButtonChecked(hwndDlg, IDC_CVP_CHKBOX_HELPWITHDWELL);
               g_pvp->m_settings.SaveValue(Settings::CVEdit, "DwellHelp"s, pcv->m_dwellHelp);
				}
				break;
				case IDC_CVP_CHKBOX_SHOWAUTOCOMPLETE:
				{
					pcv->m_displayAutoComplete = !!IsDlgButtonChecked(hwndDlg, IDC_CVP_CHKBOX_SHOWAUTOCOMPLETE);
               g_pvp->m_settings.SaveValue(Settings::CVEdit, "DisplayAutoComplete"s, pcv->m_displayAutoComplete);
				}
				break;
				case IDC_CVP_BUT_COL_BACKGROUND:
				{
					CHOOSECOLOR cc = {};
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.rgbResult = pcv->m_bgColor;
					cc.lpCustColors = pcv->m_prefCols;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&cc))
					{
						pcv->m_bgColor = cc.rgbResult;
						pcv->UpdateScinFromPrefs();
						return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
					}
				}
				break;
				case IDC_CVP_BUT_COL_BACKGROUND_SEL:
				{
					CHOOSECOLOR cc = {};
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.rgbResult = pcv->m_bgSelColor;
					cc.lpCustColors = pcv->m_prefCols;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&cc))
					{
						pcv->m_bgSelColor = cc.rgbResult;
						pcv->UpdateScinFromPrefs();
						return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
					}
				}
				break;
				case IDC_CVP_BUT_COL_EVERYTHINGELSE:
				{
					CHOOSECOLOR cc = {};
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.rgbResult = pcv->m_prefEverythingElse->m_rgb;
					cc.lpCustColors = pcv->m_prefCols; 
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&cc))
					{
						pcv->m_prefEverythingElse->m_rgb = cc.rgbResult;
						pcv->UpdateScinFromPrefs();
						return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
					}
				}
				break;
				case IDC_CVP_BUT_FONT_EVERYTHINGELSE:
				{
					pcv->m_prefEverythingElse->m_logFont.lfHeight = pcv->m_prefEverythingElse->GetHeightFromPointSize(hwndDlg);
					CHOOSEFONT cf = {};
					cf.lStructSize = sizeof(CHOOSEFONT);
					cf.Flags = CF_NOVERTFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
					cf.hDC = GetDC(hwndDlg);
					cf.hwndOwner = hwndDlg;
					cf.iPointSize = pcv->m_prefEverythingElse->m_pointSize * 10;
					cf.lpLogFont = &(pcv->m_prefEverythingElse->m_logFont);
					cf.rgbColors = pcv->m_prefEverythingElse->m_rgb;
					if (ChooseFont(&cf))
					{
						pcv->m_prefEverythingElse->m_rgb = cf.rgbColors;
						pcv->m_prefEverythingElse->m_pointSize = cf.iPointSize / 10;
						pcv->UpdateScinFromPrefs();
						return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
					}
				}
				break;
				default:
					//EverythingElse=0, default=1, consumed above
					for (size_t i = 2; i < pcv->m_lPrefsList->size(); ++i)
					{
						CVPreference* Pref = pcv->m_lPrefsList->at(i);
						if (Pref->IDC_ChkBox_code == wParamLowWord)// && Pref->IDC_ChkBox_code != 0)
						{
							Pref->ReadCheckBox(hwndDlg);
							pcv->UpdateScinFromPrefs();
							break;
						}

						if (Pref->IDC_ColorBut_code == wParamLowWord)
						{
							CHOOSECOLOR cc = {};
							cc.lStructSize = sizeof(CHOOSECOLOR);
							cc.hwndOwner = hwndDlg;
							cc.rgbResult = Pref->m_rgb;
							cc.lpCustColors = pcv->m_prefCols;
							cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
							if (ChooseColor(&cc))
							{
								Pref->m_rgb = cc.rgbResult;
								pcv->UpdateScinFromPrefs();
								return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
							}
							break;
						}

						if (Pref->IDC_Font_code == wParamLowWord)
						{
							Pref->m_logFont.lfHeight = Pref->GetHeightFromPointSize(hwndDlg);
							CHOOSEFONT cf = {};
							cf.lStructSize = sizeof(CHOOSEFONT);
							cf.Flags = CF_NOVERTFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
							cf.hDC = GetDC(hwndDlg);
							cf.hwndOwner = hwndDlg;
							cf.iPointSize = Pref->m_pointSize * 10;
							cf.lpLogFont = &Pref->m_logFont;
							cf.rgbColors = Pref->m_rgb;
							if (ChooseFont(&cf))
							{
								Pref->m_rgb = cf.rgbColors;
								Pref->m_pointSize = cf.iPointSize / 10;
								pcv->UpdateScinFromPrefs();
								return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
								//break;
							}
						}
					}// for pref
				}// end switch Button clicked
			}// end if (pcv->prefVBS)
      }// end switch Hiword Wparam
   }// case WM_COMMAND
   break;
   case WM_CLOSE:
      EndDialog(hwndDlg, TRUE);
   break;
   }
   return FALSE; // be selfish - consume all
   //return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}

void CodeViewer::UpdateScinFromPrefs()
{
	SendMessage(m_hwndScintilla, SCI_SETMOUSEDWELLTIME, m_dwellDisplayTime, 0);
	SendMessage(m_hwndScintilla, SCI_STYLESETBACK, m_prefEverythingElse->m_sciKeywordID, m_bgColor);
	SendMessage(m_hwndScintilla, SCI_SETSELBACK, m_prefEverythingElse->m_sciKeywordID, m_bgSelColor);
	m_prefEverythingElse->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);//Update internally
	SendMessage(m_hwndScintilla,SCI_STYLECLEARALL,0,0);
	SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, m_prefEverythingElse->m_rgb);
	SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, m_bgColor);
	prefDefault->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefVBS->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefSubs->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefComps->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefLiterals->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefComments->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefVPcore->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	SendMessage(m_hwndScintilla, SCI_STYLESETBACK, SCE_B_KEYWORD5, RGB(200,200,200));
	SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 4 , (LPARAM)m_wordUnderCaret.lpstrText);

	const int scriptLines = (int)SendMessage(m_hwndScintilla, SCI_GETLINECOUNT, 0, 0);
   //Update the margin width to fit the number of characters required to display the line number * font size
   SendMessage(m_hwndScintilla, SCI_SETMARGINWIDTHN, 0, (scriptLines > 0 ? (int)ceil(log10(scriptLines) + 3) : 1) * m_prefEverythingElse->m_pointSize);
}

void CodeViewer::ResizeScintillaAndLastError()
{
	const CodeViewer* const pcv = GetCodeViewerPtr();

	const CRect rc = GetClientRect();
	RECT rcStatus;
	::GetClientRect(pcv->m_hwndStatus, &rcStatus);
	const int statheight = rcStatus.bottom - rcStatus.top;

	const int scintillaHeight = rc.bottom - rc.top - statheight - (30 + 2 + 40) - (m_lastErrorWidgetVisible ? LAST_ERROR_WIDGET_HEIGHT : 0);
	::SetWindowPos(pcv->m_hwndScintilla, nullptr,
		0, 0,
		rc.right - rc.left, scintillaHeight,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	if (m_lastErrorWidgetVisible)
	{
		::SetWindowPos(pcv->m_hwndLastErrorTextArea, nullptr,
			0, (30 + 2 + 40) + scintillaHeight,
			rc.right - rc.left, LAST_ERROR_WIDGET_HEIGHT,
			SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	}
}

void CodeViewer::SetLastErrorVisibility(bool show)
{
	if (show == m_lastErrorWidgetVisible) return;
	m_lastErrorWidgetVisible = show;

	ResizeScintillaAndLastError();

	::ShowWindow(m_hwndLastErrorTextArea, show ? SW_SHOW : SW_HIDE);
}

void CodeViewer::SetLastErrorTextW(const LPCWSTR text)
{
	::SetWindowTextW(m_hwndLastErrorTextArea, text);

	// Scroll to the bottom
	SendMessage(m_hwndLastErrorTextArea, EM_LINESCROLL, 0, 9999);
}

void CodeViewer::AppendLastErrorTextW(const wstring& text)
{
	const int requiredLength = ::GetWindowTextLength(m_hwndLastErrorTextArea) + (int)text.length() + 1;
	wchar_t* buf = new wchar_t[requiredLength];

	// Get existing text from edit control and put into buffer
	::GetWindowTextW(m_hwndLastErrorTextArea, buf, requiredLength);

	// Append the new text to the buffer
	wcscat_s(buf, requiredLength, text.c_str());

	::SetWindowTextW(m_hwndLastErrorTextArea, buf);

	// Scroll to the bottom
	SendMessage(m_hwndLastErrorTextArea, EM_LINESCROLL, 0, 9999);

	delete[] buf;
}

Collection::Collection()
{
   m_fireEvents = false;
   m_stopSingleEvents = false;

   m_groupElements = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "GroupElementsInCollection"s, true);
}

STDMETHODIMP Collection::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString((WCHAR *)m_wzName);

   return S_OK;
}

HRESULT Collection::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteWideString(FID(NAME), m_wzName);

   for (int i = 0; i < m_visel.size(); ++i)
   {
      const IScriptable * const piscript = m_visel[i].GetIEditable()->GetScriptable();
      bw.WriteWideString(FID(ITEM), piscript->m_wzName);
   }

   bw.WriteBool(FID(EVNT), m_fireEvents);
   bw.WriteBool(FID(SSNG), m_stopSingleEvents);
   bw.WriteBool(FID(GREL), m_groupElements);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Collection::LoadData(IStream *pstm, PinTable *ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffReader br(pstm, this, ppt, version, hcrypthash, hcryptkey);

   br.Load();
   return S_OK;
}

bool Collection::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(NAME):
   {
      //!! workaround: due to a bug in earlier versions, it can happen that the string written was one char too long
      WCHAR tmp[MAXNAMEBUFFER+1];
      pbr->GetWideString(tmp, MAXNAMEBUFFER+1);
      memcpy(m_wzName, tmp, (MAXNAMEBUFFER-1)*sizeof(WCHAR));
      m_wzName[MAXNAMEBUFFER-1] = 0;
      break;
   }
   case FID(EVNT): pbr->GetBool(m_fireEvents); break;
   case FID(SSNG): pbr->GetBool(m_stopSingleEvents); break;
   case FID(GREL): pbr->GetBool(m_groupElements); break;
   case FID(ITEM):
   {
      //!! BUG - item list must be up to date in table (loaded) for the reverse name lookup to work
      const PinTable * const ppt = (PinTable *)pbr->m_pdata;

      //!! workaround: due to a bug in earlier versions, it can happen that the string written was twice the size
      WCHAR wzT[MAXNAMEBUFFER*2];
      pbr->GetWideString(wzT, MAXNAMEBUFFER*2); //!! rather truncate for these special cases for the comparison below?

      for (size_t i = 0; i < ppt->m_vedit.size(); ++i)
      {
         IScriptable * const piscript = ppt->m_vedit[i]->GetScriptable();
         if (piscript) // skip decals
         {
            if (!WideStrCmp(piscript->m_wzName, wzT))
            {
               piscript->GetISelect()->GetIEditable()->m_vCollection.push_back(this);
               piscript->GetISelect()->GetIEditable()->m_viCollection.push_back(m_visel.size());
               m_visel.push_back(piscript->GetISelect());
               return true;
            }
         }
      }
      break;
   }
   }
   return true;
}

STDMETHODIMP Collection::get_Count(long __RPC_FAR *plCount)
{
   *plCount = m_visel.size();
   return S_OK;
}

STDMETHODIMP Collection::get_Item(long index, IDispatch __RPC_FAR * __RPC_FAR *ppidisp)
{
   if (index < 0 || index >= m_visel.size())
      return TYPE_E_OUTOFBOUNDS;

   IDispatch * const pdisp = m_visel[index].GetDispatch();
   return pdisp->QueryInterface(IID_IDispatch, (void **)ppidisp);
}

STDMETHODIMP Collection::get__NewEnum(IUnknown** ppunk)
{
   CComObject<OMCollectionEnum> *pomenum;
   HRESULT hr = CComObject<OMCollectionEnum>::CreateInstance(&pomenum);

   if (SUCCEEDED(hr))
   {
      pomenum->Init(this);
      hr = pomenum->QueryInterface(IID_IEnumVARIANT, (void **)ppunk);
   }

   return hr;
}

STDMETHODIMP OMCollectionEnum::Init(Collection *pcol)
{
   m_pcol = pcol;
   m_index = 0;
   return S_OK;
}

STDMETHODIMP OMCollectionEnum::Next(ULONG celt, VARIANT __RPC_FAR *rgVar, ULONG __RPC_FAR *pCeltFetched)
{
   int last;
   HRESULT hr;
   const int cwanted = celt;
   int creturned;

   if (m_index + cwanted > m_pcol->m_visel.size())
   {
      hr = S_FALSE;
      last = m_pcol->m_visel.size();
      creturned = m_pcol->m_visel.size() - m_index;
   }
   else
   {
      hr = S_OK;
      last = m_index + cwanted;
      creturned = cwanted;
   }

   for (int i = m_index; i < last; ++i)
   {
      IDispatch * const pdisp = m_pcol->m_visel[i].GetDispatch();
      pdisp->QueryInterface(IID_IDispatch, (void **)&pdisp);

      V_VT(&rgVar[i - m_index]) = VT_DISPATCH;
      V_DISPATCH(&rgVar[i - m_index]) = pdisp;
   }

   m_index += creturned;

   if (pCeltFetched)
      *pCeltFetched = creturned;

   return hr;
}

STDMETHODIMP OMCollectionEnum::Skip(ULONG celt)
{
   m_index += celt;
   return (m_index >= m_pcol->m_visel.size()) ? S_FALSE : S_OK;
}

STDMETHODIMP OMCollectionEnum::Reset()
{
   m_index = 0;

   return S_OK;
}

STDMETHODIMP OMCollectionEnum::Clone(IEnumVARIANT __RPC_FAR *__RPC_FAR *ppEnum)
{
   IUnknown *punk;
   HRESULT hr = m_pcol->get__NewEnum(&punk);

   if (SUCCEEDED(hr))
   {
      hr = punk->QueryInterface(IID_IEnumVARIANT, (void **)ppEnum);

      punk->Release();
   }

   return hr;
}

void DebuggerModule::Init(CodeViewer * const pcv)
{
   m_pcv = pcv;
}

STDMETHODIMP DebuggerModule::Print(VARIANT *pvar)
{
   // Disable logging in locked tables (there is no debugger in locked mode anyway)
   if (g_pplayer->m_ptable->IsLocked())
      return S_OK;

   const bool enableLog = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "EnableLog"s, false);
   const bool logScript = enableLog && g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "LogScriptOutput"s, true);

   if (V_VT(pvar) == VT_EMPTY || V_VT(pvar) == VT_NULL || V_VT(pvar) == VT_ERROR)
   {
      if (g_pplayer->m_hwndDebugOutput)
         m_pcv->AddToDebugOutput("");
      PLOGI_IF_(PLOG_NO_DBG_OUT_INSTANCE_ID, logScript) << "Script.Print '";
      return S_OK;
   }

   CComVariant varT;
   const HRESULT hr = VariantChangeType(&varT, pvar, 0, VT_BSTR);

   if (FAILED(hr))
   {
      const LocalString ls(IDS_DEBUGNOCONVERT);
      if (g_pplayer->m_hwndDebugOutput)
         m_pcv->AddToDebugOutput(ls.m_szbuffer);
      PLOGI_IF_(PLOG_NO_DBG_OUT_INSTANCE_ID, logScript) << "Script.Print '" << ls.m_szbuffer << '\'';
      return S_OK;
   }

   const WCHAR * const wzT = V_BSTR(&varT);
   const int len = lstrlenW(wzT);

   char * const szT = new char[len + 1];
   WideCharToMultiByteNull(CP_ACP, 0, wzT, -1, szT, len + 1, nullptr, nullptr);
   if (g_pplayer->m_hwndDebugOutput)
      m_pcv->AddToDebugOutput(szT);
   PLOGI_IF_(PLOG_NO_DBG_OUT_INSTANCE_ID, logScript) << "Script.Print '" << szT << '\'';
   delete[] szT;

   return S_OK;
}

STDMETHODIMP DebuggerModule::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString(L"Debug");

   return S_OK;
}
