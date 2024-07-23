// Flasher.h: Definition of the Flasher class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_FLASHER_H__87DAB93E_7D6F_4fe4_A5F9_632FD82BDB4A__INCLUDED_)
#define AFX_FLASHER_H__87DAB93E_7D6F_4fe4_A5F9_632FD82BDB4A__INCLUDED_

#include "resource.h"       // main symbols

class FlasherData final
{
public:
   Vertex2D m_vCenter;
   float m_height;
   COLORREF m_color;
   TimerDataRoot m_tdr;
   float m_rotX, m_rotY, m_rotZ;
   int m_alpha;
   float m_intensity_scale;
   float m_modulate_vs_add;
   float m_depthBias;      // for determining depth sorting
   int  m_filterAmount;
   Filters m_filter;
   RampImageAlignment m_imagealignment;
   string m_szImageA;
   string m_szImageB;
   bool m_displayTexture;
   bool m_isVisible;
   bool m_addBlend;
   bool m_isDMD;
   string m_szLightmap;
};

/////////////////////////////////////////////////////////////////////////////
// Flasher

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
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
public:
   Flasher();
   virtual ~Flasher();

   STANDARD_EDITABLE_DECLARES(Flasher, eItemFlasher, FLASHER, 1)

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
#ifdef __STANDALONE__
   void UpdatePoint(int index, int x, int y);
#endif

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

   Flasher *CopyForPlay(PinTable *live_table);

   FlasherData m_d;

   bool m_lockedByLS;
   bool m_inPlayState;

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

   int2 m_dmdSize = int2(128,32);
   BaseTexture *m_texdmd = nullptr;

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
   STDMETHOD(get_Opacity)(/*[out, retval]*/ long *pVal);
   STDMETHOD(put_Opacity)(/*[in]*/ long newVal);
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

   STDMETHOD(put_VideoCapWidth)(/*[in]*/ long cWidth);
   STDMETHOD(put_VideoCapHeight)(/*[in]*/ long cHeight);
   STDMETHOD(put_VideoCapUpdate)(/*[in]*/ BSTR cWinTitle);

   STDMETHOD(get_DepthBias)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_DepthBias)(/*[in]*/ float newVal);
   STDMETHOD(get_ImageAlignment)(/*[out, retval]*/ RampImageAlignment *pVal);
   STDMETHOD(put_ImageAlignment)(/*[in]*/ RampImageAlignment newVal);
   STDMETHOD(get_Filter)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Filter)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Amount)(/*[out, retval]*/ long *pVal);
   STDMETHOD(put_Amount)(/*[in]*/ long newVal);
};

#endif // !defined(AFX_FLASHER_H__87DAB93E_7D6F_4fe4_A5F9_632FD82BDB4A__INCLUDED_)
