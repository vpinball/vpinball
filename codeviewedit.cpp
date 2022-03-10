#include "stdafx.h"
#include "codeviewedit.h"

UserData::UserData()
   : m_lineNum(0),
     eTyping(eUnknown)
{
}

UserData::UserData(const int LineNo, const string &Desc, const string &Name, const WordType TypeIn)
   : m_lineNum(LineNo),
     m_keyName(Name),
     eTyping(TypeIn),
     m_description(Desc)
{
}

int FindUDbyKey(const vector<UserData>& ListIn, const string& strIn, int& PosOut);

/*	FindUD - Now a human Search!
 0 =Found & UDiterOut set to point at UD in list.
-1 =Not Found 
 1 =Not Found
-2 =Zero Length string or error*/
int FindUD(const vector<UserData>& ListIn, string &strIn, vector<UserData>::const_iterator& UDiterOut, int &Pos)
{
	RemovePadding(strIn);
	if (strIn.empty() || ListIn.empty()) return -2;

	Pos = -1;
	const int KeyResult = FindUDbyKey(ListIn, strIn, Pos);

	//If it's a top level construct it will have no parents and therefore have a unique key.
	if (KeyResult == 0)
	{
		if (Pos != -1)
			UDiterOut = ListIn.begin() + Pos;
		return 0;
	}

	//Now see if it's in the Name list
	//Jumpdelta should be initialized to the maximum count of an individual key name
	//But for the moment the biggest is 64 x's in AMH
	Pos += KeyResult; //Start very close to the result of key search
	if (Pos < 0) Pos = 0;
	//Find the start of other instances of strIn by crawling up list
	//Usually (but not always) FindUDbyKey returns top of the list so its fast
	const string strSearchData = lowerCase(strIn);
	const size_t SearchWidth = strSearchData.size();
	do
	{
		--Pos;
	} while (Pos >= 0 && strSearchData.compare(lowerCase(ListIn[Pos].m_uniqueKey).substr(0, SearchWidth)) == 0);
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

	UDiterOut = ListIn.begin() + Pos;
	return result;
}

//Finds the closest UD from CurrentLine in ListIn
//On entry CurrentIdx must be set to the UD in the line
int FindClosestUD(const vector<UserData>& ListIn, const int CurrentLine, const int CurrentIdx)
{
	const string strSearchData = lowerCase(ListIn[CurrentIdx].m_keyName);
	const size_t SearchWidth = strSearchData.size();
	//Find the start of other instances of strIn by crawling up list
	int iNewPos = CurrentIdx;
	do
	{
		--iNewPos;
	} while (iNewPos >= 0 && strSearchData.compare(lowerCase(ListIn[iNewPos].m_uniqueKey).substr(0, SearchWidth)) == 0);
	++iNewPos;
	//Now at top of list
	//find nearest definition above current line
	//int ClosestLineNum = 0;
	int ClosestPos = CurrentIdx;
	int Delta = -(INT_MAX - 1);
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

static int FindUDbyKey(const vector<UserData>& ListIn, const string &strIn, int &PosOut)
{
	if (!ListIn.empty() && !strIn.empty()) // Sanity check
		return UDKeyIndexHelper<true>(ListIn, strIn, PosOut);
	else
		return -2;
}

//Needs speeding up.
UserData GetUDfromUniqueKey(const vector<UserData>& ListIn, const string &UniKey)
{
	UserData RetVal;
	RetVal.eTyping = eUnknown;
	const size_t ListSize = ListIn.size();
	for (size_t i = 0; i < ListSize; ++i)
		if (UniKey == ListIn[i].m_uniqueKey)
		{
			RetVal = ListIn[i];
			if (RetVal.eTyping != eUnknown)
				return RetVal;
		}
	return RetVal;
}

//TODO: Needs speeding up.
size_t GetUDPointerfromUniqueKey(const vector<UserData>& ListIn, const string &UniKey)
{
	const size_t ListSize = ListIn.size();
	for (size_t i = 0; i < ListSize; ++i)
		if (UniKey == ListIn[i].m_uniqueKey)
			return i;
	return -1;
}

//Assumes case insensitive sorted list
//Returns index or insertion point (-1 == error)
size_t FindOrInsertUD(vector<UserData>& ListIn, const UserData &udIn)
{
	if (ListIn.empty())	//First in
	{
		ListIn.push_back(udIn);
		return 0;
	}
	int Pos = 0;
	const int KeyFound = FindUDbyKey(ListIn, udIn.m_uniqueKey, Pos);
	if (KeyFound == 0)
	{
		//Same name, different parents?
		const vector<UserData>::const_iterator iterFound = ListIn.begin() + Pos;
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
			// assign again, as e.g. line of func/sub/var could have changed by other updates
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

bool FindOrInsertStringIntoAutolist(vector<string>& ListIn, const string &strIn)
{
	//First in the list
	if (ListIn.empty())
	{
		ListIn.push_back(strIn);
		return true;
	}
	const unsigned int ListSize = (unsigned int)ListIn.size();
	UINT32 iNewPos = 1u << 31;
	while (!(iNewPos & ListSize) && (iNewPos > 1))
		iNewPos >>= 1;
	int iJumpDelta = iNewPos >> 1;
	--iNewPos;//Zero Base
	const string strSearchData = lowerCase(strIn);
	UINT32 iCurPos;
	int result;
	while (true)
	{
		iCurPos = iNewPos;
		if (iCurPos >= ListSize)
			result = -1;
		else
			result = strSearchData.compare(lowerCase(ListIn[iCurPos]));
		if (iJumpDelta == 0 || result == 0) break;
		iNewPos = (result < 0) ? (iCurPos - iJumpDelta) : (iCurPos + iJumpDelta);
		iJumpDelta >>= 1;
	}

	if (result == 0) return false; // Already in list

	const vector<string>::iterator i = ListIn.begin() + iCurPos;

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

////////////////Preferences
CVPreference::CVPreference(
		const COLORREF crTextColor,
		const bool bDisplay, const string& szRegistryName,
		const int szScintillaKeyword, const int IDC_ChkBox,
		const int IDC_ColorBut, const int IDC_Font)
{
	memset(&m_logFont, 0, sizeof(LOGFONT));
	m_rgb = crTextColor;
	m_highlight = bDisplay;
	m_szRegName = szRegistryName;
	m_sciKeywordID = szScintillaKeyword;
	IDC_ChkBox_code = IDC_ChkBox;
	IDC_ColorBut_code = IDC_ColorBut;
	IDC_Font_code = IDC_Font;
}

void CVPreference::SetCheckBox(const HWND hwndDlg)
{
	SNDMSG(GetDlgItem(hwndDlg, IDC_ChkBox_code), BM_SETCHECK, m_highlight ? BST_CHECKED : BST_UNCHECKED, 0L);
}

void CVPreference::ReadCheckBox(const HWND hwndDlg)
{
	m_highlight = !!IsDlgButtonChecked(hwndDlg, IDC_ChkBox_code);
}

void CVPreference::GetPrefsFromReg()
{
	m_highlight = LoadValueBoolWithDefault("CVEdit", m_szRegName, m_highlight);
	m_rgb = LoadValueIntWithDefault("CVEdit", m_szRegName+"_color", m_rgb);
	m_pointSize = LoadValueIntWithDefault("CVEdit", m_szRegName+"_FontPointSize", m_pointSize);

	char bakupFaceName[LF_FACESIZE]; // to save the default font name, in case the corresponding registry entry is empty
	strncpy_s(bakupFaceName, m_logFont.lfFaceName, sizeof(bakupFaceName)-1);
	if (LoadValue("CVEdit", m_szRegName+"_Font", m_logFont.lfFaceName, LF_FACESIZE) != S_OK)
		strncpy_s(m_logFont.lfFaceName, bakupFaceName, sizeof(m_logFont.lfFaceName)-1);

	m_logFont.lfWeight = LoadValueIntWithDefault("CVEdit", m_szRegName+"_FontWeight", m_logFont.lfWeight);
	m_logFont.lfItalic = LoadValueIntWithDefault("CVEdit", m_szRegName+"_FontItalic", m_logFont.lfItalic);
	m_logFont.lfUnderline = LoadValueIntWithDefault("CVEdit", m_szRegName+"_FontUnderline", m_logFont.lfUnderline);
	m_logFont.lfStrikeOut = LoadValueIntWithDefault("CVEdit", m_szRegName+"_FontStrike", m_logFont.lfStrikeOut);
}

void CVPreference::SetPrefsToReg()
{
	SaveValueBool("CVEdit", m_szRegName, m_highlight);
	SaveValueInt("CVEdit", m_szRegName+"_color", m_rgb);
	SaveValueInt("CVEdit", m_szRegName+"_FontPointSize", m_pointSize);
	SaveValue("CVEdit", m_szRegName+"_Font", m_logFont.lfFaceName);
	SaveValueInt("CVEdit", m_szRegName+"_FontWeight", m_logFont.lfWeight);
	SaveValueInt("CVEdit", m_szRegName+"_FontItalic", m_logFont.lfItalic);
	SaveValueInt("CVEdit", m_szRegName+"_FontUnderline", m_logFont.lfUnderline);
	SaveValueInt("CVEdit", m_szRegName+"_FontStrike", m_logFont.lfStrikeOut);
}

void CVPreference::SetDefaultFont(const HWND hwndDlg)
{
	LOGFONT* const plfont = &m_logFont;
	memset(plfont, 0, sizeof(LOGFONT));
	HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	if (hFont == nullptr)
		hFont = (HFONT)GetStockObject(SYSTEM_FONT);
	GetObject(hFont, sizeof(LOGFONT), plfont);
	m_pointSize = 10;
	GetHeightFromPointSize(hwndDlg);
}

int CVPreference::GetHeightFromPointSize(const HWND hwndDlg)
{
	const CClientDC clientDC(hwndDlg);
	const int Height = -MulDiv(m_pointSize, clientDC.GetDeviceCaps(LOGPIXELSY), 72);
	return Height;
}

void CVPreference::ApplyPreferences(const HWND hwndScin, const CVPreference* DefaultPref)
{
	const int id = m_sciKeywordID;
	const bool HL = m_highlight;
	SendMessage(hwndScin, SCI_STYLESETFORE,      id, HL ? (LPARAM)m_rgb : (LPARAM)DefaultPref->m_rgb);
	SendMessage(hwndScin, SCI_STYLESETFONT,      id, HL ? (LPARAM)m_logFont.lfFaceName : (LPARAM)DefaultPref->m_logFont.lfFaceName);
	SendMessage(hwndScin, SCI_STYLESETSIZE,      id, HL ? (LPARAM)m_pointSize : (LPARAM)DefaultPref->m_pointSize);
	SendMessage(hwndScin, SCI_STYLESETWEIGHT,    id, HL ? (LPARAM)m_logFont.lfWeight : (LPARAM)DefaultPref->m_logFont.lfWeight);
	SendMessage(hwndScin, SCI_STYLESETITALIC,    id, HL ? (LPARAM)m_logFont.lfItalic : (LPARAM)DefaultPref->m_logFont.lfItalic);
	SendMessage(hwndScin, SCI_STYLESETUNDERLINE, id, HL ? (LPARAM)m_logFont.lfUnderline : (LPARAM)DefaultPref->m_logFont.lfUnderline);
	// There is no strike through in Scintilla (yet!)
}
