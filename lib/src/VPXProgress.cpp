// license:GPLv3+

#include "VPXProgress.h"

#include "VPinballLib.h"

void VPXProgress::LoadingProgressUpdated(int loaded, int total)
{
   VPinballLib::ProgressData progressData = { 100 * loaded / total };
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_LOADING, &progressData);
}
