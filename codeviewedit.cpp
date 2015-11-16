#include "stdafx.h"
#include "codeviewedit.h"
UserData::UserData()
{
	LineNum=0;
	Description="";
	KeyName="";
	eTyping = eUnknown;
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

bool UserData::FuncCompareUD (const UserData &first, const UserData &second)
{
  const string strF = lowerCase(first.KeyName);
  const string strS = lowerCase(second.KeyName);
  basic_string <char>::size_type i=0;
  while ( (i<strF.length()) && (i<strS.length() ) )
  {
	  if (strF[i]<strS[i]) return true;
	  else if (strF[i]>strS[i]) return false;
    ++i;
  }
  return ( strF.length() < strS.length() );
}

/*	FindUD - Binary Search.
0  =Found & UDiterOut set to point at UD in list.
-1 =Not Found - Insert point before UDiterOut
1  =Not Found - Insert point after UDiterOut
-2 =error*/
int UserData::FindUD(vector<UserData>* ListIn, const string &strIn,vector<UserData>::iterator& UDiterOut)
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
				const string strTableData = lowerCase(ListIn->at(iCurPos).KeyName);
				result = strSearchData.compare(strTableData);
			}
			if (iJumpDelta == 0 || result == 0) break;
			if ( result < 0 )	{ iNewPos = iCurPos - iJumpDelta; }
			else  { iNewPos = iCurPos + iJumpDelta; }
			iJumpDelta >>= 1;
		} 
		UDiterOut = ListIn->begin() + iCurPos;
	}
	return result ;
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
	return result ;
}

//Returns current Index of strIn in ListIn, or -1 if not found
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
	/// neads to consider children...
	if (result == 0)
		return iCurPos ;
	else
		return -1;
}

//Assumes case insensitive sorted list
//Returns point of insertion (-1 == error)
int UserData::FindOrInsertUD(vector<UserData>* ListIn, UserData &udIn)
{
	vector<UserData>::iterator iterFound  = ListIn->begin();
	int Pos = 0;
	if (ListIn->size() == 0)	//First in
	{
		ListIn->push_back(udIn);
		return 0;
	}
	const int KeyFound = FindUDbyKey(ListIn, udIn.UniqueKey ,iterFound, Pos);
	//Same name, different parents. 
	if ( (KeyFound == 0) && (ListIn->at(Pos).UniqueParent.compare(udIn.UniqueParent) != 0) )
	{
		ListIn->insert(iterFound, udIn);
		return Pos;
	}

	if ( (KeyFound != 0) ) 
	{
		if (KeyFound == -1) //insert before, somewhere in the middle
		{
			ListIn->insert(iterFound, udIn);
			return Pos;
		}
		else
		{
			if (KeyFound == 1) 
			{
				++iterFound;
				ListIn->insert(iterFound, udIn);
				return ++Pos;
			}
			else
			if (iterFound == ( ListIn->end() - 1) )//insert Above last element - Special case
			{//insert at end
				ListIn->push_back(udIn);
				return ListIn->size() -1;//Zero Base
			}
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
	string strLowerIn = lowerCase(strIn);
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
	LOGFONT* plfont = &this->LogFont;
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
	int Height = -MulDiv(this->PointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
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
