// 3dprimitive.h: interface for the 3dprimitive class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PRIMITIVE_H__31CD2D6B_9BDD_4B1B_BC62_B9DE588A0CAA__INCLUDED_)
#define AFX_PRIMITIVE_H__31CD2D6B_9BDD_4B1B_BC62_B9DE588A0CAA__INCLUDED_

#include "resource.h"

class PrimitiveData
{
public:
   int m_Sides;
   Vertex3Ds m_vPosition;
   Vertex3Ds m_vSize;
   Vertex3Ds m_vAxisScaleX;
   Vertex3Ds m_vAxisScaleY;
   Vertex3Ds m_vAxisScaleZ;
   float m_aRotAndTra[6];
   RotAndTraTypeEnum m_aRotAndTraTypes[6];
   char m_szImage[MAXTOKEN];
   char meshFileName[256];

   COLORREF m_TopColor;
   COLORREF m_SideColor;

   TimerDataRoot m_tdr;
   RECT boundRectangle;

   bool use3DMesh;
   bool m_TopVisible;
   bool wasVisible;
   bool m_DrawTexturesInside;
   bool useLighting;
   bool staticRendering;
};

class Primitive :
   public IDispatchImpl<IPrimitive, &IID_IPrimitive, &LIBID_VBATESTLib>,
   public CComObjectRoot,
   public CComCoClass<Primitive,&CLSID_Primitive>,
#ifdef VBA
   //public CApcaaaControl<Primitive>,
   public CApcProjectItem<Primitive>,
#endif
   public EventProxy<Primitive, &DIID_IPrimitiveEvents>,
   public IConnectionPointContainerImpl<Primitive>,
   public IProvideClassInfo2Impl<&CLSID_Primitive, &DIID_IPrimitiveEvents, &LIBID_VBATESTLib>,

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
   STDMETHOD(get_TopColor)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_TopColor)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_SideColor)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_SideColor)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_DrawTexturesInside)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DrawTexturesInside)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_TopVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_TopVisible)(/*[in]*/ VARIANT_BOOL newVal);

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
   STDMETHOD(get_AxisScaleX_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AxisScaleX_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_AxisScaleX_Z)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AxisScaleX_Z)(/*[in]*/ float newVal);
   STDMETHOD(get_AxisScaleY_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AxisScaleY_X)(/*[in]*/ float newVal);
   STDMETHOD(get_AxisScaleY_Z)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AxisScaleY_Z)(/*[in]*/ float newVal);
   STDMETHOD(get_AxisScaleZ_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AxisScaleZ_X)(/*[in]*/ float newVal);
   STDMETHOD(get_AxisScaleZ_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AxisScaleZ_Y)(/*[in]*/ float newVal);

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

   STDMETHOD(get_RotAndTraType0)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
   STDMETHOD(put_RotAndTraType0)(/*[in]*/ RotAndTraTypeEnum newVal);
   STDMETHOD(get_RotAndTraType1)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
   STDMETHOD(put_RotAndTraType1)(/*[in]*/ RotAndTraTypeEnum newVal);
   STDMETHOD(get_RotAndTraType2)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
   STDMETHOD(put_RotAndTraType2)(/*[in]*/ RotAndTraTypeEnum newVal);
   STDMETHOD(get_RotAndTraType3)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
   STDMETHOD(put_RotAndTraType3)(/*[in]*/ RotAndTraTypeEnum newVal);
   STDMETHOD(get_RotAndTraType4)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
   STDMETHOD(put_RotAndTraType4)(/*[in]*/ RotAndTraTypeEnum newVal);
   STDMETHOD(get_RotAndTraType5)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
   STDMETHOD(put_RotAndTraType5)(/*[in]*/ RotAndTraTypeEnum newVal);

   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_MeshFileName)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_MeshFileName)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_EnableLighting)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_EnableLighting)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_EnableStaticRendering)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_EnableStaticRendering)(/*[in]*/ VARIANT_BOOL newVal);

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
   STANDARD_DISPATCH_DECLARE
   STANDARD_EDITABLE_DECLARES(eItemPrimitive)


   BEGIN_CONNECTION_POINT_MAP(Primitive)
      CONNECTION_POINT_ENTRY(DIID_IPrimitiveEvents)
   END_CONNECTION_POINT_MAP()

   DECLARE_REGISTRY_RESOURCEID(IDR_Primitive)

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   // Multi-object manipulation
   virtual void GetCenter(Vertex2D * const pv) const;
   virtual void PutCenter(const Vertex2D * const pv);

   //STDMETHOD(get_Name)(BSTR *pVal) {return E_FAIL;}

   //virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName);
   void WriteRegDefaults();
   //virtual HRESULT GetTypeName(BSTR *pVal);
   //virtual int GetDialogID();
   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   //virtual IScriptable *GetScriptable() {return NULL;}

#ifdef VBA
   virtual IApcControl *GetIApcControl() {return ApcControl.GetApcControl();}
#endif

   PinTable *m_ptable;

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

   Vertex3D_NoTex2 builtin_rgvOriginal[Max_Primitive_Sides*4+2];
   Vertex3D_NoTex2 builtin_rgv[Max_Primitive_Sides*4+2];

   // So how many indices are needed?
   // 3 per Triangle top - we have m_sides triangles -> 0, 1, 2, 0, 2, 3, 0, 3, 4, ...
   // 3 per Triangle bottom - we have m_sides triangles
   // 6 per Side at the side (two triangles form a rectangle) - we have m_sides sides
   // == 12 * m_sides
   // * 2 for both cullings (m_DrawTexturesInside == true)
   // == 24 * m_sides
   // this will also be the initial sorting, when depths, Vertices and Indices are recreated, because calculateRealTimeOriginal is called.
   WORD builtin_indices[Max_Primitive_Sides*24];

   // depth calculation
   // Since we are compiling with SSE, I'll use Floating points for comparison.
   // I need m_sides values at top
   // I need m_sides values at bottom
   // I need m_sides * 2 values at the side
   // in the implementation i will use shell sort like implemented at wikipedia.
   // Other algorithms are better at presorted things, but i will have some reverse sorted elements between the presorted here. 
   // That's why insertion or bubble sort does not work fast here...
   float builtin_depth[Max_Primitive_Sides*4];

   void CalculateBuiltinOriginal();
   void CalculateBuiltin();

   //!! here starts the more general primitive stuff:

   void RecalculateMatrices();
   void RecalculateVertices();
   void UpdateMesh();
   bool BrowseFor3DMeshFile();
   void RenderObject( RenderDevice *pd3dDevice);

   virtual bool LoadMesh();
   virtual void DeleteMesh();

   PrimitiveData m_d;
   int numVertices;
   VertexBuffer *vertexBuffer;
   BOOL vertexBufferRegenerate;
   Material material;

   // Vertices for editor display
   Vector<Vertex3Ds> verticesTop;
   Vector<Vertex3Ds> verticesBottom;

   Matrix3D fullMatrix;

   Vertex3D_NoTex2 *objMeshOrg, *objMesh;
   WORD *indexList;
   int indexListSize;
};

#endif // !defined(AFX_PRIMITIVE_H__31CD2D6B-9BDD-4B1B-BC62-B9DE588A0CAA__INCLUDED_)
