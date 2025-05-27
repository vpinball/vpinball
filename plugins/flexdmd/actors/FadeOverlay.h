#pragma once

#include "Actor.h"

namespace Flex {

class FadeOverlay final : public Actor
{
public:
   FadeOverlay(const FlexDMD* pFlexDMD, const string& name)
      : Actor(pFlexDMD, name) { }
   ~FadeOverlay() override = default;

   float GetAlpha() const { return m_alpha; }
   void SetAlpha(float alpha) { m_alpha = alpha; }
   void SetColor(uint32_t color) { m_color = color; }
   void Draw(Flex::SurfaceGraphics* pGraphics) override;

private:
   float m_alpha = 1.f;
   uint32_t m_color = 0x00000000;
};

}