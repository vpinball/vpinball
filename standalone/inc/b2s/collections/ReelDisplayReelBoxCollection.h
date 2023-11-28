#pragma once

#include <map>

class B2SReelBox;

class ReelDisplayReelBoxCollection : public std::map<int, B2SReelBox*>
{
public:
   ReelDisplayReelBoxCollection();

   void Add(int key, B2SReelBox* pReelbox);

   bool IsLED() const { return m_led; }
   void SetLED(bool led) { m_led = led; }

private:
   bool m_led;
};