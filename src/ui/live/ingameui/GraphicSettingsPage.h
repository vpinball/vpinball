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
   void BuildPage();
   void OnStaticRenderDirty();

   unsigned int m_notificationId = 0;
   bool m_staticPrepassDisabled = false;
};

}
