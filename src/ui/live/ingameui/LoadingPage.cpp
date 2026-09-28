// license:GPLv3+

#include "core/stdafx.h"
#include "LoadingPage.h"

#include "core/player.h"
#include "renderer/Texture.h"
#include "ui/live/LiveUI.h"

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
               if (FileExists(m_player->m_ptable->m_filename))
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

}

LoadingPage::LoadingPage()
   : InGameUIPage("Loading & Compressing textures..."s, ""s, SaveMode::None)
{
}

void LoadingPage::BuildPage()
{
   const auto& stats = m_player->m_texLoadStats;

   if (stats.nCompressed.load(std::memory_order_relaxed) > 0 && !stats.skipCompression.load(std::memory_order_relaxed))
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info,
         "Texture compression is a one-time process per table needed for optimized performance.\nCompressed textures are cached on disk and reused on the next runs.\n"
         "It can be tuned or disabled in the graphics settings ('Compress Textures')."s));

      AddItem(std::make_unique<InGameUIItem>("Skip texture compression"s, "Stop compressing the remaining textures. They will be uploaded uncompressed."s,
         [this]()
         {
            m_player->m_liveUI->m_inGameUI.AddPage("popup/skip_texcompress"s, []() { return std::make_unique<SkipTextureCompressionPage>(); });
            m_player->m_liveUI->m_inGameUI.Navigate("popup/skip_texcompress"s);
         }));
   }

   AddItem(std::make_unique<InGameUIItem>("progress"s, ""s,
      [this](int, const InGameUIItem*)
      {
         auto& stats = m_player->m_texLoadStats;
         const int total = stats.nImagesTotal.load(std::memory_order_relaxed);
         const int done = stats.nImagesDone.load(std::memory_order_relaxed);
         const uint64_t totalPixels = stats.nPixelsTotal.load(std::memory_order_relaxed);
         const uint64_t donePixels = stats.nPixelsDone.load(std::memory_order_relaxed);
         ImGui::ProgressBar(totalPixels > 0 ? 0.001f * static_cast<float>(1000 * donePixels / totalPixels) : 0.f, ImVec2(-FLT_MIN, 0.f));
         const bool discarded = stats.skipCompression.load(std::memory_order_relaxed);
         if (discarded)
            ImGui::TextWrapped("Loading table images: %d / %d (texture compression skip requested)", done, total);
         else
            ImGui::Text("Loading table images: %d / %d", done, total);

         // List the images currently being processed by the worker threads
         ImGui::NewLine();
         vector<std::pair<const Texture*, bool>> inFlight;
         {
            const std::lock_guard<std::mutex> lock(stats.inFlightMutex);
            inFlight = stats.inFlight;
         }
         static constexpr size_t maxListed = 10;
         for (size_t i = 0; i < inFlight.size() && i < maxListed; ++i)
         {
            const Texture* const tex = inFlight[i].first;
            ImGui::TextWrapped("%s (%ux%u %s)%s", tex->m_name.c_str(), tex->m_width, tex->m_height, tex->IsHDR() ? "HDR" : "LDR", inFlight[i].second ? (discarded ? " - discarded" : " - compressing") : " - pending");
         }
         if (inFlight.size() > maxListed)
            ImGui::Text("... and %d more", static_cast<int>(inFlight.size() - maxListed));
      }));
}

}
