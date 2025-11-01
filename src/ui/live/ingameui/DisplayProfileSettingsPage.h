// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class DisplayProfileSettingsPage final : public InGameUIPage
{
public:
   DisplayProfileSettingsPage();
   void Render(float elapsed) override;

private:
   void BuildPage();
   void BuildDMDPage();
   void BuildAlphaPage();
   int m_selectedProfile = 0;
   float m_previewBrightness = 1.f;
   std::shared_ptr<BaseTexture> m_dmdTexture;
   struct Particle
   {
      int pos;
      float life;
      float lifespan;
   };
   vector<Particle> m_particles;
};

}
