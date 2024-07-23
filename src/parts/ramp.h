// Ramp.h: Definition of the Ramp class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
#define AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_

#include "resource.h"       // main symbols

class RampData final : public BaseProperty
{
public:
   TimerDataRoot m_tdr;
   float m_heightbottom;
   float m_heighttop;
   float m_widthbottom;
   float m_widthtop;
   RampType m_type;
   RampImageAlignment m_imagealignment;
   float m_leftwallheight;
   float m_rightwallheight;
   float m_leftwallheightvisible;
   float m_rightwallheightvisible;

   float m_wireDiameter;
   float m_wireDistanceX;
   float m_wireDistanceY;

   float m_depthBias;      // for determining depth sorting

   bool m_imageWalls;
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
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
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

   void RenderBlueprint(Sur *psur, const bool solid) final;

   void ClearForOverwrite() final;

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;

   void DoCommand(int icmd, int x, int y) final;

   int GetMinimumPoints() const final { return 2; }

   void FlipY(const Vertex2D& pvCenter) final;
   void FlipX(const Vertex2D& pvCenter) final;
   void Rotate(const float ang, const Vertex2D &pvCenter, const bool useElementCenter) final;
   void Scale(const float scalex, const float scaley, const Vertex2D &pvCenter, const bool useElementCenter) final;
   void Translate(const Vertex2D &pvOffset) final;

   Vertex2D GetCenter() const final { return GetPointCenter(); }
   void PutCenter(const Vertex2D &pv) final { PutPointCenter(pv); }

   void GetBoundingVertices(vector<Vertex3Ds> &pvvertex3D, const bool isLegacy) final;

   float GetDepth(const Vertex3Ds &viewDir) const final;
   ItemTypeEnum HitableGetItemType() const final { return eItemRamp; }
   void SetDefaultPhysics(const bool fromMouseClick) final;
   void ExportMesh(ObjLoader &loader) final;
   void AddPoint(int x, int y, const bool smooth) final;
   void UpdateStatusBarInfo() final;

   void WriteRegDefaults() final;

   float GetSurfaceHeight(float x, float y) const;
   bool isHabitrail() const;

   Ramp *CopyForPlay(PinTable *live_table);

   RampData m_d;

private:
   PinTable *m_ptable = nullptr;

   RenderDevice *m_rd = nullptr;

   int m_rampVertex;
   Vertex2D *m_rgvInit;    // just for setup/static drawing
   float *m_rgheightInit;

   int m_numVertices = 0;      // this goes along with dynamicVertexBuffer
   int m_numIndices = 0;
   Vertex3D_NoTex2* m_vertBuffer = nullptr;
   Vertex3D_NoTex2* m_vertBuffer2 = nullptr;
   vector<WORD> m_meshIndices;
   MeshBuffer *m_meshBuffer = nullptr;
   bool m_dynamicVertexBufferRegenerate = false;

   vector<HitObject*> m_vhoCollidable; // Objects to that may be collide selectable

   PropertyPane *m_propPhysics = nullptr;

   // Get an approximation of the curve described by the control points of this ramp.
   template <typename T>
   void GetCentralCurve(vector<T> &vv, const float _accuracy = -1.f) const
   {
      float accuracy;

      // as solid ramps are rendered into the static buffer, always use maximum precision
      if (_accuracy != -1.f)
         accuracy = _accuracy; // used for hit shape calculation, always!
      else
      {
         const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
         if (!mat->m_bOpacityActive)
            accuracy = 10.f;
         else
            accuracy = (float)m_ptable->GetDetailLevel();
      }

      accuracy = 4.0f*powf(10.0f, (10.0f - accuracy)*(float)(1.0 / 1.5)); // min = 4 (highest accuracy/detail level), max = 4 * 10^(10/1.5) = ~18.000.000 (lowest accuracy/detail level)

      IHaveDragPoints::GetRgVertex(vv, false, accuracy);
   }

   Vertex2D *GetRampVertex(int &pcvertex, float ** const ppheight, bool ** const ppfCross, float ** const ppratio, Vertex2D **const pMiddlePoints, const float _accuracy, const bool inc_width);
   void PrepareHabitrail();

   void AssignHeightToControlPoint(const RenderVertex3D &v, const float height);

   void AddJoint(vector<HitObject *> &pvho, const Vertex3Ds &v1, const Vertex3Ds &v2);
   void AddJoint2D(vector<HitObject*> &pvho, const Vertex2D& p, const float zlow, const float zhigh);
   void CheckJoint(vector<HitObject*> &pvho, const HitTriangle * const ph3d1, const HitTriangle * const ph3d2);

   void RenderStaticHabitrail(const Material * const mat);

   void GenerateVertexBuffer();

   void AddWallLineSeg(vector<HitObject*> &pvho, const Vertex2D &pv1, const Vertex2D &pv2, const bool pv3_exists, const float height1, const float height2, const float wallheight);
   void SetupHitObject(vector<HitObject*> &pvho, HitObject * obj);

   void RenderRamp(const Material * const mat);
   void CreateWire(const int numRings, const int numSegments, const Vertex2D * const midPoints, Vertex3D_NoTex2 * const rgvBuf);
   void GenerateWireMesh(Vertex3D_NoTex2 **meshBuf, Vertex3D_NoTex2 **meshBuf2);
   void GenerateRampMesh(Vertex3D_NoTex2 **meshBuf);

   Vertex3Ds m_boundingSphereCenter;
   void UpdateBounds();

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
   STDMETHOD(get_PhysicsMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_PhysicsMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_OverwritePhysics)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_OverwritePhysics)(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
