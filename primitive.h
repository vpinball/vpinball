// 3dprimitive.h: interface for the 3dprimitive class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PRIMITIVE_H__31CD2D6B_9BDD_4B1B_BC62_B9DE588A0CAA__INCLUDED_)
#define AFX_PRIMITIVE_H__31CD2D6B_9BDD_4B1B_BC62_B9DE588A0CAA__INCLUDED_

#include "resource.h"
#include <set>

class Mesh
{
public:
   Vertex3Ds middlePoint;
   struct VertData
   {
      float x, y, z;
      float nx, ny, nz;
   };
   struct FrameData
   {
      std::vector<VertData> m_frameVerts;
   };

   std::vector<FrameData> m_animationFrames;
   std::vector<Vertex3D_NoTex2> m_vertices;
   std::vector<unsigned int> m_indices;

   Mesh() { middlePoint.x = 0.0f; middlePoint.y = 0.0f; middlePoint.z = 0.0f; }
   void Clear();
   bool LoadWavefrontObj(const string& fname, const bool flipTV, const bool convertToLeftHanded);
   void SaveWavefrontObj(const string& fname, const string& description);
   bool LoadAnimation(const char *fname, const bool flipTV, const bool convertToLeftHanded);

   size_t NumVertices() const    { return m_vertices.size(); }
   size_t NumIndices() const     { return m_indices.size(); }
   void UploadToVB(VertexBuffer * vb, const float frame);
};

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

class PrimitiveData : public BaseProperty
{
public:
   int m_Sides;
   Vertex3Ds m_vPosition;
   Vertex3Ds m_vSize;
   float m_aRotAndTra[9];
   char m_szNormalMap[MAXTOKEN];
   string m_meshFileName;

   COLORREF m_SideColor;
   TimerDataRoot m_tdr;

   float m_elasticityFalloff;
   float m_depthBias;      // for determining depth sorting

   float m_edgeFactorUI;   // for drawing the mesh in the editorUI
   float m_collision_reductionFactor; // 0=none..1=pow(num_vertices,0.75)

   float m_disableLightingTop;   // was bool, now 0..1
   float m_disableLightingBelow; // 0..1

   bool m_useAsPlayfield;

   bool m_use3DMesh;
   bool m_drawTexturesInside;
   bool m_staticRendering;

   bool m_toy;
   bool m_skipRendering;
   bool m_groupdRendering;
   bool m_backfacesEnabled;
   bool m_displayTexture;     // in editor
   bool m_objectSpaceNormalMap; // matches the +X,+Y,+Z object space export/baking of Blender
};

class Primitive :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<IPrimitive, &IID_IPrimitive, &LIBID_VPinballLib>,
   //public CComObjectRoot,
   public CComCoClass<Primitive, &CLSID_Primitive>,
   public EventProxy<Primitive, &DIID_IPrimitiveEvents>,
   public IConnectionPointContainerImpl<Primitive>,
   public IProvideClassInfo2Impl<&CLSID_Primitive, &DIID_IPrimitiveEvents, &LIBID_VPinballLib>,

   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   static const int Max_Primitive_Sides = 100; //!! 100 works for sleepy, 99 doesn't

   STDMETHOD(get_Sides)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_Sides)(/*[in]*/ int newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_SideColor)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_SideColor)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_DrawTexturesInside)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DrawTexturesInside)(/*[in]*/ VARIANT_BOOL newVal);

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

   STDMETHOD(get_RotAndTra0)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotAndTra0)(/*[in]*/ float newVal);
   STDMETHOD(get_RotAndTra1)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotAndTra1)(/*[in]*/ float newVal);
   STDMETHOD(get_RotAndTra2)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotAndTra2)(/*[in]*/ float newVal);
   STDMETHOD(get_RotAndTra3)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotAndTra3)(/*[in]*/ float newVal);
   STDMETHOD(get_RotAndTra4)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotAndTra4)(/*[in]*/ float newVal);
   STDMETHOD(get_RotAndTra5)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotAndTra5)(/*[in]*/ float newVal);
   STDMETHOD(get_RotAndTra6)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotAndTra6)(/*[in]*/ float newVal);
   STDMETHOD(get_RotAndTra7)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotAndTra7)(/*[in]*/ float newVal);
   STDMETHOD(get_RotAndTra8)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotAndTra8)(/*[in]*/ float newVal);

   STDMETHOD(get_RotX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotX)(/*[in]*/ float newVal);
   STDMETHOD(get_RotY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotY)(/*[in]*/ float newVal);
   STDMETHOD(get_RotZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RotZ)(/*[in]*/ float newVal);
   STDMETHOD(get_TransX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_TransX)(/*[in]*/ float newVal);
   STDMETHOD(get_TransY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_TransY)(/*[in]*/ float newVal);
   STDMETHOD(get_TransZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_TransZ)(/*[in]*/ float newVal);
   STDMETHOD(get_ObjRotX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ObjRotX)(/*[in]*/ float newVal);
   STDMETHOD(get_ObjRotY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ObjRotY)(/*[in]*/ float newVal);
   STDMETHOD(get_ObjRotZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ObjRotZ)(/*[in]*/ float newVal);

   STDMETHOD(get_EdgeFactorUI)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_EdgeFactorUI)(/*[in]*/ float newVal);
   STDMETHOD(get_CollisionReductionFactor)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_CollisionReductionFactor)(/*[in]*/ float newVal);

   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_NormalMap)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_NormalMap)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_MeshFileName)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_MeshFileName)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_EnableStaticRendering)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_EnableStaticRendering)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_HasHitEvent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HasHitEvent)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Threshold)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Threshold)(/*[in]*/ float newVal);
   STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_IsToy)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IsToy)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_BackfacesEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_BackfacesEnabled)(/*[in]*/ VARIANT_BOOL newVal);
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
   STDMETHOD(get_DisplayTexture)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisplayTexture)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_DisableLighting)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisableLighting)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_BlendDisableLighting)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BlendDisableLighting)(/*[in]*/ float newVal);
   STDMETHOD(get_BlendDisableLightingFromBelow)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BlendDisableLightingFromBelow)(/*[in]*/ float newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_PhysicsMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_PhysicsMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_OverwritePhysics)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_OverwritePhysics)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(PlayAnim)(float startFrame, float speed);
   STDMETHOD(PlayAnimEndless)(float speed);
   STDMETHOD(StopAnim)();
   STDMETHOD(ContinueAnim)(float speed);
   STDMETHOD(ShowFrame)(float frame);
   STDMETHOD(get_HitThreshold)(/*[out, retval]*/ float *pVal);
   STDMETHOD(get_ObjectSpaceNormalMap)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ObjectSpaceNormalMap)(/*[in]*/ VARIANT_BOOL newVal);

   Primitive();
   virtual ~Primitive();

   BEGIN_COM_MAP(Primitive)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IPrimitive)

      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)

      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)

      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(Primitive)
      CONNECTION_POINT_ENTRY(DIID_IPrimitiveEvents)
   END_CONNECTION_POINT_MAP()


   STANDARD_EDITABLE_DECLARES(Primitive, eItemPrimitive, PRIMITIVE, 1)

   DECLARE_REGISTRY_RESOURCEID(IDR_PRIMITIVE)

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   // Multi-object manipulation
   virtual Vertex2D GetCenter() const;
   virtual void PutCenter(const Vertex2D& pv);

   //STDMETHOD(get_Name)(BSTR *pVal) {return E_FAIL;}

   //virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR * const wzName);
   virtual void WriteRegDefaults();

   virtual bool LoadMeshDialog();
   virtual void ExportMeshDialog();

   virtual bool IsTransparent() const;
   virtual float GetDepth(const Vertex3Ds& viewDir) const;
   virtual unsigned long long GetMaterialID() const { return m_ptable->GetMaterial(m_d.m_szMaterial)->hash(); }
   virtual unsigned long long GetImageID() const { return (unsigned long long)(m_ptable->GetImage(m_d.m_szImage)); }
   virtual ItemTypeEnum HitableGetItemType() const { return eItemPrimitive; }

   virtual void SetDefaultPhysics(bool fromMouseClick);
   virtual void ExportMesh(ObjLoader& loader);
   virtual void RenderBlueprint(Sur *psur, const bool solid);
   virtual void UpdateStatusBarInfo();

   void CreateRenderGroup(const Collection * const collection);
   void RecalculateMatrices();
   void TransformVertices();
   void RenderObject();

   static INT_PTR CALLBACK ObjImportProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

   Mesh m_mesh;

   PrimitiveData m_d;

private:
   PinTable *m_ptable;

   Matrix3D m_fullMatrix;
   int m_numGroupVertices;
   int m_numGroupIndices;
   float m_currentFrame;
   float m_speed;
   bool m_doAnimation;
   bool m_endless;

   int m_numIndices;         // only used during loading
   int m_numVertices;        // only used during loading
#ifdef COMPRESS_MESHES
   int m_compressedIndices;  // only used during loading
   int m_compressedVertices; // only used during loading
   int m_compressedAnimationVertices; // only used during loading
#endif

   bool BrowseFor3DMeshFile();
   void SetupHitObject(vector<HitObject*> &pvho, HitObject * obj);
   void AddHitEdge(vector<HitObject*> &pvho, std::set< std::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j, const Vertex3Ds &vi, const Vertex3Ds &vj);

   void CalculateBuiltinOriginal();
   void WaitForMeshDecompression();

   PropertyPane *m_propVisual;
   PropertyPane *m_propPosition;
   PropertyPane *m_propPhysics;

   std::vector<HitObject*> m_vhoCollidable; // Objects to that may be collide selectable

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

   // Vertices for editor display & hit shape
   std::vector<Vertex3Ds> m_vertices;
   std::vector<float> m_normals; // only z component actually

   VertexBuffer *m_vertexBuffer;
   IndexBuffer *m_indexBuffer;
   bool m_vertexBufferRegenerate;
};

#endif // !defined(AFX_PRIMITIVE_H__31CD2D6B-9BDD-4B1B-BC62-B9DE588A0CAA__INCLUDED_)
