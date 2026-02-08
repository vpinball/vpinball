// license:GPLv3+

#pragma once

#include "Settings.h"
#include "FileLocator.h"

class AppMsgLoop
{
public:
   virtual ~AppMsgLoop() = default;
   virtual void Initialize() = 0;
   virtual bool StepMsgLoop() = 0;
   virtual int MainMsgLoop() = 0;
};

#ifndef __STANDALONE__

class WinMsgLoop final : public CWinApp, public AppMsgLoop
{
public:
   WinMsgLoop();
   ~WinMsgLoop() override = default;
   void Initialize() override;
   bool StepMsgLoop() override;
   int MainMsgLoop() override;

protected:
   BOOL PreTranslateMessage(MSG& msg) override;

private:
   int m_idleIndex = 0;
};

#else

class StandaloneMsgLoop final : public AppMsgLoop
{
public:
   ~StandaloneMsgLoop() override = default;
   void Initialize() override;
   bool StepMsgLoop() override;
   int MainMsgLoop() override;
};

#endif


class VPApp final
{
public:
   VPApp();
   ~VPApp();

   void SetSettingsFileName(const string& path) { m_iniFileName = path; } // Must be defined before InitInstance() is called, otherwise it will be ignored
   void InitInstance();

   std::unique_ptr<AppMsgLoop> m_msgLoop;

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
   HINSTANCE GetInstanceHandle() const { return static_cast<WinMsgLoop*>(m_msgLoop.get())->GetInstanceHandle(); }
#else
   HINSTANCE GetInstanceHandle() const { return nullptr; }
#endif

private:
   string m_iniFileName; // Override default ini filename, must be defined before InitInstance
   int m_logicalNumberOfProcessors = -1;
};
