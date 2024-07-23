#pragma once

#include "Actor.h"

class Frame : public Actor
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   Frame(FlexDMD* pFlexDMD, const string& name);
   ~Frame();

   STDMETHOD(get_Name)(BSTR *pRetVal) { return Actor::get_Name(pRetVal); }
   STDMETHOD(put_Name)(BSTR pRetVal) { return Actor::put_Name(pRetVal); };
   STDMETHOD(get_x)(single *pRetVal) { return Actor::get_x(pRetVal); };
   STDMETHOD(put_x)(single pRetVal) { return Actor::put_x(pRetVal); };
   STDMETHOD(get_y)(single *pRetVal) { return Actor::get_y(pRetVal); };
   STDMETHOD(put_y)(single pRetVal) { return Actor::put_y(pRetVal); };
   STDMETHOD(get_Width)(single *pRetVal) { return Actor::get_Width(pRetVal); };
   STDMETHOD(put_Width)(single pRetVal) { return Actor::put_Width(pRetVal); };
   STDMETHOD(get_Height)(single *pRetVal) { return Actor::get_Height(pRetVal); };
   STDMETHOD(put_Height)(single pRetVal) { return Actor::put_Height(pRetVal); };
   STDMETHOD(get_visible)(VARIANT_BOOL *pRetVal) { return Actor::get_visible(pRetVal); };
   STDMETHOD(put_visible)(VARIANT_BOOL pRetVal) { return Actor::put_visible(pRetVal); };
   STDMETHOD(get_FillParent)(VARIANT_BOOL *pRetVal) { return Actor::get_FillParent(pRetVal); };
   STDMETHOD(put_FillParent)(VARIANT_BOOL pRetVal) { return Actor::put_FillParent(pRetVal); };
   STDMETHOD(get_ClearBackground)(VARIANT_BOOL *pRetVal) { return Actor::get_ClearBackground(pRetVal); };
   STDMETHOD(put_ClearBackground)(VARIANT_BOOL pRetVal) { return Actor::put_ClearBackground(pRetVal); };
   STDMETHOD(SetBounds)(single x, single y, single Width, single Height) { return Actor::SetBounds(x, y, Width, Height); };
   STDMETHOD(SetPosition)(single x, single y) { return Actor::SetPosition(x, y); };
   STDMETHOD(SetAlignedPosition)(single x, single y, Alignment alignment) { return Actor::SetAlignedPosition(x, y, alignment); };
   STDMETHOD(SetSize)(single Width, single Height) { return Actor::SetSize(Width, Height); };
   STDMETHOD(get_PrefWidth)(single *pRetVal) { return Actor::get_PrefWidth(pRetVal); };
   STDMETHOD(get_PrefHeight)(single *pRetVal) { return Actor::get_PrefHeight(pRetVal); };
   STDMETHOD(Pack)() { return Actor::Pack(); };
   STDMETHOD(Remove)() { return Actor::Remove(); };
   STDMETHOD(get_ActionFactory)(IActionFactory **pRetVal) { return Actor::get_ActionFactory(pRetVal); };
   STDMETHOD(AddAction)(IUnknown *action) { return Actor::AddAction(action); };
   STDMETHOD(ClearActions)() { return Actor::ClearActions(); };

   STDMETHOD(get_Thickness)(LONG *pRetVal);
   STDMETHOD(put_Thickness)(LONG pRetVal);
   STDMETHOD(get_BorderColor)(OLE_COLOR *pRetVal);
   STDMETHOD(put_BorderColor)(OLE_COLOR pRetVal);
   STDMETHOD(get_Fill)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Fill)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_FillColor)(OLE_COLOR *pRetVal);
   STDMETHOD(put_FillColor)(OLE_COLOR pRetVal);

   void Draw(VP::SurfaceGraphics* pGraphics) override;

private:
   int m_thickness;
   OLE_COLOR m_borderColor;
   bool m_fill;
   OLE_COLOR m_fillColor;
};