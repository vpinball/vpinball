#include "core/stdafx.h"

#ifndef __STANDALONE__
#include "scilexer.h"
#include <DbgProp.h>
#endif

#ifdef __LIBVPINBALL__
#include "lib/src/VPinballLib.h"
#endif

#ifdef __STANDALONE__
#include <sstream>
#include <climits>
#endif

#include <fstream>

static constexpr int LAST_ERROR_WIDGET_HEIGHT = 256;

//Scintilla Lexer parses only lower case unless otherwise told
static constexpr char vbsReservedWords[] =
"and as byref byval case call const "
"continue dim do each else elseif end error exit false for function global "
"goto if in loop me new next not nothing on optional or private public "
"redim rem resume select set sub then to true type while with "
"boolean byte currency date double integer long object single string type "
"variant option explicit randomize";

static const string VBvalidChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"s);

INT_PTR CALLBACK CVPrefProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


constexpr __forceinline bool IsWhitespace(const char ch) { return (ch == ' ' || ch == 9 /*tab*/); }

inline void RemovePadding(string& line)
{
   const size_t LL = line.length();
   size_t Pos = line.find_first_not_of("\n\r\t ,");
   if (Pos == string::npos)
   {
      line.clear();
      return;
   }

   if (Pos > 0)
   {
      if ((SSIZE_T)(LL - Pos) < 1)
         return;
      line = line.substr(Pos, (LL - Pos));
   }

   Pos = line.find_last_not_of("\n\r\t ,");
   if (Pos != string::npos)
   {
      if (Pos < 1)
         return;
      line = line.erase(Pos + 1);
   }
}

inline string ParseRemoveVBSLineComments(string& line)
{
   const size_t commentIdx = line.find('\'');
   if (commentIdx == string::npos)
      return string();
   string RetVal = line.substr(commentIdx + 1);
   RemovePadding(RetVal);
   if (commentIdx > 0)
      line.resize(commentIdx);
   else
      line.clear();
   return RetVal;
}


CodeViewer::CodeViewer(PinTable* psh)
{
   m_table = psh;

   szFindString[0] = '\0';
   szReplaceString[0] = '\0';
   szCaretTextBuff[0] = '\0';

#ifndef __STANDALONE__
   m_findMsgString = RegisterWindowMessage(FINDMSGSTRING);
#endif

   m_findreplaceold.lStructSize = 0; // So we know nothing has been searched for yet
}

CodeViewer::~CodeViewer()
{
   Destroy();

   for (size_t i = 0; i < m_vcvd.size(); ++i)
      delete m_vcvd[i];

#ifndef __STANDALONE__
   if (m_haccel)
      DestroyAcceleratorTable(m_haccel);
#endif
}

// strSearchData has to be lower case
template<bool uniqueKey> // otherwise keyName
static int UDKeyIndexHelper(const fi_vector<UserData>& ListIn, const string& strSearchData, int& curPosOut)
{
	const int ListSize = (int)ListIn.size();
	curPosOut = 1u << 30;
	while (!(curPosOut & ListSize) && (curPosOut > 1))
		curPosOut >>= 1;
	int iJumpDelta = curPosOut >> 1;
	--curPosOut; // Zero Base
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
	const size_t SearchWidth = strSearchData.length();
	do
	{
		--Pos;
	} while (Pos >= 0 && ListIn[Pos].m_uniqueKey.compare(0, SearchWidth, strSearchData) == 0);
	++Pos;
	// now walk down list of Keynames looking for what we want.
	if (Pos >= (int)ListIn.size())
		return -1;
	int result;
	string lc_keyName = lowerCase(ListIn[Pos].m_keyName);
	do 
	{
		result = lc_keyName.compare(strSearchData); 
		if (result == 0) return 0; //Found
		++Pos;
		if (Pos == (int)ListIn.size()) return result;

		lc_keyName = lowerCase(ListIn[Pos].m_keyName);
		result = lc_keyName.compare(0, SearchWidth, strSearchData);
	} while (result == 0); //EO SubList

	return result;
}

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
         if (g_pvp && g_pvp->GetActiveTableEditor() && g_pvp->GetActiveTableEditor()->m_pcv->m_warn_on_dupes &&
			    (udIn.eTyping == eSub || udIn.eTyping == eFunction) && // only check subs and functions
			    (iterFound->m_lineNum != udIn.m_lineNum)) // use this simple check as dupe test: are the keys on different lines?
			{
#ifndef __STANDALONE__
            const Sci_Position dwellpos = SendMessage(g_pvp->GetActiveTableEditor()->m_pcv->m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
            SendMessage(g_pvp->GetActiveTableEditor()->m_pcv->m_hwndScintilla, SCI_CALLTIPSHOW, dwellpos,
				           (LPARAM)("Duplicate Definition found: " + iterFound->m_description + " (Line: " + std::to_string(iterFound->m_lineNum+1) + ")\n                            " + udIn.m_description + " (Line: " + std::to_string(udIn.m_lineNum+1) + ')').c_str());
#endif
            g_pvp->GetActiveTableEditor()->m_pcv->m_warn_on_dupes = false;
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
	const size_t SearchWidth = strSearchData.length();
	//Find the start of other instances of strIn by crawling up list
	int iNewPos = CurrentIdx;
	do
	{
		--iNewPos;
	} while (iNewPos >= 0 && ListIn[iNewPos].m_uniqueKey.compare(0, SearchWidth, strSearchData) == 0);
	++iNewPos;
	//Now at top of list
	//find nearest definition above current line
	//int ClosestLineNum = 0;
	int ClosestPos = CurrentIdx;
	int Delta = INT_MIN;
	string lc_keyName = lowerCase(ListIn[iNewPos].m_keyName);
	do
	{
		const int NewLineNum = ListIn[iNewPos].m_lineNum;
		const int NewDelta = NewLineNum - CurrentLine;
		if (NewDelta >= Delta && NewLineNum <= CurrentLine && lc_keyName == strSearchData)
		{
			Delta = NewDelta;
			//ClosestLineNum = NewLineNum;
			ClosestPos = iNewPos;
		}
		++iNewPos;
		if (iNewPos == (int)ListIn.size())
			break;
		lc_keyName = lowerCase(ListIn[iNewPos].m_keyName);
	} while (lc_keyName.compare(0, SearchWidth, strSearchData) == 0);
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
		result = (iCurPos >= ListSize) ? -1 : strSearchData.compare(lowerCase(ListIn[iCurPos]));
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
#ifndef __STANDALONE__
   Sci_TextRange tr;
   tr.chrg.cpMin = (Sci_PositionCR)start;
   tr.chrg.cpMax = (Sci_PositionCR)end;
   tr.lpstrText = text;
   SendMessage(hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
#endif
}

// buf must be >= MAX_FIND_LENGTH chars long
// returns length of word
size_t CodeViewer::GetWordUnderCaret(char *buf)
{
#ifndef __STANDALONE__
   const LRESULT CurPos = ::SendMessage(m_hwndScintilla, SCI_GETCURRENTPOS, 0, 0 );
   m_wordUnderCaret.chrg.cpMin = (Sci_PositionCR)::SendMessage(m_hwndScintilla, SCI_WORDSTARTPOSITION, CurPos, TRUE);
   m_wordUnderCaret.chrg.cpMax = (Sci_PositionCR)::SendMessage(m_hwndScintilla, SCI_WORDENDPOSITION, CurPos, TRUE);
   m_wordUnderCaret.lpstrText = buf;
   if ((m_wordUnderCaret.chrg.cpMax - m_wordUnderCaret.chrg.cpMin) < MAX_FIND_LENGTH)
   {
      ::SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&m_wordUnderCaret);
      return m_wordUnderCaret.chrg.cpMax - m_wordUnderCaret.chrg.cpMin;
   }
   else
   {
      buf[0] = '\0';
      return 0;
   }
#else
   return 0;
#endif
}

void CodeViewer::SetClean(const SaveDirtyState sds)
{
#ifndef __STANDALONE__
   if (sds == eSaveClean)
      ::SendMessage(m_hwndScintilla, SCI_SETSAVEPOINT, 0, 0);
   m_table->m_sdsDirtyScript = sds;
#endif
}

void CodeViewer::OnScriptError(ScriptInterpreter::ErrorType type, int line, int column, const string& description, const vector<string>& stackDump)
{
   if (g_pplayer && g_pplayer->m_liveUI && !m_suppressErrorDialogs)
   {
      g_pplayer->m_liveUI->PushNotification("Script error: " + string_from_utf8_or_iso8859_1(description.data(), description.size()), 5000);
   }

#ifndef __STANDALONE__
   // Show the error in the last error log
   AppendLastErrorTextW(MakeWString(description));
   SetLastErrorVisibility(true);

   if (g_pvp && IsWindow() && !m_suppressErrorDialogs && type != ScriptInterpreter::ErrorType::DebugConsole)
   {
      if (g_pplayer)
      {
         g_pplayer->LockForegroundWindow(false);
         g_pplayer->ShowMouseCursor(true);
      }

      SetVisible(true);
      ShowWindow(SW_RESTORE);
      ColorError(line, column);
      g_pvp->EnableWindow(TRUE);
      ::SetFocus(m_hwndScintilla);

      if (g_pplayer == nullptr || g_pplayer->GetCloseState() != Player::CloseState::CS_CLOSE_APP)
      {
         std::wstringstream errorStream;
         errorStream << (type == ScriptInterpreter::ErrorType::Compile ? L"Compile error\r\n" : L"Runtime error\r\n");
         errorStream << L"-------------\r\n";
         errorStream << L"Line: " << line << " Column: " << column << "\r\n";
         errorStream << MakeWString(description) << "\r\n";
         if (!stackDump.empty())
         {
            errorStream << L"\r\nStack trace (Most recent call first):\r\n";
            for (const string& callSite : stackDump)
               errorStream << L"    " << MakeWString(callSite) << "\r\n";
         }
         errorStream << L"\r\n";
         g_pvp->EnableWindow(FALSE);
         ScriptErrorDialog scriptErrorDialog(errorStream.str());
         scriptErrorDialog.DoModal();
         m_suppressErrorDialogs = scriptErrorDialog.WasSuppressErrorsRequested();
         g_pvp->EnableWindow(TRUE);

         if (const auto pt = g_pvp->GetActiveTableEditor(); pt != nullptr)
            ::SetFocus(pt->m_table->m_tableEditor->m_pcv->m_hwndScintilla);
      }
   }
#endif
}

HRESULT CodeViewer::AddItem(IScriptable * const piscript, const bool global)
{
   CodeViewDispatch * const pcvd = new CodeViewDispatch();

   pcvd->m_wName = piscript->get_Name();
   pcvd->m_pdisp = piscript->GetDispatch();
   pcvd->m_pdisp->QueryInterface(IID_IUnknown, (void **)&pcvd->m_punk);
   pcvd->m_punk->Release();
   pcvd->m_global = global;

   if (m_vcvd.GetSortedIndex(pcvd->m_wName) != -1)
   {
      delete pcvd;
      return E_FAIL;
   }

   m_vcvd.AddSortedString(pcvd);

   // Add item to dropdown
   char * const szT = MakeChar(pcvd->m_wName.c_str());

#ifdef __STANDALONE__
   ITypeInfo* ti;
   if (SUCCEEDED(piscript->GetDispatch()->GetTypeInfo(NULL, NULL, &ti))) {
      BSTR bstrTypeName;
      if (SUCCEEDED(ti->GetDocumentation(MEMBERID_NIL, &bstrTypeName, NULL, NULL, NULL))) {
         PLOGD << "type=" << MakeString(bstrTypeName) << ", name=" << szT;
         SysFreeString(bstrTypeName);
      }
      ti->Release();
   }
#else
   const size_t index = ::SendMessage(m_hwndItemList, CB_ADDSTRING, 0, (size_t)szT);
   ::SendMessage(m_hwndItemList, CB_SETITEMDATA, index, (size_t)piscript);
#endif
   delete [] szT;
   //AndyS - WIP insert new item into autocomplete list??
   return S_OK;
}

void CodeViewer::RemoveItem(IScriptable * const piscript)
{
   const WCHAR* name = piscript->get_Name();

   const int idx = m_vcvd.GetSortedIndex(name);

   if (idx == -1)
      return;

   const CodeViewDispatch * const pcvd = m_vcvd[idx];

   _ASSERTE(pcvd);

   m_vcvd.RemoveElementAt(idx);

   // Remove item from dropdown
#ifndef __STANDALONE__
   char * const szT = MakeChar(name);
   const size_t index = ::SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   ::SendMessage(m_hwndItemList, CB_DELETESTRING, index, 0);
   delete [] szT;
#endif

   delete pcvd;
}

void CodeViewer::SelectItem(IScriptable * const piscript)
{
#ifndef __STANDALONE__
   char * const szT = MakeChar(piscript->get_Name());

   const LRESULT index = ::SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   if (index != CB_ERR)
   {
      ::SendMessage(m_hwndItemList, CB_SETCURSEL, index, 0);

      ListEventsFromItem();
   }

   delete [] szT;
#endif
}

HRESULT CodeViewer::ReplaceName(IScriptable *const piscript, const wstring &wzNew)
{
   if (m_vcvd.GetSortedIndex(wzNew) != -1)
      return E_FAIL;

   const WCHAR* name = piscript->get_Name();

   const int idx = m_vcvd.GetSortedIndex(name);
   if (idx == -1)
      return E_FAIL;

   CodeViewDispatch * const pcvd = m_vcvd[idx]; // keep pointer to old element

   _ASSERTE(pcvd);

   m_vcvd.RemoveElementAt(idx); // erase in vector

   pcvd->m_wName = wzNew;

   m_vcvd.AddSortedString(pcvd); // and re-add

   // Remove old name from dropdown and replace it with the new
#ifndef __STANDALONE__
   char * szT = MakeChar(name);
   size_t index = ::SendMessage(m_hwndItemList, CB_FINDSTRINGEXACT, ~0u, (size_t)szT);
   ::SendMessage(m_hwndItemList, CB_DELETESTRING, index, 0);
   delete [] szT;

   szT = MakeChar(wzNew.c_str());
   index = ::SendMessage(m_hwndItemList, CB_ADDSTRING, 0, (size_t)szT);
   ::SendMessage(m_hwndItemList, CB_SETITEMDATA, index, (size_t)piscript);

   ::SendMessage(m_hwndItemList, CB_SETCURSEL, index, 0);
   ListEventsFromItem(); // Just to get us into a good state
   delete [] szT;
#endif

   return S_OK;
}

void CodeViewer::SetVisible(const bool visible)
{
#ifndef __STANDALONE__
   if (!visible && !m_minimized)
   {
      const CRect rc = GetWindowRect();
      g_app->m_settings.SetEditor_CodeViewPosX((int)rc.left, false);
      g_app->m_settings.SetEditor_CodeViewPosY((int)rc.top, false);
      g_app->m_settings.SetEditor_CodeViewPosWidth(rc.right - rc.left, false);
      g_app->m_settings.SetEditor_CodeViewPosHeight(rc.bottom - rc.top, false);
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
         const int x = g_app->m_settings.GetEditor_CodeViewPosX();
         const int y = g_app->m_settings.GetEditor_CodeViewPosY();
         const int w = g_app->m_settings.GetEditor_CodeViewPosWidth();
         const int h = g_app->m_settings.GetEditor_CodeViewPosHeight();
         const POINT p { x, y };
         if (MonitorFromPoint(p, MONITOR_DEFAULTTONULL) != NULL) // Do not apply if point is offscreen
            SetWindowPos(HWND_TOP, x, y, w, h, SWP_NOMOVE | SWP_NOSIZE);
      }
      SetForegroundWindow();
   }
   m_visible = visible;
#endif
}

void CodeViewer::SetEnabled(const bool enabled)
{
#ifndef __STANDALONE__
   ::SendMessage(m_hwndScintilla, SCI_SETREADONLY, !enabled, 0);

   ::EnableWindow(m_hwndItemList, enabled);
   ::EnableWindow(m_hwndEventList, enabled);
#endif
}

void CodeViewer::SetCaption(const string& szCaption)
{
#ifndef __STANDALONE__
   string szT;
   if (!m_table->m_external_script_name.empty())
      szT = "MODIFYING EXTERNAL SCRIPT: " + m_table->m_external_script_name.string();
   else
      szT = szCaption + ' ' + LocalString(IDS_SCRIPT).m_szbuffer;
   SetWindowText(szT.c_str());
#endif
}

void CodeViewer::UpdatePrefsfromReg()
{
   m_bgColor = g_app->m_settings.GetCVEdit_BackGroundColor();
   m_bgSelColor = g_app->m_settings.GetCVEdit_BackGroundSelectionColor();
   m_displayAutoComplete = g_app->m_settings.GetCVEdit_DisplayAutoComplete();
   m_displayAutoCompleteLength = g_app->m_settings.GetCVEdit_DisplayAutoCompleteAfter();
   m_dwellDisplay = g_app->m_settings.GetCVEdit_DwellDisplay();
   m_dwellHelp = g_app->m_settings.GetCVEdit_DwellHelp();
   m_dwellDisplayTime = g_app->m_settings.GetCVEdit_DwellDisplayTime();
   for (size_t i = 0; i < m_lPrefsList->size(); ++i)
      m_lPrefsList->at(i)->GetPrefsFromReg();
}

void CodeViewer::UpdateRegWithPrefs()
{
   g_app->m_settings.SetCVEdit_BackGroundColor((int)m_bgColor, false);
   g_app->m_settings.SetCVEdit_BackGroundSelectionColor((int)m_bgSelColor, false);
   g_app->m_settings.SetCVEdit_DisplayAutoComplete(m_displayAutoComplete, false);
   g_app->m_settings.SetCVEdit_DisplayAutoCompleteAfter(m_displayAutoCompleteLength, false);
   g_app->m_settings.SetCVEdit_DwellDisplay(m_dwellDisplay, false);
   g_app->m_settings.SetCVEdit_DwellHelp(m_dwellHelp, false);
   g_app->m_settings.SetCVEdit_DwellDisplayTime(m_dwellDisplayTime, false);
   for (size_t i = 0; i < m_lPrefsList->size(); i++)
      m_lPrefsList->at(i)->SetPrefsToReg();
}

void CodeViewer::InitPreferences()
{
	memset(m_prefCols, 0, sizeof(m_prefCols));

	m_bgColor = RGB(255,255,255);
	m_bgSelColor = RGB(192,192,192);
	m_lPrefsList = new vector<CVPreference*>();

#ifndef __STANDALONE__
	m_prefEverythingElse = new CVPreference(RGB(0,0,0), true, "EverythingElse"s,  STYLE_DEFAULT, 0 , IDC_CVP_BUT_COL_EVERYTHINGELSE, IDC_CVP_BUT_FONT_EVERYTHINGELSE);
	m_lPrefsList->push_back(m_prefEverythingElse);
	prefDefault = new CVPreference(RGB(0,0,0), true, "Default"s, SCE_B_DEFAULT, 0 , 0, 0);
	m_lPrefsList->push_back(prefDefault);
	prefVBS = new CVPreference(RGB(0,0,160), true, "ShowVBS"s, SCE_B_KEYWORD, IDC_CVP_CHECKBOX_VBS, IDC_CVP_BUT_COL_VBS, IDC_CVP_BUT_FONT_VBS);
	m_lPrefsList->push_back(prefVBS);
	prefComps = new CVPreference(RGB(120,120,0), true, "ShowComponents"s, SCE_B_KEYWORD3, IDC_CVP_CHKB_COMP, IDC_CVP_BUT_COL_COMPS, IDC_CVP_BUT_FONT_COMPS);
	m_lPrefsList->push_back(prefComps);
	prefSubs = new CVPreference(RGB(120,0,120), true, "ShowSubs"s, SCE_B_KEYWORD2, IDC_CVP_CHKB_SUBS, IDC_CVP_BUT_COL_SUBS, IDC_CVP_BUT_FONT_SUBS);
	m_lPrefsList->push_back(prefSubs);
	prefComments = new CVPreference(RGB(0,120,0), true, "ShowRemarks"s, SCE_B_COMMENT, IDC_CVP_CHKB_COMMENTS, IDC_CVP_BUT_COL_COMMENTS, IDC_CVP_BUT_FONT_COMMENTS);
	m_lPrefsList->push_back(prefComments);
	prefLiterals = new CVPreference(RGB(0,120,160), true, "ShowLiterals"s, SCE_B_STRING, IDC_CVP_CHKB_LITERALS, IDC_CVP_BUT_COL_LITERALS, IDC_CVP_BUT_FONT_LITERALS);
	m_lPrefsList->push_back(prefLiterals);
	prefVPcore = new CVPreference(RGB(200,50,60), true, "ShowVPcore"s, SCE_B_KEYWORD4, IDC_CVP_CHKB_VPCORE, IDC_CVP_BUT_COL_VPCORE, IDC_CVP_BUT_FONT_VPCORE);
	m_lPrefsList->push_back(prefVPcore);
#endif

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
#ifndef __STANDALONE__
   m_haccel = LoadAccelerators(g_app->GetInstanceHandle(), MAKEINTRESOURCE(IDR_CODEVIEWACCEL)); // Accelerator keys

   m_hwndMain = GetHwnd();
   SetWindowLongPtr(GWLP_USERDATA, (size_t)this);

   /////////////////// Item / Event Lists //!! ALL THIS STUFF IS NOT RES/DPI INDEPENDENT! also see WM_SIZE handler

   m_hwndItemText = CreateWindowEx(0, "Static", "ObjectsText",
      WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 5, 0, 330, 30, m_hwndMain, nullptr, g_app->GetInstanceHandle(), 0);
   ::SetWindowText(m_hwndItemText, "Table component:");
   ::SendMessage(m_hwndItemText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndItemList = CreateWindowEx(0, "ComboBox", "Objects",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL,
      5, 30+2, 330, 400, m_hwndMain, nullptr, g_app->GetInstanceHandle(), 0);
   ::SetWindowLongPtr(m_hwndItemList, GWL_ID, IDC_ITEMLIST);
   ::SendMessage(m_hwndItemList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndEventText = CreateWindowEx(0, "Static", "EventsText",
      WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 360 + 5, 0, 330, 30, m_hwndMain, nullptr, g_app->GetInstanceHandle(), 0);
   ::SetWindowText(m_hwndEventText, "Create Sub from component:");
   ::SendMessage(m_hwndEventText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndEventList = CreateWindowEx(0, "ComboBox", "Events",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL,
      360 + 5, 30+2, 330, 400, m_hwndMain, nullptr, g_app->GetInstanceHandle(), 0);
   ::SetWindowLongPtr(m_hwndEventList, GWL_ID, IDC_EVENTLIST);
   ::SendMessage(m_hwndEventList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndFunctionText = CreateWindowEx(0, "Static", "FunctionsText",
      WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 730 + 5, 0, 330, 30, m_hwndMain, nullptr, g_app->GetInstanceHandle(), 0);
   ::SetWindowText(m_hwndFunctionText, "Go to Sub/Function:");
   ::SendMessage(m_hwndFunctionText, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   m_hwndFunctionList = CreateWindowEx(0, "ComboBox", "Functions",
      WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
      730 + 5, 30+2, 330, 400, m_hwndMain, nullptr, g_app->GetInstanceHandle(), 0);
   ::SetWindowLongPtr(m_hwndFunctionList, GWL_ID, IDC_FUNCTIONLIST);
   ::SendMessage(m_hwndFunctionList, WM_SETFONT, (size_t)GetStockObject(DEFAULT_GUI_FONT), 0);

   //////////////////////// Status Window (& Sizing Box)

   m_hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", m_hwndMain, 1);

   static constexpr int foo[4] = { 220, 420, 450, 500 };
   ::SendMessage(m_hwndStatus, SB_SETPARTS, 4, (size_t)foo);

   //////////////////////// Last error widget

   m_hwndLastErrorTextArea = CreateWindowEx(0, "Edit", "",
      WS_CHILD | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE,
      0, 0, 0, 0, m_hwndMain, nullptr, g_app->GetInstanceHandle(), 0);
   ::SendMessage(m_hwndLastErrorTextArea, EM_SETREADONLY, TRUE, 0);
   ::SendMessage(m_hwndLastErrorTextArea, WM_SETFONT, (size_t)GetStockObject(ANSI_FIXED_FONT), 0);

   //////////////////////// Scintilla text editor

   m_hwndScintilla = CreateWindowEx(0, "Scintilla", "",
      WS_CHILD | ES_NOHIDESEL | WS_VISIBLE | ES_SUNKEN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN,
      0, 30+2 +40, 0, 0, m_hwndMain, nullptr, g_app->GetInstanceHandle(), 0);

	//if still using old dll load VB lexer instead
	//use SCI_SETLEXERLANGUAGE as SCI_GETLEXER doesn't return the correct value with SCI_SETLEXER
	::SendMessage(m_hwndScintilla, SCI_SETLEXERLANGUAGE, 0, (LPARAM)"vpscript");
	const LRESULT lexVersion = ::SendMessage(m_hwndScintilla, SCI_GETLEXER, 0, 0);
	if (lexVersion != SCLEX_VPSCRIPT)
		::SendMessage(m_hwndScintilla, SCI_SETLEXER, (WPARAM)SCLEX_VBSCRIPT, 0);

	m_validChars.resize(::SendMessage(m_hwndScintilla, SCI_GETWORDCHARS, 0, (LPARAM)0), '\0');
	::SendMessage(m_hwndScintilla, SCI_GETWORDCHARS, 0, (LPARAM)m_validChars.data());
	m_stopErrorDisplay = false;

	// Create new list of user functions & Collections- filled in ParseForFunction(), first called in LoadFromStream()
	m_wordUnderCaret.lpstrText = szCaretTextBuff;

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
			VBWord.m_keyName[0] = cUpper(VBWord.m_keyName[0]);
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

   //::SendMessage(m_hwndScintilla, SCI_SETMOUSEDOWNCAPTURES ,0,0); //send mouse events through scintilla.
   ::SendMessage(m_hwndScintilla, SCI_AUTOCSETIGNORECASE, TRUE, 0);
   ::SendMessage(m_hwndScintilla, SCI_AUTOCSETCASEINSENSITIVEBEHAVIOUR, SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE,0);

   ::SendMessage(m_hwndScintilla, SCI_AUTOCSETFILLUPS, 0,(LPARAM) "[]{}()");
   ::SendMessage(m_hwndScintilla, SCI_AUTOCSTOPS, 0,(LPARAM) " ");

   //
#endif

   ParseVPCore();
#ifndef __STANDALONE__
   UpdateScinFromPrefs();

   SendMessage(WM_SIZE, 0, 0); // Make our window relay itself out
#endif
   return CWnd::OnCreate(cs);
}

void CodeViewer::Destroy()
{
#ifndef __STANDALONE__
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
#endif
}

void CodeViewer::SetScript(const string& script)
{
#ifndef __STANDALONE__
   if (m_hwndScintilla)
      ::SendMessage(m_hwndScintilla, SCI_SETTEXT, 0, (size_t)script.c_str());
   // Allow updates to take, now that we know the script size
   UpdateScinFromPrefs();
#endif
}

BOOL CodeViewer::PreTranslateMessage(MSG &msg)
{
#ifndef __STANDALONE__
   if (!IsWindow())
      return FALSE;
   
   if (m_hwndFind && ::IsDialogMessage(m_hwndFind, &msg))
      return TRUE;

   if (IsDialogMessage(msg))
      return TRUE;

#endif
   return FALSE;
}

void CodeViewer::Compile(const bool message)
{
   CComObject<ScriptInterpreter>* interpreter;
   CComObject<ScriptInterpreter>::CreateInstance(&interpreter);
   interpreter->AddRef();
   interpreter->Init(m_table);
   interpreter->SetScriptErrorHandler([this](ScriptInterpreter::ErrorType type, int line, int column, const string& description, const vector<string>& stackDump)
      { OnScriptError(type, line, column, description, stackDump); });
   interpreter->Evaluate(m_table->m_script_text, false);
   if (message && !interpreter->HasError())
      MessageBox("Compilation successful", "Compile", MB_OK);
   interpreter->Stop();
   interpreter->Release();
}

void CodeViewer::AddToDebugOutput(const string &szText)
{
#ifndef __STANDALONE__
   ::SendMessage(g_pplayer->m_hwndDebugOutput, SCI_ADDTEXT, szText.length(), (LPARAM)szText.c_str());
   ::SendMessage(g_pplayer->m_hwndDebugOutput, SCI_ADDTEXT, 1, (LPARAM)"\n");

   const size_t pos = ::SendMessage(g_pplayer->m_hwndDebugOutput, SCI_GETCURRENTPOS, 0, 0);
   const size_t line = ::SendMessage(g_pplayer->m_hwndDebugOutput, SCI_LINEFROMPOSITION, pos, 0);
   ::SendMessage(g_pplayer->m_hwndDebugOutput, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
#endif
}

void CodeViewer::ShowFindDialog()
{
#ifndef __STANDALONE__
   if (m_hwndFind == nullptr)
   {
      GetWordUnderCaret(szFindString);

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
#endif
}

void CodeViewer::ShowFindReplaceDialog()
{
#ifndef __STANDALONE__
   if (m_hwndFind == nullptr)
   {
      GetWordUnderCaret(szFindString);

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
#endif
}

void CodeViewer::Find(const FINDREPLACE * const pfr)
{
#ifndef __STANDALONE__
   if (pfr->lStructSize == 0) // Our built-in signal that we are doing 'find next' and nothing has been searched for yet
      return;

   m_findreplaceold = *pfr;

   const size_t selstart = ::SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   const size_t selend = ::SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);

   size_t startChar, stopChar;

   if (pfr->Flags & FR_DOWN)
   {
      const size_t len = ::SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
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

   ::SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0);
   ::SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);
   ::SendMessage(m_hwndScintilla, SCI_SETSEARCHFLAGS, scinfindflags, 0);
   LRESULT posFind = ::SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, strlen(pfr->lpstrFindWhat), (LPARAM)pfr->lpstrFindWhat);

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
         const size_t len = ::SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
         startChar = len;
         stopChar = selend;
      }

      ::SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0);
      ::SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);

      posFind = ::SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, strlen(pfr->lpstrFindWhat), (LPARAM)pfr->lpstrFindWhat);
   }

   if (posFind != -1)
   {
      const size_t start = ::SendMessage(m_hwndScintilla, SCI_GETTARGETSTART, 0, 0);
      const size_t end = ::SendMessage(m_hwndScintilla, SCI_GETTARGETEND, 0, 0);
      const size_t lineStart = ::SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, min(start, end), 0);
      const size_t lineEnd = ::SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, max(start, end), 0);
      for (size_t line = lineStart; line <= lineEnd; ++line)
         ::SendMessage(m_hwndScintilla, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
      ::SendMessage(m_hwndScintilla, SCI_SETSEL, start, end);

      if (!wrapped)
         ::SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)"");
      else
         ::SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)LocalString(IDS_FINDLOOPED).m_szbuffer);
   }
   else
   {
      MessageBeep(MB_ICONEXCLAMATION);
      ::SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)(string(LocalString(IDS_FINDFAILED).m_szbuffer) + pfr->lpstrFindWhat + LocalString(IDS_FINDFAILED2).m_szbuffer).c_str());
   }
#endif
}

void CodeViewer::Replace(const FINDREPLACE * const pfr)
{
#ifndef __STANDALONE__
   const size_t selstart = ::SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
   const size_t selend = ::SendMessage(m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);

   const size_t len = ::SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);

   FINDTEXTEX ft;
   ft.chrg.cpMax = (LONG)len;			// search through end of the text
   ft.chrg.cpMin = (LONG)selstart;
   if (!(pfr->Flags & (FR_REPLACE | FR_REPLACEALL)))
      ft.chrg.cpMin = (LONG)selend;
   ft.lpstrText = pfr->lpstrFindWhat;

   LONG cszReplaced = 0;
   bool replace = true;
   while (replace)
   {
      replace = false;
      const size_t cpMatch = ::SendMessage(m_hwndScintilla, SCI_FINDTEXT, (WPARAM)(pfr->Flags), (LPARAM)&ft);
      if ((SSIZE_T)cpMatch < 0)
      {
         MessageBeep(MB_ICONEXCLAMATION);
         if (cszReplaced == 0)
            ::SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)(string(LocalString(IDS_FINDFAILED).m_szbuffer) + ft.lpstrText + LocalString(IDS_FINDFAILED2).m_szbuffer).c_str());
         else
            ::SendMessage(m_hwndStatus, SB_SETTEXT, 1 | 0, (size_t)(       LocalString(IDS_REPLACEALL).m_szbuffer + (' ' + std::to_string(cszReplaced)) + ' ' + LocalString(IDS_REPLACEALL2).m_szbuffer).c_str());
      }
      else
      {
         ft.chrg.cpMin = ft.chrgText.cpMin;
         ft.chrg.cpMax = ft.chrgText.cpMax;
         ::SendMessage(m_hwndScintilla, SCI_SETSEL, ft.chrgText.cpMin, ft.chrgText.cpMax);
         if (((pfr->Flags & FR_REPLACE) && cszReplaced == 0) || (pfr->Flags & FR_REPLACEALL))
         {
            ::SendMessage(m_hwndScintilla, SCI_REPLACESEL, true, (LPARAM)pfr->lpstrReplaceWith);
            ft.chrg.cpMin = (LONG)(cpMatch + strlen(pfr->lpstrReplaceWith));
            ft.chrg.cpMax = (LONG)len;	// search through end of the text
            cszReplaced++;
            replace = true;
         }
      }
   }
#endif
}

void CodeViewer::SaveToStream(IStream *pistream, const HCRYPTHASH hcrypthash)
{
#ifndef __STANDALONE__
   size_t cchar = ::SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   char * szText = new char[cchar + 1];
   ::SendMessage(m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)szText);

   // if there was an external vbs loaded, save the script to that file
   // and ask if to save the original script also to the table
   bool save_external_script_to_table = true;
   if (!m_table->m_external_script_name.empty())
   {
      FILE* fScript;
      if ((fopen_s(&fScript, m_table->m_external_script_name.string().c_str(), "wb") == 0) && fScript)
      {
         fwrite(szText, 1, cchar, fScript);
         fclose(fScript);
      }

      save_external_script_to_table = (MessageBox("Save externally loaded .vbs script also to .vpx table?", "Visual Pinball", MB_YESNO | MB_DEFBUTTON2) == IDYES);

      if (!save_external_script_to_table)
      {
         delete[] szText;
         szText = m_table->m_original_table_script.data();
         cchar = m_table->m_original_table_script.size();
      }
   }

   ULONG writ = 0;
   pistream->Write(&cchar, (ULONG)sizeof(int), &writ);
   pistream->Write(szText, (ULONG)(cchar*sizeof(char)), &writ);

   CryptHashData(hcrypthash, (BYTE *)szText, (DWORD)cchar, 0);

   if (save_external_script_to_table)
      delete[] szText;
#endif
}

void CodeViewer::ColorLine(const int line)
{
   //!!
}

void CodeViewer::UncolorError()
{
#ifndef __STANDALONE__
   const size_t startChar = ::SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, m_errorLineNumber, 0);
   const size_t length = ::SendMessage(m_hwndScintilla, SCI_LINELENGTH, m_errorLineNumber, 0);

   ::SendMessage(m_hwndScintilla, SCI_INDICATORCLEARRANGE, startChar, length);

   m_errorLineNumber = -1;
#endif
}

void CodeViewer::ColorError(const int line, const int nchar)
{
   m_errorLineNumber = line - 1;

#ifndef __STANDALONE__
   const size_t startChar = ::SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line - 1, 0);
   const size_t length = ::SendMessage(m_hwndScintilla, SCI_LINELENGTH, line - 1, 0);

   ::SendMessage(m_hwndScintilla, SCI_INDICATORFILLRANGE, startChar, length);
   ::SendMessage(m_hwndScintilla, SCI_GOTOLINE, line, 0);
#endif
}

void CodeViewer::TellHostToSelectItem()
{
#ifndef __STANDALONE__
   const size_t index = ::SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   IScriptable * const pscript = (IScriptable *)::SendMessage(m_hwndItemList, CB_GETITEMDATA, index, 0);

   m_table->SelectItem(pscript);
#endif
}

string CodeViewer::GetParamsFromEvent(const UINT iEvent)
{
#ifndef __STANDALONE__
   const size_t index = ::SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   IScriptable * const pscript = (IScriptable *)::SendMessage(m_hwndItemList, CB_GETITEMDATA, index, 0);
   IDispatch * const pdisp = pscript->GetDispatch();
   IProvideClassInfo* pClassInfo;
   pdisp->QueryInterface(IID_IProvideClassInfo, (void **)&pClassInfo);

   if (pClassInfo)
   {
      ITypeInfo *pti;
      pClassInfo->GetClassInfo(&pti);

      TYPEATTR *pta;
      pti->GetTypeAttr(&pta);

      string szParams;
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
               BSTR rgstr[6];
               unsigned int cnames;
               /*const HRESULT hr =*/ ptiChild->GetNames(pfd->memid, rgstr, std::size(rgstr), &cnames);

               SysFreeString(rgstr[0]);
               // Add enum string to combo control
               for (unsigned int l = 1; l < cnames; ++l)
               {
                  if (l > 1)
                     szParams += ", ";
                  szParams += MakeString(rgstr[l]);
                  SysFreeString(rgstr[l]);
               }
            }

            ptiChild->ReleaseFuncDesc(pfd);
         }

         ptiChild->ReleaseTypeAttr(ptaChild);

         ptiChild->Release();
      }

      pti->ReleaseTypeAttr(pta);

      pti->Release();
      pClassInfo->Release();
      return szParams;
   }
#endif
   return string();
}

void CodeViewer::ListEventsFromItem()
{
#ifndef __STANDALONE__
   ::SendMessage(m_hwndEventList, CB_RESETCONTENT, 0, 0);
   const size_t index = ::SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   IScriptable * const pscript = (IScriptable *)::SendMessage(m_hwndItemList, CB_GETITEMDATA, index, 0);
   for (auto event : pscript->GetEventNames())
   {
      string method = MakeString(event);
      const size_t listindex = ::SendMessage(m_hwndEventList, CB_ADDSTRING, 0, (size_t)method.c_str());
   }
#endif
}

void CodeViewer::FindCodeFromEvent()
{
#ifndef __STANDALONE__
   bool found = false;

   char szItemName[512]; // Can't be longer than 32 chars, but use this buffer for concatenating
   szItemName[0] = '\0';
   char szEventName[512];
   szEventName[0] = '\0';
   size_t index = ::SendMessage(m_hwndItemList, CB_GETCURSEL, 0, 0);
   ::SendMessage(m_hwndItemList, CB_GETLBTEXT, index, (size_t)szItemName);
   index = ::SendMessage(m_hwndEventList, CB_GETCURSEL, 0, 0);
   ::SendMessage(m_hwndEventList, CB_GETLBTEXT, index, (size_t)szEventName);
   const size_t iEventIndex = ::SendMessage(m_hwndEventList, CB_GETITEMDATA, index, 0);
   strcat_s(szItemName, "_"); // VB Specific event names
   strcat_s(szItemName, szEventName);
   size_t codelen = ::SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   const size_t stopChar = codelen;
   ::SendMessage(m_hwndScintilla, SCI_TARGETWHOLEDOCUMENT, 0, 0);
   ::SendMessage(m_hwndScintilla, SCI_SETSEARCHFLAGS, SCFIND_WHOLEWORD, 0);
   LRESULT posFind;
   while ((posFind = ::SendMessage(m_hwndScintilla, SCI_SEARCHINTARGET, strlen(szItemName), (LPARAM)szItemName)) != -1)
   {
      const size_t line = ::SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, posFind, 0);
      // Check for 'sub' and make sure we're not in a comment
      const size_t beginchar = ::SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line, 0);
      bool goodMatch = true;

      const size_t lineLength = ::SendMessage(m_hwndScintilla, SCI_LINELENGTH, line, 0);
      string szLine(lineLength, '\0'); // SCI_GETLINE does not null-terminate the string
      SOURCE_TEXT_ATTR *const wzFormat = new SOURCE_TEXT_ATTR[lineLength];

      const size_t cchar = ::SendMessage(m_hwndScintilla, SCI_GETLINE, line, (LPARAM)szLine.data());
      const wstring wzText = MakeWString(szLine);
      assert(cchar == lineLength);
      assert(cchar == szLine.length());
      assert(cchar == wzText.length()); // otherwise may need to pass wzText.length() to next function????
      
      // FIXME FIXME FIXME m_pScriptDebug->GetScriptTextAttributes(wzText.c_str(), (ULONG)cchar, nullptr, 0, wzFormat);

      const size_t inamechar = posFind - beginchar - 1;

      int i;
      for (i = (int)inamechar; i >= 0; i--)
      {
         if (wzFormat[i] == SOURCETEXT_ATTR_KEYWORD)
            break;

         if (!IsWhitespace(szLine[i]) /*&& (wzFormat[i] != 0 || wzFormat[i] != SOURCETEXT_ATTR_COMMENT)*/) //!!?
            goodMatch = false;
      }

      delete[] wzFormat;

      if (i < 2) // Can't fit the word 'sub' in here
      {
         goodMatch = false;
      }
      else
      {
         // check if not "sub"
         if ((cLower(szLine[i - 2]) != 's') || (cLower(szLine[i - 1]) != 'u') || (cLower(szLine[i]) != 'b')) //!! correct like this?
            goodMatch = false;
      }

      if (goodMatch)
      {
         // We found a real sub heading - move the cursor inside of it

         found = true;

         LRESULT ichar = ::SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line + 1, 0);
         if (ichar == -1)
         {
            // The function was declared as the last line of the script - rare but possible
            ichar = ::SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, line, 0);
         }

         const size_t lineEvent = ::SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, ichar, 0);
         ::SendMessage(m_hwndScintilla, SCI_ENSUREVISIBLEENFORCEPOLICY, lineEvent, 0);
         ::SendMessage(m_hwndScintilla, SCI_SETSEL, ichar, ichar);
      }

      if (found)
         break;

      const size_t startChar = posFind + 1;
      ::SendMessage(m_hwndScintilla, SCI_SETTARGETSTART, startChar, 0);
      ::SendMessage(m_hwndScintilla, SCI_SETTARGETEND, stopChar, 0);
   }

   if (!found)
   {
      char szEnd[2] = {};
      TEXTRANGE tr;
      tr.chrg.cpMax = (LONG)codelen;
      tr.chrg.cpMin = (LONG)codelen - 1;
      tr.lpstrText = szEnd;

      // Make sure there is at least a one space gap between the last function and this new one
      ::SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (size_t)&tr);

      if (szEnd[0] != '\n')
      {
         ::SendMessage(m_hwndScintilla, SCI_SETSEL, codelen, codelen);
         ::SendMessage(m_hwndScintilla, SCI_REPLACESEL, TRUE, (size_t)"\n");
         codelen++;
      }

      if (szEnd[1] != '\n')
      {
         ::SendMessage(m_hwndScintilla, SCI_SETSEL, codelen, codelen);
         ::SendMessage(m_hwndScintilla, SCI_REPLACESEL, TRUE, (size_t)"\n");
         codelen++;
      }

      // Add the new function at the end
      ::SendMessage(m_hwndScintilla, SCI_SETSEL, codelen, codelen);

      string szNewCode = "Sub "s + szItemName + '(' + GetParamsFromEvent((UINT)iEventIndex) + ")\n\t";
      const size_t subtitlelen = szNewCode.length();
      szNewCode += "\nEnd Sub";

      ::SendMessage(m_hwndScintilla, SCI_REPLACESEL, TRUE, (size_t)szNewCode.c_str());

      ::SendMessage(m_hwndScintilla, SCI_SETSEL, codelen + subtitlelen, codelen + subtitlelen);
   }

   ::SetFocus(m_hwndScintilla);
#endif
}

void CodeViewer::ShowAutoComplete(const SCNotification *pSCN)
{
	if (!pSCN) return;

#ifndef __STANDALONE__
	const char KeyPressed = pSCN->ch;
	if (KeyPressed != '.')
	{
		const size_t intWordLen = GetWordUnderCaret(szCaretTextBuff);
		if ((int)intWordLen > m_displayAutoCompleteLength)
			::SendMessage(m_hwndScintilla, SCI_AUTOCSHOW, intWordLen, (LPARAM)m_autoCompString.c_str());
	}
	else
	{
		//Get member construct
		const LRESULT constructPos = ::SendMessage(m_hwndScintilla, SCI_GETCURRENTPOS, 0, 0 ) - 2;
		Sci_TextRange currentConstruct;
		currentConstruct.chrg.cpMin = (Sci_PositionCR)::SendMessage(m_hwndScintilla, SCI_WORDSTARTPOSITION, constructPos, TRUE);
		currentConstruct.chrg.cpMax = (Sci_PositionCR)::SendMessage(m_hwndScintilla, SCI_WORDENDPOSITION, constructPos, TRUE);
		string tmp(currentConstruct.chrg.cpMax - currentConstruct.chrg.cpMin, '\0');
		currentConstruct.lpstrText = tmp.data();
		::SendMessage(m_hwndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&currentConstruct);

		//Check Core dict first
		GetMembers(m_VPcoreDict, tmp);

		//Check Table Script
		if (m_currentMembers.empty())
		{
			GetMembers(m_pageConstructsDict, tmp);
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
		::SendMessage(m_hwndScintilla, SCI_AUTOCSHOW, 0, (LPARAM)m_autoCompMembersString.c_str());
	}
#endif

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
#ifndef __STANDALONE__
	//get word under pointer
	const Sci_Position dwellpos = pSCN ? pSCN->position : ::SendMessage(m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
	const LRESULT wordstart = ::SendMessage(m_hwndScintilla, SCI_WORDSTARTPOSITION, dwellpos, TRUE);
	const LRESULT wordfinish = ::SendMessage(m_hwndScintilla, SCI_WORDENDPOSITION, dwellpos, TRUE);
	const int CurrentLineNo = (int)::SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, dwellpos, 0);

	//return if in a comment
	const size_t lineLength = ::SendMessage(m_hwndScintilla, SCI_LINELENGTH, CurrentLineNo, 0);
	string text(lineLength, '\0'); // SCI_GETLINE does not null-terminate the string
	const size_t cchar = ::SendMessage(m_hwndScintilla, SCI_GETLINE, CurrentLineNo, (LPARAM)text.data());
	assert(cchar == lineLength);
	assert(cchar == text.length());
	if (!text.empty())
	{
		const size_t t = text.find_first_of('\'', 0);
		if (t != string::npos)
		{
			const LRESULT linestart = ::SendMessage(m_hwndScintilla, SCI_POSITIONFROMLINE, CurrentLineNo, 0);
			if (((size_t)(wordstart - linestart)) >= t) return false;
		}
	}

	// is it a valid 'word'
	string Mess;
	const UserData *gotoDefinition = nullptr;
	if (::SendMessage(m_hwndScintilla, SCI_ISRANGEWORD, wordstart, wordfinish))
	{
		//Retrieve the word
		string szDwellWord(wordfinish - wordstart, '\0');
		GetRange(m_hwndScintilla, wordstart, wordfinish, szDwellWord.data());
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
				Mess = "Component: " + szDwellWord;
		}
#ifdef _DEBUG
		if (Mess.empty())
		{
			Mess = "Test: " + szDwellWord;
		}
#endif
	}
	if (!Mess.empty())
	{
		if (tooltip)
			::SendMessage(m_hwndScintilla,SCI_CALLTIPSHOW,dwellpos, (LPARAM)Mess.c_str());
		else if (gotoDefinition)
		{
			::SendMessage(m_hwndScintilla, SCI_GOTOLINE, gotoDefinition->m_lineNum, 0);
			::SendMessage(m_hwndScintilla, SCI_GRABFOCUS, 0, 0);
		}
		else
			return false;
		return true;
	}
#endif
	return false;
}

#ifndef __STANDALONE__
void CodeViewer::MarginClick(const Sci_Position position, const int modifiers)
{
   const size_t lineClick = ::SendMessage(m_hwndScintilla, SCI_LINEFROMPOSITION, position, 0);
   if ((modifiers & SCMOD_SHIFT) && (modifiers & SCMOD_CTRL))
   {
      //FoldAll();
   }
   else
   {
      const size_t levelClick = ::SendMessage(m_hwndScintilla, SCI_GETFOLDLEVEL, lineClick, 0);
      if (levelClick & SC_FOLDLEVELHEADERFLAG)
      {
         if (modifiers & SCMOD_SHIFT)
         {
            // Ensure all children visible
            ::SendMessage(m_hwndScintilla, SCI_SETFOLDEXPANDED, lineClick, 1);
            //Expand(lineClick, true, true, 100, levelClick);
         }
         else if (modifiers & SCMOD_CTRL)
         {
            if (::SendMessage(m_hwndScintilla, SCI_GETFOLDEXPANDED, lineClick, 0))
            {
               // Contract this line and all children
               ::SendMessage(m_hwndScintilla, SCI_SETFOLDEXPANDED, lineClick, 0);
               //Expand(lineClick, false, true, 0, levelClick);
            }
            else
            {
               // Expand this line and all children
               ::SendMessage(m_hwndScintilla, SCI_SETFOLDEXPANDED, lineClick, 1);
               //Expand(lineClick, true, true, 100, levelClick);
            }
         }
         else
         {
            // Toggle this line
            ::SendMessage(m_hwndScintilla, SCI_TOGGLEFOLD, lineClick, 0);
         }
      }
   }
}
#endif

static void AddComment(const HWND m_hwndScintilla)
{
#ifndef __STANDALONE__
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
#endif
}

#ifndef __STANDALONE__
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
      string buf(lineEnd - lineStart, '\0');
      GetRange(m_hwndScintilla, lineStart, lineEnd, buf.data());
      const size_t idx = buf.find_first_of('\'');
      if (idx == 0)
      {
         SendMessage(m_hwndScintilla, SCI_SETSEL, lineStart, lineStart + 1);
         SendMessage(m_hwndScintilla, SCI_REPLACESEL, 0, (LPARAM)"");
      }
   }
   SendMessage(m_hwndScintilla, SCI_ENDUNDOACTION, 0, 0);
}
#endif

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
   const int x = g_app->m_settings.GetEditor_CodeViewPosX();
   const int y = g_app->m_settings.GetEditor_CodeViewPosY();
   const int w = g_app->m_settings.GetEditor_CodeViewPosWidth();
   const int h = g_app->m_settings.GetEditor_CodeViewPosHeight();

   cs.x = x;
   cs.y = y;
   cs.cx = w;
   cs.cy = h;
   cs.style = WS_POPUP | WS_SIZEBOX | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
   cs.hInstance = g_app->GetInstanceHandle();
   cs.lpszClass = "CVFrame";
   cs.lpszName = "Script";
}

void CodeViewer::PreRegisterClass(WNDCLASS& wc)
{
   wc.style = CS_DBLCLKS;
   wc.hInstance = g_app->GetInstanceHandle();
#ifndef __STANDALONE__
   wc.hIcon = LoadIcon(g_app->GetInstanceHandle(), MAKEINTRESOURCE(IDI_SCRIPT));
#endif
   wc.lpszClassName = "CVFrame";
#ifndef __STANDALONE__
   wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
   wc.lpszMenuName = MAKEINTRESOURCE(IDR_SCRIPTMENU);//nullptr;
   wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
#endif
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
				if (!crWord.empty())
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
				RemainingLine = RemainingLine.substr(CommPos+1);
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
				if (!crWord.empty())
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
				RemainingLine = RemainingLine.substr(CommPos+1);
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
			const size_t doubleQuoteIdx = line.find('"');
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
		line = line.substr(Pos, (LL-Pos));
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
#ifndef __STANDALONE__
	const int scriptLines = (int)::SendMessage(m_hwndScintilla, SCI_GETLINECOUNT, 0, 0);

	m_parentLevel = 0; //root
	m_currentParentKey.clear();
	//m_parentTreeInvalid = false;

	m_pageConstructsDict.clear(); //!! it's actually not needed to clear this list, BUT there is no mechanism (it seems) to delete non-existant subs/functions/etc from it, so rather redo it completely

	for (int linecount = 0; linecount < scriptLines; ++linecount)
	{
		// Read line
		const size_t lineLength = ::SendMessage(m_hwndScintilla, SCI_LINELENGTH, linecount, 0);
		if (lineLength < 3) continue;
		string text(lineLength, '\0'); // SCI_GETLINE does not null-terminate the string
		const size_t cchar = ::SendMessage(m_hwndScintilla, SCI_GETLINE, linecount, (LPARAM)text.data());
		assert(lineLength == cchar);
		assert(cchar == text.length());
		if (!text.empty())
			ReadLineToParseBrain(text, linecount, m_pageConstructsDict);
	}
	::SendMessage(m_hwndFunctionList, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
	::SendMessage(m_hwndFunctionList, CB_RESETCONTENT, 0, 0);

	//Propagate subs&funcs in menu in order
	for (fi_vector<UserData>::const_iterator i = m_pageConstructsDict.begin(); i != m_pageConstructsDict.end(); ++i) 
		if (i->eTyping < eDim)
			::SendMessage(m_hwndFunctionList, CB_ADDSTRING, 0, (LPARAM)i->m_keyName.c_str());

	::SendMessage(m_hwndFunctionList, WM_SETREDRAW, TRUE, 0);

	//Collect Objects/Components from the menu. (cheat!)
	size_t CBCount = ::SendMessage(m_hwndItemList, CB_GETCOUNT, 0, 0)-1; //Zero Based
	while ((SSIZE_T)CBCount >= 0)
	{
		const LRESULT s = ::SendMessage(m_hwndItemList, CB_GETLBTEXTLEN, CBCount, 0);
		if (s > 1)
		{
			UserData ud;
			ud.m_keyName.resize(s, '\0');
			::SendMessage(m_hwndItemList, CB_GETLBTEXT, CBCount, (LPARAM)ud.m_keyName.data());
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
	StrToLower(sSubFunOut);
	::SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 1, (LPARAM)sSubFunOut.c_str());
	StrToLower(strCompOut);
	::SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 2, (LPARAM)strCompOut.c_str());
	StrToLower(strVPcoreWords);
	::SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 3, (LPARAM)strVPcoreWords.c_str());
#endif
}

void CodeViewer::ParseVPCore()
{
	std::ifstream fCore;
   fCore.open(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts, "core.vbs"));
	if (!fCore.is_open())
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
	std::string wholeline;
	while (std::getline(fCore, wholeline)) // error or EOF?
	{
		++linecount;
		if (wholeline.length() >= 3)
			ReadLineToParseBrain(wholeline, linecount, m_VPcoreDict);
	}
}

string CodeViewer::ExtractWordOperand(const string &line, const size_t StartPos) const
{
	size_t Substart = StartPos;
	const size_t lineLength = line.length();
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
#ifndef __STANDALONE__
   return (CodeViewer *)GetWindowLongPtr(GWLP_USERDATA);
#else
   return nullptr;
#endif
}

BOOL CodeViewer::ParseClickEvents(const int id, const SCNotification *pSCN)
{
#ifndef __STANDALONE__
   CodeViewer* const pcv = GetCodeViewerPtr();
   switch (id)
   {
      case ID_COMPILE:
         pcv->Compile(true);
         return TRUE;
      case ID_SCRIPT_TOGGLE_LAST_ERROR_VISIBILITY:
         SetLastErrorVisibility(!m_lastErrorWidgetVisible); return TRUE;
      case ID_SCRIPT_PREFERENCES:
         DialogBox(g_app->GetInstanceHandle(), MAKEINTRESOURCE(IDD_CODEVIEW_PREFS), GetHwnd(), CVPrefProc); return TRUE;
      case ID_FIND:
         pcv->ShowFindDialog(); return TRUE;
      case ID_REPLACE:
         pcv->ShowFindReplaceDialog(); return TRUE;
      case ID_EDIT_FINDNEXT:
         pcv->Find(&pcv->m_findreplaceold); return TRUE;
      case ID_EDIT_UNDO:
         ::SendMessage(pcv->m_hwndScintilla, SCI_UNDO, 0, 0); return TRUE;
      case ID_EDIT_COPY:
         ::SendMessage(pcv->m_hwndScintilla, WM_COPY, 0, 0); return TRUE;
      case ID_EDIT_CUT:
         ::SendMessage(pcv->m_hwndScintilla, WM_CUT, 0, 0); return TRUE;
      case ID_EDIT_PASTE:
         ::SendMessage(pcv->m_hwndScintilla, WM_PASTE, 0, 0); return TRUE;
      case ID_ADD_COMMENT:
         AddComment(pcv->m_hwndScintilla); return TRUE;
      case ID_REMOVE_COMMENT:
         RemoveComment(pcv->m_hwndScintilla); return TRUE;
      case ID_GO_TO_DEFINITION:
         ShowTooltipOrGoToDefinition(pSCN,false); return TRUE;
   }
#endif
   return FALSE;
}

BOOL CodeViewer::ParseSelChangeEvent(const int id, const SCNotification *pSCN)
{
#ifndef __STANDALONE__
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
         pcv->m_table->DoCodeViewCommand(id); return TRUE;
      case ID_EDIT_FINDNEXT:
         pcv->Find(&pcv->m_findreplaceold); return TRUE;
      case ID_REPLACE:
         pcv->ShowFindReplaceDialog(); return TRUE;
      case ID_EDIT_UNDO:
         ::SendMessage(pcv->m_hwndScintilla, SCI_UNDO, 0, 0); return TRUE;
      case IDC_ITEMLIST:
      {
         pcv->ListEventsFromItem();
         pcv->TellHostToSelectItem();
         return TRUE;
      }
      case IDC_EVENTLIST:
         pcv->FindCodeFromEvent(); return TRUE;
      case IDC_FUNCTIONLIST:
      {
         const LRESULT Listindex = ::SendMessage(pcv->m_hwndFunctionList, CB_GETCURSEL, 0, 0);
         if (Listindex != -1)
         {
            string s(::SendMessage(pcv->m_hwndFunctionList, CB_GETLBTEXTLEN, Listindex, 0), '\0');
            /*size_t index =*/ ::SendMessage(pcv->m_hwndFunctionList, CB_GETLBTEXT, Listindex, (LPARAM)s.data());
            RemovePadding(s);
            StrToLower(s);
            int idx;
            FindUD(pcv->m_pageConstructsDict, s, idx);
            if (idx != -1)
            {
               ::SendMessage(pcv->m_hwndScintilla, SCI_GOTOLINE, pcv->m_pageConstructsDict[idx].m_lineNum, 0);
               ::SendMessage(pcv->m_hwndScintilla, SCI_GRABFOCUS, 0, 0);
            }
         }
         return TRUE;
      }
      case ID_ADD_COMMENT:
         AddComment(pcv->m_hwndScintilla); return TRUE;
      case ID_REMOVE_COMMENT:
         RemoveComment(pcv->m_hwndScintilla); return TRUE;
      case ID_GO_TO_DEFINITION:
         ShowTooltipOrGoToDefinition(pSCN, false); return TRUE;
      case ID_SHOWAUTOCOMPLETE:
         pcv->ShowAutoComplete(pSCN); return TRUE;
   }//switch (id)
#endif
   return FALSE;
}

LRESULT CodeViewer::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
   CodeViewer* const pcv = GetCodeViewerPtr();

   if (uMsg == m_findMsgString)
   {
      FINDREPLACE * const pfr = (FINDREPLACE *)lParam;
      if (pfr->Flags & FR_DIALOGTERM)
      {
         pcv->m_hwndFind = nullptr;
         const size_t selstart = ::SendMessage(pcv->m_hwndScintilla, SCI_GETSELECTIONSTART, 0, 0);
         const size_t selend = ::SendMessage(pcv->m_hwndScintilla, SCI_GETSELECTIONEND, 0, 0);
         ::SetFocus(pcv->m_hwndScintilla);
         ::SendMessage(pcv->m_hwndScintilla, SCI_SETSEL, selstart, selend);
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
            pcv->m_stopErrorDisplay = true; ///stop Error reporting WIP
            pcv->m_warn_on_dupes = true;
            pcv->ParseForFunction();
            GetApp()->SetAccelerators(pcv->m_haccel, pcv->m_hwndMain);
         }
         break;
      }
      case WM_CLOSE:
         pcv->SetVisible(false); return TRUE;
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
#else
   return 0;
#endif
}

BOOL CodeViewer::OnCommand(WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   UNREFERENCED_PARAMETER(lparam);

   CodeViewer* const pcv = GetCodeViewerPtr();

   const int code = HIWORD(wparam);
   const int id = LOWORD(wparam);

   switch (code)
   {
      case SCEN_SETFOCUS:
      {
         pcv->m_warn_on_dupes = true;
         pcv->ParseForFunction();
         return TRUE;
      }
      case SCEN_CHANGE:
      {
         //also see SCN_MODIFIED handling which does more finegrained updating calls
         if (pcv->m_errorLineNumber != -1)
            pcv->UncolorError();
         pcv->m_table->m_sdsDirtyScript = eSaveDirty;
         
         const size_t cchar = ::SendMessage(m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
         if (cchar == 0)
         {
            pcv->m_table->m_script_text = "";
         }
         else
         {
            string script;
            script.resize(cchar + 1);
            ::SendMessage(m_hwndScintilla, SCI_GETTEXT, cchar + 1, (LPARAM)script.data());
            pcv->m_table->m_script_text = script;
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
         return ParseSelChangeEvent(id, (SCNotification *)lparam);
      case BN_CLICKED:
         return ParseClickEvents(id, (SCNotification *)lparam);
   }
#endif
   return FALSE;
}

LRESULT CodeViewer::OnNotify(WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   const NMHDR* const pnmh = (LPNMHDR)lparam;
   const SCNotification* const pscn = (SCNotification*)lparam;

   const HWND hwndRE = pnmh->hwndFrom;
   const int code = pnmh->code;
   CodeViewer* const pcv = GetCodeViewerPtr();
   switch (code)
   {
      case SCN_SAVEPOINTREACHED:
      {
         pcv->m_table->m_sdsDirtyScript = eSaveClean;
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
            ::SendMessage(pcv->m_hwndScintilla, SCI_CALLTIPCANCEL, 0, 0);
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

         const string tmp = "Line " + std::to_string(line) + ", Col "+ std::to_string(column);
         ::SendMessage(pcv->m_hwndStatus, SB_SETTEXT, 0 | 0, (size_t)tmp.c_str());
         break;
      }
      case SCN_DOUBLECLICK:
      {
         pcv->GetWordUnderCaret(szCaretTextBuff);
         szLower(szCaretTextBuff);
         // set back ground colour of all words on display
         ::SendMessage(pcv->m_hwndScintilla, SCI_STYLESETBACK, SCE_B_KEYWORD5, RGB(200, 200, 200));
         ::SendMessage(pcv->m_hwndScintilla, SCI_SETKEYWORDS, 4, (LPARAM)szCaretTextBuff);
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
#else 
   return 0;
#endif
}

INT_PTR CALLBACK CVPrefProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
   CodeViewer* pcv = g_pvp->GetActiveTableEditor()->m_pcv;
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
		if (pcv->m_lPrefsList)
		{
			for (size_t i = 0; i < pcv->m_lPrefsList->size(); i++)
			{
				pcv->m_lPrefsList->at(i)->GetPrefsFromReg();
				pcv->m_lPrefsList->at(i)->SetCheckBox(hwndDlg);
			}
			pcv->m_bgColor = g_app->m_settings.GetCVEdit_BackGroundColor();
			pcv->m_bgSelColor = g_app->m_settings.GetCVEdit_BackGroundSelectionColor();
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
					g_app->m_settings.SetCVEdit_DwellDisplay(pcv->m_dwellDisplay, false);
				}
				break;
				case IDC_CVP_CHKBOX_HELPWITHDWELL:
				{
					pcv->m_dwellHelp = !!IsDlgButtonChecked(hwndDlg, IDC_CVP_CHKBOX_HELPWITHDWELL);
					g_app->m_settings.SetCVEdit_DwellHelp(pcv->m_dwellHelp, false);
				}
				break;
				case IDC_CVP_CHKBOX_SHOWAUTOCOMPLETE:
				{
					pcv->m_displayAutoComplete = !!IsDlgButtonChecked(hwndDlg, IDC_CVP_CHKBOX_SHOWAUTOCOMPLETE);
					g_app->m_settings.SetCVEdit_DisplayAutoComplete(pcv->m_displayAutoComplete, false);
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
#endif
   return FALSE; // be selfish - consume all
   //return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}

void CodeViewer::UpdateScinFromPrefs()
{
#ifndef __STANDALONE__
	::SendMessage(m_hwndScintilla, SCI_SETMOUSEDWELLTIME, m_dwellDisplayTime, 0);
	::SendMessage(m_hwndScintilla, SCI_STYLESETBACK, m_prefEverythingElse->m_sciKeywordID, m_bgColor);
	::SendMessage(m_hwndScintilla, SCI_SETSELBACK, m_prefEverythingElse->m_sciKeywordID, m_bgSelColor);
	m_prefEverythingElse->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);//Update internally
	::SendMessage(m_hwndScintilla,SCI_STYLECLEARALL,0,0);
	::SendMessage(m_hwndScintilla, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, m_prefEverythingElse->m_rgb);
	::SendMessage(m_hwndScintilla, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, m_bgColor);
	prefDefault->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefVBS->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefSubs->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefComps->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefLiterals->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefComments->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	prefVPcore->ApplyPreferences(m_hwndScintilla, m_prefEverythingElse);
	::SendMessage(m_hwndScintilla, SCI_STYLESETBACK, SCE_B_KEYWORD5, RGB(200,200,200));
	::SendMessage(m_hwndScintilla, SCI_SETKEYWORDS, 4 , (LPARAM)m_wordUnderCaret.lpstrText);

	const int scriptLines = (int)::SendMessage(m_hwndScintilla, SCI_GETLINECOUNT, 0, 0);
	//Update the margin width to fit the number of characters required to display the line number * font size
	::SendMessage(m_hwndScintilla, SCI_SETMARGINWIDTHN, 0, (scriptLines > 0 ? (int)ceil(log10((double)scriptLines) + 3.) : 1) * m_prefEverythingElse->m_pointSize);
#endif
}

void CodeViewer::ResizeScintillaAndLastError()
{
#ifndef __STANDALONE__
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
#endif
}

void CodeViewer::SetLastErrorVisibility(bool show)
{
#ifndef __STANDALONE__
   if (!IsWindow())
      return;
   if (show == m_lastErrorWidgetVisible)
      return;
	m_lastErrorWidgetVisible = show;

	ResizeScintillaAndLastError();

	::ShowWindow(m_hwndLastErrorTextArea, show ? SW_SHOW : SW_HIDE);
#endif
}

void CodeViewer::SetLastErrorTextW(const LPCWSTR text)
{
#ifndef __STANDALONE__
   if (!IsWindow())
      return;

   ::SetWindowTextW(m_hwndLastErrorTextArea, text);

	// Scroll to the bottom
	::SendMessage(m_hwndLastErrorTextArea, EM_LINESCROLL, 0, 9999);
#endif
}

void CodeViewer::AppendLastErrorTextW(const wstring& text)
{
#ifndef __STANDALONE__
   if (!IsWindow())
      return;

	const int requiredLength = ::GetWindowTextLength(m_hwndLastErrorTextArea) + (int)text.length() + 1;
	wchar_t* buf = new wchar_t[requiredLength];

	// Get existing text from edit control and put into buffer
	::GetWindowTextW(m_hwndLastErrorTextArea, buf, requiredLength);

	// Append the new text to the buffer
	wcscat_s(buf, requiredLength, text.c_str());

	::SetWindowTextW(m_hwndLastErrorTextArea, buf);

	// Scroll to the bottom
	::SendMessage(m_hwndLastErrorTextArea, EM_LINESCROLL, 0, 9999);

	delete[] buf;
#else
	PLOGE << MakeString(text);
#endif
}
