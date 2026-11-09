// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class TableMiscPage final : public InGameUIPage
{
public:
   TableMiscPage();
   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;

private:
   void BuildPage() override;
   void RequestDynamicRendererUpdate();

   unsigned int m_difficultyNotification = 0;
   float m_pendingDifficulty = 1.f; // Edited difficulty: only saved to the settings, the table applies it on its next start
   bool m_staticPrepassDisabled = false;
};

}
