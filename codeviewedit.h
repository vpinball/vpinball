#ifndef _CVEDIT_
#define _CVEDIT_

//#define STYLE_SUBFUN 10
//#define STYLE_COMPONENTS 11
//#define STYLE_VPCORE 12
//#define STYLE_CURRENTWORD 13

enum WordType {eUnknown, eClass, eSub, eFunction, ePropGet, ePropLet, ePropSet, eDim, eConst};

class UserData
{
private:
public:
	string UniqueKey;
	int LineNum; //Line No. Declaration
	string KeyName; //Name
	WordType eTyping;
	string UniqueParent;
	vector<string> Children; //Unique key
	string Description;//Brief Description
	string Comment;

public:
	UserData();
	UserData(const int LineNo, const string &Desc, const string &Name, const WordType &TypeIn);
	string lowerCase(string input);
	bool FindOrInsertStringIntoAutolist(vector<string>* ListIn, const string &strIn);
	size_t FindOrInsertUD( vector<UserData>* ListIn, UserData& udIn);
	int FindUD(vector<UserData>* ListIn, string &strIn,vector<UserData>::iterator& UDiterOut, int &Pos);
	int FindClosestUD(vector<UserData>* ListIn, const int CurrentLine, const int CurrentIdx);
	int FindUDbyKey(vector<UserData>* ListIn, const string &strIn,vector<UserData>::iterator& UDiterOut, int &PosOut);
	int UDKeyIndex(vector<UserData>* ListIn, const string &strIn);
	int UDIndex(vector<UserData>* ListIn, const string &strIn);
	UserData GetUDfromUniqueKey(vector<UserData>* ListIn, const string &UniKey);
	size_t GetUDPointerfromUniqueKey(vector<UserData>* ListIn, const string &UniKey);
	void RemovePadding(string &line);
	~UserData();
};

// CodeViewer Preferences 
class CVPrefrence
{
public:
	const char* szControlName;
	LOGFONT LogFont;
	int PointSize;
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
	void SetCheckBox(const HWND &hwndDlg);
	void ReadCheckBox(const HWND &hwndDlg);
	void SetDefaultFont(const HWND &hwndDlg);
	int GetHeightFromPointSize(const HWND &hwndDlg);
	void ApplyPreferences(const HWND &hwndScin,const CVPrefrence* DefaultPref);
	~CVPrefrence();
};

#endif //_CVEDIT_