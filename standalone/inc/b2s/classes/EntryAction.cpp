#include "core/stdafx.h"

#include "EntryAction.h"

EntryAction::EntryAction(const vector<string>& bulbs, int intervalMultiplier, bool visible, int corrector, int pulseSwitch)
{
   m_bulbs = bulbs;
   m_intervalMultiplier = intervalMultiplier;
   m_visible = visible;
   m_corrector = corrector;
   m_pulseSwitch = pulseSwitch;
};