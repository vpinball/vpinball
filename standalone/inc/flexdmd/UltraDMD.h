#pragma once

#include "flexudmd_i.h"
#include "../ultradmd/ultradmd_i.h"

#include "FlexDMD.h"
#include "FontDef.h"

#include "scenes/Sequence.h"
#include "actors/ScoreBoard.h"
#include "actors/Image.h"
#include "BaseDef.h"

#include <map>

class UltraDMD : IDispatch
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IDMDObject) {
         *ppv = reinterpret_cast<UltraDMD*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<UltraDMD*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<UltraDMD*>(this);
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
   UltraDMD(FlexDMD* pFlexDMD);
   ~UltraDMD();

   STDMETHOD(get_FloatProperty)(single *pRetVal);
   STDMETHOD(put_FloatProperty)(single pRetVal);
   STDMETHOD(Init)();
   STDMETHOD(Uninit)();
   STDMETHOD(GetMajorVersion)(LONG *pRetVal);
   STDMETHOD(GetMinorVersion)(LONG *pRetVal);
   STDMETHOD(GetBuildNumber)(LONG *pRetVal);
   STDMETHOD(SetVisibleVirtualDMD)(VARIANT_BOOL bHide, VARIANT_BOOL *pRetVal);
   STDMETHOD(SetFlipY)(VARIANT_BOOL flipY, VARIANT_BOOL *pRetVal);
   STDMETHOD(IsRendering)(VARIANT_BOOL *pRetVal);
   STDMETHOD(CancelRendering)();
   STDMETHOD(CancelRenderingWithId)(BSTR sceneId);
   STDMETHOD(Clear)();
   STDMETHOD(SetProjectFolder)(BSTR basePath);
   STDMETHOD(SetVideoStretchMode)(LONG mode);
   STDMETHOD(SetScoreboardBackgroundImage)(BSTR filename, LONG selectedBrightness, LONG unselectedBrightness);
   STDMETHOD(CreateAnimationFromImages)(LONG fps, VARIANT_BOOL loop, BSTR imagelist, LONG *pRetVal);
   STDMETHOD(RegisterVideo)(LONG videoStretchMode, VARIANT_BOOL loop, BSTR videoFilename, LONG *pRetVal);
   STDMETHOD(RegisterFont)(BSTR file, LONG *pRetVal);
   STDMETHOD(UnregisterFont)(LONG registeredFont);
   STDMETHOD(DisplayVersionInfo)();
   STDMETHOD(DisplayScoreboard)(LONG cPlayers, LONG highlightedPlayer, LONG score1, LONG score2, LONG score3, LONG score4, BSTR lowerLeft, BSTR lowerRight);
   STDMETHOD(DisplayScoreboard00)(LONG cPlayers, LONG highlightedPlayer, LONG score1, LONG score2, LONG score3, LONG score4, BSTR lowerLeft, BSTR lowerRight);
   STDMETHOD(DisplayScene00)(BSTR background, BSTR toptext, LONG topBrightness, BSTR bottomtext, LONG bottomBrightness, LONG animateIn, LONG pauseTime, LONG animateOut);
   STDMETHOD(DisplayScene00Ex)(BSTR background, BSTR toptext, LONG topBrightness, LONG topOutlineBrightness, BSTR bottomtext, LONG bottomBrightness, LONG bottomOutlineBrightness, LONG animateIn, LONG pauseTime, LONG animateOut);
   STDMETHOD(DisplayScene00ExWithId)(BSTR sceneId, VARIANT_BOOL cancelPrevious, BSTR background, BSTR toptext, LONG topBrightness, LONG topOutlineBrightness, BSTR bottomtext, LONG bottomBrightness, LONG bottomOutlineBrightness, LONG animateIn, LONG pauseTime, LONG animateOut);
   STDMETHOD(ModifyScene00)(BSTR id, BSTR toptext, BSTR bottomtext);
   STDMETHOD(ModifyScene00Ex)(BSTR id, BSTR toptext, BSTR bottomtext, LONG pauseTime);
   STDMETHOD(DisplayScene01)(BSTR sceneId, BSTR background, BSTR text, LONG textBrightness, LONG textOutlineBrightness, LONG animateIn, LONG pauseTime, LONG animateOut);
   STDMETHOD(DisplayText)(BSTR text, LONG textBrightness, LONG textOutlineBrightness);
   STDMETHOD(ScrollingCredits)(BSTR background, BSTR text, LONG textBrightness, LONG animateIn, LONG pauseTime, LONG animateOut);

   STDMETHOD(LoadSetup)();

private:
   Actor* ResolveImage(const string& filename, bool useFrame);
   Font* GetFont(const string& path, float brightness, float outlineBrightness);
   Label* GetFittedLabel(const string& text, float fillBrightness, float outlineBrightness);

   Sequence* m_pQueue;
   ScoreBoard* m_pScoreBoard;
   FontDef* m_pScoreFontText;
   FontDef* m_pScoreFontNormal;
   FontDef* m_pScoreFontHighlight;
   FontDef* m_pTwoLinesFontTop;
   FontDef* m_pTwoLinesFontBottom;
   vector<FontDef*> m_singleLineFonts;
   bool m_visible;
   int m_stretchMode;
   int m_nextId;
   std::map<int, BaseDef*> m_preloads;
   FlexDMD* m_pFlexDMD;

   ULONG m_dwRef = 0;
};