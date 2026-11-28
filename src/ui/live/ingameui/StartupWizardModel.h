// license:GPLv3+

#pragma once

#include "core/Settings.h"
#include "renderer/Window.h"

namespace VPX::InGameUI
{

// UI-independent draft and layout model of the startup wizard. All edits are
// made in a detached Settings copy; Finish is the only operation which writes
// them back to the application settings.
class StartupWizardModel final
{
public:
   enum class MainMode
   {
      Desktop,
      SingleScreen,
      Cabinet,
      VR,
   };
   enum class HostMode
   {
      Desktop,
      SingleScreen,
      Cabinet,
   };

   static constexpr int kStartupWizardVersion = 1;

   StartupWizardModel(const Settings& appSettings, const vector<VPX::Window::DisplayConfig>& displays, bool generatePreset, bool openXRHMDAvailable = false);

   Settings& GetDraft() { return m_draft; }
   const Settings& GetDraft() const { return m_draft; }
   const Settings& GetInitial() const { return m_initial; }
   const vector<VPX::Window::DisplayConfig>& GetDisplays() const { return m_displays; }

   MainMode GetMainMode() const { return m_mainMode; }
   void SetMainMode(MainMode mode);
   HostMode GetVRHostMode() const { return m_vrHostMode; }
   void SetVRHostMode(HostMode mode);
   bool ShowsCabinetStep() const { return m_mainMode == MainMode::Cabinet || m_mainMode == MainMode::VR; }

   bool IsDisplayActive(int index) const;
   bool SetDisplayActive(int index, bool active);
   int GetActiveDisplayCount() const;
   vector<int> GetOrderedActiveDisplays() const;

   void ApplyModePreset();
   void SanitizeLayoutForActiveDisplays();
   bool IsLayoutCustomized() const { return m_layoutCustomized; }
   void SetLayoutCustomized(bool customized) { m_layoutCustomized = customized; }

   int GetDisplayIndex(const string& displayId) const;
   int GetOutputDisplayIndex(VPXWindowId output) const;
   VPXWindowId GetDisplayMainOutput(int displayIndex) const;
   bool AssignOutputToDisplay(VPXWindowId output, int displayIndex);
   void SetOutputEmbedded(VPXWindowId output, bool embedded);

   bool NeedsRestart() const;
   void Finish(Settings& appSettings);
   bool IsFinished() const { return m_finished; }
   bool IsRestartRequired() const { return m_restartRequired; }

   static bool ShouldAutoOpen(const Settings& appSettings);

private:
   HostMode GetEffectiveHostMode() const;
   int SelectPlayfieldDisplay() const;
   vector<int> GetAncillaryDisplayOrder(int playfieldDisplay) const;
   void SetFloatingOutput(VPXWindowId output, int displayIndex);
   void SetEmbeddedOutput(VPXWindowId output, int x, int y, int width, int height);
   void DisableOutput(VPXWindowId output);
   void CopyOutputSettings(Settings& dst, const Settings& src) const;

   Settings m_initial;
   Settings m_draft;
   vector<VPX::Window::DisplayConfig> m_displays;
   vector<bool> m_activeDisplays;
   MainMode m_mainMode = MainMode::Desktop;
   HostMode m_vrHostMode = HostMode::Desktop;
   bool m_layoutCustomized = false;
   bool m_finished = false;
   bool m_restartRequired = false;
};

}
