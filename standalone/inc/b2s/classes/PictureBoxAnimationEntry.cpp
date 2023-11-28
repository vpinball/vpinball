#include "stdafx.h"

#include "PictureBoxAnimationEntry.h"

PictureBoxAnimationEntry::PictureBoxAnimationEntry(string on1, int waitAfterOn1, string off1, int waitAfterOff1, int pulseSwitch)
{
   m_on1 = Check4StringOrArray(on1);
   m_waitAfterOn1 = waitAfterOn1;
   m_off1 = Check4StringOrArray(off1);
   m_waitAfterOff1 = waitAfterOff1;
   m_pulseSwitch = pulseSwitch;

   m_on2.push_back(string(""));
   m_off2.push_back(string(""));
   m_waitAfterOn2 = 0;
   m_waitAfterOff2 = 0;
}

vector<string> PictureBoxAnimationEntry::Check4StringOrArray(const string& str)
{
   vector<string> ret;
   std::istringstream iss(str);
   for (std::string token; std::getline(iss, token, ','); )
      ret.push_back(token);
   return ret;
}