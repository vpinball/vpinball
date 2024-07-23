// Plunger.h: Definition of the Plunger class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PLUNGER_H__A729D2E2_D68F_4DD6_BE4B_D8AD1B8C7B66__INCLUDED_)
#define AFX_PLUNGER_H__A729D2E2_D68F_4DD6_BE4B_D8AD1B8C7B66__INCLUDED_

#include "resource.h"       // main symbols

constexpr int MAXTIPSHAPE = 256;

class PlungerData final : public BaseProperty
{
public:
   COLORREF m_color;
   Vertex2D m_v;
   float m_width;
   float m_height;
   float m_stroke;
   float m_zAdjust;
   float m_speedPull;
   float m_speedFire;
   float m_mechStrength;
   PlungerType m_type;
   int m_animFrames;
   TimerDataRoot m_tdr;
   float m_parkPosition;
   string m_szSurface;
   float m_scatterVelocity;
   float m_momentumXfer;
   char m_szTipShape[MAXTIPSHAPE];
   float m_rodDiam;
   float m_ringGap;
   float m_ringDiam;
   float m_ringWidth;
   float m_springDiam;
   float m_springGauge;
   float m_springLoops;
   float m_springEndLoops;
   bool m_mechPlunger;
   bool m_autoPlunger;
};

/////////////////////////////////////////////////////////////////////////////
//
// Plunger shape descriptor
//

// Plunger shape descriptor coordinate entry.  The plunger is
// essentially built on a virtual lathe:  it consists of a series
// of circles centered on the longitudinal axis.  Each coordinate
// gives the position along the axis of the circle, expressed as
// the distance (in standard table units) from the tip, and the
// radius of the circle, expressed as a fraction of the nominal
// plunger width (m_d.m_width).  Each coordinate also specifies
// the normal for the vertices along that circle, and the vertical
// texture offset of the vertices.  The horizontal texture offset
// is inferred in the lathing process - the center of the texture
// is mapped to the top center of each circle, and the texture
// is wrapped around the sides of the circle.
struct PlungerCoord
{
   // radius at this point, as a fraction of nominal plunger width
   float r;

   // y position, in table distance units, with the tip at 0.0
   float y;

   // texture v coordinate of the vertices on this circle
   float tv;

   // normal of the top vertex along this circle
   float nx, ny;

   void set(const float _r, const float _y, const float _tv, const float _nx, const float _ny)
   {
      r  = _r;
      y  = _y;
      tv = _tv;
      nx = _nx;
      ny = _ny;
   }
};

// Plunger 3D shape descriptor
struct PlungerDesc
{
   // Number of coordinates in the lathe list.  If there are no
   // lathe points, this is the flat plunger, which we draw as
   // an alpha image on a simple flat rectangular surface.
   int n;

   // list of lathe coordinates
   PlungerCoord *c;
};


/////////////////////////////////////////////////////////////////////////////
// Plunger

class Plunger :
   public IDispatchImpl<IPlunger, &IID_IPlunger, &LIBID_VPinballLib>,
   public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Plunger, &CLSID_Plunger>,
   public EventProxy<Plunger, &DIID_IPlungerEvents>,
   public IConnectionPointContainerImpl<Plunger>,
   public IProvideClassInfo2Impl<&CLSID_Plunger, &DIID_IPlungerEvents, &LIBID_VPinballLib>,
   //public CComObjectRootEx<CComSingleThreadModel>,

   public ISelect,
   public IEditable,
   public Hitable,
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
   BEGIN_COM_MAP(Plunger)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IPlunger)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(Plunger)
      CONNECTION_POINT_ENTRY(DIID_IPlungerEvents)
   END_CONNECTION_POINT_MAP()
   //DECLARE_NOT_AGGREGATABLE(Plunger)
   // Remove the comment from the line above if you don't want your object to
   // support aggregation.

   Plunger();
   virtual ~Plunger();

   STANDARD_EDITABLE_DECLARES(Plunger, eItemPlunger, PLUNGER, 1)

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;
   // Multi-object manipulation
   Vertex2D GetCenter() const final;
   void PutCenter(const Vertex2D &pv) final;
   void SetDefaultPhysics(const bool fromMouseClick) final;
   ItemTypeEnum HitableGetItemType() const final { return eItemPlunger; }

   void WriteRegDefaults() final;

   DECLARE_REGISTRY_RESOURCEID(IDR_PLUNGER)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   Plunger* CopyForPlay(PinTable *live_table);

   PlungerData m_d;

private:
   PinTable *m_ptable = nullptr;

   RenderDevice *m_rd = nullptr;
   MeshBuffer *m_meshBuffer = nullptr;

   HitPlunger *m_phitplunger = nullptr;

   //
   // Plunger animation details.  We calculate these in RenderSetup()
   // and cache them here to use during rendering.
   //

   // number of animation frames
   int m_cframes = 0;

   // number of vertices per animation frame
   int m_vtsPerFrame = 0;

   // number of triangle indices per frame
   int m_indicesPerFrame = 0;

   Vertex3Ds m_boundingSphereCenter;
   float m_boundingSphereRadius = -1.f;

// IPlunger
public:
   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Width)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Width)(/*[in]*/ float newVal);
   STDMETHOD(put_ZAdjust)(/*[in]*/ float newVal);
   STDMETHOD(get_ZAdjust)(/*[out, retval]*/ float *pVal);
   STDMETHOD(CreateBall)(IBall **pBallEx);
   STDMETHOD(get_FireSpeed)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_FireSpeed)(/*[in]*/ float newVal);
   STDMETHOD(get_PullSpeed)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_PullSpeed)(/*[in]*/ float newVal);
   STDMETHOD(get_MechStrength)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_MechStrength)(/*[in]*/ float newVal);
   STDMETHOD(Fire)();
   STDMETHOD(PullBack)();
   STDMETHOD(PullBackandRetract)();

   STDMETHOD(get_Type)(/*[out, retval]*/ PlungerType *pVal);
   STDMETHOD(put_Type)(/*[in]*/ PlungerType newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_AnimFrames)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_AnimFrames)(/*[in]*/ int newVal);
   STDMETHOD(get_TipShape)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_TipShape)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_RodDiam)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RodDiam)(/*[in]*/ float newVal);
   STDMETHOD(get_RingGap)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RingGap)(/*[in]*/ float newVal);
   STDMETHOD(get_RingDiam)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RingDiam)(/*[in]*/ float newVal);
   STDMETHOD(get_RingWidth)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RingWidth)(/*[in]*/ float newVal);
   STDMETHOD(get_SpringDiam)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SpringDiam)(/*[in]*/ float newVal);
   STDMETHOD(get_SpringGauge)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SpringGauge)(/*[in]*/ float newVal);
   STDMETHOD(get_SpringLoops)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SpringLoops)(/*[in]*/ float newVal);
   STDMETHOD(get_SpringEndLoops)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SpringEndLoops)(/*[in]*/ float newVal);

   STDMETHOD(get_MechPlunger)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_MechPlunger)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_AutoPlunger)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_AutoPlunger)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ParkPosition)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ParkPosition)(/*[in]*/ float newVal);
   STDMETHOD(get_Stroke)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Stroke)(/*[in]*/ float newVal);
   STDMETHOD(get_ScatterVelocity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ScatterVelocity)(/*[in]*/ float newVal);
   STDMETHOD(get_MomentumXfer)(/*[out,retval]*/ float *pVal);
   STDMETHOD(put_MomentumXfer)(/*[in]*/ float newVal);
   STDMETHOD(Position)(float *pVal); //added for mechanical plunger position
   STDMETHOD(MotionDevice)(int *pVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_PLUNGER_H__A729D2E2_D68F_4DD6_BE4B_D8AD1B8C7B66__INCLUDED_)
