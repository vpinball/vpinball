// license:GPLv3+

// Definition of the Light class

#pragma once

#include "ui/resource.h"
#include "renderer/RenderDevice.h"

enum ShadowMode : int // has to be int for loading
{
   NONE,
   RAYTRACED_BALL_SHADOWS
};

enum Fader : int
{
   FADER_NONE,
   FADER_LINEAR,
   FADER_INCANDESCENT
};

class LightData final : public BaseProperty
{
public:
   Vertex2D m_vCenter;    // xy table pos
   float m_height = 0.0f; // z pos above surface
   string m_szSurface;

   float m_falloff;
   float m_falloff_power;
   float m_intensity;
   float m_intensity_scale;
   float m_fadeSpeedUp;
   float m_fadeSpeedDown;

   float m_state; // 0..1 is modulated from off to on, 2 is blinking
   string m_rgblinkpattern;
   int m_blinkinterval;
   COLORREF m_color;
   COLORREF m_color2; // color full
   TimerDataRoot m_tdr;
   Shape m_shape;

   float m_depthBias; // for determining depth sorting

   bool m_showReflectionOnBall;

   bool m_imageMode; // For default render mode: true = pass through/no lighting, false = use surface material

   bool m_BulbLight; // Bulb halo render mode
   float m_bulbHaloHeight; // height of the halo (may be different from light z pos, for example a GI light rendered at playfield level)
   float m_transmissionScale;
   float m_modulate_vs_add;

   bool m_showBulbMesh;
   bool m_staticBulbMesh;
   float m_meshRadius;

   ShadowMode m_shadows = ShadowMode::NONE;
   Fader m_fader = Fader::FADER_LINEAR;
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

   void RenderBlueprint(Sur *psur, const bool solid) final;

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;

   void ClearForOverwrite() final;

   void EditMenu(CMenu &menu) final;
   void DoCommand(int icmd, int x, int y) final;

   void FlipY(const Vertex2D& pvCenter) final;
   void FlipX(const Vertex2D& pvCenter) final;
   void Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Translate(const Vertex2D &pvOffset) final;

   // DragPoints
   Vertex2D GetCenter() const final { return GetPointCenter(); }
   void PutCenter(const Vertex2D& pv) final { PutPointCenter(pv); }
   Vertex2D GetPointCenter() const final;
   void PutPointCenter(const Vertex2D& pv) final;
   float GetCurrentHeight() const { return m_backglass ? 0.0f : m_initSurfaceHeight + m_d.m_height; }

protected:
   RenderDevice *m_rd = nullptr;

public:
   float GetDepth(const Vertex3Ds& viewDir) const final;
   ItemTypeEnum HitableGetItemType() const final { return eItemLight; }
   void AddPoint(int x, int y, const bool smooth) final;

   void WriteRegDefaults() final;

   void InitShape();
   void setInPlayState(const float newVal);

   void RenderOutline(Sur *const psur);

   // Light definition
   LightData m_d;

   // Live data
   float m_inPlayState; // 0..1 is modulated from off to on, 2 is blinking
   float m_currentIntensity;
   double m_currentFilamentTemperature;
   float m_surfaceHeight;
   bool  m_lockedByLS;

private:
   class LightCenter final : public ISelect
   {
   public:
      LightCenter(Light *plight) : m_plight(plight) { }

      HRESULT GetTypeName(BSTR *pVal) { return m_plight->GetTypeName(pVal); }
      IDispatch *GetDispatch() { return m_plight->GetDispatch(); }
      const IDispatch *GetDispatch() const { return m_plight->GetDispatch(); }

      void Delete() { m_plight->Delete(); }
      void Uncreate() { m_plight->Uncreate(); }

      int GetSelectLevel() const { return (m_plight->m_d.m_shape == ShapeCircle) ? 1 : 2; } // Don't select light bulb twice if we have drag points

      IEditable *GetIEditable() { return (IEditable *)m_plight; }
      const IEditable *GetIEditable() const { return (const IEditable *)m_plight; }

      PinTable *GetPTable() { return m_plight->GetPTable(); }
      const PinTable *GetPTable() const { return m_plight->GetPTable(); }

      bool LoadToken(const int id, BiffReader * const pbr) { return true; }

      Vertex2D GetCenter() const { return m_plight->m_d.m_vCenter; }
      void PutCenter(const Vertex2D& pv) { m_plight->m_d.m_vCenter = pv; }

      void MoveOffset(const float dx, const float dy) {
          m_plight->m_d.m_vCenter.x += dx;
          m_plight->m_d.m_vCenter.y += dy;
      }

      ItemTypeEnum GetItemType() const { return eItemLightCenter; }

   private:
      Light *m_plight;
   };


   PinTable *m_ptable;

   Material *m_surfaceMaterial;
   Texture  *m_surfaceTexture;

   LightCenter m_lightcenter;

   MeshBuffer *m_lightmapMeshBuffer = nullptr;
   MeshBuffer *m_bulbSocketMeshBuffer = nullptr;
   MeshBuffer *m_bulbLightMeshBuffer = nullptr;
   PropertyPane *m_propVisual;

   vector<RenderVertex> m_vvertex;

   float m_initSurfaceHeight = 0.0f;
   float m_maxDist = 0.0f;
   bool  m_lightmapMeshBufferDirty = false;

   bool  m_roundLight; // pre-VPX compatibility

   Vertex3Ds m_boundingSphereCenter;
   //float m_boundingSphereRadius = -1.f;
   void UpdateBounds();

public:
   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Intensity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Intensity)(/*[in]*/ float newVal);
   STDMETHOD(get_TransmissionScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_TransmissionScale)(/*[in]*/ float newVal);
   STDMETHOD(get_IntensityScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_IntensityScale)(/*[in]*/ float newVal);
   STDMETHOD(get_BlinkInterval)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(put_BlinkInterval)(/*[in]*/ LONG newVal);
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
   STDMETHOD(get_State)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_State)(/*[in]*/ float newVal);
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
   STDMETHOD(get_Shadows)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(put_Shadows)(/*[in]*/ LONG newVal);
   STDMETHOD(get_Fader)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(put_Fader)(/*[in]*/ LONG newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(Duration)(/*[in]*/float startState, /*[in]*/LONG newVal, /*[in]*/float endState);
   STDMETHOD(get_FilamentTemperature)(/*[out, retval]*/ float *pVal);
   STDMETHOD(GetInPlayState)(/*[out, retval]*/ float* pVal);
   STDMETHOD(GetInPlayStateBool)(/*[out, retval]*/ VARIANT_BOOL* pVal);
   STDMETHOD(GetInPlayIntensity)(/*[out, retval]*/ float *pVal);


   // was: class IBlink
private:
   int m_duration;
   float m_finalLightState; // 0..1=Off..On, 2=Blinking

   U32 m_timenextblink;
   U32 m_timerDurationEndTime;
   U32 m_iblinkframe;

   void UpdateBlinker(const U32 time_msec)
   {
      if (m_timenextblink <= time_msec)
      {
         m_iblinkframe++;
         if (m_iblinkframe >= (U32)m_d.m_rgblinkpattern.length() || m_d.m_rgblinkpattern[m_iblinkframe] == '\0')
            m_iblinkframe = 0;

         m_timenextblink += m_d.m_blinkinterval;
      }
   }

   void RestartBlinker(const U32 cur_time_msec)
   {
      m_iblinkframe = 0;
      m_timenextblink = cur_time_msec + m_d.m_blinkinterval;
      m_timerDurationEndTime = cur_time_msec + m_duration;
   }
};
