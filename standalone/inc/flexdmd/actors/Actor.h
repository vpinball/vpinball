#pragma once

#include "dispex.h"

#include "../FlexDMD.h"
#include "../Layout.h"
#include "../actions/ActionFactory.h"
#include "../actions/Action.h"
#include "../../common/RendererGraphics.h"

class Actor : public IDispatch
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IActor) {
         *ppv = reinterpret_cast<Actor*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IGroupActor) {
         *ppv = reinterpret_cast<Actor*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IFrameActor) {
         *ppv = reinterpret_cast<Actor*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IImageActor) {
         *ppv = reinterpret_cast<Actor*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IVideoActor) {
         *ppv = reinterpret_cast<Actor*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_ILabelActor) {
         *ppv = reinterpret_cast<Actor*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<Actor*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<Actor*>(this);
         AddRef();
         return S_OK;
      } else {
         *ppv = NULL;
         return E_NOINTERFACE;
      }
   }

   STDMETHOD_(ULONG, AddRef)() {
      m_dwRef++;
      return m_dwRef;
   }

   STDMETHOD_(ULONG, Release)() {
      m_dwRef--;

      //if (m_dwRef == 0)
      //   delete this;

      return m_dwRef;
   }

   STDMETHOD(GetTypeInfoCount)(UINT *pCountTypeInfo) { *pCountTypeInfo = 0; return S_OK;  }
   STDMETHOD(GetTypeInfo)(UINT iInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId) { return E_NOTIMPL; }
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) { return E_NOTIMPL; }

public:
   Actor(FlexDMD* pFlexDMD, string name);
   ~Actor();

   STDMETHOD(get_Name)(BSTR *pRetVal);
   STDMETHOD(put_Name)(BSTR pRetVal);
   STDMETHOD(get_x)(single *pRetVal);
   STDMETHOD(put_x)(single pRetVal);
   STDMETHOD(get_y)(single *pRetVal);
   STDMETHOD(put_y)(single pRetVal);
   STDMETHOD(get_Width)(single *pRetVal);
   STDMETHOD(put_Width)(single pRetVal);
   STDMETHOD(get_Height)(single *pRetVal);
   STDMETHOD(put_Height)(single pRetVal);
   STDMETHOD(get_visible)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_visible)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_FillParent)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_FillParent)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_ClearBackground)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_ClearBackground)(VARIANT_BOOL pRetVal);
   STDMETHOD(SetBounds)(single x, single y, single Width, single Height);
   STDMETHOD(SetPosition)(single x, single y);
   STDMETHOD(SetAlignedPosition)(single x, single y, Alignment alignment);
   STDMETHOD(SetSize)(single Width, single Height);
   STDMETHOD(get_PrefWidth)(single *pRetVal);
   STDMETHOD(get_PrefHeight)(single *pRetVal);
   STDMETHOD(Pack)();
   STDMETHOD(Remove)();
   STDMETHOD(get_ActionFactory)(IActionFactory **pRetVal);
   STDMETHOD(AddAction)(IUnknown *action);
   STDMETHOD(ClearActions)();

   void SetFlexDMD(FlexDMD* pFlexDMD) { m_pFlexDMD = pFlexDMD; }
   FlexDMD* GetFlexDMD() { return m_pFlexDMD; }
   void SetName(const string& szName) { m_szName = szName; }
   const string& GetName() { return m_szName; }
   single GetX() { return m_x; }
   void SetX(single x) { m_x = x; }
   single GetY() { return m_y; }
   void SetY(single y) { m_y = y; }
   single GetWidth() { return m_width; }
   void SetWidth(single width) { m_width = width; }
   single GetHeight() { return m_height; }
   void SetHeight(single height) { m_height = height; }
   virtual single GetPrefWidth() { return m_prefWidth; }
   void SetPrefWidth(single prefWidth) { m_prefWidth = prefWidth; }
   virtual single GetPrefHeight() { return m_prefHeight; }
   void SetPrefHeight(single prefHeight) { m_prefHeight = prefHeight; }
   bool GetVisible() { return m_visible; }
   virtual void SetVisible( bool visible ) { m_visible = visible; }
   bool GetOnStage() { return m_onStage; }
   void SetOnStage(bool onStage) { m_onStage = onStage; this->OnStageStateChanged(); }
   void SetFillParent(bool fillParent) { m_fillParent = fillParent; }
   Group* GetParent() { return m_parent; }
   void SetParent(Group* parent) { m_parent = parent; }

   virtual void OnStageStateChanged() { }
   virtual void Update(float secondsElapsed);
   virtual void Draw(VP::SurfaceGraphics* pGraphics);

protected:
   ActionFactory* m_pActionFactory;

private:
   FlexDMD* m_pFlexDMD;
   vector<Action*> m_actions;
   bool m_onStage;
   string m_szName;
   single m_x;
   single m_y;
   single m_width;
   single m_height;
   Group* m_parent;
   bool m_fillParent;
   bool m_clearBackground;
   single m_prefWidth;
   single m_prefHeight;
   bool m_visible;

   ULONG m_dwRef = 0;
};