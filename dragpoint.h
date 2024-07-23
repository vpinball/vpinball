// DragPoint.h: Definition of the DragPoint class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_)
#define AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_

#include "resource.h"       // main symbols

class IHaveDragPoints;

/////////////////////////////////////////////////////////////////////////////
// DragPoint

class DragPoint :
   public IDispatchImpl<IControlPoint, &IID_IControlPoint, &LIBID_VPinballLib>,
   public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<DragPoint, &CLSID_DragPoint>,
   public ISelect
{
public:
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
#endif
   DragPoint() { }

   void Init(IHaveDragPoints *pihdp, const float x, const float y, const float z, const bool smooth);

   // From ISelect
   void OnLButtonDown(int x, int y) final;
   void OnLButtonUp(int x, int y) final;
   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;
   ItemTypeEnum GetItemType() const final { return eItemDragPoint; }

   // Multi-object manipulation
   Vertex2D GetCenter() const final;
   void PutCenter(const Vertex2D &pv) final;

   void EditMenu(CMenu &menu) final;
   void DoCommand(int icmd, int x, int y) final;
   void SetSelectFormat(Sur *psur) final;
   void SetMultiSelectFormat(Sur *psur) final;
   IEditable *GetIEditable() final;
   const IEditable *GetIEditable() const final;
   PinTable *GetPTable() final { return GetIEditable()->GetPTable(); }
   const PinTable *GetPTable() const final { return GetIEditable()->GetPTable(); }
   IDispatch *GetDispatch() final { return (IDispatch *)this; }
   const IDispatch *GetDispatch() const final { return (const IDispatch *)this; }

   int GetSelectLevel() const final { return 2; } // So dragpoints won't be band-selected with the main objects

   void Copy();
   void Paste();

   BEGIN_COM_MAP(DragPoint)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IControlPoint)
      COM_INTERFACE_ENTRY(ISupportErrorInfo)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(DragPoint)
   // Remove the comment from the line above if you don't want your object to
   // support aggregation.

   DECLARE_REGISTRY_RESOURCEID(IDR_DRAG_POINT)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   void Delete() final;
   void Uncreate() final;

   bool LoadToken(const int id, BiffReader *const pbr) final;

   // IControlPoint
public:
   STDMETHOD(get_TextureCoordinateU)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_TextureCoordinateU)(/*[in]*/ float newVal);
   STDMETHOD(get_IsAutoTextureCoordinate)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IsAutoTextureCoordinate)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Smooth)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Smooth)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_Z)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Z)(/*[in]*/ float newVal);
   STDMETHOD(get_CalcHeight)(/*[out, retval]*/ float *pVal);

   Vertex3Ds m_v;
   float m_calcHeight;
   float m_texturecoord;
   bool m_smooth;
   bool m_slingshot;
   bool m_autoTexture;

private:
#if defined(_M_X64) || defined(_M_AMD64) || !defined(_MSC_VER)
   IHaveDragPoints *m_pihdp;
   #define M_PIHDP m_pihdp
#else
   void *m_pihdp; // actually IHaveDragPoints, but somehow doesn't work on VS/x86, no sane solution found yet, might simply be a compiler bug
   #define M_PIHDP ((IHaveDragPoints *)m_pihdp)
#endif
   static Vertex3Ds m_copyPoint;   // coordinates of a control point to copy
   static bool      m_pointCopied;
};

class IHaveDragPoints
{
public:
   IHaveDragPoints();

   virtual ~IHaveDragPoints();

   virtual IEditable *GetIEditable() = 0;
   virtual const IEditable *GetIEditable() const = 0;
   virtual PinTable *GetPTable() = 0;
   virtual const PinTable *GetPTable() const = 0;

   virtual int GetMinimumPoints() const { return 3; }

   virtual HRESULT SavePointData(IStream *pstm, HCRYPTHASH hcrypthash);
   //virtual HRESULT InitPointLoad(IStream *pstm, HCRYPTHASH hcrypthash);
   virtual void LoadPointToken(int id, BiffReader *pbr, int version);

   virtual void ClearPointsForOverwrite();

   virtual Vertex2D GetPointCenter() const;
   virtual void PutPointCenter(const Vertex2D& pv);

   void FlipPointY(const Vertex2D& pvCenter);
   void FlipPointX(const Vertex2D& pvCenter);
   void RotateDialog();
   void RotatePoints(const float ang, const Vertex2D& pvCenter, const bool useElementCenter);
   void ScaleDialog();
   void ScalePoints(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter);
   void TranslateDialog();
   void TranslatePoints(const Vertex2D &pvOffset);
   void ReverseOrder();

   void GetTextureCoords(const vector<RenderVertex> & vv, float **ppcoords);

   friend class DragPoint;

   PropertyPane *m_propVisuals;
   PropertyPane *m_propPosition;

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/Math/DragPoint.cs
//

protected:
   template <typename T>
   void GetRgVertex(vector<T> &vv, const bool loop = true, const float accuracy = 4.f) const // 4 = maximum precision that we allow for
   {
      static const int Dim = T::Dim;    // for now, this is always 2 or 3

      const int cpoint = (int)m_vdpoint.size();
      const int endpoint = loop ? cpoint : cpoint - 1;

      T rendv2;

      for (int i = 0; i < endpoint; i++)
      {
         const CComObject<DragPoint> * const pdp1 = m_vdpoint[i];
         const CComObject<DragPoint> * const pdp2 = m_vdpoint[(i < cpoint - 1) ? (i + 1) : 0];

         if ((pdp1->m_v.x == pdp2->m_v.x) && (pdp1->m_v.y == pdp2->m_v.y) && (pdp1->m_v.z == pdp2->m_v.z))
         {
            // Special case - two points coincide
            continue;
         }

         int iprev = (pdp1->m_smooth ? i - 1 : i);
         if (iprev < 0)
            iprev = (loop ? cpoint - 1 : 0);

         int inext = (pdp2->m_smooth ? i + 2 : i + 1);
         if (inext >= cpoint)
            inext = (loop ? inext - cpoint : cpoint - 1);

         const CComObject<DragPoint> * const pdp0 = m_vdpoint[iprev];
         const CComObject<DragPoint> * const pdp3 = m_vdpoint[inext];

         CatmullCurve<Dim> cc;
         cc.SetCurve(pdp0->m_v, pdp1->m_v, pdp2->m_v, pdp3->m_v);

         T rendv1;

         rendv1.set(pdp1->m_v);
         rendv1.smooth = pdp1->m_smooth;
         rendv1.slingshot = pdp1->m_slingshot;
         rendv1.controlPoint = true;

         // Properties of last point don't matter, because it won't be added to the list on this pass (it'll get added as the first point of the next curve)
         rendv2.set(pdp2->m_v);

         RecurseSmoothLine(cc, 0.f, 1.f, rendv1, rendv2, vv, accuracy);
      }

      if (!loop)
      {
         // Add the very last point to the list because nobody else added it
         rendv2.smooth = true;
         rendv2.slingshot = false;
         rendv2.controlPoint = false;
         vv.push_back(rendv2);
      }
   }

   vector< CComObject<DragPoint>* > m_vdpoint;
};

//
// end of license:GPLv3+, back to 'old MAME'-like
//


#endif // !defined(AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_)
