// license:GPLv3+

#pragma once

#include "Settings.h"
#include "FileLocator.h"


class VPApp final
{
public:
   VPApp();
   ~VPApp();

   void SetSettingsFileName(const std::filesystem::path& path) { m_iniFileName = path; } // Must be defined before InitInstance() is called, otherwise it will be ignored
   void InitInstance();

   // overall app settings
   Settings m_settings;

   FileLocator m_fileLocator;

   void LimitMultiThreading();
   int GetLogicalNumberOfProcessors() const;

   // Global custom parameters that can be set through command line and accessed in the script via GetCustomParam(X)
   wstring m_customParameters[MAX_CUSTOM_PARAM_INDEX];

   // FIXME Deprecated command line options (supposed to be handled through INI nowadays)
   int m_disEnableTrueFullscreen = -1;
   bool m_bgles = false; // override global emission scale by m_fgles below
   float m_fgles = 0.f;

   // Script security level
   int m_securitylevel;

#ifndef __STANDALONE__
   static CComModule m_module;
   class WinApp final : public CWinApp
   {
   public:
      WinApp() = default;
   protected:
      BOOL OnIdle(LONG) override;
      BOOL PreTranslateMessage(MSG& msg) override;
   } m_winApp;
   HINSTANCE GetInstanceHandle() const { return m_winApp.GetInstanceHandle(); }
#else
   HINSTANCE GetInstanceHandle() const { return nullptr; }
#endif

private:
   std::filesystem::path m_iniFileName; // Override default ini filename, must be defined before InitInstance
   int m_logicalNumberOfProcessors = -1;
};
