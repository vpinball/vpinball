#pragma once

#include "../common.h"
#include "SegTypes.h"
#include "SegGraphics.h"

namespace B2S::seg
{

// Minimal stand-in for the retired B2SLegacy plugin's Control base class: an
// invalidated rectangle rasterized through a lazy SegGraphics backbuffer.
class SegDisplay
{
public:
   SegDisplay(const VPXPluginAPI* vpxApi)
      : m_vpxApi(vpxApi)
   {
   }
   virtual ~SegDisplay() = default;

   int GetLeft() const { return m_rect.x; }
   int GetTop() const { return m_rect.y; }
   int GetWidth() const { return m_rect.w; }
   int GetHeight() const { return m_rect.h; }
   const IRect& GetRect() const { return m_rect; }
   void SetRect(const IRect& rect)
   {
      m_rect = rect;
      if (!m_handleCreated)
      {
         m_handleCreated = true;
         OnHandleCreated();
      }
      OnResize();
   }
   bool IsVisible() const { return m_visible; }
   void SetVisible(bool visible) { m_visible = visible; }
   uint32_t GetBackColor() const { return m_backColor; }
   void SetBackColor(uint32_t backColor) { m_backColor = backColor; }
   void Invalidate() { m_invalidated = true; }
   bool IsInvalidated() const { return m_invalidated; }
   bool IsHandleCreated() const { return m_handleCreated; }

protected:
   void ClearInvalidated() { m_invalidated = false; }
   virtual void OnHandleCreated() { }
   virtual void OnResize() { }
   virtual void OnPaintBackground(SegGraphics* pGraphics)
   {
      pGraphics->SetColor(m_backColor);
      pGraphics->FillRectangle({ 0, 0, GetWidth(), GetHeight() });
   }

   const VPXPluginAPI* m_vpxApi = nullptr;
   std::unique_ptr<SegGraphics> m_pGraphics;

private:
   IRect m_rect = { 0, 0, 0, 0 };
   bool m_visible = true;
   uint32_t m_backColor = RGB(0, 0, 0);
   bool m_handleCreated = false;
   bool m_invalidated = false;
};

}
