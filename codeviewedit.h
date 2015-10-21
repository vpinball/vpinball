#ifndef _CVEDIT_
#define _CVEDIT_
#include "stdafx.h"
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

// CodeViewer Preferences

class CVPrefs
{
public:
	COLORREF rgbDefaultText;
	struct CVControl
	{
		COLORREF rgb;
		bool b;
	};
public:
	COLORREF CVColorControl (CVControl x)	{ if(x.b) return x.rgb; else return rgbDefaultText; }; 
};

#endif