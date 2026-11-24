// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

class Sampler;

namespace VPX::InGameUI
{

class LoadingPage final : public InGameUIPage
{
public:
   // caption is the phase label displayed below the progress bar ("LOADING" while a table file
   // is being parsed, "PREPARING" during the texture compression), set by the page's creator
   explicit LoadingPage(string caption);
   bool IsPlayerPauseAllowed() const override { return false; }
   void BuildPage() override;
   void CollectPanes(vector<InGameUIPane>& panes) override;

   // Any UI or cabinet input opens the skip options (mouse has a dedicated link in the screen)
   void SelectNextItem() override { OpenSkipOptionsIfAvailable(); }
   void SelectPrevItem() override { OpenSkipOptionsIfAvailable(); }
   // Initial press only: a held confirm button would reopen the options right after navigating back to this page
   void AdjustItem(float, bool isInitialPress) override
   {
      if (isInitialPress)
         OpenSkipOptionsIfAvailable();
   }
   void Save() override { OpenSkipOptionsIfAvailable(); }
   void ResetToDefaults() override { OpenSkipOptionsIfAvailable(); }
   void ResetToStoredValues() override { OpenSkipOptionsIfAvailable(); }

private:
   bool CanSkipCompression() const;
   void OpenSkipOptionsIfAvailable()
   {
      if (CanSkipCompression())
         OpenSkipOptions();
   }
   void OpenSkipOptions();
   void RenderLoadingScreen(const ImVec4& rect);
   void GenerateWireNet(const ImVec2& size);

   string m_caption;

   // Wire/dot constellation backdrop: randomly generated once per loading screen
   // instance (a fresh pattern each load), then static for its lifetime
   vector<ImVec2> m_netDots; // normalized [0,1] coordinates
   vector<std::pair<int, int>> m_netWires;
   vector<float> m_netDotThresholds; // progress value lighting each dot (FLT_MAX = never lit)
   bool m_netGenerated = false;

   std::shared_ptr<Sampler> m_logo;
   bool m_logoLoadFailed = false;
};

}
