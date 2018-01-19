// Ramp.h: Definition of the Ramp class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
#define AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_

#include "resource.h"       // main symbols

class RampData
{
public:
   char m_szMaterial[32];
   char m_szPhysicsMaterial[32];
   TimerDataRoot m_tdr;
   float m_heightbottom;
   float m_heighttop;
   float m_widthbottom;
   float m_widthtop;
   RampType m_type;
   char m_szImage[MAXTOKEN];
   RampImageAlignment m_imagealignment;
   float m_leftwallheight;
   float m_rightwallheight;
   float m_leftwallheightvisible;
   float m_rightwallheightvisible;

   float m_threshold;      // speed at which ball needs to hit to register a hit
   float m_elasticity;
   float m_friction;
   float m_scatter;
   float m_wireDiameter;
   float m_wireDistanceX;
   float m_wireDistanceY;

   float m_depthBias;      // for determining depth sorting

   bool m_fHitEvent;
   bool m_fCollidable;
   bool m_fVisible;
   bool m_fImageWalls;
   bool m_fReflectionEnabled;
   bool m_fOverwritePhysics;
};

/////////////////////////////////////////////////////////////////////////////
// Ramp

class Ramp :
   public IDispatchImpl<IRamp, &IID_IRamp, &LIBID_VPinballLib>,
   public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Ramp, &CLSID_Ramp>,
   public EventProxy<Ramp, &DIID_IRampEvents>,
   public IConnectionPointContainerImpl<Ramp>,
   public IProvideClassInfo2Impl<&CLSID_Ramp, &DIID_IRampEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IHaveDragPoints,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   Ramp();
   virtual ~Ramp();

   BEGIN_COM_MAP(Ramp)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IRamp)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(Ramp)
      CONNECTION_POINT_ENTRY(DIID_IRampEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Ramp, eItemRamp, RAMP, 1)

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
   virtual void Rotate(float ang, Vertex2D *pvCenter, const bool useElementCenter = false);
   virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter, const bool useElementsCenter=false);
   virtual void Translate(Vertex2D *pvOffset);

   virtual void GetCenter(Vertex2D * const pv) const { GetPointCenter(pv); }
   virtual void PutCenter(const Vertex2D * const pv) { PutPointCenter(pv); }

   virtual void GetBoundingVertices(std::vector<Vertex3Ds>& pvvertex3D);

   void AssignHeightToControlPoint(const RenderVertex3D &v, float height);
   virtual bool IsTransparent();
   virtual float GetDepth(const Vertex3Ds& viewDir);
   virtual unsigned long long GetMaterialID() { return m_ptable->GetMaterial(m_d.m_szMaterial)->hash(); }
   virtual unsigned long long GetImageID() { return (unsigned long long)(m_ptable->GetImage(m_d.m_szImage)); }
   virtual ItemTypeEnum HitableGetItemType() { return eItemRamp; }
   virtual void UpdatePropertyPanes();
   virtual void SetDefaultPhysics(bool fromMouseClick);
   virtual void ExportMesh(FILE *f);
   virtual void AddPoint(int x, int y, const bool smooth=false);

   void WriteRegDefaults();
   void UpdateUnitsInfo();

   // IHaveDragPoints
   virtual void GetPointDialogPanes(Vector<PropertyPane> *pvproppane);
   // end IHaveDragPoints

   PinTable *m_ptable;

   RampData m_d;

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
   VertexBuffer *dynamicVertexBuffer2;
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


   Vertex2D *GetRampVertex(int &pcvertex, float ** const ppheight, bool ** const ppfCross, float ** const ppratio, Vertex2D **const pMiddlePoints, const float _accuracy, const bool forRendering, const bool inc_width);
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
   void Ramp::GenerateRampMesh(Vertex3D_NoTex2 **meshBuf);

   // IRamp
public:
   STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
   STDMETHOD(get_VisibleLeftWallHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_VisibleLeftWallHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_VisibleRightWallHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_VisibleRightWallHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_RightWallHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RightWallHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_LeftWallHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_LeftWallHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_HasWallImage)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HasWallImage)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ImageAlignment)(/*[out, retval]*/ RampImageAlignment *pVal);
   STDMETHOD(put_ImageAlignment)(/*[in]*/ RampImageAlignment newVal);
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Type)(/*[out, retval]*/ RampType *pVal);
   STDMETHOD(put_Type)(/*[in]*/ RampType newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_WidthTop)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_WidthTop)(/*[in]*/ float newVal);
   STDMETHOD(get_WidthBottom)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_WidthBottom)(/*[in]*/ float newVal);
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
   STDMETHOD(get_WireDistanceX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_WireDistanceX)(/*[in]*/ float newVal);
   STDMETHOD(get_WireDistanceY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_WireDistanceY)(/*[in]*/ float newVal);
   STDMETHOD( get_PhysicsMaterial )(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD( put_PhysicsMaterial )(/*[in]*/ BSTR newVal);
   STDMETHOD( get_OverwritePhysics )(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD( put_OverwritePhysics )(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
