#pragma once

#include "../b2s_i.h"

class PictureBoxAnimationEntry
{
public:
   PictureBoxAnimationEntry(string on1, int waitAfterOn1, string off1, int waitAfterOff1, int pulseSwitch);

   vector<string>* GetOn1() { return &m_on1; }
   vector<string>* GetOff1() { return &m_off1; }
   vector<string>* GetOn2() { return &m_on2; }
   vector<string>* GetOff2() { return &m_off2; }
   int GetWaitAfterOn1() const { return m_waitAfterOn1; }
   int GetWaitAfterOff1() const { return m_waitAfterOff1; }
   int GetWaitAfterOn2() const { return m_waitAfterOn2; }
   int GetWaitAfterOff2() const { return m_waitAfterOff2; }
   int GetPulseSwitch() const { return m_pulseSwitch; }

private:
   vector<string> Check4StringOrArray(const string& str);

   vector<string> m_on1;
   vector<string> m_off1;
   vector<string> m_on2;
   vector<string> m_off2;
   int m_waitAfterOn1;
   int m_waitAfterOff1;
   int m_waitAfterOn2;
   int m_waitAfterOff2;
   int m_pulseSwitch;
};