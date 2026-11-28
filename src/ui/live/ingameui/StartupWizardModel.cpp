// license:GPLv3+

#include "core/stdafx.h"
#include "StartupWizardModel.h"

#include "renderer/Window.h"

namespace VPX::InGameUI
{

StartupWizardModel::StartupWizardModel(const Settings& appSettings, const vector<VPX::Window::DisplayConfig>& displays, bool generatePreset, bool openXRHMDAvailable)
   : m_displays(displays)
   , m_activeDisplays(displays.size(), true)
{
   m_initial.Load(appSettings);
   m_draft.Load(appSettings);

   if (openXRHMDAvailable)
      m_mainMode = MainMode::VR;
   else
   {
      int mainDisplay = GetOutputDisplayIndex(VPXWindowId::VPXWINDOW_Playfield);
      if (mainDisplay < 0)
         for (int i = 0; i < (int)m_displays.size(); ++i)
            if (m_displays[i].isPrimary)
            {
               mainDisplay = i;
               break;
            }
      if (mainDisplay < 0 && !m_displays.empty())
         mainDisplay = 0;
      m_mainMode = mainDisplay >= 0 && m_displays[mainDisplay].videomode.GetPixelHeight() > m_displays[mainDisplay].videomode.GetPixelWidth() ? MainMode::SingleScreen : MainMode::Desktop;
   }

   const int bgSet = m_draft.GetPlayer_BGSet();
   m_vrHostMode = bgSet == 1 ? HostMode::Cabinet : bgSet == 2 ? HostMode::Desktop : HostMode::SingleScreen;

   // Without a first-run preset, derive the active set from the existing output assignment
   for (size_t i = 0; i < m_displays.size(); ++i)
      m_activeDisplays[i] = false;
   const int playfield = GetOutputDisplayIndex(VPXWindowId::VPXWINDOW_Playfield);
   if (playfield >= 0)
      m_activeDisplays[playfield] = true;
   else if (!m_displays.empty())
      m_activeDisplays[SelectPlayfieldDisplay()] = true;
   for (int output = VPXWindowId::VPXWINDOW_Backglass; output <= VPXWindowId::VPXWINDOW_Topper; ++output)
      if (m_draft.GetWindow_Mode(output) == VPX::RenderOutput::OM_WINDOW)
         if (const int display = GetOutputDisplayIndex((VPXWindowId)output); display >= 0)
            m_activeDisplays[display] = true;

   if (generatePreset)
   {
      std::fill(m_activeDisplays.begin(), m_activeDisplays.end(), true);
      ApplyModePreset();
   }
   else
      m_layoutCustomized = true; // Manual runs preserve the existing output assignment instead of silently replacing it
}

bool StartupWizardModel::ShouldAutoOpen(const Settings& appSettings) { return appSettings.GetPlayer_StartupWizardVersion() != kStartupWizardVersion; }

void StartupWizardModel::SetMainMode(MainMode mode)
{
   if (m_mainMode == mode)
      return;
   m_mainMode = mode;
   if (mode == MainMode::Cabinet || mode == MainMode::VR)
      std::fill(m_activeDisplays.begin(), m_activeDisplays.end(), true);
   ApplyModePreset();
}

void StartupWizardModel::SetVRHostMode(HostMode mode)
{
   if (m_vrHostMode == mode)
      return;
   m_vrHostMode = mode;
   if (m_mainMode == MainMode::VR)
   {
      if (mode == HostMode::Cabinet)
         std::fill(m_activeDisplays.begin(), m_activeDisplays.end(), true);
      ApplyModePreset();
   }
}

bool StartupWizardModel::IsDisplayActive(int index) const { return index >= 0 && index < (int)m_activeDisplays.size() && m_activeDisplays[index]; }

int StartupWizardModel::GetActiveDisplayCount() const { return (int)std::count(m_activeDisplays.begin(), m_activeDisplays.end(), true); }

bool StartupWizardModel::SetDisplayActive(int index, bool active)
{
   if (index < 0 || index >= (int)m_activeDisplays.size())
      return false;
   if (!active && m_activeDisplays[index] && GetActiveDisplayCount() <= 1)
      return false;
   if (m_activeDisplays[index] == active)
      return true;
   m_activeDisplays[index] = active;
   if (!m_layoutCustomized)
      ApplyModePreset();
   else
      SanitizeLayoutForActiveDisplays();
   if (active && GetDisplayMainOutput(index) == (VPXWindowId)((int)VPXWindowId::VPXWINDOW_Topper + 1))
      for (int output = VPXWindowId::VPXWINDOW_Backglass; output <= VPXWindowId::VPXWINDOW_Topper; ++output)
         if (m_draft.GetWindow_Mode(output) == VPX::RenderOutput::OM_DISABLED)
         {
            AssignOutputToDisplay((VPXWindowId)output, index);
            break;
         }
   return true;
}

int StartupWizardModel::GetDisplayIndex(const string& displayId) const
{
   for (int i = 0; i < (int)m_displays.size(); ++i)
      if (m_displays[i].displayId == displayId)
         return i;
   return -1;
}

int StartupWizardModel::SelectPlayfieldDisplay() const
{
   int best = -1;
   int64_t bestArea = -1;
   for (int i = 0; i < (int)m_displays.size(); ++i)
   {
      if (!IsDisplayActive(i))
         continue;
      const auto& mode = m_displays[i].videomode;
      if (mode.GetPixelWidth() < mode.GetPixelHeight())
         continue;
      const int64_t area = (int64_t)mode.GetPixelWidth() * mode.GetPixelHeight();
      if (area > bestArea || (area == bestArea && best >= 0 && m_displays[i].isPrimary && !m_displays[best].isPrimary))
      {
         best = i;
         bestArea = area;
      }
   }
   if (best >= 0)
      return best;
   for (int i = 0; i < (int)m_displays.size(); ++i)
      if (IsDisplayActive(i) && m_displays[i].isPrimary)
         return i;
   for (int i = 0; i < (int)m_displays.size(); ++i)
      if (IsDisplayActive(i))
         return i;
   return 0;
}

vector<int> StartupWizardModel::GetAncillaryDisplayOrder(int playfieldDisplay) const
{
   vector<int> result;
   for (int i = 0; i < (int)m_displays.size(); ++i)
      if (i != playfieldDisplay && IsDisplayActive(i))
         result.push_back(i);
   std::ranges::sort(result,
      [this](int a, int b)
      {
         const auto& ma = m_displays[a].videomode;
         const auto& mb = m_displays[b].videomode;
         return (int64_t)mb.GetPixelWidth() * mb.GetPixelHeight() < (int64_t)ma.GetPixelWidth() * ma.GetPixelHeight();
      });
   return result;
}

vector<int> StartupWizardModel::GetOrderedActiveDisplays() const
{
   vector<int> result;
   for (int i = 0; i < (int)m_displays.size(); ++i)
      if (IsDisplayActive(i))
         result.push_back(i);
   return result;
}

StartupWizardModel::HostMode StartupWizardModel::GetEffectiveHostMode() const
{
   switch (m_mainMode)
   {
   case MainMode::Cabinet: return HostMode::Cabinet;
   case MainMode::SingleScreen: return HostMode::SingleScreen;
   case MainMode::VR: return m_vrHostMode;
   default: return HostMode::Desktop;
   }
}

void StartupWizardModel::SetFloatingOutput(VPXWindowId output, int displayIndex)
{
   if (displayIndex < 0 || displayIndex >= (int)m_displays.size())
      return;
   if (output != VPXWindowId::VPXWINDOW_Playfield)
      m_draft.SetWindow_Mode(output, VPX::RenderOutput::OM_WINDOW, false);
   m_draft.SetWindow_Display(output, m_displays[displayIndex].displayId, false);
   m_draft.SetWindow_FullScreen(output, VPX::Window::BorderlessFullscreen, false);
   const auto& mode = m_displays[displayIndex].videomode;
   m_draft.SetWindow_WndX(output, 0, false);
   m_draft.SetWindow_WndY(output, 0, false);
   m_draft.SetWindow_Width(output, mode.GetPixelWidth(), false);
   m_draft.SetWindow_Height(output, mode.GetPixelHeight(), false);
   m_draft.SetWindow_FSWidth(output, mode.GetPixelWidth(), false);
   m_draft.SetWindow_FSHeight(output, mode.GetPixelHeight(), false);
   m_draft.SetWindow_FSRefreshRate(output, mode.refreshrate, false);
   m_draft.SetWindow_FSColorDepth(output, mode.depth, false);
}

void StartupWizardModel::SetEmbeddedOutput(VPXWindowId output, int x, int y, int width, int height)
{
   m_draft.SetWindow_Mode(output, VPX::RenderOutput::OM_EMBEDDED, false);
   m_draft.SetWindow_WndX(output, x, false);
   m_draft.SetWindow_WndY(output, y, false);
   m_draft.SetWindow_Width(output, width, false);
   m_draft.SetWindow_Height(output, height, false);
}

void StartupWizardModel::DisableOutput(VPXWindowId output)
{
   if (output != VPXWindowId::VPXWINDOW_Playfield)
      m_draft.SetWindow_Mode(output, VPX::RenderOutput::OM_DISABLED, false);
}

void StartupWizardModel::ApplyModePreset()
{
   if (m_displays.empty())
      return;

   const HostMode hostMode = GetEffectiveHostMode();
   m_draft.SetPlayerVR_AskToTurnOn(m_mainMode == MainMode::VR ? 0 : 2, false);
   m_draft.SetPlayer_BGSet(hostMode == HostMode::Cabinet ? 1 : hostMode == HostMode::Desktop ? 2 : 0, false);

   const int playfield = SelectPlayfieldDisplay();
   SetFloatingOutput(VPXWindowId::VPXWINDOW_Playfield, playfield);
   const int pfW = m_displays[playfield].videomode.width;
   const int pfH = m_displays[playfield].videomode.height;
   const int embeddedW = std::max(160, pfW / 4);
   const int embeddedH = std::max(90, pfH / 4);

   if (hostMode == HostMode::Desktop)
   {
      for (size_t i = 0; i < m_activeDisplays.size(); ++i)
         m_activeDisplays[i] = (int)i == playfield;
      SetEmbeddedOutput(VPXWindowId::VPXWINDOW_Backglass, 8, 8, embeddedW, embeddedH);
      SetEmbeddedOutput(VPXWindowId::VPXWINDOW_ScoreView, std::max(8, pfW - embeddedW - 8), 8, embeddedW, embeddedH);
      DisableOutput(VPXWindowId::VPXWINDOW_Topper);
   }
   else if (hostMode == HostMode::SingleScreen)
   {
      for (size_t i = 0; i < m_activeDisplays.size(); ++i)
         m_activeDisplays[i] = (int)i == playfield;
      DisableOutput(VPXWindowId::VPXWINDOW_Backglass);
      DisableOutput(VPXWindowId::VPXWINDOW_ScoreView);
      DisableOutput(VPXWindowId::VPXWINDOW_Topper);
   }
   else
   {
      const vector<int> ancillary = GetAncillaryDisplayOrder(playfield);
      const VPXWindowId outputs[] = { VPXWindowId::VPXWINDOW_Backglass, VPXWindowId::VPXWINDOW_ScoreView, VPXWindowId::VPXWINDOW_Topper };
      for (int i = 0; i < 3; ++i)
      {
         if (i < (int)ancillary.size())
            SetFloatingOutput(outputs[i], ancillary[i]);
         else
            DisableOutput(outputs[i]);
      }
   }
   m_layoutCustomized = false;
}

int StartupWizardModel::GetOutputDisplayIndex(VPXWindowId output) const { return GetDisplayIndex(m_draft.GetWindow_Display(output)); }

VPXWindowId StartupWizardModel::GetDisplayMainOutput(int displayIndex) const
{
   if (GetOutputDisplayIndex(VPXWindowId::VPXWINDOW_Playfield) == displayIndex)
      return VPXWindowId::VPXWINDOW_Playfield;
   for (int output = VPXWindowId::VPXWINDOW_Backglass; output <= VPXWindowId::VPXWINDOW_Topper; ++output)
      if (m_draft.GetWindow_Mode(output) == VPX::RenderOutput::OM_WINDOW && GetOutputDisplayIndex((VPXWindowId)output) == displayIndex)
         return (VPXWindowId)output;
   return (VPXWindowId)(VPXWindowId::VPXWINDOW_Topper + 1); // None
}
bool StartupWizardModel::AssignOutputToDisplay(VPXWindowId output, int displayIndex)
{
   if (displayIndex < 0 || displayIndex >= (int)m_displays.size() || !IsDisplayActive(displayIndex))
      return false;
   const int playfieldDisplay = GetOutputDisplayIndex(VPXWindowId::VPXWINDOW_Playfield);
   if (output != VPXWindowId::VPXWINDOW_Playfield && displayIndex == playfieldDisplay)
      return false; // The playfield keeps its display: ancillary outputs use embedded overlays there

   if (const VPXWindowId previous = GetDisplayMainOutput(displayIndex); previous != output && previous != VPXWindowId::VPXWINDOW_Playfield && previous <= VPXWindowId::VPXWINDOW_Topper)
      DisableOutput(previous);
   SetFloatingOutput(output, displayIndex);
   m_layoutCustomized = true;
   return true;
}

void StartupWizardModel::SetOutputEmbedded(VPXWindowId output, bool embedded)
{
   if (output == VPXWindowId::VPXWINDOW_Playfield)
      return;
   if (!embedded)
   {
      DisableOutput(output);
   }
   else
   {
      const int playfield = GetOutputDisplayIndex(VPXWindowId::VPXWINDOW_Playfield);
      const int w = playfield >= 0 ? m_displays[playfield].videomode.width / 4 : 480;
      const int h = playfield >= 0 ? m_displays[playfield].videomode.height / 4 : 270;
      SetEmbeddedOutput(output, 8, 8, std::max(160, w), std::max(90, h));
   }
   m_layoutCustomized = true;
}

void StartupWizardModel::SanitizeLayoutForActiveDisplays()
{
   int playfield = GetOutputDisplayIndex(VPXWindowId::VPXWINDOW_Playfield);
   if (playfield < 0 || !IsDisplayActive(playfield))
   {
      playfield = SelectPlayfieldDisplay();
      SetFloatingOutput(VPXWindowId::VPXWINDOW_Playfield, playfield);
   }
   for (int output = VPXWindowId::VPXWINDOW_Backglass; output <= VPXWindowId::VPXWINDOW_Topper; ++output)
   {
      if (m_draft.GetWindow_Mode(output) != VPX::RenderOutput::OM_WINDOW)
         continue;
      const int display = GetOutputDisplayIndex((VPXWindowId)output);
      if (display < 0 || !IsDisplayActive(display) || display == playfield)
         DisableOutput((VPXWindowId)output);
   }
}

bool StartupWizardModel::NeedsRestart() const
{
   if (m_initial.GetPlayer_BGSet() != m_draft.GetPlayer_BGSet() || m_initial.GetPlayerVR_AskToTurnOn() != m_draft.GetPlayerVR_AskToTurnOn())
      return true;
   if (m_initial.GetWindow_Display(VPXWindowId::VPXWINDOW_Playfield) != m_draft.GetWindow_Display(VPXWindowId::VPXWINDOW_Playfield)
      || m_initial.GetWindow_FullScreen(VPXWindowId::VPXWINDOW_Playfield) != m_draft.GetWindow_FullScreen(VPXWindowId::VPXWINDOW_Playfield)
      || m_initial.GetWindow_WndX(VPXWindowId::VPXWINDOW_Playfield) != m_draft.GetWindow_WndX(VPXWindowId::VPXWINDOW_Playfield)
      || m_initial.GetWindow_WndY(VPXWindowId::VPXWINDOW_Playfield) != m_draft.GetWindow_WndY(VPXWindowId::VPXWINDOW_Playfield)
      || m_initial.GetWindow_Width(VPXWindowId::VPXWINDOW_Playfield) != m_draft.GetWindow_Width(VPXWindowId::VPXWINDOW_Playfield)
      || m_initial.GetWindow_Height(VPXWindowId::VPXWINDOW_Playfield) != m_draft.GetWindow_Height(VPXWindowId::VPXWINDOW_Playfield))
      return true;
   return m_initial.GetPlayer_ScreenWidth() != m_draft.GetPlayer_ScreenWidth() || m_initial.GetPlayer_ScreenHeight() != m_draft.GetPlayer_ScreenHeight()
      || m_initial.GetPlayer_ScreenInclination() != m_draft.GetPlayer_ScreenInclination() || m_initial.GetPlayer_LockbarWidth() != m_draft.GetPlayer_LockbarWidth()
      || m_initial.GetPlayer_LockbarHeight() != m_draft.GetPlayer_LockbarHeight() || m_initial.GetPlayer_ScreenPlayerX() != m_draft.GetPlayer_ScreenPlayerX()
      || m_initial.GetPlayer_ScreenPlayerY() != m_draft.GetPlayer_ScreenPlayerY() || m_initial.GetPlayer_ScreenPlayerZ() != m_draft.GetPlayer_ScreenPlayerZ();
}
void StartupWizardModel::CopyOutputSettings(Settings& dst, const Settings& src) const
{
   for (int output = VPXWindowId::VPXWINDOW_Playfield; output <= VPXWindowId::VPXWINDOW_Topper; ++output)
   {
      if (output != VPXWindowId::VPXWINDOW_Playfield)
         dst.SetWindow_Mode(output, src.GetWindow_Mode(output), false);
      dst.SetWindow_Display(output, src.GetWindow_Display(output), false);
      dst.SetWindow_FullScreen(output, src.GetWindow_FullScreen(output), false);
      dst.SetWindow_WndX(output, src.GetWindow_WndX(output), false);
      dst.SetWindow_WndY(output, src.GetWindow_WndY(output), false);
      dst.SetWindow_Width(output, src.GetWindow_Width(output), false);
      dst.SetWindow_Height(output, src.GetWindow_Height(output), false);
      dst.SetWindow_FSWidth(output, src.GetWindow_FSWidth(output), false);
      dst.SetWindow_FSHeight(output, src.GetWindow_FSHeight(output), false);
      dst.SetWindow_FSRefreshRate(output, src.GetWindow_FSRefreshRate(output), false);
      dst.SetWindow_FSColorDepth(output, src.GetWindow_FSColorDepth(output), false);
      if (output != VPXWindowId::VPXWINDOW_Playfield)
         dst.SetWindow_Rotation(output, src.GetWindow_Rotation(output), false);
   }
}

void StartupWizardModel::Finish(Settings& appSettings)
{
   if (m_finished)
      return;
   m_restartRequired = NeedsRestart();
   appSettings.SetPlayer_StartupWizardVersion(kStartupWizardVersion, false);
   appSettings.SetPlayer_BGSet(m_draft.GetPlayer_BGSet(), false);
   appSettings.SetPlayerVR_AskToTurnOn(m_draft.GetPlayerVR_AskToTurnOn(), false);
   CopyOutputSettings(appSettings, m_draft);
   appSettings.SetPlayer_ScreenWidth(m_draft.GetPlayer_ScreenWidth(), false);
   appSettings.SetPlayer_ScreenHeight(m_draft.GetPlayer_ScreenHeight(), false);
   appSettings.SetPlayer_ScreenInclination(m_draft.GetPlayer_ScreenInclination(), false);
   appSettings.SetPlayer_LockbarWidth(m_draft.GetPlayer_LockbarWidth(), false);
   appSettings.SetPlayer_LockbarHeight(m_draft.GetPlayer_LockbarHeight(), false);
   appSettings.SetPlayer_ScreenPlayerX(m_draft.GetPlayer_ScreenPlayerX(), false);
   appSettings.SetPlayer_ScreenPlayerY(m_draft.GetPlayer_ScreenPlayerY(), false);
   appSettings.SetPlayer_ScreenPlayerZ(m_draft.GetPlayer_ScreenPlayerZ(), false);
   appSettings.SetFrontend_TableFolders(m_draft.GetFrontend_TableFolders(), false);
   appSettings.Save();
   m_finished = true;
}

}
