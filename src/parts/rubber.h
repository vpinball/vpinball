// license:GPLv3+

// Definition of the Rubber class

#pragma once

#include "ui/resource.h"
#include "robin_hood.h"

class RubberData final : public BaseProperty
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
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
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

   void RenderBlueprint(Sur *psur, const bool solid) final;

   void ClearForOverwrite() final;

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;

   void DoCommand(int icmd, int x, int y) final;

   int GetMinimumPoints() const final { return 2; }

   void FlipY(const Vertex2D& pvCenter) final;
   void FlipX(const Vertex2D& pvCenter) final;
   void Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Translate(const Vertex2D &pvOffset) final;
   void AddPoint(int x, int y, const bool smooth) final;

   Vertex2D GetCenter() const final { return GetPointCenter(); }
   void PutCenter(const Vertex2D& pv) final { PutPointCenter(pv); }

   void GetBoundingVertices(vector<Vertex3Ds> &bounds, vector<Vertex3Ds> *const legacy_bounds) final;

   float GetDepth(const Vertex3Ds& viewDir) const final;
   ItemTypeEnum HitableGetItemType() const final { return eItemRubber; }
   void SetDefaultPhysics(const bool fromMouseClick) final;
   void ExportMesh(ObjLoader& loader) final;

   void WriteRegDefaults() final;
   void UpdateStatusBarInfo() final;

#if 0
   float GetSurfaceHeight(float x, float y) const final;
#endif

   RubberData m_d;

private:
   void AddHitEdge(class PhysicsEngine *physics, robin_hood::unordered_set<robin_hood::pair<unsigned, unsigned>> &addedEdges, const unsigned i, const unsigned j, const bool isUI);
   void SetupHitObject(class PhysicsEngine *physics, HitObject *obj, const bool isUI);

   PinTable *m_ptable;

   RenderDevice *m_rd = nullptr;
   MeshBuffer *m_meshBuffer = nullptr;
   bool m_dynamicVertexBufferRegenerate;
   bool m_bboxDirty = true;
   Vertex3Ds m_bboxMin;
   Vertex3Ds m_bboxMax;
   Vertex3Ds m_boundingSphereCenter;

   int m_numVertices; // this goes along with dynamicVertexBuffer
   int m_numIndices;

   vector<HitObject*> m_vhoCollidable; // Objects to that may be collide selectable
   vector<Vertex3D_NoTex2> m_vertices;
   vector<WORD> m_ringIndices;

   Vertex3Ds m_middlePoint;

   PropertyPane *m_propVisual;

   void GetCentralCurve(vector<RenderVertex> &vv, const float _accuracy = -1.f) const;

   Vertex2D *GetSplineVertex(int &pcvertex, bool ** const ppfCross, Vertex2D ** const pMiddlePoints, const float _accuracy = -1.f) const;

   void UpdateRubber(const bool updateVB, const float height);
   void GenerateMesh(const int _accuracy = -1, const bool createHitShape = false);
   void DrawRubberMesh(Sur * const psur);

   // IRubber
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
