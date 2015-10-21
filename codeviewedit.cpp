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


