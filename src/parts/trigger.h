// license:GPLv3+

// Definition of the Trigger class

#pragma once

#include "parts/dragpoint.h"
#include "parts/pintable.h"
#include "physics/hitable.h"
#include "renderer/Renderable.h"
#include "ui/win/resource.h"
#include "utils/eventproxy.h"

#include <memory>
#include <span>


class MeshBuffer;

class TriggerData final : public BaseProperty
{
public:
   Vertex2D m_vCenter;
   float m_radius;
   string m_szSurface;
   TriggerShape m_shape;
   float m_rotation;
   float m_scaleX;
   float m_scaleY;
   float m_hit_height; // trigger hit object height ... default 50
   float m_animSpeed;
   float m_wireThickness;
   bool m_enabled;
};

class Trigger :
   public IDispatchImpl<ITrigger, &IID_ITrigger, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Trigger, &CLSID_Trigger>,
   public EventProxy<Trigger, &DIID_ITriggerEvents>,
   public IConnectionPointContainerImpl<Trigger>,
   public IProvideClassInfo2Impl<&CLSID_Trigger, &DIID_ITriggerEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public IHitable,
   public IRenderable,
   public IScriptable,
   public IHaveDragPoints,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(MEMBERID index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) final;
#endif
   Trigger() { }
   virtual ~Trigger();

   BEGIN_COM_MAP(Trigger)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(ITrigger)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(Trigger)
   // Remove the comment from the line above if you don't want your object to
   // support aggregation.

   BEGIN_CONNECTION_POINT_MAP(Trigger)
      CONNECTION_POINT_ENTRY(DIID_ITriggerEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Trigger, eItemTrigger, TRIGGER, VIEW_PLAYFIELD)

   DECLARE_REGISTRY_RESOURCEID(IDR_TRIGGER)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   void MoveOffset(const float dx, const float dy) final;

   // Multi-object manipulation
   void FlipY(const Vertex2D& pvCenter) final;
   void FlipX(const Vertex2D& pvCenter) final;
   void Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Translate(const Vertex2D &pvOffset) final;
   Vertex2D GetCenter() const final { return GetPointCenter(); }
   Vertex2D GetScale() const final { return {m_d.m_scaleX, m_d.m_scaleY}; }
   float GetRotate() const final { return m_d.m_rotation; }

   void PutCenter(const Vertex2D& pv) final { PutPointCenter(pv); }
   Vertex2D GetPointCenter() const final;
   void PutPointCenter(const Vertex2D& pv) final;
   void ExportMesh(ObjLoader& loader) final;
   void UpdateStatusBarInfo() final { }

   void ClearForOverwrite() final;

   void WriteRegDefaults() final;

   void TriggerAnimationHit();
   void TriggerAnimationUnhit();

   // Fills 'outline' with the 2D outline of wire-shaped triggers for editor display (empty for other shapes)
   void GetWireOutline(vector<Vertex2D> &outline) const;

   TriggerData m_d;

private:
   // Regenerates the default drag point shape centered on (x, y), releasing any previously defined drag points
   void InitShape(float x, float y);

   // Non-owning views over the static mesh data matching a trigger shape
   struct StaticMeshData
   {
      std::span<const Vertex3D_NoTex2> vertices;
      std::span<const WORD> indices;
   };
   static StaticMeshData SetupMeshData(TriggerShape shape);

   // Generates the render mesh and the mesh bounding sphere center for the current m_d
   std::unique_ptr<std::vector<Vertex3D_NoTex2>> GenerateMesh(Vertex3Ds &boundingSphereCenter) const;

   // Valid through PhysicSetup/PhysicRelease
   TriggerHitCircle *m_ptriggerhitcircle = nullptr;
   Hit3DPoly *m_ptriggerhitpoly = nullptr;
   bool m_hitEvent = false;
   bool m_unhitEvent = false;

   // Valid through RenderSetup/RenderRelease
   Renderer *m_renderer = nullptr;
   std::shared_ptr<MeshBuffer> m_meshBuffer;
   std::unique_ptr<std::vector<Vertex3D_NoTex2>> m_triggerVertices;
   Vertex3Ds m_boundingSphereCenter;
   float m_animHeightOffset = 0.f;
   float m_vertexBuffer_animHeightOffset = -FLT_MAX;
   bool m_doAnimation = false;
   bool m_moveDown = false;

// ITrigger
public:
   STDMETHOD(get_TriggerShape)(/*[out, retval]*/ TriggerShape *pVal);
   STDMETHOD(put_TriggerShape)(/*[in]*/ TriggerShape newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Enabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Enabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Radius)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Radius)(/*[in]*/ float newVal);
   STDMETHOD(BallCntOver)(/*[out, retval]*/ int *pVal);
   STDMETHOD(DestroyBall)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_HitHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HitHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
   STDMETHOD(get_WireThickness)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_WireThickness)(/*[in]*/ float newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_AnimSpeed)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AnimSpeed)(/*[in]*/ float newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_CurrentAnimOffset)(/*[out, retval]*/ float *pVal);
};
