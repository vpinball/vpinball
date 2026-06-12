#pragma once


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
   int m_digits = 0;
   int m_score = 0;
};

}
