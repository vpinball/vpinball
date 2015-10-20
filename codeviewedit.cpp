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
