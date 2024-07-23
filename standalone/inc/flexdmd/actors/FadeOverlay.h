#pragma once

#include "Actor.h"

class FadeOverlay : public Actor
{
public:
   FadeOverlay(FlexDMD* pFlexDMD, const string& name);
   ~FadeOverlay();

   float GetAlpha() { return m_alpha; }
   void SetAlpha(float alpha) { m_alpha = alpha; }
   void SetColor(OLE_COLOR color) { m_color = color; }
   void Draw(VP::SurfaceGraphics* pGraphics) override;

private:
   float m_alpha;
   OLE_COLOR m_color;
};