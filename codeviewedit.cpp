#include "stdafx.h"
#include "codeviewedit.h"

UserData::UserData()
{
	m_lineNum = 0;
	eTyping = eUnknown;
}

UserData::UserData(const int LineNo, const string &Desc, const string &Name, const WordType TypeIn)
{
	m_lineNum = LineNo;
	m_description = Desc;
	m_keyName = Name;
	eTyping = TypeIn;
}

UserData::~UserData()
{
}

/*	FindUD - Now a human Search!
0  =Found & UDiterOut set to point at UD in list.
-1 =Not Found 
1  =Not Found
-2 =Zero Length string or error*/
int UserData::FindUD(vector<UserData>* ListIn, string &strIn, vector<UserData>::iterator& UDiterOut, int &Pos)
{
	RemovePadding(strIn);
	if (strIn.size() == 0 || (!ListIn) || ListIn->size() == 0) return -2;

	Pos = -1;
	const int KeyResult = FindUDbyKey(ListIn, strIn, UDiterOut, Pos);
	//If it's a top level construct it will have no parents and therefore have a unique key.
	if (KeyResult == 0) return 0;

	//Now see if it's in the Name list
	//Jumpdelta should be intalised to the maximum count of an individual key name
	//But for the momment the biggest is 64 x's in AMH
	int iNewPos = Pos + KeyResult; //Start Very close to the result of key search
	if (iNewPos < 0) iNewPos = 0;
	//Find the start of other instances of strIn by crawling up list
	//Usually (but not always) FindUDbyKey returns top of the list so its fast
	const string strSearchData = lowerCase(strIn);
	const size_t SearchWidth = strSearchData.size();
	while (true)
	{
		iNewPos--;
		if (iNewPos < 0) break;
		const string strTableData = lowerCase(ListIn->at(iNewPos).m_uniqueKey).substr(0, SearchWidth);
		if (strSearchData.compare(strTableData) != 0) break;
	}
	++iNewPos;
	// now walk down list of Keynames looking for what we want.
	int result;
	while (true)
	{
		string strTableData = lowerCase(ListIn->at(iNewPos).m_keyName);
		result = strSearchData.compare(strTableData); 
		if (result == 0) break; //Found
		++iNewPos;
		if (iNewPos == ListIn->size()) break;
		strTableData = lowerCase(ListIn->at(iNewPos).m_keyName).substr(0, SearchWidth);
		result = strSearchData.compare(strTableData);
		if (result != 0) break;	//EO SubList
	}
	UDiterOut = ListIn->begin() + iNewPos;
	Pos = iNewPos;
	return result;
}

//Finds the closest UD from CurrentLine in ListIn
//On entry CurrentIdx must be set to the UD in the line
int UserData::FindClosestUD(vector<UserData>* ListIn, const int CurrentLine, const int CurrentIdx)
{
	const string strSearchData = lowerCase(ListIn->at(CurrentIdx).m_keyName);
	const size_t SearchWidth = strSearchData.size();
	//Find the start of other instances of strIn by crawling up list
	int iNewPos = CurrentIdx;
	while (true)
	{
		iNewPos--;
		if (iNewPos < 0) break;
		const string strTableData = lowerCase(ListIn->at(iNewPos).m_uniqueKey).substr(0, SearchWidth);
		if (strSearchData.compare(strTableData) != 0) break;
	}
	++iNewPos;
	//Now at top of list
	//find nearest definition above current line
	int ClosestLineNum = 0;
	int ClosestPos = CurrentIdx;
	int Delta = -(INT_MAX - 1);
	while (true)
	{
		const int NewLineNum = ListIn->at(iNewPos).m_lineNum;
		const int NewDelta = NewLineNum - CurrentLine;
		if (NewDelta >= Delta && NewLineNum <= CurrentLine)
		{
			if (lowerCase(ListIn->at(iNewPos).m_keyName).compare(strSearchData) == 0)
			{
				Delta = NewDelta;
				ClosestLineNum = NewLineNum;
				ClosestPos = iNewPos;
			}
		}
		++iNewPos;
		if (iNewPos == ListIn->size()) break;
		const string strTableData = lowerCase(ListIn->at(iNewPos).m_keyName).substr(0, SearchWidth);
		if (strSearchData.compare(strTableData) != 0) break;
	}
	--iNewPos;
	return ClosestPos;
}

int UserData::FindUDbyKey(vector<UserData>* ListIn, const string &strIn, vector<UserData>::iterator &UDiterOut, int &PosOut)
{
	int result = -2;
	if (ListIn && (ListIn->size() > 0) && (strIn.size() > 0))// Sanity chq.
	{
		const unsigned int ListSize = (int)ListIn->size();
		UINT32 iCurPos = (ListSize >> 1);
		int iNewPos = 1u << 30;
		while ((!(iNewPos & ListSize)) && (iNewPos > 1))
		{
			iNewPos >>= 1;
		}
		int iJumpDelta = ((iNewPos) >> 1);
		--iNewPos;//Zero Base
		const string strSearchData = lowerCase(strIn);
		while (true)
		{
			iCurPos = iNewPos;
			if (iCurPos >= ListSize) { result = -1; }
			else
			{
				const string strTableData = lowerCase(ListIn->at(iCurPos).m_uniqueKey);
				result = strSearchData.compare(strTableData);
			}
			if (iJumpDelta == 0 || result == 0) break;
			if (result < 0) { iNewPos = iCurPos - iJumpDelta; }
			else  { iNewPos = iCurPos + iJumpDelta; }
			iJumpDelta >>= 1;
		} 
		UDiterOut = ListIn->begin() + iCurPos;
		PosOut = iCurPos;
	}
	return result;
}

//Returns current Index of strIn in ListIn based on m_uniqueKey, or -1 if not found
int UserData::UDKeyIndex(vector<UserData>* ListIn, const string &strIn)
{
	if ((!ListIn) || (ListIn->size() == 0) || (strIn.size() == 0)) return -1;
	int result = -2;
	const unsigned int ListSize = (int)ListIn->size();
	UINT32 iCurPos = (ListSize >> 1);
	UINT32 iNewPos = 1u << 30;
	while ((!(iNewPos & ListSize)) && (iNewPos > 1))
	{
		iNewPos >>= 1;
	}
	int iJumpDelta = ((iNewPos) >> 1);
	--iNewPos;//Zero Base
	const string strSearchData = lowerCase(strIn);
	while (true)
	{
		iCurPos = iNewPos;
		if (iCurPos >= ListSize) { result = -1; }
		else
		{
			const string strTableData = lowerCase(ListIn->at(iCurPos).m_uniqueKey);
			result = strSearchData.compare(strTableData);
		}
		if (iJumpDelta == 0 || result == 0) break;
		if (result < 0) { iNewPos = iCurPos - iJumpDelta; }
		else  { iNewPos = iCurPos + iJumpDelta; }
		iJumpDelta >>= 1;
	} 
	///TODO: neads to consider children?
	if (result == 0)
		return iCurPos;
	else
		return -1;
}

//Returns current Index of strIn in ListIn based on m_keyName, or -1 if not found
int UserData::UDIndex(vector<UserData>* ListIn, const string &strIn)
{
	if ((!ListIn) || (ListIn->size() == 0) || (strIn.size() == 0)) return -1;
	int result = -2;
	const unsigned int ListSize = (int)ListIn->size();
	UINT32 iCurPos = (ListSize >> 1);
	UINT32 iNewPos = 1u << 30;
	while ((!(iNewPos & ListSize)) && (iNewPos > 1))
	{
		iNewPos >>= 1;
	}
	int iJumpDelta = ((iNewPos) >> 1);
	--iNewPos;//Zero Base
	const string strSearchData = lowerCase(strIn);
	while (true)
	{
		iCurPos = iNewPos;
		if (iCurPos >= ListSize) { result = -1; }
		else
		{
			const string strTableData = lowerCase(ListIn->at(iCurPos).m_keyName);
			result = strSearchData.compare(strTableData);
		}
		if (iJumpDelta == 0 || result == 0) break;
		if (result < 0) { iNewPos = iCurPos - iJumpDelta; }
		else  { iNewPos = iCurPos + iJumpDelta; }
		iJumpDelta >>= 1;
	} 
	///TODO: neads to consider children?
	if (result == 0)
		return iCurPos;
	else
		return -1;
}
//Needs speeding up.
UserData UserData::GetUDfromUniqueKey(vector<UserData>* ListIn, const string &UniKey)
{
	UserData RetVal;
	RetVal.eTyping = eUnknown;
	size_t i = 0;
	const size_t ListSize = ListIn->size();
	while ((RetVal.eTyping == eUnknown) && (i < ListSize))
	{
		if (UniKey == ListIn->at(i).m_uniqueKey)
		{
			RetVal = ListIn->at(i);
		}
		++i;
	}
	return RetVal;
}
//TODO: Needs speeding up.
size_t UserData::GetUDPointerfromUniqueKey(vector<UserData>* ListIn, const string &UniKey)
{
	size_t i = 0;
	const size_t ListSize = ListIn->size();
	while (i < ListSize)
	{
		if (UniKey == ListIn->at(i).m_uniqueKey)
		{
			return i;
		}
		++i;
	}
	return -1;
}

//Assumes case insensitive sorted list
//Returns index or insertion point (-1 == error)
size_t UserData::FindOrInsertUD(vector<UserData>* ListIn, UserData &udIn)
{
	if (ListIn->size() == 0)	//First in
	{
		ListIn->push_back(udIn);
		return 0;
	}
	vector<UserData>::iterator iterFound  = ListIn->begin();
	int Pos = 0;
	const int KeyFound = FindUDbyKey(ListIn, udIn.m_uniqueKey, iterFound, Pos);
	if (KeyFound == 0)
	{
		//Same name, different parents.
		const int ParentResult = udIn.m_uniqueParent.compare(iterFound->m_uniqueParent);
		if (ParentResult == -1)
		{
			ListIn->insert(iterFound, udIn);
		}
		else if (ParentResult == 1)
		{
			++iterFound;
			++Pos;
			ListIn->insert(iterFound, udIn);
		}	
		return Pos;
	}

	if (KeyFound == -1) //insert before, somewhere in the middle
	{
		ListIn->insert(iterFound, udIn);
		return Pos;
	}
	else
	{
		if (KeyFound == 1)//insert Above last element - Special case 
		{
			++iterFound;
			ListIn->insert(iterFound, udIn);
			return ++Pos;
		}
		else
		if (iterFound == (ListIn->end() - 1))
		{//insert at end
			ListIn->push_back(udIn);
			return ListIn->size() - 1;//Zero Base
		}
	}
	return -1;
}

bool UserData::FindOrInsertStringIntoAutolist(vector<string>* ListIn, const string &strIn)
{
	//First in the list
	if (ListIn->empty())
	{
		ListIn->push_back(strIn);
		return true;
	}
	vector<string>::iterator i = ListIn->begin();
	int result = -2;
	const unsigned int ListSize = (unsigned int)ListIn->size();
	UINT32 iCurPos = (ListSize >> 1);
	UINT32 iNewPos = 1u << 31;
	while ((!(iNewPos & ListSize)) && (iNewPos > 1))
	{
		iNewPos >>= 1;
	}
	int iJumpDelta = ((iNewPos) >> 1);
	--iNewPos;//Zero Base
	const string strSearchData = lowerCase(strIn);
	while (true)
	{
		iCurPos = iNewPos;
		if (iCurPos >= ListSize) { result = -1; }
		else
		{
			const string strTableData = lowerCase(ListIn->at(iCurPos));
			result = strSearchData.compare(strTableData);
		}
		if (iJumpDelta == 0 || result == 0) break;
		if (result < 0) { iNewPos = iCurPos - iJumpDelta; }
		else  { iNewPos = iCurPos + iJumpDelta; }
		iJumpDelta >>= 1;
	} 
	i = ListIn->begin() + iCurPos;

	if (result == 0) return false; // Already in list.

	if (result == -1) //insert before, somewhere in the middle
	{
		ListIn->insert(i, strIn);
		return true;
	}

	if (i == (ListIn->end() - 1))//insert Above last element - Special case
	{
		ListIn->push_back(strIn);
		return true;
	}

	if (result == 1) 
	{
		++i;
		ListIn->insert(i, strIn);
		return true;
	}

	if (i == (ListIn->end() - 1))//insert Above last element - Special case
	{//insert at end
		ListIn->push_back(strIn);
		return true;
	}

	return false;//Oh pop poop, never should hit here.
}

////////////////Preferences
CVPrefrence::CVPrefrence()
{
   szControlName = nullptr;
   m_rgb = 0;
   m_highlight = false;
   szRegName = nullptr;
   m_sciKeywordID = 0;
}

CVPrefrence* CVPrefrence::FillCVPreference(
		const char* szCtrlNameIn, const COLORREF crTextColor,
		const bool bDisplay, const char* szRegistryName,
		const int szScintillaKeyword, const int IDC_ChkBox,
		const int IDC_ColorBut, const int IDC_Font)
{
   szControlName = szCtrlNameIn;
   m_rgb = crTextColor;
   m_highlight = bDisplay;
   szRegName = szRegistryName;
   m_sciKeywordID = szScintillaKeyword;
   IDC_ChkBox_code = IDC_ChkBox;
   IDC_ColorBut_code = IDC_ColorBut;
   IDC_Font_code = IDC_Font;
   return (CVPrefrence *)this;
}

void CVPrefrence::SetCheckBox(const HWND hwndDlg)
{
	const HWND hChkBox = GetDlgItem(hwndDlg, this->IDC_ChkBox_code);
	SNDMSG(hChkBox, BM_SETCHECK, this->m_highlight ? BST_CHECKED : BST_UNCHECKED, 0L);
}

void CVPrefrence::ReadCheckBox(const HWND hwndDlg)
{
	this->m_highlight = !!IsDlgButtonChecked(hwndDlg, this->IDC_ChkBox_code);
}

void CVPrefrence::GetPrefsFromReg()
{
	char RegEntry[33];
	strcpy_s(RegEntry, this->szRegName);
	this->m_highlight = LoadValueBoolWithDefault("CVEdit", RegEntry, this->m_highlight);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_color");
	this->m_rgb = LoadValueIntWithDefault("CVEdit", RegEntry, this->m_rgb);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontPointSize");
	this->m_pointSize = LoadValueIntWithDefault("CVEdit", RegEntry, this->m_pointSize);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_Font");
	char bakupFaceName[LF_FACESIZE]; // to save the default font name, in case the corresponding registry entry is empty
	strcpy_s(bakupFaceName, this->m_logFont.lfFaceName);
	if (LoadValueString("CVEdit", RegEntry, this->m_logFont.lfFaceName, LF_FACESIZE) != S_OK)
		strcpy_s(this->m_logFont.lfFaceName, bakupFaceName);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontWeight");
	this->m_logFont.lfWeight = LoadValueIntWithDefault("CVEdit", RegEntry, this->m_logFont.lfWeight);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontItalic");
	this->m_logFont.lfItalic = LoadValueIntWithDefault("CVEdit", RegEntry, this->m_logFont.lfItalic);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontUnderline");
	this->m_logFont.lfUnderline = LoadValueIntWithDefault("CVEdit", RegEntry, this->m_logFont.lfUnderline);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontStrike");
	this->m_logFont.lfStrikeOut = LoadValueIntWithDefault("CVEdit", RegEntry, this->m_logFont.lfStrikeOut);
}

void CVPrefrence::SetPrefsToReg()
{
	char RegEntry[33];
	strcpy_s(RegEntry, this->szRegName);
	SaveValueBool("CVEdit", RegEntry, this->m_highlight);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_color");
	SaveValueInt("CVEdit", RegEntry, this->m_rgb);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontPointSize");
	SaveValueInt("CVEdit", RegEntry, this->m_pointSize);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_Font");
	SaveValueString("CVEdit", RegEntry, this->m_logFont.lfFaceName);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontWeight");
	SaveValueInt("CVEdit", RegEntry, this->m_logFont.lfWeight);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontItalic");
	SaveValueInt("CVEdit", RegEntry, this->m_logFont.lfItalic);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontUnderline");
	SaveValueInt("CVEdit", RegEntry, this->m_logFont.lfUnderline);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontStrike");
	SaveValueInt("CVEdit", RegEntry, this->m_logFont.lfStrikeOut);
}

void CVPrefrence::SetDefaultFont(const HWND hwndDlg)
{
	LOGFONT* const plfont = &this->m_logFont;
	memset(&this->m_logFont, 0, sizeof(LOGFONT));
	HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	if (hFont == NULL)
		hFont = (HFONT)GetStockObject(SYSTEM_FONT);
	GetObject(hFont, sizeof(LOGFONT), plfont);
	this->m_pointSize = 10;
	this->GetHeightFromPointSize(hwndDlg);
}

int CVPrefrence::GetHeightFromPointSize(const HWND hwndDlg)
{
	const HDC hdc = GetDC(hwndDlg);
	const int Height = -MulDiv(this->m_pointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(hwndDlg, hdc);
	return Height;
}

void CVPrefrence::ApplyPreferences(const HWND hwndScin, const CVPrefrence* DefaultPref)
{
	const int id = this->m_sciKeywordID;
	const bool HL = this->m_highlight;
	SendMessage(hwndScin, SCI_STYLESETFORE, id, HL ? (LPARAM)this->m_rgb : (LPARAM)DefaultPref->m_rgb);
	SendMessage(hwndScin, SCI_STYLESETFONT, id, HL ? (LPARAM)this->m_logFont.lfFaceName : (LPARAM)DefaultPref->m_logFont.lfFaceName);
	SendMessage(hwndScin, SCI_STYLESETSIZE, id, HL ? (LPARAM)(this->m_pointSize) : (LPARAM)DefaultPref->m_pointSize);
	SendMessage(hwndScin, SCI_STYLESETWEIGHT, id, HL ? (LPARAM)this->m_logFont.lfWeight : (LPARAM)DefaultPref->m_logFont.lfWeight);
	SendMessage(hwndScin, SCI_STYLESETITALIC, id, HL ? (LPARAM)this->m_logFont.lfItalic : (LPARAM)DefaultPref->m_logFont.lfItalic);
	SendMessage(hwndScin, SCI_STYLESETUNDERLINE, id, HL ? (LPARAM)this->m_logFont.lfUnderline : (LPARAM)DefaultPref->m_logFont.lfUnderline);
	// There is no strike through in Scintilla (yet!)
}

CVPrefrence::~CVPrefrence()
{
	//everything should be automatically destroyed.
}
