// license:GPLv3+

#include "core/stdafx.h"
#include "StartupWizardPage.h"

#include "core/player.h"
#include "parts/pintable.h"
#include "renderer/Renderer.h"
#include "renderer/VRDevice.h"
#include "ui/live/LiveUI.h"
#include "ui/live/frontend/FrontendUI.h"
#include "ui/live/ingameui/InGameUI.h"

namespace VPX::InGameUI
{

using VPX::Properties::BoolPropertyDef;
using VPX::Properties::EnumPropertyDef;
using VPX::Properties::StringPropertyDef;

namespace
{

   const char* OutputName(VPXWindowId output)
   {
      switch (output)
      {
      case VPXWindowId::VPXWINDOW_Playfield: return "Playfield";
      case VPXWindowId::VPXWINDOW_Backglass: return "Backglass";
      case VPXWindowId::VPXWINDOW_ScoreView: return "Score View";
      default: return "Topper";
      }
   }

   bool IsOpenXRHMDAvailable(const Settings& settings)
   {
#ifdef ENABLE_XR
      VRDevice probe(settings);
      if (!probe.IsOpenXRReady())
         return false;
      probe.SetupHMD();
      return probe.IsOpenXRHMDReady();
#else
      return false;
#endif
   }

   ImVec2 FitInside(const ImVec2& pos, const ImVec2& size, float aspect)
   {
      const float scale = std::min(size.x / aspect, size.y);
      const ImVec2 fitted(aspect * scale, scale);
      return ImVec2(pos.x + (size.x - fitted.x) * 0.5f, pos.y + (size.y - fitted.y) * 0.5f);
   }

   void DrawAspectFrame(ImDrawList* dl, const ImVec2& pos, const ImVec2& size, float aspect, ImU32 color, const char* label, int labelCorner)
   {
      const ImVec2 framePos = FitInside(pos, size, aspect);
      const float scale = std::min(size.x / aspect, size.y);
      const ImVec2 frameSize(aspect * scale, scale);
      if (frameSize.x < 8.f || frameSize.y < 8.f)
         return;
      dl->AddRect(framePos, framePos + frameSize, color, 0.f, 0, 2.f);
      ImFont* const font = ImGui::GetFont();
      const float fontSize = std::min(ImGui::GetFontSize() * 0.9f, std::max(10.f, frameSize.y * 0.16f));
      const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, label);
      ImVec2 textPos;
      if (labelCorner == 0) // top left
         textPos = framePos + ImVec2(5.f, 4.f);
      else if (labelCorner == 1) // bottom right
         textPos = framePos + frameSize - textSize - ImVec2(5.f, 4.f);
      else // centered
         textPos = framePos + (frameSize - textSize) * 0.5f;
      dl->AddText(font, fontSize, textPos, color, label);
   }

   void DrawOutputTestCard(ImDrawList* dl, const ImVec2& pos, const ImVec2& size, bool enabled)
   {
      dl->AddRectFilled(pos, pos + size, IM_COL32(0, 0, 0, 255));
      dl->AddRect(pos, pos + size, IM_COL32(0, 220, 0, enabled ? 255 : 110), 0.f, 0, 2.5f);
      const int alpha = enabled ? 255 : 90;
      DrawAspectFrame(dl, pos, size, 4.f / 3.f, IM_COL32(255, 220, 0, alpha), "4:3 Backglass", 0);
      DrawAspectFrame(dl, pos, size, 16.f / 9.f, IM_COL32(255, 40, 40, alpha), "16:9 Video", 1);
      DrawAspectFrame(dl, pos, size, 4.f, IM_COL32(0, 230, 255, alpha), "4:1 DMD", 2);
      if (!enabled)
      {
         const char* text = "Output disabled";
         const float fontSize = ImGui::GetFontSize();
         const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.f, text);
         dl->AddRectFilled(pos + (size - textSize) * 0.5f - ImVec2(6.f, 4.f), pos + (size + textSize) * 0.5f + ImVec2(6.f, 4.f), IM_COL32(0, 0, 0, 190));
         dl->AddText(ImGui::GetFont(), fontSize, pos + (size - textSize) * 0.5f, IM_COL32(255, 255, 255, 220), text);
      }
   }

   std::unique_ptr<InGameUIItem> CustomEnumItem(const EnumPropertyDef& prop, const std::function<int()>& getValue, const std::function<void(int, int)>& onChange)
   {
      return std::make_unique<InGameUIItem>(prop, getValue, [](const Settings&) { return 0; }, onChange, [](Settings&) {}, [](int, Settings&, bool) {});
   }

   std::unique_ptr<InGameUIItem> CustomBoolItem(const BoolPropertyDef& prop, const std::function<bool()>& getValue, const std::function<void(bool)>& onChange)
   {
      return std::make_unique<InGameUIItem>(prop, getValue, [](const Settings&) { return false; }, onChange, [](Settings&) {}, [](bool, Settings&, bool) {});
   }

   std::unique_ptr<InGameUIItem> CustomStringItem(const StringPropertyDef& prop, const std::function<string()>& getValue, const std::function<void(const string&, const string&)>& onChange)
   {
      return std::make_unique<InGameUIItem>(prop, getValue, [](const Settings&) -> string { return ""s; }, onChange, [](Settings&) {}, [](const string&, Settings&, bool) {});
   }

}

StartupWizardPage::StartupWizardPage()
   : InGameUIPage("Startup Wizard"s, "Configure this machine for playing Visual Pinball."s, SaveMode::None)
   , m_model(std::make_unique<StartupWizardModel>(g_settingsService.GetAppSettings(), VPX::Window::GetDisplays(), StartupWizardModel::ShouldAutoOpen(g_settingsService.GetAppSettings()),
        IsOpenXRHMDAvailable(g_settingsService.GetAppSettings())))
{
}

int StartupWizardPage::GetStepCount() const { return (int)GetSteps().size(); }

const string& StartupWizardPage::GetInfo() const
{
   static const string welcome = "Configure this machine for playing Visual Pinball."s;
   static const string playMode = "Choose the machine type and how Visual Pinball uses your displays."s;
   static const string activeDisplays = "Select the physical displays Visual Pinball may use."s;
   static const string playfield = "Configure the display and window layout of the main playfield renderer."s;
   static const string backglass = "Choose how the backglass output is displayed."s;
   static const string scoreView = "Choose how the score view output is displayed."s;
   static const string topper = "Choose how the topper output is displayed."s;
   static const string cabinet = "Enter the physical cabinet dimensions and player position."s;
   static const string tableFolder = "Choose the folders scanned for pinball tables."s;
   static const string finished = "Review how the saved machine setup is applied."s;
   if (m_model->IsFinished() && m_model->IsRestartRequired())
      return finished;
   switch (GetStepKind(m_step))
   {
   case StepKind::Welcome: return welcome;
   case StepKind::PlayMode: return playMode;
   case StepKind::ActiveDisplays: return activeDisplays;
   case StepKind::Output:
      switch (GetOutputStepOutput(m_step))
      {
      case VPXWindowId::VPXWINDOW_Playfield: return playfield;
      case VPXWindowId::VPXWINDOW_Backglass: return backglass;
      case VPXWindowId::VPXWINDOW_ScoreView: return scoreView;
      default: return topper;
      }
   case StepKind::Cabinet: return cabinet;
   default: return tableFolder;
   }
}

vector<StartupWizardPage::StepKind> StartupWizardPage::GetSteps() const
{
   vector<StepKind> steps { StepKind::Welcome, StepKind::PlayMode };
   if (m_model->GetDisplays().size() > 1)
      steps.push_back(StepKind::ActiveDisplays);
   for (int output = 0; output < 4; ++output)
      steps.push_back(StepKind::Output);
   if (m_model->ShowsCabinetStep())
      steps.push_back(StepKind::Cabinet);
   steps.push_back(StepKind::TableFolder);
   return steps;
}

StartupWizardPage::StepKind StartupWizardPage::GetStepKind(int step) const
{
   const auto steps = GetSteps();
   return step >= 0 && step < (int)steps.size() ? steps[step] : StepKind::TableFolder;
}

VPXWindowId StartupWizardPage::GetOutputStepOutput(int step) const
{
   int outputStep = -1;
   const auto steps = GetSteps();
   for (int i = 0; i <= step && i < (int)steps.size(); ++i)
      if (steps[i] == StepKind::Output)
         ++outputStep;
   return (VPXWindowId)clamp(outputStep, 0, (int)VPXWindowId::VPXWINDOW_Topper);
}

int StartupWizardPage::CabinetStepIndex() const
{
   const auto steps = GetSteps();
   for (int i = 0; i < (int)steps.size(); ++i)
      if (steps[i] == StepKind::Cabinet)
         return i;
   return -1;
}

string StartupWizardPage::GetStepTitle(int step) const
{
   switch (GetStepKind(step))
   {
   case StepKind::Welcome: return "Welcome"s;
   case StepKind::PlayMode: return "Play Mode"s;
   case StepKind::ActiveDisplays: return "Active Displays"s;
   case StepKind::Output: return OutputName(GetOutputStepOutput(step));
   case StepKind::Cabinet: return "Cabinet Dimensions"s;
   default: return "Table Folder"s;
   }
}

void StartupWizardPage::GoToStep(int step)
{
   step = clamp(step, 0, GetStepCount() - 1);
   if (m_step == step)
      return;
   m_step = step;
   RequestRebuild();
}

void StartupWizardPage::PreviousStep() { GoToStep(m_step - 1); }

void StartupWizardPage::NextStep()
{
   if (m_step == GetStepCount() - 1)
      Finish();
   else
      GoToStep(m_step + 1);
}

void StartupWizardPage::BackOrCancel()
{
   if (m_step == 0)
      Cancel();
   else
      PreviousStep();
}

void StartupWizardPage::Cancel() { CloseOrBack(); }

void StartupWizardPage::CloseAfterFinish() { CloseOrBack(); }

void StartupWizardPage::CloseOrBack()
{
   if (m_player->m_liveUI->m_inGameUI.GetNavigationHistory().size() > 1)
      m_player->m_liveUI->m_inGameUI.NavigateBack();
   else
      m_player->m_liveUI->HideUI();
}

void StartupWizardPage::RestartNow()
{
   PinTable* const table = m_player->m_ptable;
   table->AddRef();
   m_player->RequestSessionRestart(table, m_player->m_playMode, Player::VROverride::Auto, false);
}

bool StartupWizardPage::IsFooterAction(int index) const { return std::ranges::find(m_footerItems, index) != m_footerItems.end(); }

void StartupWizardPage::AdjustItem(float direction, bool isInitialPress)
{
   // Footer buttons are step navigation actions: holding a flipper button must not chain through several steps
   if (!isInitialPress && IsFooterAction(GetSelectedItemIndex()))
      return;
   InGameUIPage::AdjustItem(direction, isInitialPress);
}

void StartupWizardPage::Finish()
{
   if (m_model->IsFinished())
      return;
   m_model->Finish(g_settingsService.GetAppSettings());
   ApplyLiveAncillaryChanges();
   if (m_player->m_playMode == Player::PlayMode::Frontend)
      m_player->m_liveUI->m_frontendUI.Rescan();
   if (m_model->IsRestartRequired())
      RequestRebuild();
   else
      CloseOrBack();
}

void StartupWizardPage::ApplyLiveAncillaryChanges()
{
   Settings& settings = g_settingsService.GetActiveSettings();
   for (int output = VPXWindowId::VPXWINDOW_Backglass; output <= VPXWindowId::VPXWINDOW_Topper; ++output)
   {
      VPX::RenderOutput& renderOutput = output == VPXWindowId::VPXWINDOW_Backglass ? m_player->m_backglassOutput
         : output == VPXWindowId::VPXWINDOW_ScoreView                              ? m_player->m_scoreViewOutput
                                                                                   : m_player->m_topperOutput;
      const auto mode = static_cast<VPX::RenderOutput::OutputMode>(m_model->GetDraft().GetWindow_Mode(output));
      const bool windowWasCreated = renderOutput.GetMode() == VPX::RenderOutput::OM_WINDOW;
      if (windowWasCreated && mode != VPX::RenderOutput::OM_WINDOW)
         m_player->m_renderer->m_renderDevice->RemoveWindow(renderOutput.GetWindow());
      renderOutput.SetMode(settings, mode);
      if (!windowWasCreated && mode == VPX::RenderOutput::OM_WINDOW && renderOutput.GetWindow() != nullptr)
      {
         m_player->m_renderer->m_renderDevice->AddWindow(renderOutput.GetWindow());
         renderOutput.GetWindow()->Show();
      }
      if (mode == VPX::RenderOutput::OM_EMBEDDED && renderOutput.GetEmbeddedWindow() != nullptr)
      {
         renderOutput.SetPos(m_model->GetDraft().GetWindow_WndX(output), m_model->GetDraft().GetWindow_WndY(output));
         renderOutput.SetWidth(m_model->GetDraft().GetWindow_Width(output));
         renderOutput.SetHeight(m_model->GetDraft().GetWindow_Height(output));
      }
      m_player->m_renderer->SetAncillaryWindowRotation((VPXWindowId)output, 90 * m_model->GetDraft().GetWindow_Rotation(output));
   }
   m_player->m_renderer->InvalidateLayout();
}
void StartupWizardPage::AddFloatSetting(const VPX::Properties::PropertyRegistry::PropId propId, const char* format)
{
   AddItem(std::make_unique<InGameUIItem>(
      propId, 1.f, format, //
      [this, propId]() { return m_model->GetDraft().GetFloat(propId); }, //
      [this, propId](float, float v) { m_model->GetDraft().Set(propId, v, false); }));
}

void StartupWizardPage::AddFooterItems()
{
   const auto addAction = [this](const string& label, const string& tooltip, const std::function<void()>& action)
   {
      AddItem(std::make_unique<InGameUIItem>(label, tooltip, action));
      m_footerItems.push_back((int)GetItems().size() - 1);
   };
   if (m_model->IsFinished() && m_model->IsRestartRequired())
   {
      addAction("Restart now"s, "Restart the player session to apply the machine setup."s, [this]() { RestartNow(); });
      addAction("Continue without restart"s, "Close the wizard. The saved setup will be used by the next session."s, [this]() { CloseAfterFinish(); });
      return;
   }
   if (m_step > 0)
      addAction("Previous"s, "Return to the previous wizard step."s, [this]() { PreviousStep(); });
   addAction(m_step == GetStepCount() - 1 ? "Finish"s : "Next"s, m_step == GetStepCount() - 1 ? "Save this machine setup."s : "Continue to the next wizard step."s, [this]() { NextStep(); });
}

void StartupWizardPage::AddDisplayModeStep()
{
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info,
      "Choose how Visual Pinball uses your displays:\n"
      "- Desktop: Traditional play mode on a single landscape display with backglass and DMD embedded in the main window.\n"
      "- Single screen: play on a single portrait display without embedding additional outputs.\n"
      "- Cabinet: play on a rotated landscape main screen with separate fullscreen outputs.\n"
      "- VR: enable headset rendering, with optional desktop mirror."s));
   AddItem(CustomEnumItem(
      EnumPropertyDef(""s, ""s, "Display mode"s, "Machine type and display use"s, false, 0, 0, { "Desktop"s, "Single screen"s, "Cabinet"s, "VR"s }), //
      [this]() { return (int)m_model->GetMainMode(); }, //
      [this](int, int v)
      {
         m_model->SetMainMode((StartupWizardModel::MainMode)v);
         RequestRebuild();
      }));
   if (m_model->GetMainMode() == StartupWizardModel::MainMode::VR)
      AddItem(CustomEnumItem(
         EnumPropertyDef(
            ""s, ""s, "Non-VR host mode"s, "Display layout used for the desktop mirror and when VR is not running"s, false, 0, 0, { "Desktop"s, "Single screen"s, "Cabinet"s }), //
         [this]() { return (int)m_model->GetVRHostMode(); }, //
         [this](int, int v)
         {
            m_model->SetVRHostMode((StartupWizardModel::HostMode)v);
            RequestRebuild();
         }));
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Changing the mode regenerates the output layout preset. It remains a draft until Finish."s));
}

void StartupWizardPage::AddActiveDisplaysStep()
{
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info,
      "Select the physical displays Visual Pinball may use. At least one display must remain active. The preview shows the operating-system layout and the output assigned to each display."s));
   const auto& displays = m_model->GetDisplays();
   for (int i = 0; i < (int)displays.size(); ++i)
   {
      const string label
         = std::format("{}{}x{} {}", displays[i].isPrimary ? "* " : "", displays[i].videomode.GetPixelWidth(), displays[i].videomode.GetPixelHeight(), displays[i].displayName);
      AddItem(CustomBoolItem(
         BoolPropertyDef(""s, std::format("Display{}", i), label, "Use this physical display for Visual Pinball outputs."s, false, true), //
         [this, i]() { return m_model->IsDisplayActive(i); }, //
         [this, i](bool v)
         {
            if (!m_model->SetDisplayActive(i, v) && !v)
               m_player->m_liveUI->PushNotification("At least one display must remain active"s, 3000);
            RequestRebuild();
         }));
   }
   AddItem(std::make_unique<InGameUIItem>("Reset layout to mode defaults"s, "Regenerate the output assignment preset for the selected display mode."s,
      [this]()
      {
         m_model->ApplyModePreset();
         RequestRebuild();
      }));
}

void StartupWizardPage::AddCabinetStep()
{
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Cabinet dimensions"s));
   AddFloatSetting(Settings::m_propPlayer_ScreenWidth, "%4.1f cm");
   AddFloatSetting(Settings::m_propPlayer_ScreenHeight, "%4.1f cm");
   AddFloatSetting(Settings::m_propPlayer_ScreenInclination, "%4.2f deg");
   AddFloatSetting(Settings::m_propPlayer_LockbarWidth, "%4.1f cm");
   AddFloatSetting(Settings::m_propPlayer_LockbarHeight, "%4.1f cm");
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Player position"s));
   AddFloatSetting(Settings::m_propPlayer_ScreenPlayerX, "%4.1f cm");
   AddFloatSetting(Settings::m_propPlayer_ScreenPlayerY, "%4.1f cm");
   AddFloatSetting(Settings::m_propPlayer_ScreenPlayerZ, "%4.1f cm");
}

void StartupWizardPage::AddTableFolderStep()
{
   if (m_model->IsFinished() && m_model->IsRestartRequired())
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Markdown,
         "# Setup saved\n\nThe machine setup has been saved. Some display, cabinet or VR changes require restarting the player session before they can be applied.\n\nUse **Restart now** to recreate the session, or **Continue without restart** to apply them the next time Visual Pinball starts."s));
      return;
   }
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Choose the root folder scanned by the frontend for tables. Additional roots may be separated by semicolons."s));
   AddItem(CustomStringItem(
      StringPropertyDef("Frontend"s, "TableFolders"s, "Table folder"s, "Root folder recursively scanned for pinball tables."s, false, ""s, StringPropertyDef::PathType::Folder), //
      [this]() { return m_model->GetDraft().GetFrontend_TableFolders(); }, //
      [this](const string&, const string& v) { m_model->GetDraft().SetFrontend_TableFolders(v, false); }));
   if (m_model->GetDraft().GetFrontend_TableFolders().empty())
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "No table folder is configured yet. You may finish now and configure it later from the frontend settings."s));
}
void StartupWizardPage::AddOutputSettingsStep(VPXWindowId output)
{
   const auto addInt = [this](VPXWindowId output, int offset, const char* format)
   {
      const auto propId = offset == 0 ? Settings::m_propWindow_WndX[output]
         : offset == 1                ? Settings::m_propWindow_WndY[output]
         : offset == 2                ? Settings::m_propWindow_Width[output]
                                      : Settings::m_propWindow_Height[output];
      AddItem(std::make_unique<InGameUIItem>(
         propId, format, //
         [this, output, offset]()
         {
            return offset == 0 ? m_model->GetDraft().GetWindow_WndX(output)
               : offset == 1   ? m_model->GetDraft().GetWindow_WndY(output)
               : offset == 2   ? m_model->GetDraft().GetWindow_Width(output)
                               : m_model->GetDraft().GetWindow_Height(output);
         },
         [this, output, offset](int, int v)
         {
            if (offset == 0)
               m_model->GetDraft().SetWindow_WndX(output, v, false);
            else if (offset == 1)
               m_model->GetDraft().SetWindow_WndY(output, v, false);
            else if (offset == 2)
               m_model->GetDraft().SetWindow_Width(output, v, false);
            else
               m_model->GetDraft().SetWindow_Height(output, v, false);
            m_model->SetLayoutCustomized(true);
         }));
   };

   const auto addRotation = [this](VPXWindowId output)
   {
      AddItem(std::make_unique<InGameUIItem>(
         Settings::m_propWindow_Rotation[output], //
         [this, output]() { return m_model->GetDraft().GetWindow_Rotation(output); }, //
         [this, output](int, int v)
         {
            m_model->GetDraft().SetWindow_Rotation(output, v, false);
            m_model->SetLayoutCustomized(true);
         }));
   };

   const auto addFullscreenAndGeometry = [&](VPXWindowId output, bool embedded)
   {
      if (!embedded)
         AddItem(std::make_unique<InGameUIItem>(
            Settings::m_propWindow_FullScreen[output], //
            [this, output]() { return m_model->GetDraft().GetWindow_FullScreen(output); }, //
            [this, output](int, int v)
            {
               m_model->GetDraft().SetWindow_FullScreen(output, v, false);
               m_model->SetLayoutCustomized(true);
               RequestRebuild();
            }));
      if (embedded || m_model->GetDraft().GetWindow_FullScreen(output) == VPX::Window::Windowed)
      {
         addInt(output, 0, embedded ? "%d" : "%d px");
         addInt(output, 1, embedded ? "%d" : "%d px");
         addInt(output, 2, embedded ? "%d" : "%d px");
         addInt(output, 3, embedded ? "%d" : "%d px");
      }
      if (output != VPXWindowId::VPXWINDOW_Playfield)
         addRotation(output);
   };

   const auto& displays = m_model->GetDisplays();
   vector<int> selectableDisplays;
   vector<string> displayNames;
   const int playfieldDisplay = m_model->GetOutputDisplayIndex(VPXWindowId::VPXWINDOW_Playfield);
   for (int i = 0; i < (int)displays.size(); ++i)
   {
      if (!m_model->IsDisplayActive(i) || (output != VPXWindowId::VPXWINDOW_Playfield && i == playfieldDisplay))
         continue;
      selectableDisplays.push_back(i);
      displayNames.push_back(
         std::format("{}{}x{} {}", displays[i].isPrimary ? "* " : "", displays[i].videomode.GetPixelWidth(), displays[i].videomode.GetPixelHeight(), displays[i].displayName));
   }

   if (output == VPXWindowId::VPXWINDOW_Playfield)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Choose the display and window layout used by the main playfield renderer."s));
      const int currentDisplay = m_model->GetOutputDisplayIndex(output);
      int currentPos = 0;
      for (int i = 0; i < (int)selectableDisplays.size(); ++i)
         if (selectableDisplays[i] == currentDisplay)
            currentPos = i;
      AddItem(CustomEnumItem(
         EnumPropertyDef(""s, "PlayfieldDisplay"s, "Display"s, "Physical display used by the playfield."s, false, 0, 0, displayNames), //
         [currentPos]() { return currentPos; }, //
         [this, selectableDisplays](int, int v)
         {
            if (v >= 0 && v < (int)selectableDisplays.size())
               m_model->AssignOutputToDisplay(VPXWindowId::VPXWINDOW_Playfield, selectableDisplays[v]);
            RequestRebuild();
         }));
      addFullscreenAndGeometry(output, false);
      return;
   }

   const bool canFloat = !selectableDisplays.empty();
   const bool isFloating = m_model->GetDraft().GetWindow_Mode(output) == VPX::RenderOutput::OM_WINDOW;
   const bool isEmbedded = m_model->GetDraft().GetWindow_Mode(output) == VPX::RenderOutput::OM_EMBEDDED;
   vector<string> modes { "Disabled"s, "Embedded in playfield"s };
   if (canFloat)
      modes.push_back("Separate display"s);
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info,
      canFloat ? "Choose whether this output is disabled, embedded in the playfield window, or shown on its own display."s
               : "Choose whether this output is disabled or embedded in the playfield window, then adjust its embedded layout."s));
   AddItem(CustomEnumItem(
      EnumPropertyDef(""s, std::format("{}Mode", (int)output), "Output mode"s, "How this output is laid out."s, false, 0, 0, modes), //
      [isFloating, isEmbedded]() { return isFloating ? 2
                                      : isEmbedded   ? 1
                                                     : 0; }, //
      [this, output, selectableDisplays, playfieldDisplay](int, int v)
      {
         if (v == 0)
            m_model->SetOutputEmbedded(output, false);
         else if (v == 1)
            m_model->SetOutputEmbedded(output, true);
         else
         {
            int display = -1;
            for (const int candidate : selectableDisplays)
               if (candidate != playfieldDisplay)
               {
                  display = candidate;
                  break;
               }
            if (display >= 0)
               m_model->AssignOutputToDisplay(output, display);
         }
         RequestRebuild();
      }));

   if (isEmbedded)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Embedded layout"s));
      addFullscreenAndGeometry(output, true);
   }
   else if (isFloating)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Display layout"s));
      const int currentDisplay = m_model->GetOutputDisplayIndex(output);
      int currentPos = 0;
      for (int i = 0; i < (int)selectableDisplays.size(); ++i)
         if (selectableDisplays[i] == currentDisplay)
            currentPos = i;
      AddItem(CustomEnumItem(
         EnumPropertyDef(""s, std::format("{}Display", (int)output), "Display"s, "Physical display used by this output."s, false, 0, 0, displayNames), //
         [currentPos]() { return currentPos; }, //
         [this, output, selectableDisplays](int, int v)
         {
            if (v >= 0 && v < (int)selectableDisplays.size())
               m_model->AssignOutputToDisplay(output, selectableDisplays[v]);
            RequestRebuild();
         }));
      addFullscreenAndGeometry(output, false);
   }
}

void StartupWizardPage::BuildPage()
{
   m_footerItems.clear();
   if (m_model->IsFinished() && m_model->IsRestartRequired())
   {
      AddTableFolderStep();
   }
   else if (m_step == 0)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Markdown,
         "# Welcome to Visual Pinball\n\nThis wizard configures the display mode, physical displays, output layout, cabinet dimensions, and the folder containing your tables.\n\nNothing is saved until you select **Finish**. **Cancel** discards the whole draft."s));
   }
   else
      switch (GetStepKind(m_step))
      {
      case StepKind::PlayMode: AddDisplayModeStep(); break;
      case StepKind::ActiveDisplays: AddActiveDisplaysStep(); break;
      case StepKind::Output: AddOutputSettingsStep(GetOutputStepOutput(m_step)); break;
      case StepKind::Cabinet: AddCabinetStep(); break;
      default: AddTableFolderStep(); break;
      }
   AddFooterItems();
}

void StartupWizardPage::RenderDisplayPreview(const ImVec4& rect) const
{
   const auto& displays = m_model->GetDisplays();
   if (displays.empty())
      return;
   int minX = displays[0].left, minY = displays[0].top, maxX = displays[0].left + displays[0].videomode.width, maxY = displays[0].top + displays[0].videomode.height;
   for (const auto& display : displays)
   {
      minX = std::min(minX, display.left);
      minY = std::min(minY, display.top);
      maxX = std::max(maxX, display.left + display.videomode.width);
      maxY = std::max(maxY, display.top + display.videomode.height);
   }
   const float scale = std::min((rect.z - 20.f) / std::max(1, maxX - minX), (rect.w - 20.f) / std::max(1, maxY - minY));
   ImDrawList* const dl = ImGui::GetWindowDrawList();
   for (int i = 0; i < (int)displays.size(); ++i)
   {
      const auto& display = displays[i];
      const ImVec2 pos(rect.x + 10.f + (display.left - minX) * scale, rect.y + 10.f + (display.top - minY) * scale);
      const ImVec2 size(display.videomode.width * scale, display.videomode.height * scale);
      const bool active = m_model->IsDisplayActive(i);
      const ImU32 fill = active ? IM_COL32(45, 80, 125, 210) : IM_COL32(45, 45, 55, 130);
      const ImU32 edge = active ? IM_COL32(120, 180, 255, 255) : IM_COL32(100, 100, 110, 180);
      dl->AddRectFilled(pos, pos + size, fill, 4.f);
      dl->AddRect(pos, pos + size, edge, 4.f, 0, 1.5f);
      string label = display.displayName;
      if (const VPXWindowId output = m_model->GetDisplayMainOutput(i); output <= VPXWindowId::VPXWINDOW_Topper)
         label += "\n"s + OutputName(output);
      else if (!active)
         label += "\nInactive"s;
      dl->AddText(ImGui::GetFont(), std::max(12.f, ImGui::GetFontSize() * 0.8f), pos + ImVec2(6.f, 6.f), IM_COL32(255, 255, 255, active ? 255 : 130), label.c_str());
   }
}

void StartupWizardPage::RenderOutputPreview(VPXWindowId output, const ImVec4& rect) const
{
   ImDrawList* const dl = ImGui::GetWindowDrawList();
   const ImVec2 panePos(rect.x + 8.f, rect.y + 8.f);
   const ImVec2 paneSize(std::max(1.f, rect.z - 16.f), std::max(1.f, rect.w - 16.f));
   const auto outputDisplaySize = [this](VPXWindowId output)
   {
      int width = std::max(1, m_model->GetDraft().GetWindow_Width(output));
      int height = std::max(1, m_model->GetDraft().GetWindow_Height(output));
      if (m_model->GetDraft().GetWindow_FullScreen(output) != VPX::Window::Windowed)
      {
         const int displayIndex = m_model->GetOutputDisplayIndex(output);
         if (displayIndex >= 0 && displayIndex < (int)m_model->GetDisplays().size())
         {
            width = std::max(1, m_model->GetDisplays()[displayIndex].videomode.GetPixelWidth());
            height = std::max(1, m_model->GetDisplays()[displayIndex].videomode.GetPixelHeight());
         }
      }
      return ImVec2((float)width, (float)height);
   };

   const int mode = m_model->GetDraft().GetWindow_Mode(output);
   if (mode == VPX::RenderOutput::OM_EMBEDDED)
   {
      const ImVec2 playfieldSize = outputDisplaySize(VPXWindowId::VPXWINDOW_Playfield);
      const float playfieldAspect = playfieldSize.x / playfieldSize.y;
      const ImVec2 containerPos = FitInside(panePos, paneSize, playfieldAspect);
      const float scale = std::min(paneSize.x / playfieldAspect, paneSize.y);
      const ImVec2 containerSize(playfieldAspect * scale, scale);
      dl->AddRectFilled(containerPos, containerPos + containerSize, IM_COL32(24, 24, 30, 255));
      dl->AddRect(containerPos, containerPos + containerSize, IM_COL32(130, 130, 145, 255), 0.f, 0, 1.f);
      const ImVec2 outputPos(containerPos.x + (float)m_model->GetDraft().GetWindow_WndX(output) / playfieldSize.x * containerSize.x,
         containerPos.y + (float)m_model->GetDraft().GetWindow_WndY(output) / playfieldSize.y * containerSize.y);
      const ImVec2 outputSize(std::max(2.f, (float)m_model->GetDraft().GetWindow_Width(output) / playfieldSize.x * containerSize.x),
         std::max(2.f, (float)m_model->GetDraft().GetWindow_Height(output) / playfieldSize.y * containerSize.y));
      dl->PushClipRect(containerPos, containerPos + containerSize, true);
      DrawOutputTestCard(dl, outputPos, outputSize, true);
      dl->PopClipRect();
      dl->AddText(containerPos + ImVec2(5.f, 4.f), IM_COL32(255, 255, 255, 140), "Playfield window");
      return;
   }

   const ImVec2 outputSizePixels = outputDisplaySize(output);
   const float outputAspect = outputSizePixels.x / outputSizePixels.y;
   const ImVec2 outputPos = FitInside(panePos, paneSize, outputAspect);
   const float scale = std::min(paneSize.x / outputAspect, paneSize.y);
   const ImVec2 outputSize(outputAspect * scale, scale);
   DrawOutputTestCard(dl, outputPos, outputSize, mode != VPX::RenderOutput::OM_DISABLED);
   if (mode == VPX::RenderOutput::OM_WINDOW)
   {
      string label = std::format("{} x {}", (int)outputSizePixels.x, (int)outputSizePixels.y);
      dl->AddText(outputPos + ImVec2(5.f, 4.f), IM_COL32(255, 255, 255, 180), label.c_str());
   }
}

void StartupWizardPage::CollectPanes(vector<InGameUIPane>& panes)
{
   InGameUIPage::CollectPanes(panes);
   if (GetStepKind(m_step) == StepKind::ActiveDisplays)
   {
      InGameUIPane pane;
      pane.placement = InGameUIPane::Placement::Header;
      pane.id = "displayLayout"s;
      pane.measure = [](float availW, float availH) { return ImVec2(availW, std::min(availH, std::max(180.f, availW * 0.32f))); };
      pane.render = [this](const ImVec4& rect) { RenderDisplayPreview(rect); };
      panes.push_back(std::move(pane));
   }
   else if (GetStepKind(m_step) == StepKind::Output && GetOutputStepOutput(m_step) != VPXWindowId::VPXWINDOW_Playfield)
   {
      InGameUIPane pane;
      pane.placement = InGameUIPane::Placement::Header;
      pane.id = "outputLayout"s;
      pane.measure = [](float availW, float availH) { return ImVec2(availW, std::min(availH, std::max(180.f, availW * 0.38f))); };
      pane.render = [this, output = GetOutputStepOutput(m_step)](const ImVec4& rect) { RenderOutputPreview(output, rect); };
      panes.push_back(std::move(pane));
   }
}

}
