// license:GPLv3+

#include "core/stdafx.h"

#include "ScoreViewPage.h"
#include "imgui/imgui.h"

namespace VPX::InGameUI
{

ScoreViewPage::ScoreViewPage()
   : InGameUIPage("Score View"s,
     "Adjust the position and size of the score view window\nOnly available when using embedded in playfield output"s, SaveMode::Both)
{
   if (!ShouldShow())
      return;

   const int screenWidth = m_player->m_playfieldWnd->GetWidth();
   const int screenHeight = m_player->m_playfieldWnd->GetHeight();

   const int currentW = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetWidth();
   const int currentH = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetHeight();

   if (currentH > 0) {
      m_aspectRatio = static_cast<float>(currentW) / static_cast<float>(currentH);
      m_initialAspectRatio = m_aspectRatio;
   }
   else {
      m_aspectRatio = 128.0f / 32.0f;
      m_initialAspectRatio = m_aspectRatio;
   }

   const string assetsPath = g_pvp->m_myPath + "assets" + PATH_SEPARATOR_CHAR + "ui" + PATH_SEPARATOR_CHAR;

   m_posNone = BaseTexture::CreateFromFile(assetsPath + "pos-none.png");
   m_posUpperLeft = BaseTexture::CreateFromFile(assetsPath + "pos-upper-left.png");
   m_posUpperCenter = BaseTexture::CreateFromFile(assetsPath + "pos-upper-center.png");
   m_posUpperRight = BaseTexture::CreateFromFile(assetsPath + "pos-upper-right.png");
   m_posLowerLeft = BaseTexture::CreateFromFile(assetsPath + "pos-lower-left.png");
   m_posLowerCenter = BaseTexture::CreateFromFile(assetsPath + "pos-lower-center.png");
   m_posLowerRight = BaseTexture::CreateFromFile(assetsPath + "pos-lower-right.png");

   auto presetButtons = std::make_unique<InGameUIItem>(
      ""s, "Aspect ratio and position preset buttons"s,
      [this](int itemIndex, const InGameUIItem* item) {
         const float buttonSize = 32.0f;
         const float buttonSpacing = 5.0f;
         const float sectionSpacing = 15.0f;

         int currentW = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetWidth();
         int currentH = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetHeight();
         const bool sizeZero = (currentW == 0 || currentH == 0);

         const float aspectButtonWidth = 60.0f;

         ImGui::BeginGroup();
         {
            ImGui::Text("Presets");
            ImGui::SameLine(0, 10);

            if (ImGui::Button("128x16", ImVec2(aspectButtonWidth, buttonSize))) {
               ApplySize(128, 16);
            }
            ImGui::SameLine(0, buttonSpacing);

            if (ImGui::Button("128x32", ImVec2(aspectButtonWidth, buttonSize))) {
               ApplySize(128, 32);
            }
            ImGui::SameLine(0, buttonSpacing);

            if (ImGui::Button("194x64", ImVec2(aspectButtonWidth, buttonSize))) {
               ApplySize(194, 64);
            }
         }
         ImGui::EndGroup();

         ImGui::Spacing();

         ImGui::BeginGroup();
         {
            if (sizeZero) {
               ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
            }

            vector<std::pair<std::shared_ptr<BaseTexture>, PresetPosition>> allPositions = {
               {m_posNone, PresetPosition::None},
               {m_posUpperLeft, PresetPosition::UpperLeft},
               {m_posUpperCenter, PresetPosition::UpperCenter},
               {m_posUpperRight, PresetPosition::UpperRight},
               {m_posLowerLeft, PresetPosition::LowerLeft},
               {m_posLowerCenter, PresetPosition::LowerCenter},
               {m_posLowerRight, PresetPosition::LowerRight}
            };

            for (size_t i = 0; i < allPositions.size(); ++i) {
               const auto& [texture, position] = allPositions[i];
               if (texture) {
                  ImTextureID texId = m_player->m_renderer->m_renderDevice->m_texMan.LoadTexture(texture.get(), false);
                  if (texId && ImGui::ImageButton(("pos_" + std::to_string(i)).c_str(),
                                                 texId, ImVec2(buttonSize, buttonSize)) && !sizeZero) {
                     ApplyPosition(position);
                  }
               }
               if (i < allPositions.size() - 1) {
                  ImGui::SameLine(0, buttonSpacing);
               }
            }

            if (sizeZero) {
               ImGui::PopStyleVar();
            }
         }
         ImGui::EndGroup();
      }
   );
   AddItem(std::move(presetButtons));

   auto customControls = std::make_unique<InGameUIItem>(
      ""s, "Position controls"s,
      [this, screenWidth, screenHeight](int itemIndex, const InGameUIItem* item) {
         int x, y, w, h;
         m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetPos(x, y);
         w = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetWidth();
         h = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetHeight();

         const bool isZeroSize = (w <= 0 || h <= 0);
         m_lastSizeZero = isZeroSize;

         auto renderEnhancedSlider = [this, screenWidth, screenHeight, isZeroSize](
            const char* label, int& value, int min_val, int currentW, int currentH, bool isX) {

            int max_val;
            if (isZeroSize) {
               max_val = isX ? screenWidth : screenHeight;
            } else {
               if (isX) {
                  max_val = std::max(0, screenWidth - currentW);
               } else {
                  max_val = std::max(0, screenHeight - currentH);
               }
            }

            ImGui::Text("%s", label);

            if (isZeroSize) {
               ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
            }

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 85);
            if (isZeroSize) {
               ImGui::BeginDisabled();
            }
            ImGui::SliderInt(("##" + string(label)).c_str(), &value, min_val, max_val);
            if (isZeroSize) {
               ImGui::EndDisabled();
            }
            ImGui::SameLine(0, 5);

            if (isZeroSize) {
               ImGui::BeginDisabled();
            }
            if (ImGui::Button(("-##" + string(label)).c_str(), ImVec2(30, 0))) {
               value = max(min_val, value - 1);
            }
            if (isZeroSize) {
               ImGui::EndDisabled();
            }
            ImGui::SameLine(0, 5);

            if (isZeroSize) {
               ImGui::BeginDisabled();
            }
            if (ImGui::Button(("+##" + string(label)).c_str(), ImVec2(30, 0))) {
               value = min(max_val, value + 1);
            }
            if (isZeroSize) {
               ImGui::EndDisabled();
            }

            if (isZeroSize) {
               ImGui::PopStyleVar();
            }
         };

         int newX = x;
         renderEnhancedSlider("X", newX, 0, w, h, true);

         int newY = y;
         renderEnhancedSlider("Y", newY, 0, w, h, false);

         if (newX != x || newY != y) {
            m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndX"s, newX);
            m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndY"s, newY);

            if (m_player->m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED) {
               m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetPos(newX, newY);
            }
         }
      }
   );
   AddItem(std::move(customControls));

   auto lockAspectRatio = std::make_unique<InGameUIItem>(
      "Lock Aspect Ratio"s, "Keep width and height proportional when resizing"s, true,
      [this]() { return m_lockAspectRatio; },
      [this](bool v) { m_lockAspectRatio = v; },
      [](Settings&) { },
      [](bool, const Settings&, bool) { });
   AddItem(std::move(lockAspectRatio));

   auto sizeControls = std::make_unique<InGameUIItem>(
      ""s, "Width and height controls"s,
      [this, screenWidth, screenHeight](int itemIndex, const InGameUIItem* item) {
         int x, y, w, h;
         m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetPos(x, y);
         w = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetWidth();
         h = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetHeight();

         auto renderEnhancedSizeSlider = [this, screenWidth, screenHeight](
            const char* label, int& value, int min_val, int currentX, int currentY, bool isWidth) {

            const float ar = GetSafeAspectRatio();
            int max_val;
            if (isWidth) {
               max_val = GetAllowedMaxWidth(ar, screenWidth, screenHeight, currentX, currentY);
            } else {
               max_val = GetAllowedMaxHeight(ar, screenWidth, screenHeight, currentX, currentY);
            }

            ImGui::Text("%s", label);

            const string controlId = string(label);
            const bool isActiveControl = (m_activeControl == controlId);

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 85);
            const int oldValue = value;
            if (ImGui::SliderInt(("##" + controlId).c_str(), &value, min_val, max_val)) {
               m_activeControl = controlId;
            }
            ImGui::SameLine(0, 5);

            if (ImGui::Button(("-##" + controlId).c_str(), ImVec2(30, 0))) {
               value = max(min_val, value - 1);
               m_activeControl = controlId;
            }
            ImGui::SameLine(0, 5);

            if (ImGui::Button(("+##" + controlId).c_str(), ImVec2(30, 0))) {
               value = min(max_val, value + 1);
               m_activeControl = controlId;
            }

            return (value != oldValue);
         };

         int newW = w;
         int newH = h;

         bool widthChanged = renderEnhancedSizeSlider("Width", newW, 0, x, y, true);
         bool heightChanged = renderEnhancedSizeSlider("Height", newH, 0, x, y, false);

         if (m_lockAspectRatio && (widthChanged || heightChanged)) {
            const float ar = GetSafeAspectRatio();

            if (m_activeControl == "Width" && widthChanged) {
               ApplyFromWidth(newW, ar, screenWidth, screenHeight, newW, newH);
               m_aspectRatio = ar;
            } else if (m_activeControl == "Height" && heightChanged) {
               ApplyFromHeight(newH, ar, screenWidth, screenHeight, newW, newH);
               m_aspectRatio = ar;
            }
         }

         if (!m_lockAspectRatio && newW > 0 && newH > 0) {
            m_aspectRatio = static_cast<float>(newW) / static_cast<float>(newH);
         }

         int newX = clamp(x, 0, max(0, screenWidth - newW));
         int newY = clamp(y, 0, max(0, screenHeight - newH));

         if (newX != x || newY != y || newW != w || newH != h) {
            m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndX"s, newX);
            m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndY"s, newY);
            m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWidth"s, newW);
            m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewHeight"s, newH);

            if (m_player->m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED) {
               m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetPos(newX, newY);
               m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetSize(newW, newH);
            }
         }
      }
   );
   AddItem(std::move(sizeControls));

   m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetPos(m_initialX, m_initialY);
   m_initialWidth = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetWidth();
   m_initialHeight = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetHeight();
}

float ScoreViewPage::GetSafeAspectRatio() const
{
   const float initialAR = m_initialAspectRatio > 0 ? m_initialAspectRatio : 1.0f;
   return (std::isfinite(m_aspectRatio) && m_aspectRatio > 0) ? m_aspectRatio : initialAR;
}

int ScoreViewPage::GetAllowedMaxWidth(float ar, int screenWidth, int screenHeight, int x, int y) const
{
   return std::max(0, std::min(screenWidth - x, static_cast<int>((screenHeight - y) * ar)));
}

int ScoreViewPage::GetAllowedMaxHeight(float ar, int screenWidth, int screenHeight, int x, int y) const
{
   return std::max(0, std::min(screenHeight - y, static_cast<int>((screenWidth - x) / ar)));
}

void ScoreViewPage::ApplyFromWidth(int desiredW, float ar, int screenWidth, int screenHeight, int& w, int& h) const
{
   int x, y;
   m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetPos(x, y);

   const int maxW = GetAllowedMaxWidth(ar, screenWidth, screenHeight, x, y);
   const int newW = clamp(desiredW, 0, maxW);
   w = newW;
   h = static_cast<int>(std::round(newW / ar));
}

void ScoreViewPage::ApplyFromHeight(int desiredH, float ar, int screenWidth, int screenHeight, int& w, int& h) const
{
   int x, y;
   m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetPos(x, y);

   const int maxH = GetAllowedMaxHeight(ar, screenWidth, screenHeight, x, y);
   const int newH = clamp(desiredH, 0, maxH);
   h = newH;
   w = static_cast<int>(std::round(newH * ar));
}

void ScoreViewPage::ApplySize(int width, int height)
{
   if (!ShouldShow())
      return;

   const int screenWidth = m_player->m_playfieldWnd->GetWidth();
   const int screenHeight = m_player->m_playfieldWnd->GetHeight();

   const float aspect = (float)width / (float)height;
   int x, y;
   m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetPos(x, y);
   const int widthLimit = screenWidth - x;
   const int heightLimit = screenHeight - y;

   int newW = std::min(widthLimit, static_cast<int>(std::max(0.0f, screenWidth * 0.5f)));
   int newH = static_cast<int>(newW / aspect);

   if (newH > heightLimit) {
      newH = heightLimit;
      newW = static_cast<int>(newH * aspect);
   }

   m_aspectRatio = aspect;
   x = clamp(x, 0, std::max(0, screenWidth - newW));
   y = clamp(y, 0, std::max(0, screenHeight - newH));

   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndX"s, x);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndY"s, y);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWidth"s, newW);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewHeight"s, newH);

   if (m_player->m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED) {
      m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetPos(x, y);
      m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetSize(newW, newH);
   }
}

void ScoreViewPage::ApplyPosition(PresetPosition position)
{
   if (!ShouldShow())
      return;

   const int screenWidth = m_player->m_playfieldWnd->GetWidth();
   const int screenHeight = m_player->m_playfieldWnd->GetHeight();

   const int width = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetWidth();
   const int height = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetHeight();

   int x, y;
   switch (position) {
      case PresetPosition::None:
         x = 0;
         y = 0;
         m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWidth"s, 0);
         m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewHeight"s, 0);
         if (m_player->m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED) {
            m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetSize(0, 0);
         }
         break;
      case PresetPosition::UpperLeft:
         x = 0;
         y = 0;
         break;
      case PresetPosition::UpperCenter:
         x = (screenWidth - width) / 2;
         y = 0;
         break;
      case PresetPosition::UpperRight:
         x = screenWidth - width;
         y = 0;
         break;
      case PresetPosition::LowerLeft:
         x = 0;
         y = screenHeight - height;
         break;
      case PresetPosition::LowerCenter:
         x = (screenWidth - width) / 2;
         y = screenHeight - height;
         break;
      case PresetPosition::LowerRight:
         x = screenWidth - width;
         y = screenHeight - height;
         break;
   }

   x = clamp(x, 0, max(0, screenWidth - width));
   y = clamp(y, 0, max(0, screenHeight - height));

   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndX"s, x);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndY"s, y);

   if (m_player->m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED) {
      m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetPos(x, y);
   }

}

bool ScoreViewPage::ShouldShow() const
{
   return m_player->m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED;
}

bool ScoreViewPage::IsDefaults() const
{
   const Settings& settings = m_player->m_ptable->m_settings;

   int value;
   if (settings.LoadValue(Settings::ScoreView, "ScoreViewWndX"s, value) && value != 0) return false;
   if (settings.LoadValue(Settings::ScoreView, "ScoreViewWndY"s, value) && value != 0) return false;
   if (settings.LoadValue(Settings::ScoreView, "ScoreViewWidth"s, value) && value != 128) return false;
   if (settings.LoadValue(Settings::ScoreView, "ScoreViewHeight"s, value) && value != 32) return false;

   return true;
}

bool ScoreViewPage::IsModified() const
{
   int currentX, currentY;
   m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetPos(currentX, currentY);
   const int currentWidth = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetWidth();
   const int currentHeight = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetHeight();

   return (currentX != m_initialX || currentY != m_initialY ||
           currentWidth != m_initialWidth || currentHeight != m_initialHeight);
}

void ScoreViewPage::ResetToDefaults()
{
   if (IsDefaults()) return;

   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndX"s, 0);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndY"s, 0);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWidth"s, 128);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewHeight"s, 32);

   if (m_player->m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED) {
      m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetPos(0, 0);
      m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetSize(128, 32);
   }

   m_aspectRatio = 128.0f / 32.0f;
}

void ScoreViewPage::ResetToInitialValues()
{
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndX"s, m_initialX);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWndY"s, m_initialY);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewWidth"s, m_initialWidth);
   m_player->m_ptable->m_settings.SaveValue(Settings::ScoreView, "ScoreViewHeight"s, m_initialHeight);

   if (m_player->m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED) {
      m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetPos(m_initialX, m_initialY);
      m_player->m_scoreViewOutput.GetEmbeddedWindow()->SetSize(m_initialWidth, m_initialHeight);
   }

   if (m_initialHeight > 0) {
      m_aspectRatio = static_cast<float>(m_initialWidth) / static_cast<float>(m_initialHeight);
   }
}

void ScoreViewPage::SaveGlobally()
{
   int currentX, currentY;
   m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetPos(currentX, currentY);
   const int currentWidth = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetWidth();
   const int currentHeight = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetHeight();

   Settings& tableSettings = m_player->m_ptable->m_settings;
   tableSettings.DeleteValue(Settings::ScoreView, "ScoreViewWndX"s);
   tableSettings.DeleteValue(Settings::ScoreView, "ScoreViewWndY"s);
   tableSettings.DeleteValue(Settings::ScoreView, "ScoreViewWidth"s);
   tableSettings.DeleteValue(Settings::ScoreView, "ScoreViewHeight"s);
   tableSettings.Save();

   Settings& globalSettings = g_pvp->m_settings;
   globalSettings.SaveValue(Settings::ScoreView, "ScoreViewWndX"s, currentX);
   globalSettings.SaveValue(Settings::ScoreView, "ScoreViewWndY"s, currentY);
   globalSettings.SaveValue(Settings::ScoreView, "ScoreViewWidth"s, currentWidth);
   globalSettings.SaveValue(Settings::ScoreView, "ScoreViewHeight"s, currentHeight);
   globalSettings.Save();

   m_initialX = currentX;
   m_initialY = currentY;
   m_initialWidth = currentWidth;
   m_initialHeight = currentHeight;
}

void ScoreViewPage::SaveTableOverride()
{
   int currentX, currentY;
   m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetPos(currentX, currentY);
   const int currentWidth = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetWidth();
   const int currentHeight = m_player->m_scoreViewOutput.GetEmbeddedWindow()->GetHeight();

   Settings& tableSettings = m_player->m_ptable->m_settings;
   tableSettings.SaveValue(Settings::ScoreView, "ScoreViewWndX"s, currentX);
   tableSettings.SaveValue(Settings::ScoreView, "ScoreViewWndY"s, currentY);
   tableSettings.SaveValue(Settings::ScoreView, "ScoreViewWidth"s, currentWidth);
   tableSettings.SaveValue(Settings::ScoreView, "ScoreViewHeight"s, currentHeight);
   tableSettings.Save();

   m_initialX = currentX;
   m_initialY = currentY;
   m_initialWidth = currentWidth;
   m_initialHeight = currentHeight;
}

}
