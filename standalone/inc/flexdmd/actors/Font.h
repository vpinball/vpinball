#pragma once

#include "../FlexDMD.h"
#include "../bmfonts/BitmapFont.h"
#include "../../common/RendererGraphics.h"

class AssetManager;

class Font : public IDispatch  {
public:
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<Font*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<Font*>(this);
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
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId) { return E_NOTIMPL; }
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) { return E_NOTIMPL; }

public:
   Font(AssetManager* pAssetManager, AssetSrc* pAssetSrc);
   ~Font();

   void DrawCharacter(VP::SurfaceGraphics* pGraphics, char character, char previousCharacter, float& x, float& y);
   SDL_Rect MeasureFont(const string& text);
   void DrawText_(VP::SurfaceGraphics* pGraphics, float x, float y, const string& text);
   BitmapFont* GetBitmapFont() { return m_pBitmapFont; }

private:
   BitmapFont* m_pBitmapFont;
   SDL_Surface** m_textures;

   ULONG m_dwRef = 0;
};