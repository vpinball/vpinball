#pragma once

class EntryAction
{
public:
   EntryAction(vector<string> bulbs, int intervalMultiplier, bool visible, int corrector, int pulseSwitch);

   vector<string>* GetBulbs() { return &m_bulbs; }
   int GetIntervalMultiplier() const { return m_intervalMultiplier; }
   bool IsVisible() const { return m_visible; }
   int GetCorrector() const { return m_corrector; }
   int GetPulseSwitch() const { return m_pulseSwitch; }

private:   
   vector<string> m_bulbs;
   int m_intervalMultiplier;
   bool m_visible;
   int m_corrector;
   int m_pulseSwitch;
};