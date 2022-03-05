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
   ~UserData() {}
};

bool FindOrInsertStringIntoAutolist(vector<string>* const ListIn, const string& strIn);
size_t FindOrInsertUD(vector<UserData>* const ListIn, const UserData& udIn);
int FindUD(vector<UserData>* const ListIn, string& strIn, vector<UserData>::iterator& UDiterOut, int& Pos);
int FindClosestUD(const vector<UserData>* const ListIn, const int CurrentLine, const int CurrentIdx);
int UDKeyIndex(const vector<UserData>* const ListIn, const string& strIn);
int UDIndex(const vector<UserData>* const ListIn, const string& strIn);
UserData GetUDfromUniqueKey(const vector<UserData>* const ListIn, const string& UniKey);
size_t GetUDPointerfromUniqueKey(const vector<UserData>* const ListIn, const string& UniKey);


// CodeViewer Preferences
class CVPreference
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
   string m_szRegName;
   bool m_highlight;

public:
   CVPreference(
      const COLORREF crTextColor,
      const bool bDisplay, const string& szRegistryName,
      const int szScintillaKeyword, const int IDC_ChkBox,
      const int IDC_ColorBut, const int IDC_Font);
   ~CVPreference() {}

   void GetPrefsFromReg();
   void SetPrefsToReg();
   void SetCheckBox(const HWND hwndDlg);
   void ReadCheckBox(const HWND hwndDlg);
   void SetDefaultFont(const HWND hwndDlg);
   int GetHeightFromPointSize(const HWND hwndDlg);
   void ApplyPreferences(const HWND hwndScin, const CVPreference* DefaultPref);
};
