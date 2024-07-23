#pragma once

#include <vector>

#include "../../common/RendererGraphics.h"

class Control {
public:
   Control();
   virtual ~Control();

   virtual void OnPaint(VP::RendererGraphics* pGraphics);
   virtual void OnHandleCreated();

   Control* GetParent() const { return m_pParent; }
   void AddControl(Control* control);
   vector<Control*>* GetControls() { return &m_children; }
   Control* GetControl(const string& szName);
   string GetName() const { return m_szName; }
   void SetName(const string& szName) { m_szName = szName; }
   int GetLeft() const { return m_rect.x; }
   void SetLeft(int left) { m_rect.x = left; }
   int GetTop() const { return m_rect.y; }
   void SetTop(int top) { m_rect.y = top; }
   int GetWidth() const { return m_rect.w; }
   void SetWidth(int width) { m_rect.w = width; }
   int GetHeight() const { return m_rect.h; }
   void SetHeight(int height) { m_rect.h = height; }
   SDL_Point GetLocation() { return { m_rect.x, m_rect.y }; }
   void SetLocation(const SDL_Point& location) { m_rect.x = location.x; m_rect.y = location.y; }
   SDL_Rect GetSize() { return { 0, 0, m_rect.w, m_rect.h }; }
   void SetSize(const SDL_Rect& size) { m_rect.w = size.w; m_rect.h = size.h; }
   SDL_Rect& GetRect() { return m_rect; }
   bool IsVisible();
   virtual void SetVisible(bool visible);
   OLE_COLOR GetBackColor() const { return m_backColor; }
   void SetBackColor(OLE_COLOR backColor) { m_backColor = backColor; }
   void BringToFront();
   void Refresh();
   void Invalidate();
   bool IsInvalidated();
   bool IsHandleCreated() const { return m_handleCreated; }
   SDL_Surface* GetBackgroundImage() const { return m_pBackgroundImage; }
   void SetBackgroundImage(SDL_Surface* pBackgroundImage);

private:
   Control* m_pParent;
   string m_szName;
   vector<Control*> m_children;
   SDL_Rect m_rect;
   bool m_visible;
   OLE_COLOR m_backColor;
   SDL_Surface* m_pBackgroundImage;
   bool m_handleCreated;
   bool m_invalidated;
};