#ifndef _CVEDIT_
#define _CVEDIT_
#include "stdafx.h"
#include "regutil.h"
class UserData
{
public:
	string strKeyName; //Unique Name
	string strDescription;//Brief Description
	int intLineNum; //Line No. Declaration

	UserData();
	UserData(const int LineNo, const string &Desc, const string &Name);
	bool FuncCompareUD (const UserData &first, const UserData &second);
	string lowerCase(string input);
	bool FindOrInsertStringIntoAutolist(vector<string>* ListIn, string strIn);
	bool FindOrInsertUD( vector<UserData>* ListIn,const UserData& udIn);
	int FindUD(vector<UserData>* ListIn, const string &strIn,vector<UserData>::iterator& UDiterOut);
	~UserData();
};
class CVPrefrence;
// CodeViewer Preferences 
class CVPrefrence
{
public:
	LOGFONT PrefLogFont;
	const char* szControlName;
	COLORREF rgb ;
	bool Highlight;
	const char *szRegName;
	int SciKeywordID;
	int IDC_ChkBox_code;
	int IDC_ColorBut_code;
	int IDC_Font_code;
	CVPrefrence();
	CVPrefrence* FillCVPreference( \
		const char* szCtrlNameIn,const COLORREF &crTextColor, \
		const bool &bDisplay, const char* szRegistryName, \
		const int &szScintillaKeyword, const int &IDC_ChkBox, \
		const int &IDC_ColorBut, const int &IDC_Font);
	void GetPrefsFromReg();
	void SetPrefsToReg();
	void SetColorText(const HWND &hwndScin);
	void SetCheckBox(const HWND &hwndDlg);
	void ReadCheckBox(const HWND &hwndDlg);
	void SetDefaultFont();
	void ApplyFontScin(const HWND &hwndScin);
	~CVPrefrence();
};

#endif //_CVEDIT_