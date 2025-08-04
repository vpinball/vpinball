#pragma once

#include "../common.h"
#include <vector>


namespace B2SLegacy {

class Control {
public:
   Control(VPXPluginAPI* vpxApi);
   virtual ~Control();

   virtual void OnPaint(VPXRenderContext2D* const ctx);
   virtual void OnHandleCreated();

   Control* GetParent() const { return m_pParent; }
   void AddControl(Control* control);
   vector<Control*>* GetControls() { return &m_children; }
   Control* GetControl(const string& szName) const;
   const string& GetName() const { return m_szName; }
   void SetName(const string& szName) { m_szName = szName; }
   int GetLeft() const { return m_rect.x; }
   void SetLeft(int left) { m_rect.x = left; }
   int GetTop() const { return m_rect.y; }
   void SetTop(int top) { m_rect.y = top; }
   int GetWidth() const { return m_rect.w; }
   void SetWidth(int width) { m_rect.w = width; }
   int GetHeight() const { return m_rect.h; }
   void SetHeight(int height) { m_rect.h = height; }
   SDL_Point GetLocation() const { return { m_rect.x, m_rect.y }; }
   void SetLocation(const SDL_Point& location) { m_rect.x = location.x; m_rect.y = location.y; }
   SDL_Rect GetSize() const { return { 0, 0, m_rect.w, m_rect.h }; }
   void SetSize(const SDL_Rect& size) { m_rect.w = size.w; m_rect.h = size.h; }
   const SDL_Rect& GetRect() const { return m_rect; }
   bool IsVisible() const;
   virtual void SetVisible(bool visible);
   uint32_t GetBackColor() const { return m_backColor; }
   void SetBackColor(uint32_t backColor) { m_backColor = backColor; }
   void BringToFront();
   void Refresh();
   void Invalidate();
   bool IsInvalidated() const;
   bool IsHandleCreated() const { return m_handleCreated; }
   VPXTexture GetBackgroundImage() const { return m_pBackgroundImage; }
   void SetBackgroundImage(VPXTexture pBackgroundImage);

protected:
   VPXPluginAPI* m_vpxApi = nullptr;

private:
   Control* m_pParent;
   string m_szName;
   vector<Control*> m_children;
   SDL_Rect m_rect;
   bool m_visible;
   uint32_t m_backColor;
   VPXTexture m_pBackgroundImage;
   bool m_handleCreated;
   bool m_invalidated;
};

}
