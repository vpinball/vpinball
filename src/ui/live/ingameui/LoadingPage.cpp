// license:GPLv3+

#include "core/stdafx.h"
#include "LoadingPage.h"

#include "core/player.h"
#include "core/VPApp.h"
#include "renderer/Sampler.h"
#include "renderer/Texture.h"
#include "ui/live/LiveUI.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <set>

namespace VPX::InGameUI
{

namespace
{

   // Popup page proposing the different scopes for discarding the remaining texture compressions
   class SkipTextureCompressionPage final : public InGameUIPage
   {
   public:
      SkipTextureCompressionPage()
         : InGameUIPage("Skip texture compression ?"s,
              "The remaining textures will be uploaded to the GPU uncompressed, using more GPU memory.\n"
              "Texture compression can also be adjusted in the graphics settings ('Compress Textures')."s,
              SaveMode::None)
      {
      }

      bool IsPlayerPauseAllowed() const override { return false; }
      bool IsBackAllowedWhileLoading() const override { return true; }

   private:
      void BuildPage() override
      {
         AddItem(std::make_unique<InGameUIItem>("For this run only"s, "Skip compressing the remaining textures, only for this session."s,
            [this]()
            {
               m_player->m_texLoadStats.skipCompression.store(true, std::memory_order_relaxed);
               m_player->m_liveUI->m_inGameUI.NavigateBack();
            }));
         AddItem(std::make_unique<InGameUIItem>("For this table"s, "Persist a table override disabling texture compression, applying on this run and the following ones."s,
            [this]()
            {
               m_player->m_texLoadStats.skipCompression.store(true, std::memory_order_relaxed);
               if (m_player->m_ptable->IsOnDisk())
               {
                  Settings& tableSettings = g_settingsService.GetActiveSettings();
                  tableSettings.Set(Settings::m_propPlayer_CompressTextures, false, true);
                  tableSettings.Save();
               }
               else
                  m_player->m_liveUI->PushNotification("You need to save the table before saving table setting overrides"s, 5000);
               m_player->m_liveUI->m_inGameUI.NavigateBack();
            }));
         AddItem(std::make_unique<InGameUIItem>("For all tables"s, "Disable texture compression in the application settings, applying to all tables."s,
            [this]()
            {
               m_player->m_texLoadStats.skipCompression.store(true, std::memory_order_relaxed);
               Settings& appSettings = g_settingsService.GetAppSettings();
               appSettings.Set(Settings::m_propPlayer_CompressTextures, false, false);
               appSettings.Save();
               m_player->m_liveUI->m_inGameUI.NavigateBack();
            }));
      }
   };

   void DrawSpacedText(ImDrawList* const drawList, ImFont* const font, const float fontSize, const ImVec2& center, const float spacing, const ImU32 col, const char* const text)
   {
      float width = -spacing;
      for (const char* c = text; *c; c++)
         width += font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, c, c + 1).x + spacing;
      ImVec2 pos(center.x - width * 0.5f, center.y);
      for (const char* c = text; *c; c++)
      {
         drawList->AddText(font, fontSize, pos, col, c, c + 1);
         pos.x += font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, c, c + 1).x + spacing;
      }
   }
}

LoadingPage::LoadingPage()
   : InGameUIPage("Loading & Compressing textures..."s, ""s, SaveMode::None)
{
}

bool LoadingPage::CanSkipCompression() const
{
   const auto& stats = m_player->m_texLoadStats;
   return stats.nCompressed.load(std::memory_order_relaxed) > 0 && !stats.skipCompression.load(std::memory_order_relaxed);
}

void LoadingPage::BuildPage()
{
   if (CanSkipCompression())
      AddItem(std::make_unique<InGameUIItem>("Skip texture compression"s, "Stop compressing the remaining textures. They will be uploaded uncompressed."s, [this]() { OpenSkipOptions(); }));
}

void LoadingPage::CollectPanes(vector<InGameUIPane>& panes)
{
   InGameUIPage::CollectPanes(panes);

   InGameUIPane& pane = panes.emplace_back();
   pane.placement = InGameUIPane::Placement::Modal;
   pane.id = "loadingScreen"s;
   pane.render = [this](const ImVec4& rect) { RenderLoadingScreen(rect); };
}

void LoadingPage::OpenSkipOptions()
{
   m_player->m_liveUI->m_inGameUI.AddPage("popup/skip_texcompress"s, []() { return std::make_unique<SkipTextureCompressionPage>(); });
   m_player->m_liveUI->m_inGameUI.Navigate("popup/skip_texcompress"s);
}

void LoadingPage::GenerateWireNet(const ImVec2& size)
{
   m_netGenerated = true;
   std::mt19937 rng(std::random_device {}());

   // The dot count is derived from the display area to keep a constant net density whatever the resolution / DPI scale
   const int nDots = std::clamp(static_cast<int>(size.x * size.y / 16000.f), 40, 400);
   std::uniform_real_distribution<float> pos(0.02f, 0.98f);
   m_netDots.reserve(nDots);
   for (int i = 0; i < nDots; i++)
      m_netDots.emplace_back(pos(rng), pos(rng));

   // Each dot is wired to its 2 nearest neighbors within a distance cap, and a few long
   // range wires are sprinkled on top to give the net its constellation look
   constexpr float maxWireDist = 0.14f;
   std::set<std::pair<int, int>> wires;
   for (int i = 0; i < nDots; i++)
   {
      vector<std::pair<float, int>> dist;
      dist.reserve(nDots);
      for (int j = 0; j < nDots; j++)
      {
         if (j == i)
            continue;
         const float dx = m_netDots[i].x - m_netDots[j].x;
         const float dy = m_netDots[i].y - m_netDots[j].y;
         dist.emplace_back(dx * dx + dy * dy, j);
      }
      std::partial_sort(dist.begin(), dist.begin() + 3, dist.end());
      for (int k = 0, linked = 0; k < 3 && linked < 2; k++)
      {
         if (dist[k].first > maxWireDist * maxWireDist)
            break;
         wires.emplace(std::min(i, dist[k].second), std::max(i, dist[k].second));
         linked++;
      }
   }
   std::uniform_int_distribution<int> pick(0, nDots - 1);
   for (int i = 0; i < nDots / 6; i++)
   {
      const int a = pick(rng);
      const int b = pick(rng);
      if (a != b)
         wires.emplace(std::min(a, b), std::max(a, b));
   }
   m_netWires.assign(wires.begin(), wires.end());

   // Landmark dots (one per progress step), picked spread over the net and ordered left
   // to right so they light up following the progress bar direction
   const int nLandmarks = std::min(10, nDots);
   m_netDotThresholds.assign(nDots, FLT_MAX);
   vector<int> shuffled(nDots);
   std::iota(shuffled.begin(), shuffled.end(), 0);
   std::shuffle(shuffled.begin(), shuffled.end(), rng);
   vector<int> landmarks;
   landmarks.reserve(nLandmarks);
   for (int pass = 0; pass < 2 && (int)landmarks.size() < nLandmarks; pass++)
   {
      const float minSpread = pass == 0 ? 0.2f : 0.f;
      for (const int idx : shuffled)
      {
         if ((int)landmarks.size() >= nLandmarks)
            break;
         bool isSpread = true;
         for (const int l : landmarks)
         {
            const float dx = m_netDots[idx].x - m_netDots[l].x;
            const float dy = m_netDots[idx].y - m_netDots[l].y;
            if (dx * dx + dy * dy < minSpread * minSpread)
            {
               isSpread = false;
               break;
            }
         }
         if (isSpread)
            landmarks.push_back(idx);
      }
   }
   std::sort(landmarks.begin(), landmarks.end(), [this](const int a, const int b) { return m_netDots[a].x < m_netDots[b].x; });
   for (int k = 0; k < (int)landmarks.size(); k++)
      m_netDotThresholds[landmarks[k]] = (k + 1.f) / landmarks.size();
}

void LoadingPage::RenderLoadingScreen(const ImVec4& rect)
{
   const ImVec2 origin(rect.x, rect.y);
   const ImVec2 size(rect.z, rect.w);

   ImGui::SetNextWindowPos(origin);
   ImGui::SetNextWindowSize(size);
   ImGui::Begin("##LoadingScreen", nullptr,
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
         | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
   ImDrawList* const drawList = ImGui::GetWindowDrawList();

   if (!m_netGenerated)
      GenerateWireNet(size);

   const auto& stats = m_player->m_texLoadStats;
   const uint64_t totalPixels = stats.nPixelsTotal.load(std::memory_order_relaxed);
   const uint64_t donePixels = stats.nPixelsDone.load(std::memory_order_relaxed);
   const float progress = totalPixels > 0 ? std::clamp(static_cast<float>(donePixels) / static_cast<float>(totalPixels), 0.f, 1.f) : 0.f;

   // Dark gray vertical gradient backdrop
   drawList->AddRectFilledMultiColor(origin, origin + size, IM_COL32(38, 38, 44, 255), IM_COL32(38, 38, 44, 255), IM_COL32(16, 16, 20, 255), IM_COL32(16, 16, 20, 255));

   // Static wire/dot constellation: each wire is shaded as a gradient between its endpoints' lit levels
   vector<float> lit(m_netDots.size(), 0.f);
   for (size_t i = 0; i < m_netDots.size(); i++)
      lit[i] = std::clamp((progress - m_netDotThresholds[i]) / 0.04f, 0.f, 1.f);
   const float wireThickness = std::max(1.f, size.x * 0.0006f);
   const auto wireColor = [](const float l)
   {
      const int shade = 90 + static_cast<int>(l * 50.f);
      return IM_COL32(shade, shade, shade + 4, 38 + static_cast<int>(l * 70.f));
   };
   for (const auto& wire : m_netWires)
   {
      const ImVec2 a(origin.x + m_netDots[wire.first].x * size.x, origin.y + m_netDots[wire.first].y * size.y);
      const ImVec2 b(origin.x + m_netDots[wire.second].x * size.x, origin.y + m_netDots[wire.second].y * size.y);
      const ImU32 colA = wireColor(lit[wire.first]);
      const ImU32 colB = wireColor(lit[wire.second]);
      if (colA == colB)
      {
         drawList->AddLine(a, b, colA, wireThickness);
      }
      else
      {
         const ImVec4 fA = ImGui::ColorConvertU32ToFloat4(colA);
         const ImVec4 fB = ImGui::ColorConvertU32ToFloat4(colB);
         constexpr int segs = 6;
         for (int s = 0; s < segs; s++)
         {
            const float t0 = static_cast<float>(s) / segs;
            const float t1 = static_cast<float>(s + 1) / segs;
            const float tm = (t0 + t1) * 0.5f;
            drawList->AddLine(a + (b - a) * t0, a + (b - a) * t1,
               ImGui::ColorConvertFloat4ToU32(ImVec4(fA.x + (fB.x - fA.x) * tm, fA.y + (fB.y - fA.y) * tm, fA.z + (fB.z - fA.z) * tm, fA.w + (fB.w - fA.w) * tm)), wireThickness);
         }
      }
   }
   const float dotRadius = std::max(1.5f, size.x * 0.0016f);
   for (size_t i = 0; i < m_netDots.size(); i++)
   {
      const ImVec2 c(origin.x + m_netDots[i].x * size.x, origin.y + m_netDots[i].y * size.y);
      const float l = lit[i];
      if (l > 0.f)
         drawList->AddCircleFilled(c, dotRadius * 2.f, IM_COL32(200, 200, 210, static_cast<int>(l * 25.f)));
      const int shade = 80 + static_cast<int>(l * 50.f);
      drawList->AddCircleFilled(c, dotRadius * (1.f + 0.4f * l), IM_COL32(shade, shade, shade + 6, 140 + static_cast<int>(l * 60.f)));
   }

   // Logo, horizontally centered in the upper mid of the screen
   if (!m_logo && !m_logoLoadFailed)
   {
      if (std::shared_ptr<const BaseTexture> tex = BaseTexture::CreateFromFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, "vpinball-logo.png")); tex != nullptr)
         m_logo = std::make_shared<Sampler>(m_player->m_renderer->m_renderDevice, "InGameUILoadingLogo"s, tex, false);
      else
         m_logoLoadFailed = true;
   }
   if (m_logo)
   {
      const float logoW = std::min(size.x * 0.32f, size.y * 0.45f * m_logo->GetWidth() / m_logo->GetHeight());
      const float logoH = logoW * m_logo->GetHeight() / m_logo->GetWidth();
      const ImVec2 logoMin(origin.x + (size.x - logoW) * 0.5f, origin.y + size.y * 0.30f - logoH * 0.5f);
      drawList->AddImage(m_logo, logoMin, logoMin + ImVec2(logoW, logoH));
   }

   // Minimalistic progress bar: a thin white line spanned by a thicker gray-to-white gradient tip
   const float barY = origin.y + size.y * 0.60f;
   const float barX0 = origin.x + size.x * 0.075f;
   const float barX1 = origin.x + size.x * 0.925f;
   const float tipX = barX0 + (barX1 - barX0) * progress;
   const float segH = std::max(4.f, size.y * 0.008f);
   drawList->AddLine(ImVec2(barX0, barY), ImVec2(barX1, barY), IM_COL32(255, 255, 255, 150), std::max(1.f, size.y * 0.002f));
   if (progress > 0.f)
   {
      drawList->AddRectFilledMultiColor(ImVec2(barX0, barY - segH * 0.5f), ImVec2(tipX, barY + segH * 0.5f), IM_COL32(60, 60, 66, 255), IM_COL32(255, 255, 255, 255),
         IM_COL32(255, 255, 255, 255), IM_COL32(60, 60, 66, 255));
      drawList->AddCircleFilled(ImVec2(barX0, barY), segH * 0.5f, IM_COL32(60, 60, 66, 255));
      drawList->AddCircleFilled(ImVec2(tipX, barY), segH * 0.5f, IM_COL32(255, 255, 255, 255));
   }

   // "LOADING" caption below the bar
   ImFont* const overlayFont = m_player->m_liveUI->GetOverlayFont() ? m_player->m_liveUI->GetOverlayFont() : ImGui::GetFont();
   const float capSize = std::clamp(size.y * 0.028f, 12.f, 42.f);
   DrawSpacedText(drawList, overlayFont, capSize, ImVec2(origin.x + size.x * 0.5f, barY + segH + size.y * 0.015f), capSize * 0.35f, IM_COL32(160, 160, 166, 255), "LOADING");

   // Bottom info bar: explains the compression process and gives access to the skip options
   const bool canSkip = CanSkipCompression();
   const string info = canSkip                                ? "Compressing textures (one-time operation, results are cached for future launches).\nYou can adjust compression settings by pressing any UI button."s
      : stats.skipCompression.load(std::memory_order_relaxed) ? "Loading the remaining textures with compression skipped."s
                                                              : "Loading table textures."s;
   ImFont* const font = ImGui::GetFont();
   const float fontSize = ImGui::GetFontSize();
   const char* const link = canSkip ? "Options..." : nullptr;
   const float gap = link ? fontSize * 1.5f : 0.f;
   const ImVec2 infoSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, info.c_str());
   const ImVec2 linkSize = link ? font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, link) : ImVec2(0.f, 0.f);
   const float fitScale = std::min(1.f, size.x * 0.92f / (infoSize.x + gap + linkSize.x));
   const float drawSize = fontSize * fitScale;
   const ImVec2 infoDrawSize = infoSize * fitScale;
   const ImVec2 linkDrawSize = linkSize * fitScale;
   // The bar grows to fit the info text, which may span multiple lines
   const float infoBarH = std::max({ 30.f, size.y * 0.05f, infoDrawSize.y + drawSize });
   const ImVec2 infoBarMin(origin.x, origin.y + size.y - infoBarH);
   drawList->AddRectFilled(infoBarMin, origin + size, IM_COL32(0, 0, 0, 110));
   const float textY = infoBarMin.y + (infoBarH - infoDrawSize.y) * 0.5f;
   const float linkY = infoBarMin.y + (infoBarH - linkDrawSize.y) * 0.5f;
   float textX = origin.x + (size.x - infoDrawSize.x - gap - linkDrawSize.x) * 0.5f;
   drawList->AddText(font, drawSize, ImVec2(textX, textY), IM_COL32(140, 140, 146, 255), info.c_str());
   textX += infoDrawSize.x + gap;
   if (link)
   {
      // Only interactive while this page is the active one (the screen keeps drawing under an opened options page)
      const bool interactive = IsActive();
      ImGui::SetCursorScreenPos(ImVec2(textX, infoBarMin.y));
      ImGui::InvisibleButton("##LoadingOptions", ImVec2(linkDrawSize.x, infoBarH));
      const ImU32 linkCol = interactive && ImGui::IsItemHovered() ? IM_COL32(235, 235, 240, 255) : IM_COL32(190, 190, 198, 255);
      drawList->AddText(font, drawSize, ImVec2(textX, linkY), linkCol, link);
      drawList->AddLine(ImVec2(textX, linkY + drawSize + 1.f), ImVec2(textX + linkDrawSize.x, linkY + drawSize + 1.f), linkCol);
      if (interactive && ImGui::IsItemClicked())
         OpenSkipOptions();
   }

   ImGui::End();
}
}
