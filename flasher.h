// Flasher.h: Definition of the Flasher class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_FLASHER_H__87DAB93E_7D6F_4fe4_A5F9_632FD82BDB4A__INCLUDED_)
#define AFX_FLASHER_H__87DAB93E_7D6F_4fe4_A5F9_632FD82BDB4A__INCLUDED_

#include "resource.h"       // main symbols

class FlasherData
{
public:
   Vertex2D m_vCenter;
   float m_height;
   COLORREF m_color;
   TimerDataRoot m_tdr;
   float m_rotX, m_rotY, m_rotZ;
   int m_fAlpha;
   float m_intensity_scale;
   float m_modulate_vs_add;
   float m_depthBias;      // for determining depth sorting
   int  m_fFilterAmount;
   Filters m_filter;
   RampImageAlignment m_imagealignment;
   char m_szImageA[MAXTOKEN];
   char m_szImageB[MAXTOKEN];
   bool m_fDisplayTexture;
   bool m_IsVisible;
   bool m_fAddBlend;
   bool m_IsDMD;
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

      virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);
      virtual void GetPointDialogPanes(Vector<PropertyPane> *pvproppane);
      virtual void ClearForOverwrite();

      virtual void RenderBlueprint(Sur *psur, const bool solid=false);

      virtual void FlipY(Vertex2D * const pvCenter);
      virtual void FlipX(Vertex2D * const pvCenter);
      virtual void Rotate(float ang, Vertex2D *pvCenter);
      virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
      virtual void Translate(Vertex2D *pvOffset);
      virtual void MoveOffset(const float dx, const float dy);
      virtual void SetObjectPos();

      virtual int GetMinimumPoints() const { return 2; }

      virtual void GetCenter(Vertex2D * const pv) const { *pv = m_d.m_vCenter; }
      virtual void PutCenter(const Vertex2D * const pv) { m_d.m_vCenter = *pv; m_ptable->SetDirtyDraw(); }
      virtual void DoCommand(int icmd, int x, int y);

      virtual bool IsTransparent() { return !m_d.m_IsDMD; }
      virtual float GetDepth(const Vertex3Ds& viewDir)
      {
         return m_d.m_depthBias + viewDir.x * m_d.m_vCenter.x + viewDir.y * m_d.m_vCenter.y + viewDir.z * m_d.m_height;
      }
      virtual unsigned long long GetMaterialID() { return 64-1; } //!! some constant number
      virtual unsigned long long GetImageID()
      {
		  Texture * const pinA = m_ptable->GetImage(m_d.m_szImageA);
		  Texture * const pinB = m_ptable->GetImage(m_d.m_szImageB);
		  Texture *tex = NULL;
		  if (pinA && !pinB)
			  tex = pinA;
		  else if (!pinA && pinB)
			  tex = pinB;
		  return (unsigned long long)tex;
      }
      virtual bool IsDMD() { return m_d.m_IsDMD; }
      virtual ItemTypeEnum HitableGetItemType() { return eItemFlasher; }

      virtual void UpdatePropertyPanes();

      void WriteRegDefaults();
      void UpdateMesh();
      void InitShape();

      PinTable *m_ptable;

      FlasherData m_d;
      unsigned int numVertices;
      int numPolys;
      float minx, maxx, miny, maxy;
      Vertex3D_TexelOnly *vertices;

      VertexBuffer *dynamicVertexBuffer;
      IndexBuffer *dynamicIndexBuffer;
      
	  PropertyPane *m_propVisual;
	  
	  bool dynamicVertexBufferRegenerate;
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
