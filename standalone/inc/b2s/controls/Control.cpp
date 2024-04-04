#include "stdafx.h"

#include "Control.h"

Control::Control()
{  
   m_pParent = NULL;
   m_szName = "";
   m_rect = { 0, 0, 0, 0 };
   m_visible = true;
   m_backColor = RGB(0, 0, 0);
   m_handleCreated = false;
   m_pBackgroundImage = NULL;
}

Control::~Control()
{
}

void Control::OnPaint(VP::RendererGraphics* pGraphics)
{
   for (Control* child : m_children)
      child->OnPaint(pGraphics);

   m_invalidated = false;
}

void Control::OnHandleCreated()
{
}

void Control::AddControl(Control* control)
{
   m_children.push_back(control);
   control->m_pParent = this;
   control->m_handleCreated = true;
   control->OnHandleCreated();
}

Control* Control::GetControl(const string& szName)
{
   for (Control* child : m_children) {
      if (child->GetName() == szName)
         return child;
   }
   return NULL;
}

bool Control::IsVisible()
{
   return m_visible && (!m_pParent || m_pParent->IsVisible());
}

void Control::SetVisible(bool visible)
{
   if (m_visible != visible) {
      m_visible = visible;
      Invalidate();
   }
}

void Control::SetBackgroundImage(SDL_Surface* pBackgroundImage)
{
   if (m_pBackgroundImage != pBackgroundImage) {
      m_pBackgroundImage = pBackgroundImage;
      Invalidate();
   }
}

void Control::BringToFront()
{
   if (m_pParent) {
      auto it = std::find(m_pParent->m_children.begin(), m_pParent->m_children.end(), this);
      if (it != m_pParent->m_children.end()) {
         m_pParent->m_children.erase(it);
         m_pParent->m_children.push_back(this);
      }
   }

   m_invalidated = true;
}

void Control::Refresh()
{
   m_invalidated = true;
}

void Control::Invalidate()
{
   m_invalidated = true;
}

bool Control::IsInvalidated()
{
   if (m_invalidated)
      return true;

   for (Control* child : m_children) {
      if (child->IsInvalidated())
         return true;
   }

   return false;
}