#pragma once

#include "common.h"

namespace WMP {

class WMPCore;

class WMPControls
{
public:
   WMPControls(WMPCore* pCore);
   ~WMPControls();

   PSC_IMPLEMENT_REFCOUNT()

   void Play();
   void Stop();
   void Pause();

   double GetCurrentPosition() const;
   void SetCurrentPosition(double position);

   bool GetIsAvailable(const string& item) const { PSC_NOT_IMPLEMENTED("GetIsAvailable is not implemented"); return false; }
   void FastForward() { PSC_NOT_IMPLEMENTED("FastForward is not implemented"); }
   void FastReverse() { PSC_NOT_IMPLEMENTED("FastReverse is not implemented"); }
   string GetCurrentPositionString() const { PSC_NOT_IMPLEMENTED("GetCurrentPositionString is not implemented"); return string(); }
   void Next() { PSC_NOT_IMPLEMENTED("Next is not implemented"); }
   void Previous() { PSC_NOT_IMPLEMENTED("Previous is not implemented"); }

private:
   WMPCore* m_pCore;
};

}
