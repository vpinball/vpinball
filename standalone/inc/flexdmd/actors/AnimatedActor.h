#pragma once

#include "Actor.h"

class AnimatedActor : public Actor
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   AnimatedActor(FlexDMD* pFlexDMD, const string& name);
   ~AnimatedActor();

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

   void Missing34();
   void Missing35();
   void Missing36();
   void Missing37();
   STDMETHOD(get_Length)(single *pRetVal);
   STDMETHOD(get_Loop)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Loop)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_Paused)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Paused)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_PlaySpeed)(single *pRetVal);
   STDMETHOD(put_PlaySpeed)(single pRetVal);
   STDMETHOD(Seek)(single posInSeconds);

   void Update(float delta) override;
   virtual void Seek_(float posInSeconds) { };
   virtual void Advance(float delta);
   virtual void Rewind();
   virtual void ReadNextFrame() = 0;

   float GetFrameTime() { return m_frameTime; }
   void SetFrameTime(float frameTime) { m_frameTime = frameTime; }
   float GetFrameDuration() { return m_frameDuration; }
   void SetFrameDuration(float frameDuration) { m_frameDuration = frameDuration; }
   void SetTime(float time) { m_time = time; }
   void SetEndOfAnimation(bool endOfAnimation) { m_endOfAnimation = endOfAnimation; }
   Scaling GetScaling() { return m_scaling; }
   void SetScaling(Scaling scaling) { m_scaling = scaling; }
   Alignment GetAlignment() { return m_alignment; }
   void SetAlignment(Alignment alignment) { m_alignment = alignment; }
   virtual float GetLength() { return m_length; }
   void SetLength(float length) { m_length = length; }
   bool GetLoop() { return m_loop; }
   void SetLoop(float loop) { m_loop = loop; }

private:
   float m_frameTime;
   float m_frameDuration;
   float m_time;
   bool m_endOfAnimation;

   Scaling m_scaling;
   Alignment m_alignment;
   bool m_paused;
   bool m_loop;
   float m_length;
   float m_playSpeed;
};