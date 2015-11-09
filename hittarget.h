// 3dprimitive.h: interface for the 3dprimitive class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_HITTARGET_H__A67DE998_7D97_4E03_BE91_55BFD3A48DB6__INCLUDED_)
#define AFX_HITTARGET_H__A67DE998_7D97_4E03_BE91_55BFD3A48DB6__INCLUDED_

#include "resource.h"
#include <set>

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

class HitTargetData
{
public:
   int m_Sides;
   Vertex3Ds m_vPosition;
   Vertex3Ds m_vSize;
   float m_rotX, m_rotY, m_rotZ;
   char m_szImage[MAXTOKEN];
   char m_meshFileName[256];

   char m_szMaterial[32];
   COLORREF m_SideColor;

   TimerDataRoot m_tdr;

   float m_threshold;			// speed at which ball needs to hit to register a hit
   float m_elasticity;
   float m_elasticityFalloff;
   float m_friction;
   float m_scatter;

   float m_depthBias;      // for determining depth sorting
   bool m_fVisible;
   bool m_staticRendering;
   bool m_fDisableLighting;

   bool m_fHitEvent;
   bool m_fCollidable;
   bool m_fSkipRendering;
   bool m_fReflectionEnabled;
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
public:
   static const int Max_Primitive_Sides = 100; //!! 100 works for sleepy, 99 doesn't

   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_SideColor)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_SideColor)(/*[in]*/ OLE_COLOR newVal);

   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_Z)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Z)(/*[in]*/ float newVal);
   STDMETHOD(get_Size_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Size_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Size_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Size_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_Size_Z)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Size_Z)(/*[in]*/ float newVal);

   STDMETHOD(get_RotX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotX)(/*[in]*/ float newVal);
   STDMETHOD(get_RotY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotY)(/*[in]*/ float newVal);
   STDMETHOD(get_RotZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotZ)(/*[in]*/ float newVal);

   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_EnableStaticRendering)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_EnableStaticRendering)(/*[in]*/ VARIANT_BOOL newVal);

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
   STDMETHOD(get_DisableLighting)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisableLighting)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);

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

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   // Multi-object manipulation
   virtual void GetCenter(Vertex2D * const pv) const;
   virtual void PutCenter(const Vertex2D * const pv);

   //STDMETHOD(get_Name)(BSTR *pVal) {return E_FAIL;}

   //virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName);
   void WriteRegDefaults();
   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   PinTable *m_ptable;

   virtual bool IsTransparent();
   virtual float GetDepth(const Vertex3Ds& viewDir);
   virtual unsigned long long GetMaterialID() { return m_ptable->GetMaterial(m_d.m_szMaterial)->hash(); }

   virtual void UpdatePropertyPanes();
   virtual void SetDefaultPhysics(bool fromMouseClick);

   void    RecalculateMatrices();
   void    TransformVertices();

   static INT_PTR CALLBACK ObjImportProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

   Mesh m_mesh;

   HitTargetData m_d;
   Matrix3D fullMatrix;
   int m_numGroupVertices;
   int m_numGroupIndices;

private:        // private member functions

   int numIndices;         // only used during loading
   int numVertices;        // only used during loading
#ifdef COMPRESS_MESHES
   int compressedIndices;  // only used during loading
   int compressedVertices; // only used during loading
#endif

   void UpdateEditorView();

   bool BrowseFor3DMeshFile();
   void RenderObject(RenderDevice *pd3dDevice);
   void SetupHitObject(Vector<HitObject> * pvho, HitObject * obj);
   void AddHitEdge(Vector<HitObject> * pvho, std::set< std::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j, const Vertex3Ds &vi, const Vertex3Ds &vj);

   void CalculateBuiltinOriginal();

   PropertyPane *m_propVisual;
   PropertyPane *m_propPosition;
   PropertyPane *m_propPhysics;

private:        // private data members

   Vector<HitObject> m_vhoCollidable; // Objects to that may be collide selectable

   //!! outdated(?) information (along with the variable decls) for the old builtin primitive code, kept for reference:

   // Vertices for 3d Display
   //	Vertex3D_NoTex2 rgv3DTopOriginal[Max_Primitive_Sides+1]; // without transformation at index=0 is the middle point
   //	Vertex3D_NoTex2 rgv3DBottomOriginal[Max_Primitive_Sides+1];

   // these will be deleted:
   //	Vertex3D_NoTex2 rgv3DTop[Max_Primitive_Sides]; // with transformation
   //	WORD wTopIndices[Max_Primitive_Sides*6]; // *6 because of each point could be a triangle (*3) and for both sides because of culling (*2)
   //	Vertex3D_NoTex2 rgv3DBottom[Max_Primitive_Sides];
   //	WORD wBottomIndices[Max_Primitive_Sides*6];

   // OK here are our vertices that should be drawn:
   // Index				: Length		: Description
   // 0					: 1				: Middle Point Top
   // 1					: m_sides		: Top Vertices (no special order, will be sorted via Indices)
   // m_sides+1			: 1				: Middle Point Bottom
   // m_sides+2			: m_sides		: Bottom Vertices
   // m_sides*2 + 2		: m_sides+1		: Top Sides (with normals to the side) the first/last pioint is doubled, for textures
   // m_sides*3 + 3		: m_sides+1		: bottom Sides (With Normals to the side)
   //Example: 4 sides
   // Index				: Length		: Description
   // 0					: 1				: Middle Point Top
   // 1 to 4				: 4				: Top Vertices (no special order, will be sorted via Indices)
   // 5					: 1				: Middle Point Bottom
   // 6 to 9				: 4				: Bottom Vertices
   // 10 to 13		 		: 4				: Top Sides (with normals to the side)
   // 14 to 17				: 4				: bottom Sides (With Normals to the side)
   // These Vertices will always be complete. even if the user does not want to draw them (sides disabled or top/bottom disabled).
   // maybe they are not updated anymore, but they will be there.

   // per side i will use the following mem:
   // 13 * float * sides * 3 (vertices) = 13 * 4 * sides * 3 = 156 * sides bytes
   // word * 24 (indices) * sides = 4 * 24 * sides = 104 * sides bytes
   // float * 4 * sides = 16 * sidesm
   // so we will have: 276 bytes per side.
   // at 100 sides: 27.6 kb... per primitive That's OK
   // additional mem:
   // 13 * float * 2 (additional middle points at top and bottom)
   // = nothing...

   // Vertices for editor display
   std::vector<Vertex3Ds> vertices;
   std::vector<float> normals; // only z component actually


   VertexBuffer *vertexBuffer;
   IndexBuffer *indexBuffer;
   bool vertexBufferRegenerate;
};

#endif // !defined(AFX_HITTARGET_H__A67DE998_7D97_4E03_BE91_55BFD3A48DB6__INCLUDED_)
