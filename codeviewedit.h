#ifndef _CVEDIT_
#define _CVEDIT_

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
	~UserData();
};

// CodeViewer Preferences

class CVPrefs
{
public:
	COLORREF rgbDefaultText;
	struct ControlBut
	{
		COLORREF rgb;
		bool b;
	};
public:
	COLORREF ColorBut (ControlBut x)	{ if(x.b) return x.rgb; else return rgbDefaultText; }; 
};

#endif