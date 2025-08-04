#pragma once

#include <map>


namespace B2SLegacy {

class ControlInfo;
class B2SData;

class ControlCollection final : public vector<ControlInfo*>
{
public:
   void Add(ControlInfo* pControlInfo);
   int GetScore() const { return m_score; }
   void SetScore(B2SData* pB2SData, int score);

private:
   int m_digits;
   int m_score;
};

}
