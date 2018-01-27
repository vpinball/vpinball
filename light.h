// Light.h: Definition of the Light class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_LIGHT_H__7445FDB1_1FBE_4975_9AB6_367E6D16098F__INCLUDED_)
#define AFX_LIGHT_H__7445FDB1_1FBE_4975_9AB6_367E6D16098F__INCLUDED_

#include "resource.h"       // main symbols
#include "RenderDevice.h"

class LightData
{
public:
   Vertex2D m_vCenter;
   float m_falloff;
   float m_falloff_power;
   float m_intensity;
   float m_intensity_scale;
   float m_fadeSpeedUp;
   float m_fadeSpeedDown;
   U32 m_time_msec;
   float m_currentIntensity;
   float m_transmissionScale;
   float m_modulate_vs_add;
   float m_meshRadius;
   //float m_meshIntensity;
   //float m_meshFadeSpeed;
   LightState m_state;
   COLORREF m_color;
   COLORREF m_color2;
   TimerDataRoot m_tdr;
   Shape m_shape;
   //float m_borderwidth;
   //COLORREF m_bordercolor;
   //Material normalMatr;
   //Material borderMatr;
   //float normalR,normalG,normalB;
   //float borderR,borderG,borderB;
   char m_szSurface[MAXTOKEN];
   char m_szOffImage[MAXTOKEN];

   float m_depthBias;      // for determining depth sorting
   float m_bulbHaloHeight;

   bool m_fVisible;
   bool m_imageMode; // true = pass through/no lighting, false = use surface material
   bool m_BulbLight;
   bool m_showBulbMesh;
   bool m_showReflectionOnBall;
   bool m_staticBulbMesh;
};

class LightCenter : public ISelect
{
public:
   LightCenter(Light *plight);
   virtual HRESULT GetTypeName(BSTR *pVal);
   virtual IDispatch *GetDispatch();
   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   virtual void Delete();
   virtual void Uncreate();

   virtual int GetSelectLevel();

   virtual IEditable *GetIEditable();

   virtual PinTable *GetPTable();

   virtual BOOL LoadToken(int id, BiffReader *pbr) { return fTrue; }

   virtual void GetCenter(Vertex2D * const pv) const;
   virtual void PutCenter(const Vertex2D * const pv);

   virtual void MoveOffset(const float dx, const float dy);

   virtual ItemTypeEnum GetItemType() const { return eItemLightCenter; }

   Light *m_plight;
};

/////////////////////////////////////////////////////////////////////////////
// Light

class IBlink
{
public:
   char m_rgblinkpattern[33];
   int m_blinkinterval;
   int m_duration;
   int m_finalState;

   int m_timenextblink;
   int m_timerDurationEndTime;
   int m_iblinkframe;

   void UpdateBlinker(const int time_msec)
   {
      if (m_timenextblink <= time_msec)
      {
         m_iblinkframe++;
         if (m_rgblinkpattern[m_iblinkframe] == 0)
            m_iblinkframe = 0;

         m_timenextblink += m_blinkinterval;
      }
   }

   void RestartBlinker(const int cur_time_msec)
   {
      m_iblinkframe = 0;
      m_timenextblink = cur_time_msec + m_blinkinterval;
      m_timerDurationEndTime = cur_time_msec + m_duration;
   }
};

class Light :
   public IDispatchImpl<ILight, &IID_ILight, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Light, &CLSID_Light>,
   public EventProxy<Light, &DIID_ILightEvents>,
   public IConnectionPointContainerImpl<Light>,
   public IProvideClassInfo2Impl<&CLSID_Light, &DIID_ILightEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IHaveDragPoints,
   public IScriptable,
   public IBlink,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   Light();
   virtual ~Light();
   BEGIN_COM_MAP(Light)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(ILight)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(Light)
   // Remove the comment from the line above if you don't want your object to
   // support aggregation.

   BEGIN_CONNECTION_POINT_MAP(Light)
      CONNECTION_POINT_ENTRY(DIID_ILightEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Light, eItemLight, LIGHT, 3)

      DECLARE_REGISTRY_RESOURCEID(IDR_LIGHT)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   void RenderOutline(Sur * const psur);
   virtual void RenderBlueprint(Sur *psur, const bool solid=false);

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();

   virtual void ClearForOverwrite();

   void UpdateLightShapeHeight();
   void PrepareMoversCustom();

   virtual void EditMenu(HMENU hmenu);
   virtual void DoCommand(int icmd, int x, int y);

   virtual void FlipY(Vertex2D * const pvCenter);
   virtual void FlipX(Vertex2D * const pvCenter);
   virtual void Rotate(float ang, Vertex2D *pvCenter, const bool useElementCenter=false);
   virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter, const bool useElementsCenter=false);
   virtual void Translate(Vertex2D *pvOffset);

   // DragPoints
   virtual void GetCenter(Vertex2D * const pv) const { GetPointCenter(pv); }
   virtual void PutCenter(const Vertex2D * const pv) { PutPointCenter(pv); }
   virtual void GetPointCenter(Vertex2D * const pv) const;
   virtual void PutPointCenter(const Vertex2D * const pv);

   virtual bool IsTransparent() { return m_d.m_BulbLight || (m_surfaceMaterial && m_surfaceMaterial->m_bOpacityActive); }
   virtual bool RenderToLightBuffer()		{ return m_d.m_BulbLight && (m_d.m_transmissionScale > 0.f); }
   virtual float GetDepth(const Vertex3Ds& viewDir);
   virtual unsigned long long GetMaterialID() { return m_surfaceMaterial ? m_surfaceMaterial->hash() : 64-2; } //!! 2 = some constant number
   virtual unsigned long long GetImageID() { return (m_d.m_BulbLight ? 0 : (unsigned long long)(m_ptable->GetImage(m_d.m_szOffImage))); }
   virtual ItemTypeEnum HitableGetItemType() { return eItemLight; }
   virtual void UpdatePropertyPanes();
   virtual void AddPoint(int x, int y, const bool smooth=true);

   void WriteRegDefaults();
   void FreeBuffers();

   void        InitShape();
   void		lockLight();
   void		unLockLight();
   void		setLightStateBypass(const LightState newVal);
   void		setLightState(const LightState newVal);
   void        RenderBulbMesh(RenderDevice *pd3dDevice, COLORREF color);

   PinTable *m_ptable;

   LightData m_d;
   LightState m_realState;
   std::vector<RenderVertex> m_vvertex;

   float m_initSurfaceHeight;
   float m_surfaceHeight;
   bool  m_updateLightShape;

   // Run-time
private:
   Material *m_surfaceMaterial;
   Texture *m_surfaceTexture;

   LightCenter m_lightcenter;

   //>>> Added By Chris
   bool	m_fLockedByLS;
   bool    m_roundLight;
   //<<<

   int customMoverVertexNum;
   int customMoverIndexNum;
   VertexBuffer *customMoverVBuffer;
   IndexBuffer  *customMoverIBuffer;
   VertexBuffer *bulbLightVBuffer;
   IndexBuffer  *bulbLightIndexBuffer;
   VertexBuffer *bulbSocketVBuffer;
   IndexBuffer  *bulbSocketIndexBuffer;
   PropertyPane *m_propVisual;
   // ILight

   Texture *GetDisplayTexture();

public:
   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Intensity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Intensity)(/*[in]*/ float newVal);
   STDMETHOD(get_TransmissionScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_TransmissionScale)(/*[in]*/ float newVal);
   STDMETHOD(get_IntensityScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_IntensityScale)(/*[in]*/ float newVal);
   STDMETHOD(get_BlinkInterval)(/*[out, retval]*/ long *pVal);
   STDMETHOD(put_BlinkInterval)(/*[in]*/ long newVal);
   STDMETHOD(get_BlinkPattern)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_BlinkPattern)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_ColorFull)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_ColorFull)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_State)(/*[out, retval]*/ LightState *pVal);
   STDMETHOD(put_State)(/*[in]*/ LightState newVal);
   STDMETHOD(get_Falloff)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Falloff)(/*[in]*/ float newVal);
   STDMETHOD(get_FalloffPower)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_FalloffPower)(/*[in]*/ float newVal);
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_ImageMode)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ImageMode)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_DepthBias)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_DepthBias)(/*[in]*/ float newVal);
   STDMETHOD(get_FadeSpeedUp)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_FadeSpeedUp)(/*[in]*/ float newVal);
   STDMETHOD(get_FadeSpeedDown)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_FadeSpeedDown)(/*[in]*/ float newVal);
   STDMETHOD(get_Bulb)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Bulb)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ShowBulbMesh)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowBulbMesh)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_StaticBulbMesh)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_StaticBulbMesh)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ScaleBulbMesh)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ScaleBulbMesh)(/*[in]*/ float newVal);
   STDMETHOD(get_BulbModulateVsAdd)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BulbModulateVsAdd)(/*[in]*/ float newVal);
   STDMETHOD(get_BulbHaloHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BulbHaloHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_ShowReflectionOnBall)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowReflectionOnBall)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD( Duration )(/*[in]*/long startState, /*[in]*/long newVal, /*[in]*/long endState);
};

#endif // !defined(AFX_LIGHT_H__7445FDB1_1FBE_4975_9AB6_367E6D16098F__INCLUDED_)
