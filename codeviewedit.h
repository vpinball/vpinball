#pragma once

//#define STYLE_SUBFUN 10
//#define STYLE_COMPONENTS 11
//#define STYLE_VPCORE 12
//#define STYLE_CURRENTWORD 13

enum WordType { eUnknown, eClass, eSub, eFunction, ePropGet, ePropLet, ePropSet, eDim, eConst };

class UserData
{
public:
   string m_uniqueKey;
   int m_lineNum;             // Line No. Declaration
   string m_keyName;          // Name
   WordType eTyping;
   string m_uniqueParent;
   vector<string> m_children; // Unique key
   string m_description;      // Brief Description
   string m_comment;

   UserData();
   UserData(const int LineNo, const string &Desc, const string &Name, const WordType TypeIn);
   ~UserData();

   bool FindOrInsertStringIntoAutolist(vector<string>* ListIn, const string &strIn);
   size_t FindOrInsertUD(vector<UserData>* ListIn, UserData& udIn);
   int FindUD(vector<UserData>* ListIn, string &strIn, vector<UserData>::iterator& UDiterOut, int &Pos);
   int FindClosestUD(vector<UserData>* ListIn, const int CurrentLine, const int CurrentIdx);
   int FindUDbyKey(vector<UserData>* ListIn, const string &strIn, vector<UserData>::iterator& UDiterOut, int &PosOut);
   int UDKeyIndex(vector<UserData>* ListIn, const string &strIn);
   int UDIndex(vector<UserData>* ListIn, const string &strIn);
   UserData GetUDfromUniqueKey(vector<UserData>* ListIn, const string &UniKey);
   size_t GetUDPointerfromUniqueKey(vector<UserData>* ListIn, const string &UniKey);
};

// CodeViewer Preferences 
class CVPrefrence
{
public:
   LOGFONT m_logFont;
   int m_pointSize;
   COLORREF m_rgb;
   int m_sciKeywordID;
   int IDC_ChkBox_code;
   int IDC_ColorBut_code;
   int IDC_Font_code;

private:
   const char* szControlName; // unused
   const char *szRegName;
   bool m_highlight;

public:
   CVPrefrence();
   CVPrefrence* FillCVPreference(
      const char* szCtrlNameIn, const COLORREF crTextColor,
      const bool bDisplay, const char* szRegistryName,
      const int szScintillaKeyword, const int IDC_ChkBox,
      const int IDC_ColorBut, const int IDC_Font);
   ~CVPrefrence();

   void GetPrefsFromReg();
   void SetPrefsToReg();
   void SetCheckBox(const HWND hwndDlg);
   void ReadCheckBox(const HWND hwndDlg);
   void SetDefaultFont(const HWND hwndDlg);
   int GetHeightFromPointSize(const HWND hwndDlg);
   void ApplyPreferences(const HWND hwndScin, const CVPrefrence* DefaultPref);
};
