#pragma once

#include <map>


namespace B2SLegacy {

class B2SReelBox;

class ReelDisplayReelBoxCollection final : public std::map<int, B2SReelBox*>
{
public:
   ReelDisplayReelBoxCollection();

   void Add(int key, B2SReelBox* pReelbox);

   bool IsLED() const { return m_led; }
   void SetLED(bool led) { m_led = led; }

private:
   bool m_led;
};

}
