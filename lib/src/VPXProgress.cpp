// license:GPLv3+

#include "VPXProgress.h"

#include "VPinballLib.h"

#include <algorithm>


void VPXProgress::SetProgress(unsigned int progress)
{
   m_progress = progress;
   VPinballLib::ProgressData progressData = { 100u * std::min(m_progress, m_total) / std::max(1u, m_total) };
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_LOADING, &progressData);
}

void VPXProgress::SetLength(unsigned int length)
{
   m_total = length;
   VPinballLib::ProgressData progressData = { 100u * std::min(m_progress, m_total) / std::max(1u, m_total) };
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_LOADING, &progressData);
}
