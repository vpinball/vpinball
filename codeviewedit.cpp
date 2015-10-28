#include "stdafx.h"
#include "codeviewedit.h"
UserData::UserData()
{
	intLineNum=0;
	strDescription="";
	strKeyName="";
}

UserData::~UserData()
{
}

UserData::UserData(const int LineNo, const string &Desc, const string &Name)
{
	intLineNum=LineNo;
	strDescription=Desc;
	strKeyName=Name;
}

string UserData::lowerCase(string input)
{
   for (string::iterator it = input.begin(); it != input.end(); ++it)
      *it = tolower(*it);
   return input;
}

bool UserData::FuncCompareUD (const UserData &first, const UserData &second)
{
  const string strF = lowerCase(first.strKeyName);
  const string strS = lowerCase(second.strKeyName);
  basic_string <char>::size_type i=0;
  while ( (i<strF.length()) && (i<strS.length() ) )
  {
	  if (strF[i]<strS[i]) return true;
	  else if (strF[i]>strS[i]) return false;
    ++i;
  }
  return ( strF.length() < strF.length() );
}

/*	FindUD - Binary Search.
0  =Found & UDiterOut set to point at UD in list.
-1 =Not Found - Insert point before UDiterOut
1  =Not Found - Insert point after UDiterOut
-2 =error*/
int UserData::FindUD(vector<UserData>* ListIn, const string &strIn, vector<UserData>::iterator &UDiterOut)
{
	int result = -2;
	if (ListIn && (strIn.size() > 2) )// Sanity chq.
	{
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
		do
		{
			iCurPos = iNewPos;
			if (iCurPos >= ListSize) { result = -1; }
			else
			{
				const string strTableData = lowerCase(ListIn->at(iCurPos).strKeyName);
				result = strSearchData.compare(strTableData);
			}
			if (iJumpDelta == 0 || result == 0) break;
			if ( result < 0 )	{ iNewPos = iCurPos - iJumpDelta; }
			else  { iNewPos = iCurPos + iJumpDelta; }
			iJumpDelta >>= 1;
		} while (iNewPos >= 0);
		UDiterOut = ListIn->begin() + iCurPos;
	}
	return result ;
}

//Assumes case insensitive sorted list (found = false):
bool UserData::FindOrInsertUD(vector<UserData>* ListIn,const UserData &udIn)
{
	if (ListIn->size() == 0)	//First in
	{
		ListIn->push_back(udIn);
		return true;
	}
	vector<UserData>::iterator iterFound;
	const int KeyFound = FindUD(ListIn, udIn.strKeyName , iterFound);
	if (KeyFound == 0)return false;//Already Exists.
	if (KeyFound == -1) //insert before, somewhere in the middle
	{
		ListIn->insert(iterFound, udIn);
		return true;
	}
	if (iterFound == ( ListIn->end() - 1) )
	{//insert at end
		ListIn->push_back(udIn);
		return true;
	}

	if (KeyFound == 1) //insert after, somewhere in the middle
	{
		++iterFound;
		ListIn->insert(iterFound, udIn);
		return true;
	}
	return false;//Oh pop poop, never should hit here.
}

bool UserData::FindOrInsertStringIntoAutolist(vector<string>* ListIn,const string strIn)
{
	//First in the list
	if (ListIn->empty())
	{
		ListIn->push_back(strIn);
		return true;
	}
	string strLowerIn = lowerCase(strIn);
	vector<string>::iterator i = ListIn->begin();
	int counter = ListIn->size();
	int result = -1;
	while (counter)
	{
		const string strLowerComp = lowerCase(string(i->data()));
		result = strLowerComp.compare(strLowerIn);
		if (result < 0)
		{
			++i;
			counter--;
		}
		else break;
	}
	if (result == 0) return false;//Already Exists.
	if (result > 0 && (counter != 0))//Add new ud somewhere in middle.
	{
		ListIn->insert(i,strIn);
		return true;
	}
	if (result > 0 && (counter == 0))	//It's new and at the very bottom.
	{
		ListIn->push_back(strIn);
		return true;
	}
	else //it's 1 above the last on the list.
	{
		ListIn->insert(i,strIn);
		return true;
	}
	return false;//Oh pop poop, never should hit here.
}

COLORREF g_crDefaultTextColor = 0;


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
	{this->Highlight = true;}
	else
	{this->Highlight = false;}
}

void CVPrefrence::GetPrefsFromReg()
{
	int intT = 0;
	char RegEntry[33] = {0};
	strcpy_s(RegEntry, this->szRegName);
	this->Highlight = GetRegBoolWithDefault("CVEdit", RegEntry,true);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_color");
	GetRegInt("CVEdit", RegEntry,&intT);
	this->rgb = intT;
}

void CVPrefrence::SetPrefsToReg()
{
	int intT = 0;
	char RegEntry[33] = {0};
	strcpy_s(RegEntry, this->szRegName);
	SetRegValueBool("CVEdit", RegEntry, this->Highlight);
	ZeroMemory(RegEntry, 33);
	strcpy_s(RegEntry, this->szRegName);
	strcat_s(RegEntry, "_color");
	intT = this->rgb;
	SetRegValueInt("CVEdit", RegEntry,intT);
}

void CVPrefrence::SetColorText(const HWND &hwndScin)
{
	SendMessage(hwndScin, SCI_STYLESETFORE, this->SciKeywordID,this->Highlight ? this->rgb : g_crDefaultTextColor);
}

void CVPrefrence::SetDefaultFont()
{
	LOGFONT* plfont = &this->PrefLogFont;
	ZeroMemory(plfont, sizeof(LOGFONT) );
	HFONT hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	if (hFont == NULL)
		hFont = (HFONT) GetStockObject(SYSTEM_FONT);
	GetObject(hFont, sizeof(LOGFONT), plfont);
	//StrCpy(plfont->lfFaceName, "Courier");
	//plfont->lfHeight = 10;
}

void CVPrefrence::ApplyFontScin(const HWND &hwndScin)
{
	SendMessage(hwndScin, SCI_STYLESETFONT, this->SciKeywordID, (LPARAM)this->PrefLogFont.lfFaceName );
	SendMessage(hwndScin, SCI_STYLESETSIZE, this->SciKeywordID, (ULONG)(this->PrefLogFont.lfHeight) );
	SendMessage(hwndScin, SCI_STYLESETWEIGHT, this->SciKeywordID, (LPARAM)this->PrefLogFont.lfWeight );
	SendMessage(hwndScin, SCI_STYLESETITALIC, this->SciKeywordID, (LPARAM)this->PrefLogFont.lfItalic );
	SendMessage(hwndScin, SCI_STYLESETUNDERLINE, this->SciKeywordID, (LPARAM)this->PrefLogFont.lfUnderline );
}

CVPrefrence::~CVPrefrence()
{
	//everything automatically detroyed 2260
}

