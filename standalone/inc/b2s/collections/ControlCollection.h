#pragma once

#include <map>

class ControlInfo;
class B2SData;

class ControlCollection : public vector<ControlInfo*>
{
public:
   void Add(ControlInfo* pItem);
   int GetScore() const { return m_score; }
   void SetScore(B2SData* pB2SData, int score);

private:
   int m_digits;
   int m_score;
};