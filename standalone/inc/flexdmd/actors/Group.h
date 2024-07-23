#pragma once

#include "Actor.h"

class Group : public Actor
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   Group(FlexDMD* pFlexDMD, const string& name);
   ~Group();

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

   STDMETHOD(get_Clip)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Clip)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_ChildCount)(LONG *pRetVal);
   STDMETHOD(HasChild)(BSTR Name, VARIANT_BOOL *pRetVal);
   STDMETHOD(GetGroup)(BSTR Name, IGroupActor **pRetVal);
   STDMETHOD(GetFrame)(BSTR Name, IFrameActor **pRetVal);
   STDMETHOD(GetLabel)(BSTR Name, ILabelActor **pRetVal);
   STDMETHOD(GetVideo)(BSTR Name, IVideoActor **pRetVal);
   STDMETHOD(GetImage)(BSTR Name, IImageActor **pRetVal);
   STDMETHOD(RemoveAll)();
   STDMETHOD(AddActor)(IUnknown *child);
   STDMETHOD(RemoveActor)(IUnknown *child);

   void AddActor(Actor* actor);
   void AddActorAt(Actor* actor, int index);
   void RemoveActor(Actor* actor);
   vector<Actor*> GetChildren() { return m_children; }
   void Update(float delta) override;
   void Draw(VP::SurfaceGraphics* pGraphics) override;
   void OnStageStateChanged() override;

private:
   Group* GetRoot();
   Actor* Get(string name);

   bool m_clip;
   vector<Actor*> m_children;
};