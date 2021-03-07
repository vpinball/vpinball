// Rubber.h: Definition of the Rubber class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_RUBBER_H__B0715DC0_002F_11E4_9191_0800200C9A66__INCLUDED_)
#define AFX_RUBBER_H__B0715DC0_002F_11E4_9191_0800200C9A66__INCLUDED_

#include "resource.h"       // main symbols

class RubberData : public BaseProperty
{
public:
   TimerDataRoot m_tdr;
   float m_height;
   float m_hitHeight;
   int m_thickness;
   float m_elasticityFalloff;
   float m_rotX;
   float m_rotY;
   float m_rotZ;
   bool m_staticRendering;
   bool m_showInEditor;
};

/////////////////////////////////////////////////////////////////////////////
// Rubber

class Rubber :
   public IDispatchImpl<IRubber, &IID_IRubber, &LIBID_VPinballLib>,
   public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Rubber, &CLSID_Rubber>,
   public EventProxy<Rubber, &DIID_IRubberEvents>,
   public IConnectionPointContainerImpl<Rubber>,
   public IProvideClassInfo2Impl<&CLSID_Rubber, &DIID_IRubberEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IHaveDragPoints,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   Rubber();
   virtual ~Rubber();

   BEGIN_COM_MAP(Rubber)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IRubber)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(Rubber)
      CONNECTION_POINT_ENTRY(DIID_IRubberEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Rubber, eItemRubber, RUBBER, 1)

      //DECLARE_NOT_AGGREGATABLE(Rubber)
      // Remove the comment from the line above if you don't want your object to
      // support aggregation.

      DECLARE_REGISTRY_RESOURCEID(IDR_RUBBER)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   virtual void RenderBlueprint(Sur *psur, const bool solid);

   virtual void ClearForOverwrite();

   float GetSurfaceHeight(float x, float y) const;

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();

   virtual void DoCommand(int icmd, int x, int y);

   virtual int GetMinimumPoints() const { return 2; }

   virtual void FlipY(const Vertex2D& pvCenter);
   virtual void FlipX(const Vertex2D& pvCenter);
   virtual void Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter);
   virtual void Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter);
   virtual void Translate(const Vertex2D &pvOffset);
   virtual void AddPoint(int x, int y, const bool smooth);

   virtual Vertex2D GetCenter() const { return GetPointCenter(); }
   virtual void PutCenter(const Vertex2D& pv) { PutPointCenter(pv); }

   virtual void GetBoundingVertices(std::vector<Vertex3Ds>& pvvertex3D);

   virtual float GetDepth(const Vertex3Ds& viewDir) const;
   virtual unsigned long long GetMaterialID() const { return m_ptable->GetMaterial(m_d.m_szMaterial)->hash(); }
   virtual unsigned long long GetImageID() const { return (unsigned long long)(m_ptable->GetImage(m_d.m_szImage)); }
   virtual ItemTypeEnum HitableGetItemType() const { return eItemRubber; }
   virtual void SetDefaultPhysics(bool fromMouseClick);
   virtual void ExportMesh(ObjLoader& loader);

   virtual void WriteRegDefaults();
   virtual void UpdateStatusBarInfo();

   RubberData m_d;

private:
   void AddHitEdge(vector<HitObject*> &pvho, std::set< std::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j);
   void SetupHitObject(vector<HitObject*> &pvho, HitObject * obj);

   PinTable *m_ptable;

   Vertex2D *m_rgvInit;    // just for setup/static drawing

   int m_numVertices;      // this goes along with dynamicVertexBuffer
   int m_numIndices;

   std::vector<HitObject*> m_vhoCollidable; // Objects to that may be collide selectable
   std::vector<Vertex3D_NoTex2> m_vertices;
   std::vector<WORD> m_ringIndices;

   Vertex3Ds m_middlePoint;

   VertexBuffer *m_dynamicVertexBuffer;
   IndexBuffer *m_dynamicIndexBuffer;
   bool m_dynamicVertexBufferRegenerate;

   PropertyPane *m_propVisual;
   PropertyPane *m_propPhysics;

   void GetCentralCurve(std::vector<RenderVertex> &vv, const float _accuracy = -1.f) const;

   Vertex2D *GetSplineVertex(int &pcvertex, bool ** const ppfCross, Vertex2D ** const pMiddlePoints, const float _accuracy = -1.f);

   void GenerateVertexBuffer();

   void RenderObject();
   void UpdateRubber(const bool updateVB, const float height);
   void GenerateMesh(const int _accuracy = -1, const bool createHitShape = false);
   void DrawRubberMesh(Sur * const psur);

   // IRamp
public:
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Height)(/*[in]*/ float newVal);
   STDMETHOD(get_HitHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HitHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_Thickness)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_Thickness)(/*[in]*/ int newVal);
   STDMETHOD(get_HasHitEvent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HasHitEvent)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_EnableStaticRendering)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_EnableStaticRendering)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_EnableShowInEditor)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_EnableShowInEditor)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
   STDMETHOD(get_ElasticityFalloff)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ElasticityFalloff)(/*[in]*/ float newVal);
   STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Friction)(/*[in]*/ float newVal);
   STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scatter)(/*[in]*/ float newVal);
   STDMETHOD(get_RotX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotX)(/*[in]*/ float newVal);
   STDMETHOD(get_RotY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotY)(/*[in]*/ float newVal);
   STDMETHOD(get_RotZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotZ)(/*[in]*/ float newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_PhysicsMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_PhysicsMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_OverwritePhysics)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_OverwritePhysics)(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
