#pragma once


namespace B2SLegacy {

class EntryAction final
{
public:
   EntryAction(const vector<string>& bulbs, int intervalMultiplier, bool visible, int corrector, int pulseSwitch);

   const vector<string>* GetBulbs() const { return &m_bulbs; }
   int GetIntervalMultiplier() const { return m_intervalMultiplier; }
   bool IsVisible() const { return m_visible; }
   int GetCorrector() const { return m_corrector; }
   int GetPulseSwitch() const { return m_pulseSwitch; }

private:
   vector<string> m_bulbs;
   int m_intervalMultiplier = 0;
   bool m_visible = false;
   int m_corrector = 0;
   int m_pulseSwitch = 0;
};

}
