// license:GPLv3+

// Definition of the Flasher class

#pragma once

#include "ui/resource.h"

class FlasherData final
{
public:
   COLORREF m_color;
   float m_depthBias = 0.f; // for determining depth sorting

   enum RenderMode
   {
      FLASHER,    // Custom blended images
      DMD,        // Dot matrix display (Plasma, LED, ...)
      DISPLAY,    // Screen (CRT, LCD, ...)
      ALPHASEG    // Alphanumeric segment display (VFD, Plasma, LED, ...)
   };
   RenderMode m_renderMode = RenderMode::FLASHER;

   // For DMD, Alphanum and Display rendering mode
   int m_renderStyle = 0;                 // application defined style profile reference
   string m_imageSrcLink;                 // image source (default is script)

   // For DMD, render the glass
   // string m_szImageA;                  // glass image is store as image A
   float m_glassRoughness = 0.f;
   COLORREF m_glassAmbient = 0x000000;
   float m_glassPadTop = 0.f;
   float m_glassPadBottom = 0.f;
   float m_glassPadLeft = 0.f;
   float m_glassPadRight = 0.f;

   // For flasher rendering mode
   int m_filterAmount;
   Filters m_filter;
   RampImageAlignment m_imagealignment;
   string m_szImageA;
   string m_szImageB;
   bool m_displayTexture;
   bool m_isVisible = true;
   bool m_addBlend;

   int m_alpha;
   float m_intensity_scale;
   float m_modulate_vs_add;
   string m_szLightmap;

   Vertex2D m_vCenter;
   float m_height;
   float m_rotX, m_rotY, m_rotZ;

   TimerDataRoot m_tdr;
};

class Flasher :
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<Flasher, &CLSID_Flasher>,
   public IDispatchImpl<IFlasher, &IID_IFlasher, &LIBID_VPinballLib>,
   public EventProxy<Flasher, &DIID_IFlasherEvents>,
   public IConnectionPointContainerImpl<Flasher>,
   public IProvideClassInfo2Impl<&CLSID_Flasher, &DIID_IFlasherEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IHaveDragPoints,
   public IScriptable,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
   Flasher();
   virtual ~Flasher();

   STANDARD_EDITABLE_DECLARES(Flasher, eItemFlasher, FLASHER, 3)

   BEGIN_COM_MAP(Flasher)
      COM_INTERFACE_ENTRY(IFlasher)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   DECLARE_REGISTRY_RESOURCEID(IDR_Flasher)

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   BEGIN_CONNECTION_POINT_MAP(Flasher)
      CONNECTION_POINT_ENTRY(DIID_IFlasherEvents)
   END_CONNECTION_POINT_MAP()


   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   void ClearForOverwrite() final;

   void RenderBlueprint(Sur *psur, const bool solid) final;

   void FlipY(const Vertex2D& pvCenter) final;
   void FlipX(const Vertex2D& pvCenter) final;
   void Rotate(const float ang, const Vertex2D &pvCenter, const bool useElementCenter) final;
   void Scale(const float scalex, const float scaley, const Vertex2D &pvCenter, const bool useElementCenter) final;
   void Translate(const Vertex2D &pvOffset) final;
   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;

   int GetMinimumPoints() const final { return 2; }

   Vertex2D GetCenter() const final { return m_d.m_vCenter; }
   void PutCenter(const Vertex2D& pv) final { m_d.m_vCenter = pv; }
   void DoCommand(int icmd, int x, int y) final;

   void AddPoint(int x, int y, const bool smooth) final;

protected:
   RenderDevice *m_rd = nullptr;

public:
   void UpdatePoint(int index, float x, float y);

   float GetDepth(const Vertex3Ds& viewDir) const final
   {
      return m_d.m_depthBias + viewDir.x * m_d.m_vCenter.x + viewDir.y * m_d.m_vCenter.y + viewDir.z * m_d.m_height;
   }
   ItemTypeEnum HitableGetItemType() const final { return eItemFlasher; }

   void WriteRegDefaults() final;

   int GetAlpha() const
   {
      return m_d.m_alpha;
   }
   void SetAlpha(const int value)
   {
      m_d.m_alpha = max(value,0);
   }
   int GetFilterAmount() const
   {
      return m_d.m_filterAmount;
   }
   void SetFilterAmount(const int value)
   {
      m_d.m_filterAmount = max(value,0);
   }

   //BaseTexture* GetVideoCap(const string& szName);

   void setInPlayState(const bool newVal);

   FlasherData m_d;

   bool m_lockedByLS;
   bool m_inPlayState;

   BaseTexture *m_dmdFrame = nullptr;

private:
   void InitShape();

   PinTable *m_ptable;

   unsigned int m_numVertices;
   int m_numPolys;
   float m_minx, m_maxx, m_miny, m_maxy;
   Vertex3D_NoTex2 *m_vertices;

   PropertyPane *m_propVisual;

   bool m_dynamicVertexBufferRegenerate;
   MeshBuffer *m_meshBuffer = nullptr;

   void ResetVideoCap();
   bool m_isVideoCap = false;
   int m_videoCapWidth = 0;
   int m_videoCapHeight = 0;
   RECT m_videoSourceRect;
   HWND m_videoCapHwnd = nullptr;
   BaseTexture* m_videoCapTex = nullptr;

   int2 m_dmdSize = int2(0,0);

   Light *m_lightmap = nullptr;

   // IFlasher
public:
   STDMETHOD(get_ImageA)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_ImageA)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_ImageB)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_ImageB)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Height)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_RotX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotX)(/*[in]*/ float newVal);
   STDMETHOD(get_RotY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotY)(/*[in]*/ float newVal);
   STDMETHOD(get_RotZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotZ)(/*[in]*/ float newVal);
   STDMETHOD(get_Opacity)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(put_Opacity)(/*[in]*/ LONG newVal);
   STDMETHOD(get_IntensityScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_IntensityScale)(/*[in]*/ float newVal);
   STDMETHOD(get_ModulateVsAdd)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ModulateVsAdd)(/*[in]*/ float newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_DisplayTexture)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisplayTexture)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_AddBlend)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_AddBlend)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_DMD)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DMD)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(put_DMDWidth)(/*[in]*/ int pVal);
   STDMETHOD(put_DMDHeight)(/*[in]*/ int pVal);
   STDMETHOD(put_DMDPixels)(/*[in]*/ VARIANT pVal);
   STDMETHOD(put_DMDColoredPixels)(/*[in]*/ VARIANT pVal);

   STDMETHOD(put_VideoCapWidth)(/*[in]*/ LONG cWidth);
   STDMETHOD(put_VideoCapHeight)(/*[in]*/ LONG cHeight);
   STDMETHOD(put_VideoCapUpdate)(/*[in]*/ BSTR cWinTitle);

   STDMETHOD(get_DepthBias)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_DepthBias)(/*[in]*/ float newVal);
   STDMETHOD(get_ImageAlignment)(/*[out, retval]*/ RampImageAlignment *pVal);
   STDMETHOD(put_ImageAlignment)(/*[in]*/ RampImageAlignment newVal);
   STDMETHOD(get_Filter)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Filter)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Amount)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(put_Amount)(/*[in]*/ LONG newVal);
};
