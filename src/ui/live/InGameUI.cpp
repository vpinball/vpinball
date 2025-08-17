// license:GPLv3+

#include "core/stdafx.h"

#include "imgui/imgui.h"

#include "InGameUI.h"
#include "LiveUI.h"

#include "renderer/VRDevice.h"

#include "core/TableDB.h"

#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"


InGameUI::InGameUI(LiveUI &liveUI)
   : m_liveUI(liveUI)
{
   m_openTimeMs = msec();
   m_app = g_pvp;
   m_player = g_pplayer;
   m_table = m_player->m_pEditorTable;
   m_live_table = m_player->m_ptable;
   m_pininput = &(m_player->m_pininput);
   m_renderer = m_player->m_renderer;
}

InGameUI::~InGameUI()
{
}

void InGameUI::Open()
{
   if (m_isOpened)
      return;
   m_isOpened = true;
   m_tweakPages.clear();
   if (!m_table->m_rules.empty())
      m_tweakPages.push_back(TP_Rules);
   if (m_renderer->m_stereo3D != STEREO_VR)
      m_tweakPages.push_back(TP_PointOfView);
   #ifdef ENABLE_XR
   // Legacy OpenVR does not support dynamic repositioning through LiveUI (especially overall scale, this would need to be rewritten but not done as this is planned for deprecation)
   else
      m_tweakPages.push_back(TP_VRPosition);
   #endif
   m_tweakPages.push_back(TP_TableOption);
   for (int j = 0; j < Settings::GetNPluginSections(); j++)
   {
      int nOptions = 0;
      const int nCustomOptions = (int)Settings::GetPluginSettings().size();
      for (int i = 0; i < nCustomOptions; i++)
         if ((Settings::GetPluginSettings()[i].section == Settings::Plugin00 + j) && (Settings::GetPluginSettings()[i].showMask & VPX_OPT_SHOW_TWEAK))
            nOptions++;
      if (nOptions > 0)
         m_tweakPages.push_back((TweakPage)(TP_Plugin00 + j));
   }
   if (!m_table->m_description.empty())
      m_tweakPages.push_back(TP_Info);
   m_activeTweakPageIndex = 0;
   m_activeTweakIndex = 0;
   UpdateTweakPage();
}

void InGameUI::Close()
{
   if (!m_isOpened)
      return;
   m_isOpened = false;
   if (m_staticPrepassDisabled)
      m_renderer->DisableStaticPrePass(false);
   m_live_table->FireOptionEvent(3); // Tweak mode closed event
}

void InGameUI::UpdateTweakPage()
{
   m_tweakPageOptions.clear();
   m_tweakPageOptions.push_back(BS_Page);
   switch (m_tweakPages[m_activeTweakPageIndex])
   {
   case TP_Info:
   case TP_Rules:
      break;
   case TP_VRPosition:
      m_tweakPageOptions.push_back(BS_VROrientation);
      m_tweakPageOptions.push_back(BS_VRX);
      m_tweakPageOptions.push_back(BS_VRY);
      m_tweakPageOptions.push_back(BS_VRZ);
      m_tweakPageOptions.push_back(BS_VRScale);
      m_tweakPageOptions.push_back(BS_AR_VR);
      break;
   case TP_PointOfView:
      switch (m_live_table->mViewSetups[m_live_table->m_BG_current_set].mMode)
      {
      case VLM_LEGACY:
         m_tweakPageOptions.push_back(BS_ViewMode);
         m_tweakPageOptions.push_back(BS_LookAt);
         m_tweakPageOptions.push_back(BS_FOV);
         m_tweakPageOptions.push_back(BS_Layback);
         m_tweakPageOptions.push_back(BS_XYZScale);
         m_tweakPageOptions.push_back(BS_XScale);
         m_tweakPageOptions.push_back(BS_YScale);
         m_tweakPageOptions.push_back(BS_ZScale);
         m_tweakPageOptions.push_back(BS_XOffset);
         m_tweakPageOptions.push_back(BS_YOffset);
         m_tweakPageOptions.push_back(BS_ZOffset);
         break;
      case VLM_CAMERA:
         m_tweakPageOptions.push_back(BS_ViewMode);
         m_tweakPageOptions.push_back(BS_FOV);
         m_tweakPageOptions.push_back(BS_ViewHOfs);
         m_tweakPageOptions.push_back(BS_ViewVOfs);
         m_tweakPageOptions.push_back(BS_XYZScale);
         m_tweakPageOptions.push_back(BS_XScale);
         m_tweakPageOptions.push_back(BS_YScale);
         m_tweakPageOptions.push_back(BS_ZScale);
         m_tweakPageOptions.push_back(BS_LookAt);
         m_tweakPageOptions.push_back(BS_XOffset);
         m_tweakPageOptions.push_back(BS_YOffset);
         m_tweakPageOptions.push_back(BS_ZOffset);
         break;
      case VLM_WINDOW:
         m_tweakPageOptions.push_back(BS_ViewMode);
         m_tweakPageOptions.push_back(BS_ViewHOfs);
         m_tweakPageOptions.push_back(BS_ViewVOfs);
         m_tweakPageOptions.push_back(BS_XYZScale);
         m_tweakPageOptions.push_back(BS_XScale);
         m_tweakPageOptions.push_back(BS_YScale);
         m_tweakPageOptions.push_back(BS_WndTopZOfs);
         m_tweakPageOptions.push_back(BS_WndBottomZOfs);
         m_tweakPageOptions.push_back(BS_XOffset);
         m_tweakPageOptions.push_back(BS_YOffset);
         m_tweakPageOptions.push_back(BS_ZOffset);
         break;
      }
      break;
   case TP_TableOption:
   {
      const int nCustomOptions = (int)m_live_table->m_settings.GetTableSettings().size();
      for (int i = 0; i < nCustomOptions; i++)
         m_tweakPageOptions.push_back((BackdropSetting)(BS_Custom + i));
      m_tweakPageOptions.push_back(BS_DayNight);
      m_tweakPageOptions.push_back(BS_Exposure);
      if (!m_player || !m_player->m_renderer->m_HDRforceDisableToneMapper || !m_player->m_playfieldWnd->IsWCGBackBuffer())
         m_tweakPageOptions.push_back(BS_Tonemapper);
      m_tweakPageOptions.push_back(BS_Difficulty);
      m_tweakPageOptions.push_back(BS_Volume);
      m_tweakPageOptions.push_back(BS_BackglassVolume);
      m_tweakPageOptions.push_back(BS_PlayfieldVolume);
      break;
   }
   default: // Plugin options
   {
      const int nCustomOptions = (int)Settings::GetPluginSettings().size();
      for (int i = 0; i < nCustomOptions; i++)
         if (Settings::GetPluginSettings()[i].section == Settings::Plugin00 + (m_tweakPages[m_activeTweakPageIndex] - TP_Plugin00) && (Settings::GetPluginSettings()[i].showMask & VPX_OPT_SHOW_TWEAK))
            m_tweakPageOptions.push_back((BackdropSetting)(BS_Custom + i));
      break;
   }
   }
   if (m_activeTweakIndex >= (int)m_tweakPageOptions.size())
      m_activeTweakIndex = (int)m_tweakPageOptions.size() - 1;
}

void InGameUI::HandleTweakInput()
{
   const uint32_t now = msec();
   static uint32_t lastHandle = now;
   const uint32_t sinceLastInputHandleMs = now - lastHandle;
   lastHandle = now;

   BackdropSetting activeTweakSetting = m_tweakPageOptions[m_activeTweakIndex];
   PinTable * const table = m_live_table;

   // Legacy leyboard fly camera when in ingame option. Remove ?
   if (m_live_table->m_settings.LoadValueBool(Settings::Player, "EnableCameraModeFlyAround"s))
   {
      if (!ImGui::IsKeyDown(ImGuiKey_LeftAlt) && !ImGui::IsKeyDown(ImGuiKey_RightAlt))
      {
         if (ImGui::IsKeyDown(ImGuiKey_LeftArrow))
            m_renderer->m_cam.x += 10.0f;
         if (ImGui::IsKeyDown(ImGuiKey_RightArrow))
            m_renderer->m_cam.x -= 10.0f;
         if (ImGui::IsKeyDown(ImGuiKey_UpArrow))
            m_renderer->m_cam.y += 10.0f;
         if (ImGui::IsKeyDown(ImGuiKey_DownArrow))
            m_renderer->m_cam.y -= 10.0f;
      }
      else
      {
         if (ImGui::IsKeyDown(ImGuiKey_UpArrow))
            m_renderer->m_cam.z += 10.0f;
         if (ImGui::IsKeyDown(ImGuiKey_DownArrow))
            m_renderer->m_cam.z -= 10.0f;
         if (ImGui::IsKeyDown(ImGuiKey_LeftArrow))
            m_renderer->m_inc += 0.01f;
         if (ImGui::IsKeyDown(ImGuiKey_RightArrow))
            m_renderer->m_inc -= 0.01f;
      }
   }

   PinInput::InputState state = m_player->m_pininput.GetInputState();
   for (int i = 0; i < eCKeys; i++)
   {
      const EnumAssignKeys keycode = static_cast<EnumAssignKeys>(i);
      int keyEvent;
      if (state.IsKeyPressed(keycode, m_prevInputState))
         keyEvent = 1;
      else if (state.IsKeyReleased(keycode, m_prevInputState))
         keyEvent = 2;
      else if (state.IsKeyDown(keycode))
         keyEvent = 0;
      else
         continue;

      // Handle scrolling in rules/infos
      if ((m_tweakPages[m_activeTweakPageIndex] == TP_Rules || m_tweakPages[m_activeTweakPageIndex] == TP_Info)
         && (keycode == eRightMagnaSave || keycode == eLeftMagnaSave) && (keyEvent != 2))
      {
         const float speed = ImGui::GetFontBaked()->Size * 0.5f;
         if (keycode == eLeftMagnaSave)
            m_tweakScroll -= speed;
         else if (keycode == eRightMagnaSave)
            m_tweakScroll += speed;
      }

      if (keycode == eEscape && keyEvent == 2)
         Close();

      if (keycode == eLeftFlipperKey || keycode == eRightFlipperKey)
      {
         static uint32_t startOfPress = 0;
         static float floatFraction = 1.0f;
         if (keyEvent != 0)
         {
            startOfPress = now;
            floatFraction = 1.0f;
         }
         if (keyEvent == 2) // Do not react on key up (only key down or long press)
            continue;
         const bool up = keycode == eRightFlipperKey;
         const float step = up ? 1.f : -1.f;
         const float absIncSpeed = (float)sinceLastInputHandleMs * 0.001f * min(50.f, 0.75f + (float)(now - startOfPress) / 300.0f);
         const float incSpeed = up ? absIncSpeed : -absIncSpeed;

         // Since we need less than 1 int per frame for eg volume, we need to keep track of the float value
         // and step every n frames.
         floatFraction += absIncSpeed * 10.f;
         int absIntStep = 0;
         if (floatFraction >= 1.f)
         {
            absIntStep = static_cast<int>(floatFraction);
            floatFraction = floatFraction - (float)absIntStep;
         }
         const int intStep = up ? absIntStep : -absIntStep;

         ViewSetup &viewSetup = table->mViewSetups[table->m_BG_current_set];
         const bool isWindow = viewSetup.mMode == VLM_WINDOW;
         bool modified = true;
         switch (activeTweakSetting)
         {
         // UI navigation
         case BS_Page:
         {
            m_tweakState[activeTweakSetting] = 0;
            if (keyEvent != 1) // Only keydown
               continue;
            int stepi = up ? 1 : (int)m_tweakPages.size() - 1;
            m_activeTweakPageIndex = ((m_activeTweakPageIndex + stepi) % (int)m_tweakPages.size());
            m_activeTweakIndex = 0;
            m_tweakScroll = 0.f;
            UpdateTweakPage();
            break;
         }

         // View setup settings
         case BS_ViewMode:
         {
            if (keyEvent != 1) // Only keydown
               continue;
            DisableStaticPrepass();
            int vlm = viewSetup.mMode + (int)step;
            viewSetup.mMode = vlm < 0 ? VLM_WINDOW : vlm >= 3 ? VLM_LEGACY : (ViewLayoutMode)vlm;
            UpdateTweakPage();
            break;
         }
         case BS_LookAt:
            DisableStaticPrepass();
            viewSetup.mLookAt += incSpeed;
            break;
         case BS_FOV:
            DisableStaticPrepass();
            viewSetup.mFOV += incSpeed;
            break;
         case BS_Layback:
            DisableStaticPrepass();
            viewSetup.mLayback += incSpeed;
            break;
         case BS_ViewHOfs:
            DisableStaticPrepass();
            viewSetup.mViewHOfs += incSpeed;
            break;
         case BS_ViewVOfs:
            DisableStaticPrepass();
            viewSetup.mViewVOfs += incSpeed;
            break;
         case BS_XYZScale:
            DisableStaticPrepass();
            viewSetup.mSceneScaleX += 0.005f * incSpeed;
            viewSetup.mSceneScaleY += 0.005f * incSpeed;
            viewSetup.mSceneScaleZ += 0.005f * incSpeed;
            break;
         case BS_XScale:
            DisableStaticPrepass();
            viewSetup.mSceneScaleX += 0.005f * incSpeed;
            break;
         case BS_YScale:
            DisableStaticPrepass();
            viewSetup.mSceneScaleY += 0.005f * incSpeed;
            break;
         case BS_ZScale:
            DisableStaticPrepass();
            viewSetup.mSceneScaleZ += 0.005f * incSpeed;
            break;
         case BS_XOffset:
            DisableStaticPrepass();
            if (isWindow)
               table->m_settings.SaveValue(Settings::Player, "ScreenPlayerX"s, table->m_settings.LoadValueFloat(Settings::Player, "ScreenPlayerX"s) + 0.5f * incSpeed);
            else
               viewSetup.mViewX += 10.f * incSpeed;
            break;
         case BS_YOffset:
            DisableStaticPrepass();
            if (isWindow)
               table->m_settings.SaveValue(Settings::Player, "ScreenPlayerY"s, table->m_settings.LoadValueFloat(Settings::Player, "ScreenPlayerY"s) + 0.5f * incSpeed);
            else
               viewSetup.mViewY += 10.f * incSpeed;
            break;
         case BS_ZOffset:
            DisableStaticPrepass();
            if (isWindow)
               table->m_settings.SaveValue(Settings::Player, "ScreenPlayerZ"s, table->m_settings.LoadValueFloat(Settings::Player, "ScreenPlayerZ"s) + 0.5f * incSpeed);
            else
               viewSetup.mViewZ += (viewSetup.mMode == VLM_LEGACY ? 100.f : 10.f) * incSpeed;
            break;
         case BS_WndTopZOfs:
            DisableStaticPrepass();
            viewSetup.mWindowTopZOfs += 10.f * incSpeed;
            break;
         case BS_WndBottomZOfs:
            DisableStaticPrepass();
            viewSetup.mWindowBottomZOfs += 10.f * incSpeed;
            break;

         // VR Position
         case BS_VRScale: m_player->m_vrDevice->SetLockbarWidth(clamp(m_player->m_vrDevice->GetLockbarWidth() + 1.f * incSpeed, 5.f, 200.f)); break;
         case BS_VROrientation: m_player->m_vrDevice->SetSceneOrientation(m_player->m_vrDevice->GetSceneOrientation() + 1.f * incSpeed); break;
         case BS_VRX: { Vertex3Ds pos = m_player->m_vrDevice->GetSceneOffset(); pos.x += 1.f * incSpeed; m_player->m_vrDevice->SetSceneOffset(pos); break; }
         case BS_VRY: { Vertex3Ds pos = m_player->m_vrDevice->GetSceneOffset(); pos.y += 1.f * incSpeed; m_player->m_vrDevice->SetSceneOffset(pos); break; }
         case BS_VRZ: { Vertex3Ds pos = m_player->m_vrDevice->GetSceneOffset(); pos.z += 1.f * incSpeed; m_player->m_vrDevice->SetSceneOffset(pos); break; }
         case BS_AR_VR: if (keyEvent == 1) m_renderer->m_vrApplyColorKey = !m_renderer->m_vrApplyColorKey; break;

         // Table customization
         case BS_DayNight:
            DisableStaticPrepass();
            m_renderer->m_globalEmissionScale = clamp(m_renderer->m_globalEmissionScale + incSpeed * 0.05f, 0.f, 1.f);
            m_renderer->MarkShaderDirty();
            m_live_table->FireOptionEvent(1); // Table option changed event
            break;
         case BS_Difficulty:
            table->m_globalDifficulty = clamp(table->m_globalDifficulty + incSpeed * 0.05f, 0.f, 1.f);
            m_live_table->FireOptionEvent(1); // Table option changed event
            break;
         case BS_Volume:
            m_player->m_MusicVolume = clamp(m_player->m_MusicVolume + intStep, 0, 100);
            m_player->m_SoundVolume = clamp(m_player->m_SoundVolume + intStep, 0, 100);
            m_player->UpdateVolume();
            m_live_table->FireOptionEvent(1); // Table option changed event
            break;
         case BS_BackglassVolume:
            m_player->m_MusicVolume = clamp(m_player->m_MusicVolume + intStep, 0, 100);
            m_player->UpdateVolume();
            m_live_table->FireOptionEvent(1); // Table option changed event
            break;
         case BS_PlayfieldVolume:
            m_player->m_SoundVolume = clamp(m_player->m_SoundVolume + intStep, 0, 100);
            m_player->UpdateVolume();
            m_live_table->FireOptionEvent(1); // Table option changed event
            break;
         case BS_Exposure:
            DisableStaticPrepass();
            m_renderer->m_exposure = clamp(m_renderer->m_exposure + incSpeed * 0.05f, 0.f, 2.0f);
            m_live_table->FireOptionEvent(1); // Table option changed event
            break;
         case BS_Tonemapper:
            if (keyEvent == 1)
            {
               int tm = m_renderer->m_toneMapper + static_cast<int>(step);
               #ifdef ENABLE_BGFX
               if (tm < 0)
                  tm = ToneMapper::TM_AGX_PUNCHY;
               if (tm > ToneMapper::TM_AGX_PUNCHY)
                  tm = ToneMapper::TM_REINHARD;
               #else
               if (tm < 0)
                  tm = ToneMapper::TM_NEUTRAL;
               if (tm > ToneMapper::TM_NEUTRAL)
                  tm = ToneMapper::TM_REINHARD;
               #endif
               m_renderer->m_toneMapper = static_cast<ToneMapper>(tm);
               m_live_table->FireOptionEvent(1); // Table option changed event
            }
            break;

         default:
            if (activeTweakSetting >= BS_Custom)
            {
               const vector<Settings::OptionDef> &customOptions = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? m_live_table->m_settings.GetTableSettings() : Settings::GetPluginSettings();
               if (activeTweakSetting < BS_Custom + (int)customOptions.size())
               {
                  const auto& opt = customOptions[activeTweakSetting - BS_Custom];
                  float nTotalSteps = (opt.maxValue - opt.minValue) / opt.step;
                  int nMsecPerStep = nTotalSteps < 20.f ? 500 : max(5, 250 - (int)(now - startOfPress) / 10); // discrete vs continuous sliding
                  int nSteps = (now - m_lastTweakKeyDown) / nMsecPerStep;
                  if (keyEvent == 1)
                  {
                     nSteps = 1;
                     m_lastTweakKeyDown = now - nSteps * nMsecPerStep;
                  }
                  if (nSteps > 0)
                  {
                     m_lastTweakKeyDown += nSteps * nMsecPerStep;
                     float value = m_live_table->m_settings.LoadValueWithDefault(opt.section, opt.id, opt.defaultValue);
                     if (!opt.literals.empty())
                     {
                        value += (float)nSteps * opt.step * step;
                        while (value < opt.minValue)
                           value += opt.maxValue - opt.minValue + 1;
                        while (value > opt.maxValue)
                           value -= opt.maxValue - opt.minValue + 1;
                     }
                     else
                        value = clamp(value + (float)nSteps * opt.step * step, opt.minValue, opt.maxValue);
                     table->m_settings.SaveValue(opt.section, opt.id, value);
                     if (opt.section == Settings::TableOption)
                        m_live_table->FireOptionEvent(1); // Table option changed event
                     else
                        VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(VPXPluginAPIImpl::GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_SETTINGS_CHANGED), nullptr);
                  }
                  else
                     modified = false;
               }
            }
            else
            {
               assert(false);
               break;
            }
         }
         m_tweakState[activeTweakSetting] |= modified ? 1 : 0;
      }
      else if (keyEvent == 1) // Key down
      {
         if (keycode == eLeftTiltKey && m_live_table->m_settings.LoadValueBool(Settings::Player, "EnableCameraModeFlyAround"s))
            m_live_table->mViewSetups[m_live_table->m_BG_current_set].mViewportRotation -= 1.0f;
         else if (keycode == eRightTiltKey && m_live_table->m_settings.LoadValueBool(Settings::Player, "EnableCameraModeFlyAround"s))
            m_live_table->mViewSetups[m_live_table->m_BG_current_set].mViewportRotation += 1.0f;
         else if (keycode == eStartGameKey) // Save tweak page
         {
            string iniFileName = m_live_table->GetSettingsFileName();
            string message;
            if (m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
            {
               message = "Point of view"s;
               m_live_table->mViewSetups[m_live_table->m_BG_current_set].SaveToTableOverrideSettings(m_table->m_settings, m_live_table->m_BG_current_set);
               if (m_live_table->m_BG_current_set == BG_FULLSCREEN)
               { // Player position is saved as an override (not saved if equal to app settings)
                  m_table->m_settings.SaveValue(Settings::Player, "ScreenPlayerX"s, m_live_table->m_settings.LoadValueFloat(Settings::Player, "ScreenPlayerX"s), true);
                  m_table->m_settings.SaveValue(Settings::Player, "ScreenPlayerY"s, m_live_table->m_settings.LoadValueFloat(Settings::Player, "ScreenPlayerY"s), true);
                  m_table->m_settings.SaveValue(Settings::Player, "ScreenPlayerZ"s, m_live_table->m_settings.LoadValueFloat(Settings::Player, "ScreenPlayerZ"s), true);
               }
               // The saved value are the new base value, so all fields are marked as untouched
               for (int i2 = BS_ViewMode; i2 < BS_WndBottomZOfs; i2++)
                  m_tweakState[i2] = 0;
            }
            else if (m_tweakPages[m_activeTweakPageIndex] == TP_VRPosition)
            {
               // Note that scene offset is not saved per table but as an app setting
               m_player->m_vrDevice->SaveVRSettings(g_pvp->m_settings);
               m_table->m_settings.SaveValue(Settings::PlayerVR, "UsePassthroughColor"s, m_renderer->m_vrApplyColorKey);
            }
            else if (m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
            {
               // Day Night slider
               if (m_tweakState[BS_DayNight] == 1)
               {
                  m_table->m_settings.SaveValue(Settings::Player, "OverrideTableEmissionScale"s, true);
                  m_table->m_settings.SaveValue(Settings::Player, "DynamicDayNight"s, false);
                  m_table->m_settings.SaveValue(Settings::Player, "EmissionScale"s, m_renderer->m_globalEmissionScale);
               }
               else if (m_tweakState[BS_DayNight] == 2)
               {
                  m_table->m_settings.DeleteValue(Settings::Player, "OverrideTableEmissionScale"s);
                  m_table->m_settings.DeleteValue(Settings::Player, "DynamicDayNight"s);
                  m_table->m_settings.DeleteValue(Settings::Player, "EmissionScale"s);
               }
               m_tweakState[BS_DayNight] = 0;
               // Exposure slider
               if (m_tweakState[BS_Exposure] == 1)
                  m_table->m_settings.SaveValue(Settings::TableOverride, "Exposure"s, m_renderer->m_exposure);
               else if (m_tweakState[BS_Exposure] == 2)
                  m_table->m_settings.DeleteValue(Settings::TableOverride, "Exposure"s);
               m_tweakState[BS_Exposure] = 0;
               // Tonemapper
               if (m_tweakState[BS_Tonemapper] == 1)
                  m_table->m_settings.SaveValue(Settings::TableOverride, "ToneMapper"s, m_renderer->m_toneMapper);
               else if (m_tweakState[BS_Tonemapper] == 2)
                  m_table->m_settings.DeleteValue(Settings::TableOverride, "ToneMapper"s);
               m_tweakState[BS_Tonemapper] = 0;
               // Difficulty
               if (m_tweakState[BS_Difficulty] != 0)
                  m_liveUI.PushNotification("You have changed the difficulty level\nThis change will only be applied after restart.", 10000);
               if (m_tweakState[BS_Difficulty] == 1)
                  m_table->m_settings.SaveValue(Settings::TableOverride, "Difficulty"s, m_live_table->m_globalDifficulty);
               else if (m_tweakState[BS_Difficulty] == 2)
                  m_table->m_settings.DeleteValue(Settings::TableOverride, "Difficulty"s);
               m_tweakState[BS_Difficulty] = 0;
               // Music/sound volume
               if (m_tweakState[BS_BackglassVolume] == 1)
                  m_table->m_settings.SaveValue(Settings::Player, "MusicVolume"s, m_player->m_MusicVolume);
               else if (m_tweakState[BS_BackglassVolume] == 2)
                  m_table->m_settings.DeleteValue(Settings::Player, "MusicVolume"s);
               m_tweakState[BS_BackglassVolume] = 0;
               if (m_tweakState[BS_PlayfieldVolume] == 1)
                  m_table->m_settings.SaveValue(Settings::Player, "SoundVolume"s, m_player->m_SoundVolume);
               else if (m_tweakState[BS_PlayfieldVolume] == 2)
                  m_table->m_settings.DeleteValue(Settings::Player, "SoundVolume"s);
               m_tweakState[BS_PlayfieldVolume] = 0;
            }
            // Custom table/plugin options
            if (m_tweakPages[m_activeTweakPageIndex] >= TP_TableOption)
            {
               const vector<Settings::OptionDef> &customOptions = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? m_live_table->m_settings.GetTableSettings() : Settings::GetPluginSettings();
               message = m_tweakPages[m_activeTweakPageIndex] > TP_TableOption ? "Plugin options"s : "Table options"s;
               const int nOptions = (int)customOptions.size();
               for (int i2 = 0; i2 < nOptions; i2++)
               {
                  const auto& opt = customOptions[i2];
                  if ((opt.section == Settings::TableOption && m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
                     || (opt.section > Settings::TableOption && m_tweakPages[m_activeTweakPageIndex] == static_cast<int>(TP_Plugin00) + static_cast<int>(opt.section) - static_cast<int>(Settings::Plugin00)))
                  {
                     if (m_tweakState[BS_Custom + i2] == 2)
                        m_table->m_settings.DeleteValue(opt.section, opt.id);
                     else
                        m_table->m_settings.SaveValue(opt.section, opt.id, m_live_table->m_settings.LoadValueWithDefault(opt.section, opt.name, opt.defaultValue));
                     m_tweakState[BS_Custom + i2] = 0;
                  }
               }
            }
            if (m_table->m_filename.empty() || !FileExists(m_table->m_filename))
            {
               m_liveUI.PushNotification("You need to save your table before exporting user settings"s, 5000);
            }
            else
            {
               m_table->m_settings.SaveToFile(iniFileName);
               m_liveUI.PushNotification(message + " exported to " + iniFileName, 5000);
            }
            if (g_pvp->m_povEdit && m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
               g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
         }
         else if (keycode == ePlungerKey) // Reset tweak page
         {
            if (m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
            {
               DisableStaticPrepass();
               // Remove custom day/night and get back to the one of the table, eventually overriden by app (not table) settings
               // FIXME we just default to the table value, missing the app settings being applied (like day/night from lat/lon,... see in player.cpp)
               m_tweakState[BS_DayNight] = 2;
               m_renderer->m_globalEmissionScale = m_table->m_globalEmissionScale;

               // Exposure
               m_tweakState[BS_Exposure] = 2;
               m_renderer->m_exposure = m_table->m_settings.LoadValueWithDefault(Settings::TableOverride, "Exposure"s, m_table->GetExposure());

               // Tonemapper
               m_tweakState[BS_Tonemapper] = 2;
               m_renderer->m_toneMapper = (ToneMapper)m_table->m_settings.LoadValueWithDefault(Settings::TableOverride, "ToneMapper"s, m_table->GetToneMapper());

               // Remove custom difficulty and get back to the one of the table, eventually overriden by app (not table) settings
               m_tweakState[BS_Difficulty] = 2;
               m_live_table->m_globalDifficulty = g_pvp->m_settings.LoadValueWithDefault(Settings::TableOverride, "Difficulty"s, m_table->m_difficulty);

               // Music/sound volume
               m_player->m_MusicVolume = m_table->m_settings.LoadValueWithDefault(Settings::Player, "MusicVolume"s, 100);
               m_player->m_SoundVolume = m_table->m_settings.LoadValueWithDefault(Settings::Player, "SoundVolume"s, 100);

               m_renderer->MarkShaderDirty();
            }
            else if (m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
            {
               DisableStaticPrepass();
               for (int i2 = BS_ViewMode; i2 < BS_WndBottomZOfs; i2++)
                  m_tweakState[i2] = 2;
               ViewSetupID id = table->m_BG_current_set;
               ViewSetup &viewSetup = table->mViewSetups[id];
               viewSetup.mViewportRotation = 0.f;
               const bool portrait = m_player->m_playfieldWnd->GetWidth() < m_player->m_playfieldWnd->GetHeight();
               switch (id)
               {
               case BG_DESKTOP:
               case BG_FSS:
                  m_liveUI.PushNotification("POV reset to default values"s, 5000);
                  if (id == BG_DESKTOP && !portrait)
                  { // Desktop
                     viewSetup.mMode = (ViewLayoutMode)g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopMode"s, VLM_CAMERA);
                     viewSetup.mViewX = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamX"s, 0.f));
                     viewSetup.mViewY = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamY"s, 20.f));
                     viewSetup.mViewZ = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamZ"s, 70.f));
                     viewSetup.mSceneScaleX = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleX"s, 1.f);
                     viewSetup.mSceneScaleY = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleY"s, 1.f);
                     viewSetup.mSceneScaleZ = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleZ"s, 1.f);
                     viewSetup.mFOV = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopFov"s, 50.f);
                     viewSetup.mLookAt = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopLookAt"s, 25.0f);
                     viewSetup.mViewVOfs = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopViewVOfs"s, 14.f);
                  }
                  else
                  { // FSS
                     viewSetup.mMode = (ViewLayoutMode)g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSMode"s, VLM_CAMERA);
                     viewSetup.mViewX = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamX"s, 0.f));
                     viewSetup.mViewY = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamY"s, 20.f));
                     viewSetup.mViewZ = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamZ"s, 70.f));
                     viewSetup.mSceneScaleX = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleX"s, 1.f);
                     viewSetup.mSceneScaleY = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleY"s, 1.f);
                     viewSetup.mSceneScaleZ = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleZ"s, 1.f);
                     viewSetup.mFOV = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSFov"s, 77.f);
                     viewSetup.mLookAt = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSLookAt"s, 50.0f);
                     viewSetup.mViewVOfs = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSViewVOfs"s, 22.f);
                  }
                  break;
               case BG_FULLSCREEN:
               {
                  const float screenWidth = g_pvp->m_settings.LoadValueFloat(Settings::Player, "ScreenWidth"s);
                  const float screenHeight = g_pvp->m_settings.LoadValueFloat(Settings::Player, "ScreenHeight"s);
                  if (screenWidth <= 1.f || screenHeight <= 1.f)
                  {
                     m_liveUI.PushNotification("You must setup your screen size before using Window mode"s, 5000);
                  }
                  else
                  {
                     float topHeight = table->m_glassTopHeight;
                     float bottomHeight = table->m_glassBottomHeight;
                     if (bottomHeight == topHeight)
                     { // If table does not define the glass position (for table without it, when loading we set the glass as horizontal)
                        TableDB db;
                        db.Load();
                        int bestSizeMatch = db.GetBestSizeMatch(table->GetTableWidth(), table->GetHeight(), topHeight);
                        if (bestSizeMatch >= 0)
                        {
                           bottomHeight = INCHESTOVPU(db.m_data[bestSizeMatch].glassBottom);
                           topHeight = INCHESTOVPU(db.m_data[bestSizeMatch].glassTop);
                           m_liveUI.PushNotification("Missing glass position guessed to be " + std::to_string(db.m_data[bestSizeMatch].glassBottom) + "\" / " + std::to_string(db.m_data[bestSizeMatch].glassTop) + "\" (" + db.m_data[bestSizeMatch].name + ')', 5000);
                        }
                        else
                        {
                           m_liveUI.PushNotification("The table is missing glass position and no good guess was found."s, 5000);
                        }
                     }
                     const float scale = (screenHeight / table->GetTableWidth()) * (table->GetHeight() / screenWidth);
                     const bool isFitted = (viewSetup.mViewHOfs == 0.f) && (viewSetup.mViewVOfs == -2.8f) && (viewSetup.mSceneScaleY == scale) && (viewSetup.mSceneScaleX == scale);
                     viewSetup.mMode = VLM_WINDOW;
                     viewSetup.mViewHOfs = 0.f;
                     viewSetup.mViewVOfs = isFitted ? 0.f : -2.8f;
                     viewSetup.mSceneScaleX = scale;
                     viewSetup.mSceneScaleY = isFitted ? 1.f : scale;
                     viewSetup.mWindowBottomZOfs = bottomHeight;
                     viewSetup.mWindowTopZOfs = topHeight;
                     m_liveUI.PushNotification(isFitted ? "POV reset to default values (stretch to fit)"s : "POV reset to default values (no stretching)"s, 5000);
                  }
                  break;
               }
               case BG_INVALID:
               case NUM_BG_SETS: assert(false); break;
               }
               m_renderer->m_cam = Vertex3Ds(0.f, 0.f, 0.f);
               UpdateTweakPage();
            }
            else if (m_tweakPages[m_activeTweakPageIndex] == TP_VRPosition)
            {
               m_player->m_vrDevice->RecenterTable();
            }
            // Reset custom table/plugin options
            else if (m_tweakPages[m_activeTweakPageIndex] >= TP_TableOption)
            {
               if (m_tweakPages[m_activeTweakPageIndex] > TP_TableOption)
                  m_liveUI.PushNotification("Plugin options reset to default values"s, 5000);
               else
                  m_liveUI.PushNotification("Table options reset to default values"s, 5000);
               const vector<Settings::OptionDef> &customOptions = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? m_live_table->m_settings.GetTableSettings() : Settings::GetPluginSettings();
               const int nOptions = (int)customOptions.size();
               for (int i2 = 0; i2 < nOptions; i2++)
               {
                  const auto& opt = customOptions[i2];
                  if ((opt.section == Settings::TableOption && m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
                     || (opt.section > Settings::TableOption && m_tweakPages[m_activeTweakPageIndex] == static_cast<int>(TP_Plugin00) + static_cast<int>(opt.section) - static_cast<int>(Settings::Plugin00)))
                  {
                     if (m_tweakState[BS_Custom + i2] == 2)
                        m_table->m_settings.DeleteValue(opt.section, opt.id);
                     else
                        m_table->m_settings.SaveValue(opt.section, opt.id, m_live_table->m_settings.LoadValueWithDefault(opt.section, opt.id, opt.defaultValue));
                     m_tweakState[BS_Custom + i2] = 0;
                  }
               }
               if (m_tweakPages[m_activeTweakPageIndex] > TP_TableOption)
                  VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(VPXPluginAPIImpl::GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_SETTINGS_CHANGED), nullptr);
               else
                  m_live_table->FireOptionEvent(2); // custom option resetted event
            }
         }
         else if (keycode == eAddCreditKey) // Undo tweaks of page
         {
            if (g_pvp->m_povEdit)
               // Tweak mode from command line => quit
               g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
            else
            {
               // Undo POV: copy from startup table to the live one
               if (m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
               {
                  DisableStaticPrepass();
                  m_liveUI.PushNotification("POV undo to startup values"s, 5000);
                  ViewSetupID id = m_live_table->m_BG_current_set;
                  const PinTable *const __restrict src = m_player->m_pEditorTable;
                  PinTable *const __restrict dst = m_live_table;
                  dst->mViewSetups[id] = src->mViewSetups[id];
                  dst->mViewSetups[id].ApplyTableOverrideSettings(m_live_table->m_settings, id);
                  m_renderer->m_cam = Vertex3Ds(0.f, 0.f, 0.f);
               }
               if (m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
               {
                  // TODO undo Day/Night, difficulty, ...
               }
            }
         }
         else if (keycode == eRightMagnaSave || keycode == eLeftMagnaSave)
         {
            if (keycode == eRightMagnaSave)
            {
               m_activeTweakIndex++;
               if (m_activeTweakIndex >= (int) m_tweakPageOptions.size())
                  m_activeTweakIndex = 0;
            }
            else
            {
               m_activeTweakIndex--;
               if (m_activeTweakIndex < 0)
                  m_activeTweakIndex = (int)m_tweakPageOptions.size() - 1;
            }
         }
      }
      else if (keyEvent == 0) // Continuous keypress
      {
         if ((keycode == ePlungerKey) && (m_tweakPages[m_activeTweakPageIndex] == TP_VRPosition))
            m_player->m_vrDevice->RecenterTable();
         else if (keycode == eLeftTiltKey && m_live_table->m_settings.LoadValueBool(Settings::Player, "EnableCameraModeFlyAround"s))
            m_live_table->mViewSetups[m_live_table->m_BG_current_set].mViewportRotation -= 1.0f;
         else if (keycode == eRightTiltKey && m_live_table->m_settings.LoadValueBool(Settings::Player, "EnableCameraModeFlyAround"s))
            m_live_table->mViewSetups[m_live_table->m_BG_current_set].mViewportRotation += 1.0f;
      }
   }
   m_prevInputState = state;
}

void InGameUI::Update()
{
   if (!m_isOpened)
      return;

   HandleTweakInput();

   ImGui::PushFont(m_liveUI.GetOverlayFont(), m_liveUI.GetOverlayFont()->LegacySize);
   PinTable *const table = m_live_table;
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   const float FontSize = ImGui::GetFontBaked()->Size;
   ImVec2 minSize(min(FontSize * (m_tweakPages[m_activeTweakPageIndex] == TP_Rules ? 35.0f
                                : m_tweakPages[m_activeTweakPageIndex] == TP_Info  ? 45.0f
                                                                                   : 30.0f),
                  min(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y)),0.f);
   ImVec2 maxSize(ImGui::GetIO().DisplaySize.x - 2.f * FontSize, 0.8f * ImGui::GetIO().DisplaySize.y - 1.f * FontSize);
   ImGui::SetNextWindowBgAlpha(0.5f);
   if (m_player->m_vrDevice)
      ImGui::SetNextWindowPos(ImVec2(0.5f * ImGui::GetIO().DisplaySize.x, 0.5f * ImGui::GetIO().DisplaySize.y), 0, ImVec2(0.5f, 0.5f));
   else
      ImGui::SetNextWindowPos(ImVec2(0.5f * ImGui::GetIO().DisplaySize.x, 0.8f * ImGui::GetIO().DisplaySize.y), 0, ImVec2(0.5f, 1.f));
   ImGui::SetNextWindowSizeConstraints(minSize, maxSize);
   ImGui::Begin("TweakUI", nullptr, window_flags);

   ViewSetupID vsId = table->m_BG_current_set;
   ViewSetup &viewSetup = table->mViewSetups[vsId];
   const bool isLegacy = viewSetup.mMode == VLM_LEGACY;
   const bool isCamera = viewSetup.mMode == VLM_CAMERA;
   const bool isWindow = viewSetup.mMode == VLM_WINDOW;

   BackdropSetting activeTweakSetting = m_tweakPageOptions[m_activeTweakIndex];
   if (ImGui::BeginTable("TweakTable", 4, /* ImGuiTableFlags_Borders */ 0))
   {
      static float vWidth = 50.f;
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, vWidth);
      ImGui::TableSetupColumn("Unit", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
      #define CM_ROW(id, label, format, value, unit) \
      { \
         char buf[1024]; snprintf(buf, sizeof(buf), format, value); \
         ImGui::TableNextColumn(); ImGui::TextUnformatted(label); ImGui::TableNextColumn(); \
         float textWidth = ImGui::CalcTextSize(buf).x; vWidth = max(vWidth, textWidth); \
         if (textWidth < vWidth) ImGui::SameLine(vWidth - textWidth); \
         ImGui::TextUnformatted(buf); ImGui::TableNextColumn(); \
         ImGui::TextUnformatted(unit); ImGui::TableNextColumn(); \
         ImGui::TextUnformatted(m_tweakState[id] == 0 ? "  " : m_tweakState[id] == 1 ? " **" : " *"); ImGui::TableNextRow(); \
      }
      #define CM_SKIP_LINE {ImGui::TableNextColumn(); ImGui::Dummy(ImVec2(0.f, m_liveUI.GetDPI() * 3.f)); ImGui::TableNextRow();}
      const float realToVirtual = viewSetup.GetRealToVirtualScale(table);
      for (int setting : m_tweakPageOptions)
      {
         const bool highlight = (setting == activeTweakSetting)
                             || (activeTweakSetting == BS_XYZScale && (setting == BS_XScale || setting == BS_YScale || setting == BS_ZScale))
                             || (activeTweakSetting == BS_Volume && (setting == BS_BackglassVolume || setting == BS_PlayfieldVolume));
         if (highlight)
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
         if (setting >= BS_Custom)
         {
            const vector<Settings::OptionDef> &customOptions = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? m_live_table->m_settings.GetTableSettings() : Settings::GetPluginSettings();
            if (setting - BS_Custom >= (int)customOptions.size())
               continue;
            const Settings::OptionDef &opt = customOptions[setting - BS_Custom];
            float value = table->m_settings.LoadValueWithDefault(opt.section, opt.id, opt.defaultValue);
            const string label = opt.name + ": ";
            if (!opt.literals.empty()) // List of values
            {
               int index = (int) (value - opt.minValue);
               if (index < 0 || index >= (int)opt.literals.size())
                  index = (int)(opt.defaultValue - opt.minValue);
               CM_ROW(setting, label.c_str(), "%s", opt.literals[index].c_str(), "");
            }
            else if (opt.unit == Settings::OT_PERCENT) // Percent value
            {
               CM_ROW(setting, label.c_str(), "%.1f", 100.f * value, "%");
            }
            else // OT_NONE
            {
               CM_ROW(setting, label.c_str(), "%.1f", value, "");
            }
         }
         else switch (setting)
         {
         case BS_Page: {
            const int page = m_tweakPages[m_activeTweakPageIndex];
            string title;
            if (page >= TP_Plugin00)
            {
               const string& sectionName = Settings::GetSectionName((Settings::Section)(Settings::Plugin00 + page - TP_Plugin00));
               const std::shared_ptr<MsgPlugin> plugin = sectionName.length() > 7 ? MsgPluginManager::GetInstance().GetPlugin(std::string_view(sectionName).substr(7)) : nullptr;
               if (plugin)
                  title = plugin->m_name + " Plugin";
               else
                  title = "Invalid Plugin"s;
            }
            else 
               title = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? "Table Options"s
                     : m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView ? "Point of View"s
                     : m_tweakPages[m_activeTweakPageIndex] == TP_VRPosition  ? "VR Scene Position"s
                     : m_tweakPages[m_activeTweakPageIndex] == TP_Rules       ? "Rules"s
                                                                              : "Information"s;
            CM_ROW(setting, "Page "s.append(std::to_string(1 + m_activeTweakPageIndex)).append(1,'/').append(std::to_string(m_tweakPages.size())).c_str(), "%s", title.c_str(), "");
            CM_SKIP_LINE;
            break;
         }

         // View setup
         case BS_ViewMode: CM_ROW(setting, "View Layout Mode:", "%s", isLegacy ? "Legacy" : isCamera ? "Camera" : "Window", ""); CM_SKIP_LINE; break;
         case BS_XYZScale: break;
         case BS_XScale: CM_ROW(setting, "Table X Scale", "%.1f", 100.f * viewSetup.mSceneScaleX / realToVirtual, "%"); break;
         case BS_YScale: CM_ROW(setting, isWindow ? "Table YZ Scale" : "Table Y Scale", "%.1f", 100.f * viewSetup.mSceneScaleY / realToVirtual, "%"); break;
         case BS_ZScale: CM_ROW(setting, "Table Z Scale", "%.1f", 100.f * viewSetup.mSceneScaleZ / realToVirtual, "%"); CM_SKIP_LINE; break;
         case BS_LookAt:  if (isLegacy) { CM_ROW(setting, "Inclination", "%.1f", viewSetup.mLookAt, "deg"); } else { CM_ROW(setting, "Look at", "%.1f", viewSetup.mLookAt, "%"); } break;
         case BS_XOffset: CM_ROW(setting, isLegacy ? "X Offset" : isWindow ? "Player X" : "Camera X", "%.1f", isWindow ? table->m_settings.LoadValueFloat(Settings::Player, "ScreenPlayerX"s) : VPUTOCM(viewSetup.mViewX), "cm"); break;
         case BS_YOffset: CM_ROW(setting, isLegacy ? "Y Offset" : isWindow ? "Player Y" : "Camera Y", "%.1f", isWindow ? table->m_settings.LoadValueFloat(Settings::Player, "ScreenPlayerY"s) : VPUTOCM(viewSetup.mViewY), "cm"); break;
         case BS_ZOffset: CM_ROW(setting, isLegacy ? "Z Offset" : isWindow ? "Player Z" : "Camera Z", "%.1f", isWindow ? table->m_settings.LoadValueFloat(Settings::Player, "ScreenPlayerZ"s) : VPUTOCM(viewSetup.mViewZ), "cm"); CM_SKIP_LINE; break;
         case BS_FOV: CM_ROW(setting, "Field Of View (overall scale)", "%.1f", viewSetup.mFOV, "deg"); break;
         case BS_Layback: CM_ROW(setting, "Layback", "%.1f", viewSetup.mLayback, ""); CM_SKIP_LINE; break;
         case BS_ViewHOfs: CM_ROW(setting, "Horizontal Offset", "%.1f", viewSetup.mViewHOfs, isWindow ? "cm" : ""); break;
         case BS_ViewVOfs: CM_ROW(setting, "Vertical Offset", "%.1f", viewSetup.mViewVOfs, isWindow ? "cm" : ""); CM_SKIP_LINE; break;
         case BS_WndTopZOfs: CM_ROW(setting, "Window Top Z Ofs.", "%.1f", VPUTOCM(viewSetup.mWindowTopZOfs), "cm"); break;
         case BS_WndBottomZOfs: CM_ROW(setting, "Window Bottom Z Ofs.", "%.1f", VPUTOCM(viewSetup.mWindowBottomZOfs), "cm"); CM_SKIP_LINE; break;

         // VR Position
         case BS_VROrientation: CM_ROW(setting, "Scene Orientation", "%.1f", m_player->m_vrDevice->GetSceneOrientation(), "°"); break;
         case BS_VRX: CM_ROW(setting, "Scene Offset X", "%.1f", m_player->m_vrDevice->GetSceneOffset().x, "cm"); break;
         case BS_VRY: CM_ROW(setting, "Scene Offset Y", "%.1f", m_player->m_vrDevice->GetSceneOffset().y, "cm"); break;
         case BS_VRZ: CM_ROW(setting, "Scene Offset Z", "%.1f", m_player->m_vrDevice->GetSceneOffset().z, "cm"); break;
         case BS_VRScale: CM_ROW(setting, "Lockbar width", "%.1f", m_player->m_vrDevice->GetLockbarWidth(), "cm"); break;
         case BS_AR_VR: CM_ROW(setting, "Color Keyed Passthrough:", "%s", m_renderer->m_vrApplyColorKey ? "Enabled" : "Disabled", ""); break;

         // Table options
         case BS_DayNight: CM_ROW(setting, "Day Night: ", "%.1f", 100.f * m_renderer->m_globalEmissionScale, "%"); break;
         case BS_Difficulty:
            char label[64];
            snprintf(label, std::size(label), "Difficulty (%.2f° slope and trajectories scattering):", m_live_table->GetPlayfieldSlope());
            CM_ROW(setting, label, "%.1f", 100.f * m_live_table->m_globalDifficulty, "%");
            break;
         case BS_Exposure: CM_ROW(setting, "Exposure: ", "%.1f", 100.f * m_renderer->m_exposure, "%"); break;
         case BS_Tonemapper: CM_ROW(setting, "Tonemapper: ", "%s", m_renderer->m_toneMapper == TM_REINHARD   ? "Reinhard"
                                                                 : m_renderer->m_toneMapper == TM_FILMIC     ? "Filmic" 
                                                                 : m_renderer->m_toneMapper == TM_NEUTRAL    ? "Neutral"
                                                                 : m_renderer->m_toneMapper == TM_AGX        ? "AgX"
                                                                 : m_renderer->m_toneMapper == TM_AGX_PUNCHY ? "AgX Punchy"
                                                                 : m_renderer->m_toneMapper == TM_WCG_SPLINE ? "WCG Display" : "Invalid",""); // Should not happen as this tonemapper is not exposed to user
            break;
         case BS_Volume: break;
         case BS_BackglassVolume: CM_ROW(setting, "Backglass Volume: ", "%d", m_player->m_MusicVolume, "%"); break;
         case BS_PlayfieldVolume: CM_ROW(setting, "Playfield Volume: ", "%d", m_player->m_SoundVolume, "%"); break;

         }
         if (highlight)
            ImGui::PopStyleColor();
      }
      #undef CM_ROW
      #undef CM_SKIP_LINE
      ImGui::EndTable();
   }

   if (m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
   {
      if (isLegacy)
      {
         // Useless for absolute mode: the camera is always where we put it
         Matrix3D view = m_renderer->GetMVP().GetView();
         view.Invert();
         const vec3 pos = view.GetOrthoNormalPos();
         ImGui::Text("Camera at X: %.0fcm Y: %.0fcm Z: %.0fcm, Rotation: %.2f", 
            VPUTOCM(pos.x - 0.5f * m_live_table->m_right), 
            VPUTOCM(pos.y - m_live_table->m_bottom), 
            VPUTOCM(pos.z), viewSetup.mViewportRotation);
         ImGui::NewLine();
      }
      if (isWindow)
      {
         // Do not show it as it is more confusing than helpful due to the use of different coordinate systems in settings vs live edit
         //ImGui::Text("Camera at X: %.1fcm Y: %.1fcm Z: %.1fcm", VPUTOCM(viewSetup.mViewX), VPUTOCM(viewSetup.mViewY), VPUTOCM(viewSetup.mViewZ));
         //ImGui::NewLine();
         if (m_live_table->m_settings.LoadValueFloat(Settings::Player, "ScreenWidth"s) <= 1.f)
         {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::TextUnformatted("You are using 'Window' mode but haven't defined your display physical size.");
            ImGui::TextUnformatted("This will break the overall scale as well as the stereo rendering.");
            ImGui::NewLine();
            ImGui::PopStyleColor();
         }
         viewSetup.SetWindowModeFromSettings(m_live_table);
      }
      m_renderer->InitLayout();
   }

   if (m_tweakPages[m_activeTweakPageIndex] == TP_Rules)
   {
      static float lastHeight = 0.f, maxScroll = 0.f;
      m_tweakScroll = clamp(m_tweakScroll, 0.f, maxScroll);
      ImGui::SetNextWindowScroll(ImVec2(0.f, m_tweakScroll));
      ImGui::SetNextWindowSizeConstraints(ImVec2(0.f, 0.f), ImVec2(FLT_MAX, lastHeight));
      if (ImGui::BeginChild("Rules", ImVec2(0.f, 0.f), 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar /* | ImGuiWindowFlags_AlwaysVerticalScrollbar */))
      {
         m_liveUI.SetMarkdownStartId(ImGui::GetItemID());
         ImGui::Markdown(m_table->m_rules.c_str(), m_table->m_rules.length(), m_liveUI.GetMarkdownConfig());
         lastHeight = ImGui::GetCursorPos().y - ImGui::GetCursorStartPos().y; // Height of content
         maxScroll = ImGui::GetScrollMaxY();
      }
      ImGui::EndChild();
      ImGui::NewLine();
   }
   else if (m_tweakPages[m_activeTweakPageIndex] == TP_Info)
   {
      static float lastHeight = 0.f, maxScroll = 0.f;
      m_tweakScroll = clamp(m_tweakScroll, 0.f, maxScroll);
      ImGui::SetNextWindowScroll(ImVec2(0.f, m_tweakScroll));
      ImGui::SetNextWindowSizeConstraints(ImVec2(0.f, 0.f), ImVec2(FLT_MAX, lastHeight));
      if (ImGui::BeginChild("Info", ImVec2(0.f, 0.f), 0, ImGuiWindowFlags_NoBackground))
      {
         m_liveUI.SetMarkdownStartId(ImGui::GetItemID());
         ImGui::Markdown(m_table->m_description.c_str(), m_table->m_description.length(), m_liveUI.GetMarkdownConfig());
         lastHeight = ImGui::GetCursorPos().y - ImGui::GetCursorStartPos().y; // Height of content
         maxScroll = ImGui::GetScrollMaxY();
      }
      ImGui::EndChild();
      ImGui::NewLine();
   }

   ImGui::NewLine();
   vector<string> infos;
   if (m_tweakPages[m_activeTweakPageIndex] != TP_Rules && m_tweakPages[m_activeTweakPageIndex] != TP_Info)
   {
      infos.push_back("Plunger Key:   Reset page to defaults"s);
      if (m_app->m_povEdit)
      {
         infos.push_back("Start Key:   Export settings and quit"s);
         infos.push_back("Credit Key:   Quit without export"s);
      }
      else
      {
         infos.push_back("Start Key:   Export settings file"s);
         infos.push_back("Credit Key:   Reset page to old values"s);
      }
      infos.push_back("Magna save keys:   Previous/Next option"s);
      if (m_live_table->m_settings.LoadValueBool(Settings::Player, "EnableCameraModeFlyAround"s))
      {
         infos.push_back("Nudge key:   Rotate table orientation"s);
         infos.push_back("Arrows & Left Alt Key:   Navigate around"s);
      }
   }
   infos.push_back(activeTweakSetting == BS_Page ? "Flipper keys:   Previous/Next page"s : "Flipper keys:   Adjust highlighted value"s);
   const uint32_t info = ((msec() - m_openTimeMs) / 2000u) % (uint32_t)infos.size();
   ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
   m_liveUI.CenteredText(infos[info]);
   ImGui::PopStyleColor();

   ImGui::End();
   ImGui::PopFont();
}
