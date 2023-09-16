#pragma once

#include <map>

class ControlInfo;

class ControlCollection : public vector<ControlInfo*>
{
public:
   void Add(ControlInfo* pItem);
   int GetScore() const { return m_score; }
   void SetScore(int score);

private:
   int m_digits;
   int m_score;
};