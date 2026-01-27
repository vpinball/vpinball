#include "../common.h"

#include "Control.h"
#include "../forms/Form.h"
#include "../utils/VPXGraphics.h"

namespace B2SLegacy {

Control::Control(VPXPluginAPI* vpxApi)
   : m_vpxApi(vpxApi)
{
   m_pParent = nullptr;
   m_szName.clear();
   m_rect = { 0, 0, 0, 0 };
   m_visible = true;
   m_backColor = RGB(0, 0, 0);
   m_handleCreated = false;
   m_pBackgroundImage = nullptr;
}

Control::~Control()
{
}

void Control::OnPaint(VPXRenderContext2D* const ctx)
{
   if (IsVisible() && m_pGraphics) {
      m_pGraphics->Clear();
      OnPaintBackground(m_pGraphics.get());
   }

   for (Control* child : m_children)
      child->OnPaint(ctx);

   m_invalidated = false;
}

void Control::OnHandleCreated()
{
}

void Control::OnPaintBackground(VPXGraphics* pGraphics)
{
   pGraphics->SetColor(m_backColor);
   SDL_Rect rect = { 0, 0, GetWidth(), GetHeight() };
   pGraphics->FillRectangle(rect);
}

void Control::AddControl(Control* control)
{
   m_children.push_back(control);
   control->m_pParent = this;
   control->m_handleCreated = true;
   control->OnHandleCreated();
}

Control* Control::GetControl(const string& szName) const
{
   for (Control* child : m_children) {
      if (child->GetName() == szName)
         return child;
   }
   return nullptr;
}

bool Control::IsVisible() const
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

void Control::SetBackgroundImage(VPXTexture pBackgroundImage)
{
   if (m_pBackgroundImage != pBackgroundImage) {
      // TODO: if (m_pBackgroundImage)  // We probably shouldn't delete because it could be referenced somewhere else
      //   m_vpxApi->DeleteTexture(m_pBackgroundImage);
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

bool Control::IsInvalidated() const
{
   if (m_invalidated)
      return true;

   for (Control* child : m_children) {
      if (child->IsInvalidated())
         return true;
   }

   return false;
}


}
