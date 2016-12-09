#include "stdafx.h"
#include "codeviewedit.h"
UserData::UserData()
{
	LineNum=0;
	Description="";
	KeyName="";
	eTyping = eUnknown;
	UniqueParent = "";
	Comment="";

}

UserData::UserData(const int LineNo, const string &Desc, const string &Name, const WordType &TypeIn)
{
	LineNum=LineNo;
	Description=Desc;
	KeyName=Name;
	eTyping = TypeIn;
}

UserData::~UserData()
{
}

string UserData::lowerCase(string input)
{
   for (string::iterator it = input.begin(); it != input.end(); ++it)
      *it = tolower(*it);
   return input;
}

void UserData::RemovePadding(string &line)
{
	const size_t LL = line.length();
	size_t Pos = (line.find_first_not_of("\n\r\t ,"));
	if (Pos == -1)
	{
		line.clear();
		return;
	}
	if (Pos > 0)
	{
		if ( (LL-Pos) < 1 ) return;
		line = line.substr(Pos, (LL-Pos) );
	}

	Pos =  (line.find_last_not_of("\n\r\t ,"));
	if (Pos != -1)
	{
		if ( Pos < 1 ) return;
		line = line.erase(Pos+1);
	}
}

/*	FindUD - Now a human Search!
0  =Found & UDiterOut set to point at UD in list.
-1 =Not Found 
1  =Not Found
-2 =Zero Length string or error*/
int UserData::FindUD(vector<UserData>* ListIn, string &strIn, vector<UserData>::iterator& UDiterOut, int &Pos)
{
	int result = -2;
	RemovePadding(strIn);
	if (strIn.size() == 0 || (!ListIn) ) return -2;

	Pos = -1;
	const int KeyResult = FindUDbyKey(ListIn, strIn, UDiterOut, Pos);
	//If it's a top level construct it will have no parents and therefore have a unique key.
	if (KeyResult == 0) return 0;

	//Now see if it's in the Name list
	//Jumpdelta should be intalised to the maximum count of am individual KeyName
	//But for the momment the biggest is 64 x's in AMH
	int iNewPos = Pos + KeyResult;//Start Very close to the result of key search
	if (iNewPos < 0) iNewPos = 0;
	//Find the start of other instances of strIn by crawling up list
	//Usually (but not always) FindUDbyKey returns top of the list so its fast
	const string strSearchData = lowerCase(strIn);
	string strTableData = "";
	const size_t SearchWidth = strSearchData.size();
	while (true)
	{
		iNewPos-- ;
		if (iNewPos < 0) break;
		strTableData = lowerCase(ListIn->at(iNewPos).UniqueKey).substr(0, SearchWidth);
		if (strSearchData.compare(strTableData) != 0) break;
	}
	++iNewPos;
	// now walk down list of Keynames looking for what we want.
	while (true)
	{
		strTableData = lowerCase(ListIn->at(iNewPos).KeyName);
		result = strSearchData.compare(strTableData); 
		if (result == 0) break; //Found
		++iNewPos;
		if (iNewPos == ListIn->size() ) break;
		strTableData = lowerCase(ListIn->at(iNewPos).KeyName).substr(0, SearchWidth);
		result = strSearchData.compare(strTableData);
		if (result != 0) break;	//EO SubList
	}
	UDiterOut = ListIn->begin() + iNewPos;
	Pos = iNewPos;
	return result ;
}

//Finds the closest UD from CurrentLine in ListIn
//On entry CurrentIdx must be set to the UD in the line
int UserData::FindClosestUD(vector<UserData>* ListIn, const int CurrentLine, const int CurrentIdx)
{
	const string strSearchData = lowerCase(ListIn->at(CurrentIdx).KeyName);
	const size_t SearchWidth = strSearchData.size();
	//Find the start of other instances of strIn by crawling up list
	int iNewPos = CurrentIdx;
	string strTableData = "";
	while (true)
	{
		iNewPos-- ;
		if (iNewPos < 0) break;
		strTableData = lowerCase(ListIn->at(iNewPos).UniqueKey).substr(0, SearchWidth);
		if (strSearchData.compare(strTableData) != 0) break;
	}
	++iNewPos;
	//Now at top of list
	//find nearest definition above current line
	int ClosestLineNum = 0;
	int ClosestPos = CurrentIdx;
	int Delta = - (INT_MAX - 1);
	while (true)
	{
		const int NewLineNum = ListIn->at(iNewPos).LineNum;
		const int NewDelta = NewLineNum - CurrentLine;
		if (NewDelta >= Delta && NewLineNum <= CurrentLine)
		{
			if (lowerCase(ListIn->at(iNewPos).KeyName).compare(strSearchData) == 0)
			{
				Delta = NewDelta;
				ClosestLineNum = NewLineNum;
				ClosestPos = iNewPos;
			}
		}
		++iNewPos;
		if (iNewPos == ListIn->size() ) break;
		strTableData = lowerCase(ListIn->at(iNewPos).KeyName).substr(0, SearchWidth);
		if (strSearchData.compare(strTableData) != 0) break;
	}
	--iNewPos;
	return ClosestPos;
}

int UserData::FindUDbyKey(vector<UserData>* ListIn, const string &strIn, vector<UserData>::iterator &UDiterOut, int &PosOut )
{
	int result = -2;
	if (ListIn && (strIn.size() > 0) )// Sanity chq.
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
		const string strSearchData = lowerCase( strIn );
		while (true)
		{
			iCurPos = iNewPos;
			if (iCurPos >= ListSize) { result = -1; }
			else
			{
				const string strTableData = lowerCase(ListIn->at(iCurPos).UniqueKey);
				result = strSearchData.compare(strTableData);
			}
			if (iJumpDelta == 0 || result == 0) break;
			if ( result < 0 )	{ iNewPos = iCurPos - iJumpDelta; }
			else  { iNewPos = iCurPos + iJumpDelta; }
			iJumpDelta >>= 1;
		} 
		UDiterOut = ListIn->begin() + iCurPos;
		PosOut = iCurPos;
	}
	return result;
}

//Returns current Index of strIn in ListIn based on UniqueKey, or -1 if not found
int UserData::UDKeyIndex(vector<UserData>* ListIn, const string &strIn)
{
	if ( (!ListIn) || (strIn.size() <= 0) ) return -1;
	int result = -2;
	const unsigned int ListSize = (int)ListIn->size();
	UINT32 iCurPos = (ListSize >> 1);
	UINT32 iNewPos = 1u << 30;
	while ( (!(iNewPos & ListSize)) && (iNewPos > 1) )
   {
      iNewPos >>= 1;
   }
	int iJumpDelta = ((iNewPos) >> 1);
	--iNewPos;//Zero Base
	const string strSearchData = lowerCase( strIn );
	while (true)
	{
		iCurPos = iNewPos;
		if (iCurPos >= ListSize) { result = -1; }
		else
		{
			const string strTableData = lowerCase(ListIn->at(iCurPos).UniqueKey);
			result = strSearchData.compare(strTableData);
		}
		if (iJumpDelta == 0 || result == 0) break;
		if ( result < 0 )	{ iNewPos = iCurPos - iJumpDelta; }
		else  { iNewPos = iCurPos + iJumpDelta; }
		iJumpDelta >>= 1;
	} 
	///TODO: neads to consider children?
	if (result == 0)
		return iCurPos;
	else
		return -1;
}

//Returns current Index of strIn in ListIn based on KeyName, or -1 if not found
int UserData::UDIndex(vector<UserData>* ListIn, const string &strIn)
{
	if ( (!ListIn) || (strIn.size() <= 0) ) return -1;
	int result = -2;
	const unsigned int ListSize = (int)ListIn->size();
	UINT32 iCurPos = (ListSize >> 1);
	UINT32 iNewPos = 1u << 30;
	while ( (!(iNewPos & ListSize)) && (iNewPos > 1) )
   {
      iNewPos >>= 1;
   }
	int iJumpDelta = ((iNewPos) >> 1);
	--iNewPos;//Zero Base
	const string strSearchData = lowerCase( strIn );
	while (true)
	{
		iCurPos = iNewPos;
		if (iCurPos >= ListSize) { result = -1; }
		else
		{
			const string strTableData = lowerCase(ListIn->at(iCurPos).KeyName);
			result = strSearchData.compare(strTableData);
		}
		if (iJumpDelta == 0 || result == 0) break;
		if ( result < 0 )	{ iNewPos = iCurPos - iJumpDelta; }
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
	while ( (RetVal.eTyping == eUnknown) && (i < ListSize) )
	{
		if (UniKey == ListIn->at(i).UniqueKey)
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
		if (UniKey == ListIn->at(i).UniqueKey)
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
	const int KeyFound = FindUDbyKey(ListIn, udIn.UniqueKey, iterFound, Pos);
	if (KeyFound == 0)
	{
		//Same name, different parents.
		const int ParentResult = udIn.UniqueParent.compare(iterFound->UniqueParent);
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
		if (iterFound == ( ListIn->end() - 1) )
		{//insert at end
			ListIn->push_back(udIn);
			return ListIn->size() -1;//Zero Base
		}
	}
	return -1;
}

bool UserData::FindOrInsertStringIntoAutolist(vector<string>* ListIn,const string &strIn)
{
	//First in the list
	if (ListIn->empty())
	{
		ListIn->push_back(strIn);
		return true;
	}
	vector<string>::iterator i = ListIn->begin();
	int result = -2;
	const unsigned int ListSize = (int)ListIn->size();
	UINT32 iCurPos = (ListSize >> 1);
	UINT32 iNewPos = 1u << 31;
	while ((!(iNewPos & ListSize)) && (iNewPos > 1))
   {
      iNewPos >>= 1;
   }
	int iJumpDelta = ((iNewPos) >> 1);
	--iNewPos;//Zero Base
	const string strSearchData = lowerCase( strIn );
	while (true)
	{
		iCurPos = iNewPos;
		if (iCurPos >= ListSize) { result = -1; }
		else
		{
			const string strTableData = lowerCase(ListIn->at(iCurPos) );
			result = strSearchData.compare(strTableData);
		}
		if (iJumpDelta == 0 || result == 0) break;
		if ( result < 0 )	{ iNewPos = iCurPos - iJumpDelta; }
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

	if (i == ( ListIn->end() - 1) )//insert Above last element - Special case
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

	if (i == ( ListIn->end() - 1) )//insert Above last element - Special case
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
		rgb = 0;
		Highlight = false;
		szRegName = nullptr;
		SciKeywordID = 0;
}

CVPrefrence* CVPrefrence::FillCVPreference( \
		const char* szCtrlNameIn,const COLORREF &crTextColor, \
		const bool &bDisplay, const char* szRegistryName, \
		const int &szScintillaKeyword, const int &IDC_ChkBox, \
		const int &IDC_ColorBut, const int &IDC_Font)
{
	szControlName = szCtrlNameIn;
	rgb = crTextColor;
	Highlight = bDisplay;
	szRegName = szRegistryName;
	SciKeywordID = szScintillaKeyword;
	IDC_ChkBox_code = IDC_ChkBox;
	IDC_ColorBut_code = IDC_ColorBut;
	IDC_Font_code = IDC_Font;
	return (CVPrefrence *)this;
}

void CVPrefrence::SetCheckBox(const HWND &hwndDlg)
{
	const HWND hChkBox = GetDlgItem(hwndDlg,this->IDC_ChkBox_code);
	SNDMSG(hChkBox, BM_SETCHECK, this->Highlight ? BST_CHECKED : BST_UNCHECKED, 0L);
}

void CVPrefrence::ReadCheckBox(const HWND &hwndDlg)
{
	if(IsDlgButtonChecked(hwndDlg,this->IDC_ChkBox_code))
		this->Highlight = true;
	else
		this->Highlight = false;
}

void CVPrefrence::GetPrefsFromReg()
{
	char RegEntry[33] = {0};
	strcpy_s(RegEntry, this->szRegName);
	this->Highlight = GetRegBoolWithDefault("CVEdit", RegEntry, this->Highlight );
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_color");
	this->rgb = GetRegIntWithDefault("CVEdit", RegEntry,this->rgb);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontPointSize");
	this->PointSize = GetRegIntWithDefault ( "CVEdit", RegEntry, this->PointSize);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_Font");
	GetRegString( "CVEdit", RegEntry, this->LogFont.lfFaceName, 32);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontWeight");
	this->LogFont.lfWeight = GetRegIntWithDefault( "CVEdit", RegEntry, this->LogFont.lfWeight);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontItalic");
	this->LogFont.lfItalic = GetRegIntWithDefault( "CVEdit", RegEntry, this->LogFont.lfItalic);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontUnderline");
	this->LogFont.lfUnderline = GetRegIntWithDefault( "CVEdit", RegEntry, this->LogFont.lfUnderline);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontStrike");
	this->LogFont.lfStrikeOut = GetRegIntWithDefault( "CVEdit", RegEntry, this->LogFont.lfStrikeOut);
}

void CVPrefrence::SetPrefsToReg()
{
	char RegEntry[33] = {0};
	strcpy_s(RegEntry, this->szRegName);
	SetRegValueBool("CVEdit", RegEntry, this->Highlight);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_color");
	SetRegValueInt("CVEdit", RegEntry,this->rgb);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontPointSize");
	SetRegValueInt( "CVEdit", RegEntry, this->PointSize);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_Font");
	SetRegValueString( "CVEdit", RegEntry, this->LogFont.lfFaceName);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontWeight");
	SetRegValueInt( "CVEdit", RegEntry, this->LogFont.lfWeight);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontItalic");
	SetRegValueInt( "CVEdit", RegEntry, this->LogFont.lfItalic);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontUnderline");
	SetRegValueInt( "CVEdit", RegEntry, this->LogFont.lfUnderline);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_FontStrike");
	SetRegValueInt( "CVEdit", RegEntry, this->LogFont.lfStrikeOut);
}

void CVPrefrence::SetDefaultFont(const HWND &hwndDlg)
{
	LOGFONT* const plfont = &this->LogFont;
	memset(&this->LogFont, 0, sizeof(LOGFONT) );
	HFONT hFont = (HFONT) GetStockObject(ANSI_FIXED_FONT);
	if (hFont == NULL)
		hFont = (HFONT) GetStockObject(SYSTEM_FONT);
	GetObject(hFont, sizeof(LOGFONT), plfont);
	this->PointSize = 10;
	this->GetHeightFromPointSize(hwndDlg);
}

int CVPrefrence::GetHeightFromPointSize(const HWND &hwndDlg)
{
	const HDC hdc = GetDC(hwndDlg);
	const int Height = -MulDiv(this->PointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(hwndDlg,hdc);
	return Height;
}

void CVPrefrence::ApplyPreferences(const HWND &hwndScin,const CVPrefrence* DefaultPref)
{
	const int id = this->SciKeywordID;
	const bool HL = this->Highlight;
	SendMessage(hwndScin, SCI_STYLESETFORE, id, HL ? this->rgb : DefaultPref->rgb);
	SendMessage(hwndScin, SCI_STYLESETFONT, id, HL ? (LPARAM)this->LogFont.lfFaceName : (LPARAM)DefaultPref->LogFont.lfFaceName);
	SendMessage(hwndScin, SCI_STYLESETSIZE, id, HL ? (ULONG)(this->PointSize) : (LPARAM)DefaultPref->PointSize);
	SendMessage(hwndScin, SCI_STYLESETWEIGHT, id, HL ? (LPARAM)this->LogFont.lfWeight : (LPARAM)DefaultPref->LogFont.lfWeight);
	SendMessage(hwndScin, SCI_STYLESETITALIC, id, HL ? (LPARAM)this->LogFont.lfItalic : (LPARAM)DefaultPref->LogFont.lfItalic);
	SendMessage(hwndScin, SCI_STYLESETUNDERLINE, id, HL ? (LPARAM)this->LogFont.lfUnderline : (LPARAM)DefaultPref->LogFont.lfUnderline);
	// There is no strike through in Scintilla (yet!)
}

CVPrefrence::~CVPrefrence()
{
	//everything should be automatically detroyed.
}
