// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class DisplayProfileSettingsPage final : public InGameUIPage
{
public:
   DisplayProfileSettingsPage();
   void Update(float elapsedS) override;
   void CollectPanes(vector<InGameUIPane>& panes) override;

private:
   void BuildPage() override;
   void BuildDMDPage();
   void BuildAlphaPage();
   void RenderDisplayPreview(const ImVec4& rect);

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
   struct rgb
   {
      int r, g, b;
   };
   rgb m_srgbLit;
   rgb m_srgbUnlit;
};

}
