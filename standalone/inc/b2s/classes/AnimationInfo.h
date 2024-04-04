#pragma once

class AnimationInfo
{
public:
   AnimationInfo(const string& szAnimationName, bool inverted);
  
   const string& GetAnimationName() const { return m_szAnimationName; }
   void SetAnimationName(const string& szAnimationName) { m_szAnimationName = szAnimationName; }
   bool IsInverted() const { return m_inverted; }
   void SetInverted(bool inverted) { m_inverted = inverted; }

private:
   string m_szAnimationName;
   bool m_inverted;
};