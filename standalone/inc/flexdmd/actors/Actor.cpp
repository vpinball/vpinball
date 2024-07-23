#include "stdafx.h"

#include "Actor.h"
#include "Group.h"

Actor::Actor(FlexDMD* pFlexDMD, string name)
{
   m_pFlexDMD = pFlexDMD;
   m_onStage = false;
   m_szName = name;
   m_x = 0;
   m_y = 0;
   m_width = 0;
   m_height = 0;
   m_parent = NULL;
   m_fillParent = false;
   m_clearBackground = false;
   m_prefWidth = 0;
   m_prefHeight = 0;
   m_visible = true;

   m_pActionFactory = new ActionFactory(this);
   m_pActionFactory->AddRef();
}

Actor::~Actor()
{
   m_pActionFactory->Release();
}

STDMETHODIMP Actor::get_Name(BSTR *pRetVal)
{
   CComBSTR Val(m_szName.c_str());
   *pRetVal = Val.Detach();

   return S_OK;
}

STDMETHODIMP Actor::put_Name(BSTR pRetVal)
{
   m_szName = MakeString(pRetVal);

   return S_OK;
}

STDMETHODIMP Actor::get_x(single *pRetVal)
{
   *pRetVal = m_x;

   return S_OK;
}

STDMETHODIMP Actor::put_x(single pRetVal)
{
   m_x = pRetVal;

   return S_OK;
}

STDMETHODIMP Actor::get_y(single *pRetVal)
{
   *pRetVal = m_y;

   return S_OK;
}

STDMETHODIMP Actor::put_y(single pRetVal)
{
   m_y = pRetVal;

   return S_OK;
}

STDMETHODIMP Actor::get_Width(single *pRetVal)
{
   *pRetVal = m_width;

   return S_OK;
}

STDMETHODIMP Actor::put_Width(single pRetVal)
{
   m_width = pRetVal;

   return S_OK;
}

STDMETHODIMP Actor::get_Height(single *pRetVal)
{
   *pRetVal = m_height;

   return S_OK;
}

STDMETHODIMP Actor::put_Height(single pRetVal)
{
   m_height = pRetVal;

   return S_OK;
}

STDMETHODIMP Actor::get_visible(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_visible ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Actor::put_visible(VARIANT_BOOL pRetVal)
{
   SetVisible(pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP Actor::get_FillParent(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_fillParent ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Actor::put_FillParent(VARIANT_BOOL pRetVal)
{
   m_fillParent = (pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP Actor::get_ClearBackground(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_clearBackground ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Actor::put_ClearBackground(VARIANT_BOOL pRetVal)
{
   m_clearBackground = (pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP Actor::SetBounds(single x,single y,single Width,single Height)
{
   m_x = x;
   m_y = y;
   m_width = Width;
   m_height = Height;

   return S_OK;
}

STDMETHODIMP Actor::SetPosition(single x,single y)
{
   m_x = x;
   m_y = y;

   return S_OK;
}

STDMETHODIMP Actor::SetAlignedPosition(single x, single y, Alignment alignment)
{
   switch (alignment) {
      case Alignment_BottomLeft:
      case Alignment_Left:
      case Alignment_TopLeft:
         m_x = x;
         break;
      case Alignment_Bottom:
      case Alignment_Center:
      case Alignment_Top:
         m_x = x - m_width * 0.5f;
         break;
      case Alignment_BottomRight:
      case Alignment_Right:
      case Alignment_TopRight:
         m_x = x - m_width;
         break;
   }

   switch (alignment) {
      case Alignment_BottomLeft:
      case Alignment_Bottom:
      case Alignment_BottomRight:
         m_y = y - m_height;
         break;
      case Alignment_Left:
      case Alignment_Center:
      case Alignment_Right:
         m_y = y - m_height * 0.5f;
         break;
      case Alignment_TopLeft:
      case Alignment_Top:
      case Alignment_TopRight:
         m_y = y;
         break;
   }

   return S_OK;
}

STDMETHODIMP Actor::SetSize(single Width,single Height)
{
   m_width = Width;
   m_height = Height;

   return S_OK;
}

STDMETHODIMP Actor::get_PrefWidth(single *pRetVal)
{
   *pRetVal = GetPrefWidth();

   return S_OK;
}

STDMETHODIMP Actor::get_PrefHeight(single *pRetVal)
{
   *pRetVal = GetPrefHeight();

   return S_OK;
}

STDMETHODIMP Actor::Pack()
{
   m_width = GetPrefWidth();
   m_height = GetPrefHeight();

   return S_OK;
}

STDMETHODIMP Actor::Remove()
{
   if (m_parent != NULL)
      m_parent->RemoveActor(this);

   return S_OK;
}

STDMETHODIMP Actor::get_ActionFactory(IActionFactory **pRetVal)
{
   return m_pActionFactory->QueryInterface(IID_IActionFactory, (void**)pRetVal);
}

STDMETHODIMP Actor::AddAction(IUnknown *action)
{
   Action* pAction = dynamic_cast<Action*>(action);
   m_actions.push_back(pAction);

   return S_OK;
}

STDMETHODIMP Actor::ClearActions()
{
   m_actions.clear();

   return S_OK;
}

void Actor::Update(float secondsElapsed)
{
   if (!m_onStage) assert("Update was called on an actor which is not on stage.");
   for (auto it = m_actions.begin(); it != m_actions.end(); ) {
      if ((*it)->Update(secondsElapsed))
        it = m_actions.erase(it);
      else
        ++it;
   }

   if (m_fillParent && m_parent != NULL)
      SetBounds(0, 0, m_parent->m_width, m_parent->m_height);
}

void Actor::Draw(VP::SurfaceGraphics* pGraphics)
{
   if (!m_onStage) assert("Update was called on an actor which is not on stage.");
   if (m_visible && m_clearBackground) {
      pGraphics->SetColor(RGB(0, 0, 0));
      pGraphics->FillRectangle( { (int)m_x, (int)m_y, (int)m_width, (int)m_height } );
   }
}