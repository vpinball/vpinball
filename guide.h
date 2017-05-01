// Ramp.h: Definition of the Ramp class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_GUIDE_H__9E875565_C5C1_482A_B05D_273743E1D1C0__INCLUDED_)
#define AFX_GUIDE_H__9E875565_C5C1_482A_B05D_273743E1D1C0__INCLUDED_

#include "resource.h"       // main symbols

class GuideData
{
public:
   char m_szMaterial[32];
   char m_szPhysicsMaterial[32];
   TimerDataRoot m_tdr;
   float m_heightbottom;
   float m_heighttop;

   float m_threshold;      // speed at which ball needs to hit to register a hit
   float m_elasticity;
   float m_friction;
   float m_scatter;
   float m_wireDiameter;

   float m_depthBias;      // for determining depth sorting

   bool m_fHitEvent;
   bool m_fCollidable;
   bool m_fVisible;
   bool m_fReflectionEnabled;
   bool m_fOverwritePhysics;
};

/////////////////////////////////////////////////////////////////////////////
// Ramp

class Guide :
   public IDispatchImpl<IGuide, &IID_IGuide, &LIBID_VPinballLib>,
   public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Guide, &CLSID_Guide>,
   public EventProxy<Guide, &DIID_IGuideEvents>,
   public IConnectionPointContainerImpl<Guide>,
   public IProvideClassInfo2Impl<&CLSID_Guide, &DIID_IGuideEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IHaveDragPoints,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   Guide();
   virtual ~Guide();

   BEGIN_COM_MAP(Guide)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IGuide)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(Guide)
      CONNECTION_POINT_ENTRY(DIID_IGuideEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Guide, eItemGuide, GUIDE, 1)

      //DECLARE_NOT_AGGREGATABLE(Ramp)
      // Remove the comment from the line above if you don't want your object to
      // support aggregation.

      DECLARE_REGISTRY_RESOURCEID(IDR_RAMP)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   void RenderOutline(Sur * const psur, const bool solid=false);
   virtual void RenderBlueprint(Sur *psur, const bool solid = false);

   virtual void ClearForOverwrite();

   float GetSurfaceHeight(float x, float y);

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();

   virtual void DoCommand(int icmd, int x, int y);

   virtual int GetMinimumPoints() const { return 2; }

   virtual void FlipY(Vertex2D * const pvCenter);
   virtual void FlipX(Vertex2D * const pvCenter);
   virtual void Rotate(float ang, Vertex2D *pvCenter);
   virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
   virtual void Translate(Vertex2D *pvOffset);

   virtual void GetCenter(Vertex2D * const pv) const { GetPointCenter(pv); }
   virtual void PutCenter(const Vertex2D * const pv) { PutPointCenter(pv); }

   virtual void GetBoundingVertices(std::vector<Vertex3Ds>& pvvertex3D);

   void AssignHeightToControlPoint(const RenderVertex3D &v, float height);
   virtual bool IsTransparent();
   virtual float GetDepth(const Vertex3Ds& viewDir);
   virtual unsigned long long GetMaterialID() { return m_ptable->GetMaterial(m_d.m_szMaterial)->hash(); }
   virtual void UpdatePropertyPanes();
   virtual void SetDefaultPhysics(bool fromMouseClick);
   virtual void ExportMesh(FILE *f);
   virtual void AddPoint(int x, int y, const bool smooth=false);

   void WriteRegDefaults();

   // IHaveDragPoints
   virtual void GetPointDialogPanes(Vector<PropertyPane> *pvproppane);
   // end IHaveDragPoints

   PinTable *m_ptable;

   GuideData m_d;

private:
   int rampVertex;
   Vertex2D *rgvInit;    // just for setup/static drawing
   float *rgheightInit;

   int m_numVertices;      // this goes along with dynamicVertexBuffer
   int m_numIndices;
   Vertex3D_NoTex2* m_vertBuffer;
   Vertex3D_NoTex2* m_vertBuffer2;
   std::vector<WORD> m_meshIndices;

   std::vector<HitObject*> m_vhoCollidable; // Objects to that may be collide selectable

   VertexBuffer *dynamicVertexBuffer;
   IndexBuffer *dynamicIndexBuffer;
   bool dynamicVertexBufferRegenerate;

   PropertyPane *m_propPosition;
   PropertyPane *m_propPhysics;

   bool isHabitrail() const;
   static const float HIT_SHAPE_DETAIL_LEVEL;

   // Get an approximation of the curve described by the control points of this ramp.
   template <typename T>
   void GetCentralCurve(std::vector<T> &vv, const float _accuracy=-1.f )
   {
      float accuracy;
      if (_accuracy==-1.f)
         accuracy = 4.0f*powf(10.0f, (10.0f - m_ptable->GetDetailLevel())*(float)(1.0 / 1.5)); // min = 4 (highest accuracy/detail level), max = 4 * 10^(10/1.5) = ~18.000.000 (lowest accuracy/detail level)
       else
          accuracy = 4.0f*powf(10.0f, (10.0f - _accuracy)*(float)(1.0 / 1.5)); // used for hit shape calculation, always!
      IHaveDragPoints::GetRgVertex(vv, false, accuracy);
   }


   Vertex2D *GetGuideVertex(int &pcvertex, float ** const ppheight, bool ** const ppfCross, float ** const ppratio, Vertex2D **const pMiddlePoints, const float _accuracy, const bool forRendering, const bool inc_width);
   void prepareHabitrail(RenderDevice* pd3dDevice);
   void AddJoint(Vector<HitObject> * pvho, const Vertex3Ds& v1, const Vertex3Ds& v2);
   void AddJoint2D(Vector<HitObject> * pvho, const Vertex2D& p, const float zlow, const float zhigh);
   void CheckJoint(Vector<HitObject> * const pvho, const HitTriangle * const ph3d1, const HitTriangle * const ph3d2);

   void RenderStaticHabitrail(RenderDevice* pd3dDevice, const Material * const mat);
   void RenderPolygons(RenderDevice* pd3dDevice, int offset, WORD * const rgicrosssection, const int start, const int stop);

   void GenerateVertexBuffer(RenderDevice* pd3dDevice);

   void AddLine(Vector<HitObject> * const pvho, const Vertex2D &pv1, const Vertex2D &pv2, const bool pv3_exists, const float height1, const float height2);
   void SetupHitObject(Vector<HitObject> * pvho, HitObject * obj);

   void RenderRamp(RenderDevice *pd3dDevice, const Material * const mat);
   void CreateWire(const int numRings, const int numSegments, const Vertex2D * const midPoints, Vertex3D_NoTex2 * const rgvBuf);
   void GenerateWireMesh(Vertex3D_NoTex2 **meshBuf, Vertex3D_NoTex2 **meshBuf2);
   void Guide::GenerateRampMesh(Vertex3D_NoTex2 **meshBuf);

   // IRamp
public:
   STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_HeightTop)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HeightTop)(/*[in]*/ float newVal);
   STDMETHOD(get_HeightBottom)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HeightBottom)(/*[in]*/ float newVal);

   STDMETHOD(get_HasHitEvent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HasHitEvent)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Threshold)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Threshold)(/*[in]*/ float newVal);
   STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Friction)(/*[in]*/ float newVal);
   STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scatter)(/*[in]*/ float newVal);

   STDMETHOD(get_DepthBias)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_DepthBias)(/*[in]*/ float newVal);
   STDMETHOD(get_WireDiameter)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_WireDiameter)(/*[in]*/ float newVal);
   STDMETHOD( get_PhysicsMaterial )(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD( put_PhysicsMaterial )(/*[in]*/ BSTR newVal);
   STDMETHOD( get_OverwritePhysics )(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD( put_OverwritePhysics )(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_GUIDE_H__9E875565_C5C1_482A_B05D_273743E1D1C0__INCLUDED_)
