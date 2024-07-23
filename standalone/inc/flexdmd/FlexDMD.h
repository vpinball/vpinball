#pragma once

#include "DMDUtil/DMDUtil.h"

#include "flexdmd_i.h"
#include "AssetManager.h"
#include "../common/SurfaceGraphics.h"
#include "../common/DMDWindow.h"

#include <thread>

class Group;
class AnimatedActor;

class FlexDMD : public IDispatch
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IFlexDMD) {
         *ppv = reinterpret_cast<FlexDMD*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<FlexDMD*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<FlexDMD*>(this);
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
      ULONG dwRef = --m_dwRef;

      if (dwRef == 0)
         delete this;

      return dwRef;
   }

   STDMETHOD(GetTypeInfoCount)(UINT *pCountTypeInfo) { *pCountTypeInfo = 0; return S_OK;  }
   STDMETHOD(GetTypeInfo)(UINT iInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   FlexDMD();
   ~FlexDMD();

   STDMETHOD(get_Version)(LONG *pRetVal);
   STDMETHOD(get_RuntimeVersion)(LONG *pRetVal);
   STDMETHOD(put_RuntimeVersion)(LONG pRetVal);
   STDMETHOD(get_Run)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Run)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_Show)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Show)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_GameName)(BSTR *pRetVal);
   STDMETHOD(put_GameName)(BSTR pRetVal);
   STDMETHOD(get_Width)(unsigned short *pRetVal);
   STDMETHOD(put_Width)(unsigned short pRetVal);
   STDMETHOD(get_Height)(unsigned short *pRetVal);
   STDMETHOD(put_Height)(unsigned short pRetVal);
   STDMETHOD(get_Color)(OLE_COLOR *pRetVal);
   STDMETHOD(put_Color)(OLE_COLOR pRetVal);
   STDMETHOD(get_RenderMode)(RenderMode *pRetVal);
   STDMETHOD(put_RenderMode)(RenderMode pRetVal);
   STDMETHOD(get_ProjectFolder)(BSTR *pRetVal);
   STDMETHOD(put_ProjectFolder)(BSTR pRetVal);
   STDMETHOD(get_TableFile)(BSTR *pRetVal);
   STDMETHOD(put_TableFile)(BSTR pRetVal);
   STDMETHOD(get_Clear)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Clear)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_DmdColoredPixels)(VARIANT *pRetVal);
   STDMETHOD(get_DmdPixels)(VARIANT *pRetVal);
   STDMETHOD(putref_Segments)(VARIANT rhs);
   STDMETHOD(get_Stage)(IGroupActor **pRetVal);
   STDMETHOD(LockRenderThread)();
   STDMETHOD(UnlockRenderThread)();
   STDMETHOD(NewGroup)(BSTR Name, IGroupActor **pRetVal);
   STDMETHOD(NewFrame)(BSTR Name, IFrameActor **pRetVal);
   STDMETHOD(NewLabel)(BSTR Name, IUnknown *Font,BSTR Text, ILabelActor **pRetVal);
   STDMETHOD(NewVideo)(BSTR Name, BSTR video, IVideoActor **pRetVal);
   STDMETHOD(NewImage)(BSTR Name, BSTR image, IImageActor **pRetVal);
   STDMETHOD(NewFont)(BSTR Font, OLE_COLOR tint, OLE_COLOR borderTint, LONG borderSize, IUnknown **pRetVal);
   STDMETHOD(NewUltraDMD)(IUltraDMD **pRetVal);

   Font* NewFont(string szFont, OLE_COLOR tint, OLE_COLOR borderTint, LONG borderSize);
   AnimatedActor* NewVideo(string szVideo, string szName);
   LONG GetRuntimeVersion() { return m_runtimeVersion; }
   int GetWidth() { return m_width; }
   int GetHeight() { return m_height; }
   RenderMode GetRenderMode() { return m_renderMode; }
   void SetRenderMode(RenderMode renderMode) { m_renderMode = renderMode; }
   OLE_COLOR GetDMDColor() { return m_dmdColor; }
   void SetDMDColor(OLE_COLOR dmdColor) { m_dmdColor = dmdColor; }
   VP::SurfaceGraphics* GetGraphics() { return m_pGraphics; }
   Group* GetStage() { return m_pStage; }
   AssetManager* GetAssetManager() { return m_pAssetManager; }

private:
   void RenderLoop();
   void ShowDMD(bool show);

   string m_szGameName;
   string m_szTableFile;
   int m_frameRate;
   LONG m_runtimeVersion;
   bool m_clear;
   int m_renderLockCount;
   UINT16 m_segData1[128];
   UINT16 m_segData2[128];
   int m_width;
   int m_height;
   VP::SurfaceGraphics* m_pGraphics;
   Group* m_pStage;
   RenderMode m_renderMode;
   OLE_COLOR m_dmdColor;
   AssetManager* m_pAssetManager;
   VP::DMDWindow* m_pDMDWindow;
   bool m_show;
   DMDUtil::DMD* m_pDMD;
   DMDUtil::RGB24DMD* m_pRGB24DMD;
   std::thread* m_pThread;
   bool m_run;

   ULONG m_dwRef = 0;
};