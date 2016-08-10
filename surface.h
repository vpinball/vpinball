#pragma once
// Surface.h : Declaration of the Surface

#ifndef __SURFACE_H_
#define __SURFACE_H_

#include "resource.h"       // main symbols

class SurfaceData
{
public:
   Vertex2D m_Center;
   TimerDataRoot m_tdr;
   float m_threshold;			// speed at which ball needs to hit to register a hit
   float m_slingshot_threshold;	// speed at which ball needs to trigger slingshot 
   char m_szImage[MAXTOKEN];
   char m_szSideImage[MAXTOKEN];
   char m_szTopMaterial[32];
   char m_szSideMaterial[32];
   char m_szSlingShotMaterial[32];
   char m_szPhysicsMaterial[32];
   float m_heightbottom;
   float m_heighttop;
   float m_slingshotforce;
   float m_elasticity;
   float m_friction;
   float m_scatter;
   bool m_fDroppable;
   bool m_fFlipbook;           // if enabled, dropped walls are not rendered
   bool m_fDisplayTexture;     // in editor
   bool m_fSideVisible;
   bool m_fEnabled;
   bool m_fCollidable; //wall must be droppable too!
   bool m_fIsBottomSolid; //is the bottom closed (lower side of the 'cube') or not (legacy behavior has bottom open, e.g. balls can drop into walls from below, or leave them if inside walls (if bottom area is large enough of course))
   bool m_fSlingshotAnimation;
   bool m_fDisableLighting;
   bool m_fTopBottomVisible;
   bool m_fHitEvent;
   bool m_fReflectionEnabled;
   bool m_fOverwritePhysics;
   bool m_fInner; //!! Deprecated, do not use! Always true after loading! (was: Inside or outside wall)
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
public:
   Surface();
   virtual ~Surface();

   HRESULT InitTarget(PinTable * const ptable, const float x, const float y, bool fromMouseClick);

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

   // IHaveDragPoints
   virtual void GetPointDialogPanes(Vector<PropertyPane> *pvproppane);
   // end IHaveDragPoints

   // IEditable
   virtual void WriteRegDefaults();
   virtual void RenderBlueprint(Sur *psur, const bool solid=false);

   virtual void GetBoundingVertices(std::vector<Vertex3Ds>& pvvertex3D);
   virtual void ClearForOverwrite();
   // end IEditable

   // ISelect
   virtual void FlipY(Vertex2D * const pvCenter);
   virtual void FlipX(Vertex2D * const pvCenter);
   virtual void Rotate(float ang, Vertex2D *pvCenter);
   virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
   virtual void Translate(Vertex2D *pvOffset);
   virtual void MoveOffset(const float dx, const float dy);

   virtual void GetCenter(Vertex2D * const pv) const { GetPointCenter(pv); }
   virtual void PutCenter(const Vertex2D * const pv) { PutPointCenter(pv); }

   virtual void DoCommand(int icmd, int x, int y);

   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);
   // end ISelect

   virtual float GetDepth(const Vertex3Ds& viewDir) { return viewDir.z * m_d.m_heighttop; }
   virtual unsigned long long GetMaterialID()
   {
      unsigned long long h = 0;
      if (m_d.m_fSideVisible)
         h = m_ptable->GetMaterial(m_d.m_szSideMaterial)->hash();
      if (m_d.m_fTopBottomVisible)
         h = m_ptable->GetMaterial(m_d.m_szTopMaterial)->hash();
      return h;
   }
   virtual unsigned long long GetImageID()
   {
      Texture* tex = NULL;
      if (m_d.m_fSideVisible)
         tex = m_ptable->GetImage(m_d.m_szSideImage);
      if (m_d.m_fTopBottomVisible)
         tex = m_ptable->GetImage(m_d.m_szImage);
      return (unsigned long long)tex;
   }
   virtual bool IsTransparent();
   virtual void UpdatePropertyPanes();
   virtual void SetDefaultPhysics(bool fromMouseClick);
   virtual void ExportMesh(FILE *f);
   virtual void AddPoint(int x, int y, const bool smooth=false);

private:
   void CurvesToShapes(Vector<HitObject> * const pvho);
   void SetupHitObject(Vector<HitObject> * pvho, HitObject * obj);
   void AddLine(Vector<HitObject> * const pvho, const RenderVertex &pv1, const RenderVertex &pv2);

   void RenderSlingshots(RenderDevice* pd3dDevice);
   void RenderWallsAtHeight(RenderDevice* pd3dDevice, const bool fDrop);
   void PrepareWallsAtHeight(RenderDevice* pd3dDevice);
   void PrepareSlingshots(RenderDevice *pd3dDevice);
   void GenerateMesh(std::vector<Vertex3D_NoTex2> &topBuf, std::vector<Vertex3D_NoTex2> &sideBuf, std::vector<WORD> &topBottomIndices, std::vector<WORD> &sideIndices);

   void FreeBuffers();

   PinTable *m_ptable;
   BSTR m_bstrName;

   std::vector<LineSegSlingshot*> m_vlinesling;

   std::vector<HitObject*> m_vhoDrop; // Objects to disable when dropped
   std::vector<HitObject*> m_vhoCollidable; // Objects to that may be collide selectable

   unsigned int numVertices, numPolys;

   VertexBuffer *slingshotVBuffer;
   VertexBuffer *VBuffer;
   IndexBuffer *IBuffer;

   PropertyPane *m_propPhysics;

   bool m_isWall;
   bool m_isDynamic;
   bool m_fIsDropped;

   // ISurface
public:
   bool m_fDisabled;
   SurfaceData m_d;

   STDMETHOD(get_SideVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_SideVisible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_SideImage)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_SideImage)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);

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
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_SlingshotThreshold)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SlingshotThreshold)(/*[in]*/ float newVal);
   STDMETHOD(get_SlingshotAnimation)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_SlingshotAnimation)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD( get_SlingshotMaterial )(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD( put_SlingshotMaterial )(/*[in]*/ BSTR newVal);
   STDMETHOD( get_PhysicsMaterial )(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD( put_PhysicsMaterial )(/*[in]*/ BSTR newVal);
   STDMETHOD( get_OverwritePhysics)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD( put_OverwritePhysics )(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHODIMP PlaySlingshotHit();
};

#endif //__SURFACE_H_
