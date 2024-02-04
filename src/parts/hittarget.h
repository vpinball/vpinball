// 3dprimitive.h: interface for the 3dprimitive class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_HITTARGET_H__A67DE998_7D97_4E03_BE91_55BFD3A48DB6__INCLUDED_)
#define AFX_HITTARGET_H__A67DE998_7D97_4E03_BE91_55BFD3A48DB6__INCLUDED_

#include "resource.h"
#include "robin_hood.h"

// Indices for RotAndTra:
//     RotX = 0
//     RotY = 1
//     RotZ = 2
//     TraX = 3
//     TraY = 4
//     TraZ = 5
//  ObjRotX = 6
//  ObjRotY = 7
//  ObjRotZ = 8

class HitTargetData : public BaseProperty
{
public:
   Vertex3Ds m_vPosition;
   Vertex3Ds m_vSize;
   float m_rotZ;
   TargetType m_targetType;

   TimerDataRoot m_tdr;

   float m_elasticityFalloff;
   float m_dropSpeed;
   int   m_raiseDelay;

   float m_depthBias;      // for determining depth sorting
   float m_disableLightingTop; // was bool, now 0..1
   float m_disableLightingBelow; // 0..1

   bool m_legacy;
   bool m_isDropped;
};

class HitTarget :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<IHitTarget, &IID_IHitTarget, &LIBID_VPinballLib>,
   //public CComObjectRoot,
   public CComCoClass<HitTarget, &CLSID_HitTarget>,
   public EventProxy<HitTarget, &DIID_IHitTargetEvents>,
   public IConnectionPointContainerImpl<HitTarget>,
   public IProvideClassInfo2Impl<&CLSID_HitTarget, &DIID_IHitTargetEvents, &LIBID_VPinballLib>,

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
   static const float DROP_TARGET_LIMIT;

   HitTarget();
   virtual ~HitTarget();

   BEGIN_COM_MAP(HitTarget)
       COM_INTERFACE_ENTRY(IDispatch)
       COM_INTERFACE_ENTRY(IHitTarget)

       COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)

       COM_INTERFACE_ENTRY(IPerPropertyBrowsing)

       COM_INTERFACE_ENTRY(IProvideClassInfo)
       COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(HitTarget)
       CONNECTION_POINT_ENTRY(DIID_IHitTargetEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(HitTarget, eItemHitTarget, TARGET, 1)

   DECLARE_REGISTRY_RESOURCEID(IDR_HITTARGET)

   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);

   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_Z)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Z)(/*[in]*/ float newVal);
   STDMETHOD(get_ScaleX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ScaleX)(/*[in]*/ float newVal);
   STDMETHOD(get_ScaleY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ScaleY)(/*[in]*/ float newVal);
   STDMETHOD(get_ScaleZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ScaleZ)(/*[in]*/ float newVal);

   STDMETHOD(get_Orientation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Orientation)(/*[in]*/ float newVal);

   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);

   STDMETHOD(get_HasHitEvent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HasHitEvent)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Threshold)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Threshold)(/*[in]*/ float newVal);
   STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
   STDMETHOD(get_ElasticityFalloff)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ElasticityFalloff)(/*[in]*/ float newVal);
   STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Friction)(/*[in]*/ float newVal);
   STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scatter)(/*[in]*/ float newVal);
   STDMETHOD(get_DepthBias)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_DepthBias)(/*[in]*/ float newVal);
   STDMETHOD(get_DropSpeed)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_DropSpeed)(/*[in]*/ float newVal);
   STDMETHOD(get_CurrentAnimOffset)(/*[out, retval]*/ float *pVal);
   STDMETHOD(get_DisableLighting)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisableLighting)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_BlendDisableLighting)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BlendDisableLighting)(/*[in]*/ float newVal);
   STDMETHOD(get_BlendDisableLightingFromBelow)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BlendDisableLightingFromBelow)(/*[in]*/ float newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_IsDropped)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IsDropped)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_LegacyMode)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_LegacyMode)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_DrawStyle)(/*[out, retval]*/ TargetType *pVal);
   STDMETHOD(put_DrawStyle)(/*[in]*/ TargetType newVal);
   STDMETHOD(get_RaiseDelay)(/*[out, retval]*/ long *pVal);
   STDMETHOD(put_RaiseDelay)(/*[in]*/ long newVal);
   STDMETHOD(get_PhysicsMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_PhysicsMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_OverwritePhysics)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_OverwritePhysics)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_HitThreshold)(/*[out, retval]*/ float *pVal);


   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   // Multi-object manipulation
   virtual Vertex2D GetCenter() const;
   virtual void PutCenter(const Vertex2D& pv);

   //STDMETHOD(get_Name)(BSTR *pVal) {return E_FAIL;}

   //virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR * const wzName);
   void WriteRegDefaults() final;

   float GetDepth(const Vertex3Ds& viewDir) const final;
   ItemTypeEnum HitableGetItemType() const final { return eItemHitTarget; }

   void SetDefaultPhysics(const bool fromMouseClick) final;
   void ExportMesh(ObjLoader& loader) final;

   void GenerateMesh(vector<Vertex3D_NoTex2> &buf);
   void TransformVertices();
   void SetMeshType(const TargetType type);
   void UpdateStatusBarInfo() final;

   HitTarget *CopyForPlay(PinTable *live_table);

   HitTargetData m_d;

   bool             m_hitEvent;

private:

   void UpdateTarget();
   void SetupHitObject(vector<HitObject*> &pvho, HitObject * obj, const bool setHitObject);
   void AddHitEdge(vector<HitObject*> &pvho, robin_hood::unordered_set< robin_hood::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j, const Vertex3Ds &vi, const Vertex3Ds &vj, const bool setHitObject = true);

   PinTable        *m_ptable = nullptr;

   RenderDevice *m_rd = nullptr;
   const Vertex3D_NoTex2 *m_vertices = nullptr; // pointer just to the existing hittargets hardcoded in arrays
   const WORD      *m_indices = nullptr; // dto.
   unsigned int     m_numVertices = 0;
   unsigned int     m_numIndices = 0;
   MeshBuffer *m_meshBuffer = nullptr;

   PropertyPane *m_propVisual = nullptr;
   PropertyPane *m_propPosition = nullptr;
   PropertyPane *m_propPhysics = nullptr;

   vector<HitObject*> m_vhoCollidable; // Objects to that may be collide selectable

   // Vertices for editor display & hit shape
   vector<Vertex3Ds> m_hitUIVertices;
   vector<Vertex3D_NoTex2> m_transformedVertices;
   U32   m_timeStamp;
   float m_moveAnimationOffset;
   bool  m_moveAnimation;
   bool  m_moveDown;
};

#endif // !defined(AFX_HITTARGET_H__A67DE998_7D97_4E03_BE91_55BFD3A48DB6__INCLUDED_)
