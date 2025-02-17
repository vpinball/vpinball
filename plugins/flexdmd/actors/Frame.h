#pragma once

#include "Actor.h"

class Frame final : public Actor
{
public:
   Frame(FlexDMD* pFlexDMD, const string& name)
      : Actor(pFlexDMD, name) { }
   ~Frame() = default;

   ActorType GetType() const override { return AT_Frame; }

   int GetThickness() const { return m_thickness; }
   void SetThickness(int v) { m_thickness = v; }
   ColorRGBA32 GetBorderColor() const { return m_borderColor; }
   void SetBorderColor(ColorRGBA32 v) { m_borderColor = v; }
   bool GetFill() const { return m_fill; }
   void SetFill(bool v) { m_fill = v; }
   ColorRGBA32 GetFillColor() const { return m_fillColor; }
   void SetFillColor(ColorRGBA32 v) { m_fillColor = v; }

   void Draw(VP::SurfaceGraphics* pGraphics) override;

private:
   int m_thickness = 2;
   ColorRGBA32 m_borderColor = 0x00FFFFFF;
   bool m_fill = false;
   ColorRGBA32 m_fillColor = 0x00000000;
};
