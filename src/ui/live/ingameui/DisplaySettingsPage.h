// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

#include "plugins/VPXPlugin.h"

namespace VPX::InGameUI
{

class DisplayHomePage final : public InGameUIPage
{
public:
   DisplayHomePage();

private:
   void BuildPage() override;
};


class DisplaySettingsPage final : public InGameUIPage
{
public:
   explicit DisplaySettingsPage(VPXWindowId wndId);
   void Close(bool isBackwardAnimation) override;
   void Render(float elapsedS) override;

   void ResetToStoredValues() override;
   void ResetToDefaults() override;

private:
   void ResetARLock();
   bool IsSeenRotated();   // Embedded output on a quarter turned playfield: width and height are swapped as seen by the player
   int2 GetLockedAspect(); // Locked aspect ratio in output pixels (the lock is chosen as seen by the player)
   void StartFitToContent(bool enabled);
   void FitToContent();
   void BuildPage() override;
   void BuildWindowPage();
   void BuildEmbeddedPage();
   VPX::RenderOutput& GetOutput(VPXWindowId wndId);
   VPX::Window* GetWindow();

   void OnStaticRenderDirty();
   bool m_staticPrepassDisabled = false;

   const VPXWindowId m_wndId;
   const bool m_isMainWindow;
   vector<Window::DisplayConfig> m_displays;
   vector<string> m_displayNames;
   int m_arLock = 0;
   bool m_fitToContent = false; // Output just enabled: fit it to the content once its renderer has drawn
   unsigned int m_delayApplyNotifId = 0;
};

}
