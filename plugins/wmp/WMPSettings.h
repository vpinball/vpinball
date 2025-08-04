#pragma once

#include "common.h"

namespace WMP {

class WMPCore;

class WMPSettings
{
public:
   WMPSettings(WMPCore* pCore);
   ~WMPSettings();

   PSC_IMPLEMENT_REFCOUNT()

   bool GetAutoStart() const;
   void SetAutoStart(bool autoStart);
   bool GetMute() const;
   void SetMute(bool mute);
   long GetVolume() const;
   void SetVolume(long volume);
   double GetRate() const { PSC_NOT_IMPLEMENTED("GetRate is not implemented"); return 1.0; }
   void SetRate(double rate) { PSC_NOT_IMPLEMENTED("SetRate is not implemented"); }
   long GetBalance() const { PSC_NOT_IMPLEMENTED("GetBalance is not implemented"); return 0; }
   void SetBalance(long balance) { PSC_NOT_IMPLEMENTED("SetBalance is not implemented"); }
   long GetPlayCount() const { PSC_NOT_IMPLEMENTED("GetPlayCount is not implemented"); return 1; }
   void SetPlayCount(long count) { PSC_NOT_IMPLEMENTED("SetPlayCount is not implemented"); }
   bool GetIsAvailable(const string& item) const { PSC_NOT_IMPLEMENTED("GetIsAvailable is not implemented"); return false; }
   string GetBaseURL() const { PSC_NOT_IMPLEMENTED("GetBaseURL is not implemented"); return string(); }
   void SetBaseURL(const string& url) { PSC_NOT_IMPLEMENTED("SetBaseURL is not implemented"); }
   string GetDefaultFrame() const { PSC_NOT_IMPLEMENTED("GetDefaultFrame is not implemented"); return string(); }
   void SetDefaultFrame(const string& frame) { PSC_NOT_IMPLEMENTED("SetDefaultFrame is not implemented"); }
   bool GetInvokeURLs() const { PSC_NOT_IMPLEMENTED("GetInvokeURLs is not implemented"); return false; }
   void SetInvokeURLs(bool invoke) { PSC_NOT_IMPLEMENTED("SetInvokeURLs is not implemented"); }
   bool GetMode(const string& mode) const { PSC_NOT_IMPLEMENTED("GetMode is not implemented"); return false; }
   void SetMode(const string& mode, bool value) { PSC_NOT_IMPLEMENTED("SetMode is not implemented"); }
   bool GetEnableErrorDialogs() const { PSC_NOT_IMPLEMENTED("GetEnableErrorDialogs is not implemented"); return false; }
   void SetEnableErrorDialogs(bool enable) { PSC_NOT_IMPLEMENTED("SetEnableErrorDialogs is not implemented"); }

private:
   WMPCore* m_pCore;
};

}
