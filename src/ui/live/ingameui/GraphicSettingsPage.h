// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class GraphicSettingsPage final : public InGameUIPage
{
public:
   GraphicSettingsPage();

private:
   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;
   void BuildPage() override;
   void OnStaticRenderDirty();

   unsigned int m_notificationId = 0;
   bool m_staticPrepassDisabled = false;
   float m_pendingAAFactor = 0.f;
   int m_pendingMSAASamples = 0;
};

}
