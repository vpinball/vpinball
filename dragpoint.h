// DragPoint.h: Definition of the DragPoint class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_)
#define AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_

#include "resource.h"       // main symbols

//class Surface;

class IHaveDragPoints
{
public:
   virtual ~IHaveDragPoints();

   virtual IEditable *GetIEditable() = 0;
   virtual PinTable *GetPTable() = 0;

   virtual int GetMinimumPoints() const { return 3; }

   virtual HRESULT SavePointData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   //virtual HRESULT InitPointLoad(IStream *pstm, HCRYPTHASH hcrypthash);
   virtual void LoadPointToken(int id, BiffReader *pbr, int version);

   virtual void ClearPointsForOverwrite();

   virtual void GetPointCenter(Vertex2D * const pv) const;
   virtual void PutPointCenter(const Vertex2D * const pv);

   void FlipPointY(Vertex2D *pvCenter);
   void FlipPointX(Vertex2D *pvCenter);
   void RotateDialog();
   void RotatePoints(float ang, Vertex2D *pvCenter);
   void ScaleDialog();
   void ScalePoints(float scalex, float scaley, Vertex2D *pvCenter);
   void TranslateDialog();
   void TranslatePoints(Vertex2D *pvOffset);
   void ReverseOrder();
   virtual void GetPointDialogPanes(Vector<PropertyPane> *pvproppane);

   void GetTextureCoords(const std::vector<RenderVertex> & vv, float **ppcoords);

   friend class DragPoint;

protected:
   template <typename T>
   void GetRgVertex(std::vector<T> &vv, const bool loop = true, const float accuracy = (float)(1.0 / (0.5*0.5)))
   {
      static const int Dim = T::Dim;    // for now, this is always 2 or 3

      const int cpoint = m_vdpoint.Size();
      const int endpoint = loop ? cpoint : cpoint - 1;

      T rendv2;

      for (int i = 0; i < endpoint; i++)
      {
         const CComObject<DragPoint> * const pdp1 = m_vdpoint.ElementAt(i);
         const CComObject<DragPoint> * const pdp2 = m_vdpoint.ElementAt((i < cpoint - 1) ? (i + 1) : 0);

         if ((pdp1->m_v.x == pdp2->m_v.x) && (pdp1->m_v.y == pdp2->m_v.y) && (pdp1->m_v.z == pdp2->m_v.z))
         {
            // Special case - two points coincide
            continue;
         }

         int iprev = (pdp1->m_fSmooth ? i - 1 : i);
         if (iprev < 0)
            iprev = (loop ? cpoint - 1 : 0);

         int inext = (pdp2->m_fSmooth ? i + 2 : i + 1);
         if (inext >= cpoint)
            inext = (loop ? inext - cpoint : cpoint - 1);

         const CComObject<DragPoint> * const pdp0 = m_vdpoint.ElementAt(iprev);
         const CComObject<DragPoint> * const pdp3 = m_vdpoint.ElementAt(inext);

         CatmullCurve<Dim> cc;
         cc.SetCurve(pdp0->m_v, pdp1->m_v, pdp2->m_v, pdp3->m_v);

         T rendv1;

         rendv1.set(pdp1->m_v);
         rendv1.fSmooth = pdp1->m_fSmooth;
         rendv1.fSlingshot = pdp1->m_fSlingshot;
         rendv1.fControlPoint = true;

         // Properties of last point don't matter, because it won't be added to the list on this pass (it'll get added as the first point of the next curve)
         rendv2.set(pdp2->m_v);

         RecurseSmoothLine(cc, 0, 1, rendv1, rendv2, vv, accuracy);
      }

      if (!loop)
      {
         // Add the very last point to the list because nobody else added it
         rendv2.fSmooth = true;
         rendv2.fSlingshot = false;
         rendv2.fControlPoint = false;
         vv.push_back(rendv2);
      }
   }

   Vector< CComObject<DragPoint> > m_vdpoint;
};

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
   DragPoint() { m_v.z = 0.0f; m_calcHeight = 0.0f; }

   void Init(IHaveDragPoints *pihdp, const float x, const float y, const float z = 0.f);

   // From ISelect
   virtual void OnLButtonDown(int x, int y);
   virtual void OnLButtonUp(int x, int y);
   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   virtual ItemTypeEnum GetItemType() { return eItemDragPoint; }

   // Multi-object manipulation
   virtual void GetCenter(Vertex2D * const pv) const;
   virtual void PutCenter(const Vertex2D * const pv);

   virtual void EditMenu(HMENU hmenu);
   virtual void DoCommand(int icmd, int x, int y);
   virtual void SetSelectFormat(Sur *psur);
   virtual void SetMultiSelectFormat(Sur *psur);
   virtual PinTable *GetPTable() { return m_pihdp->GetIEditable()->GetPTable(); }
   virtual IEditable *GetIEditable() { return m_pihdp->GetIEditable(); }
   virtual IDispatch *GetDispatch();
   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   virtual int GetSelectLevel() { return 2; } // So dragpoints won't be band-selected with the main objects

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

   virtual void Delete();
   virtual void Uncreate();

   virtual BOOL LoadToken(int id, BiffReader *pbr);

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
   IHaveDragPoints *m_pihdp;
   bool m_fSmooth;
   bool m_fSlingshot;
   bool m_fAutoTexture;
};

#endif // !defined(AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_)
