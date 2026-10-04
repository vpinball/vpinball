// license:GPLv3+

#pragma once

#include "FileLocator.h"


class VPApp final
{
public:
   VPApp();
   ~VPApp();

   void SetCommandLineCustomSettingsFileName(const std::filesystem::path& path) { m_commandLineCustomSettingsFileName = path; } // Must be defined before InitInstance() is called, otherwise it will be ignored
   void InitInstance(bool isPlay = false);

   FileLocator m_fileLocator;

   void LimitMultiThreading();
   int GetLogicalNumberOfProcessors() const;

   // Global custom parameters that can be set through command line and accessed in the script via GetCustomParam(X)
   wstring m_customParameters[MAX_CUSTOM_PARAM_INDEX];

#ifndef ENABLE_BGFX
   // FIXME Deprecated command line options (supposed to be handled through INI nowadays)
   bool m_bgles = false; // override global emission scale by m_fgles below
   float m_fgles = 0.f;
#endif

   // Script security level
   int m_securitylevel;

#ifdef VPX_HAS_REGISTERED_TYPELIB
   static CComModule m_module;
#endif
#ifndef __STANDALONE__
   class WinApp final : public CWinApp
   {
   public:
      WinApp() = default;
#ifdef VPX_ENABLE_WIN32_EDITOR
   protected:
      BOOL OnIdle(LONG) override;
      BOOL PreTranslateMessage(MSG& msg) override;
#endif
   } m_winApp;
   HINSTANCE GetInstanceHandle() const { return m_winApp.GetInstanceHandle(); }
#endif

private:
   std::filesystem::path m_commandLineCustomSettingsFileName; // Override default ini filename, must be defined before InitInstance
   int m_logicalNumberOfProcessors = -1;
};
