#pragma once
// Surface.h : Declaration of the Surface

#include "resource.h"       // main symbols

class SurfaceData final : public BaseProperty
{
public:
   Vertex2D m_Center;
   TimerDataRoot m_tdr;
   float m_slingshot_threshold;	// speed at which ball needs to trigger slingshot 
   string m_szSideImage;
   string m_szTopMaterial;
   string m_szSideMaterial;
   string m_szSlingShotMaterial;
   float m_heightbottom;
   float m_heighttop;
   float m_slingshotforce;
   float m_disableLightingTop;   // was bool, now 0..1
   float m_disableLightingBelow; // 0..1
   float m_elasticityFalloff;    // old tables had this fixed to 0.0
   bool m_droppable;
   bool m_flipbook;              // if enabled, dropped walls are not rendered
   bool m_displayTexture;        // in editor
   bool m_sideVisible;
   bool m_enabled;
   bool m_isBottomSolid;         // is the bottom closed (lower side of the 'cube') or not (legacy behavior has bottom open, e.g. balls can drop into walls from below, or leave them if inside walls (if bottom area is large enough of course))
   bool m_slingshotAnimation;
   bool m_topBottomVisible;
   bool m_inner; //!! Deprecated, do not use! Always true after loading! (was: Inside or outside wall)
};

/////////////////////////////////////////////////////////////////////////////
// Surface
class Surface :
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<Surface, &CLSID_Wall>,
   public IDispatchImpl<IWall, &IID_IWall, &LIBID_VPinballLib>,
   public EventProxy<Surface, &DIID_IWallEvents>,
   public IConnectionPointContainerImpl<Surface>,
   public IProvideClassInfo2Impl<&CLSID_Wall, &DIID_IWallEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IHaveDragPoints,
   public IScriptable,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
   //public EditableImpl<Surface>
{
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
public:

   Surface();
   virtual ~Surface();

   //HRESULT InitTarget(PinTable * const ptable, const float x, const float y, const bool fromMouseClick);

   STANDARD_EDITABLE_DECLARES(Surface, eItemSurface, WALL, 1)

   BEGIN_COM_MAP(Surface)
      COM_INTERFACE_ENTRY(IWall)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   DECLARE_REGISTRY_RESOURCEID(IDR_SURFACE)

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   BEGIN_CONNECTION_POINT_MAP(Surface)
      CONNECTION_POINT_ENTRY(DIID_IWallEvents)
   END_CONNECTION_POINT_MAP()

   // IEditable
   void WriteRegDefaults() final;
   void RenderBlueprint(Sur *psur, const bool solid) final;

   void GetBoundingVertices(vector<Vertex3Ds> &pvvertex3D, const bool isLegacy) /*const*/ final;
   void ClearForOverwrite() final;
   // end IEditable

   // ISelect
   void FlipY(const Vertex2D& pvCenter) final;
   void FlipX(const Vertex2D& pvCenter) final;
   void Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Translate(const Vertex2D &pvOffset) final;
   void MoveOffset(const float dx, const float dy) final;

   Vertex2D GetCenter() const final { return GetPointCenter(); }
   void PutCenter(const Vertex2D& pv) final { PutPointCenter(pv); }

   void DoCommand(int icmd, int x, int y) final;
   // end ISelect

   float GetDepth(const Vertex3Ds& viewDir) const final { return viewDir.z * m_d.m_heighttop; }
   ItemTypeEnum HitableGetItemType() const final { return eItemSurface; }

protected:
   void RenderSlingshots();
   void RenderWallsAtHeight(const bool drop, const bool isReflectionPass);

   RenderDevice *m_rd = nullptr;

public:
   void SetDefaultPhysics(const bool fromMouseClick) final;
   void ExportMesh(ObjLoader& loader) final;
   void AddPoint(int x, int y, const bool smooth) final;
   void UpdateStatusBarInfo() final;

   float    GetSlingshotStrength() const { return m_d.m_slingshotforce * (float)(1.0/10.0); }
   void     SetSlingshotStrength(const float value)
   {
       m_d.m_slingshotforce = value * 10.0f;
   }

   bool     StaticRendering() const
   {
      if (m_d.m_droppable)
         return false;
      if (m_rd != nullptr) // Static behavior is cached since changing the material could break rendering (is it still valid since we now allow to disable/enable static prerendering while playing)
         return !m_isDynamic;
      if (m_d.m_sideVisible)
      {
         if (m_ptable->GetMaterial(m_d.m_szSideMaterial)->m_bOpacityActive)
            return true;
      }
      if (m_d.m_topBottomVisible)
      {
         if (m_ptable->GetMaterial(m_d.m_szTopMaterial)->m_bOpacityActive)
            return true;
      }
      return false;
   }

   Surface *CopyForPlay(PinTable *live_table);

   SurfaceData m_d;
   bool m_disabled;

private:
   void CurvesToShapes(vector<HitObject*> &pvho);
   void SetupHitObject(vector<HitObject*> &pvho, HitObject * const obj);
   void AddLine(vector<HitObject*> &pvho, const RenderVertex &pv1, const RenderVertex &pv2);

   void GenerateMesh(vector<Vertex3D_NoTex2> &topBuf, vector<Vertex3D_NoTex2> &sideBuf, vector<WORD> &topBottomIndices, vector<WORD> &sideIndices);

   PinTable *m_ptable;

   vector<LineSegSlingshot*> m_vlinesling;

   vector<HitObject*> m_vhoDrop; // Objects to disable when dropped
   vector<HitObject*> m_vhoCollidable; // Objects to that may be collide selectable

   unsigned int m_numVertices, m_numPolys;

   MeshBuffer *m_slingshotMeshBuffer = nullptr;
   MeshBuffer *m_meshBuffer = nullptr;

   PropertyPane *m_propPhysics;

   bool m_isWall;
   bool m_isDynamic;
   bool m_isDropped;

   Vertex3Ds m_boundingSphereCenter;
   //float m_boundingSphereRadius = -1.f;
   void UpdateBounds();

// ISurface
public:
   STDMETHOD(get_SideVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_SideVisible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_SideImage)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_SideImage)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
   STDMETHOD(get_ElasticityFalloff)(/*[out, retval]*/ float* pVal);
   STDMETHOD(put_ElasticityFalloff)(/*[in]*/ float newVal);

   STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Friction)(/*[in]*/ float newVal);
   STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scatter)(/*[in]*/ float newVal);

   STDMETHOD(get_SlingshotStrength)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SlingshotStrength)(/*[in]*/ float newVal);
   STDMETHOD(get_DisplayTexture)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisplayTexture)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_IsDropped)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IsDropped)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_CanDrop)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_CanDrop)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_FlipbookAnimation)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_FlipbookAnimation)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_IsBottomSolid)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IsBottomSolid)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_TopMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_TopMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_HeightTop)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HeightTop)(/*[in]*/ float newVal);
   STDMETHOD(get_HeightBottom)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HeightBottom)(/*[in]*/ float newVal);
   STDMETHOD(get_ImageAlignment)(/*[out, retval]*/ ImageAlignment *pVal);
   STDMETHOD(put_ImageAlignment)(/*[in]*/ ImageAlignment newVal);
   STDMETHOD(get_SideMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_SideMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Threshold)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Threshold)(/*[in]*/ float newVal);
   STDMETHOD(get_HasHitEvent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HasHitEvent)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Disabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Disabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_DisableLighting)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisableLighting)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_BlendDisableLighting)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BlendDisableLighting)(/*[in]*/ float newVal);
   STDMETHOD(get_BlendDisableLightingFromBelow)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BlendDisableLightingFromBelow)(/*[in]*/ float newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_SlingshotThreshold)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SlingshotThreshold)(/*[in]*/ float newVal);
   STDMETHOD(get_SlingshotAnimation)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_SlingshotAnimation)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_SlingshotMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_SlingshotMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_PhysicsMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_PhysicsMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_OverwritePhysics)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_OverwritePhysics)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHODIMP PlaySlingshotHit();
};
