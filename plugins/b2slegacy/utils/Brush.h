#pragma once

#include <SDL3/SDL.h>

namespace B2SLegacy {

class GraphicsPath;

enum BrushType {
   BrushType_Solid = 0,
   BrushType_PathGradient = 1
};

class Brush
{
public:
   virtual ~Brush() { }

   virtual BrushType GetBrushType() const = 0;
};

class SolidBrush final : public Brush
{
public:
   SolidBrush(uint32_t color, uint8_t alpha = 255) : m_color(color), m_alpha(alpha) { }

   BrushType GetBrushType() const override { return BrushType_Solid; }
   uint32_t GetColor() const { return m_color; }
   uint8_t GetAlpha() const { return m_alpha; }

private:
   uint32_t m_color = 0;
   uint8_t m_alpha = 255;
};

class PathGradientBrush final : public Brush
{
public:
   PathGradientBrush(GraphicsPath* pPath) : m_pPath(pPath) { }

   BrushType GetBrushType() const override { return BrushType_PathGradient; }
   GraphicsPath* GetPath() const { return m_pPath; }
   void SetCenterColor(uint32_t centerColor, uint8_t centerAlpha = 255) { m_centerColor = centerColor; m_centerAlpha = centerAlpha; }
   uint32_t GetCenterColor() const { return m_centerColor; }
   uint8_t GetCenterAlpha() const { return m_centerAlpha; }
   void SetSurroundColor(uint32_t surroundColor, uint8_t surroundAlpha = 255) { m_surroundColor = surroundColor; m_surroundAlpha = surroundAlpha; }
   uint32_t GetSurroundColor() const { return m_surroundColor; }
   uint8_t GetSurroundAlpha() const { return m_surroundAlpha; }
   void SetFocusScales(const SDL_FPoint& focusScales) { m_focusScales = focusScales; }
   const SDL_FPoint& GetFocusScales() const { return m_focusScales; }

private:
   GraphicsPath* m_pPath = nullptr;
   uint32_t m_centerColor = 0;
   uint8_t m_centerAlpha = 255;
   uint32_t m_surroundColor = 0;
   uint8_t m_surroundAlpha = 255;
   SDL_FPoint m_focusScales = { 0.0f, 0.0f };
};

}
