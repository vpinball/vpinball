#pragma once

class AnimationInfo
{
public:
   AnimationInfo(const string& szName, bool inverted);
  
   const string& GetName() const { return m_szName; }
   void SetName(const string& szName) { m_szName = szName; }
   bool IsInverted() const { return m_inverted; }
   void SetInverted(bool inverted) { m_inverted = inverted; }

private:
   string m_szName;
   bool m_inverted;
};