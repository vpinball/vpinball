// DragPoint.h: Definition of the DragPoint class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_)
#define AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_

#include "resource.h" // main symbols

//class Surface;

class IHaveDragPoints
{
public:
  IHaveDragPoints();

  virtual ~IHaveDragPoints();

  virtual IEditable* GetIEditable() = 0;
  virtual const IEditable* GetIEditable() const = 0;
  virtual PinTable* GetPTable() = 0;
  virtual const PinTable* GetPTable() const = 0;

  virtual int GetMinimumPoints() const { return 3; }

  virtual HRESULT SavePointData(IStream* pstm, HCRYPTHASH hcrypthash);
  //virtual HRESULT InitPointLoad(IStream *pstm, HCRYPTHASH hcrypthash);
  virtual void LoadPointToken(int id, BiffReader* pbr, int version);

  virtual void ClearPointsForOverwrite();

  virtual Vertex2D GetPointCenter() const;
  virtual void PutPointCenter(const Vertex2D& pv);

  void FlipPointY(const Vertex2D& pvCenter);
  void FlipPointX(const Vertex2D& pvCenter);
  void RotateDialog();
  void RotatePoints(const float ang, const Vertex2D& pvCenter, const bool useElementCenter);
  void ScaleDialog();
  void ScalePoints(const float scalex,
                   const float scaley,
                   const Vertex2D& pvCenter,
                   const bool useElementCenter);
  void TranslateDialog();
  void TranslatePoints(const Vertex2D& pvOffset);
  void ReverseOrder();

  void GetTextureCoords(const std::vector<RenderVertex>& vv, float** ppcoords);

  friend class DragPoint;

  PropertyPane* m_propVisuals;
  PropertyPane* m_propPosition;

  //
  // license:GPLv3+
  // Ported at: VisualPinball.Engine/Math/DragPoint.cs
  //

protected:
  template<typename T>
  void GetRgVertex(std::vector<T>& vv,
                   const bool loop = true,
                   const float accuracy = 4.f) const // 4 = maximum precision that we allow for
  {
    static const int Dim = T::Dim; // for now, this is always 2 or 3

    const int cpoint = (int)m_vdpoint.size();
    const int endpoint = loop ? cpoint : cpoint - 1;

    T rendv2;

    for (int i = 0; i < endpoint; i++)
    {
      const CComObject<DragPoint>* const pdp1 = m_vdpoint[i];
      const CComObject<DragPoint>* const pdp2 = m_vdpoint[(i < cpoint - 1) ? (i + 1) : 0];

      if ((pdp1->m_v.x == pdp2->m_v.x) && (pdp1->m_v.y == pdp2->m_v.y) &&
          (pdp1->m_v.z == pdp2->m_v.z))
      {
        // Special case - two points coincide
        continue;
      }

      int iprev = (pdp1->m_smooth ? i - 1 : i);
      if (iprev < 0)
      {
        iprev = (loop ? cpoint - 1 : 0);
      }

      int inext = (pdp2->m_smooth ? i + 2 : i + 1);
      if (inext >= cpoint)
      {
        inext = (loop ? inext - cpoint : cpoint - 1);
      }

      const CComObject<DragPoint>* const pdp0 = m_vdpoint[iprev];
      const CComObject<DragPoint>* const pdp3 = m_vdpoint[inext];

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

  vector<CComObject<DragPoint>*> m_vdpoint;
};

//
// end of license:GPLv3+, back to 'old MAME'-like
//

/////////////////////////////////////////////////////////////////////////////
// DragPoint

class DragPoint : public IDispatchImpl<IControlPoint, &IID_IControlPoint, &LIBID_VPinballLib>,
                  public ISupportErrorInfo,
                  public CComObjectRoot,
                  public CComCoClass<DragPoint, &CLSID_DragPoint>,
                  public ISelect
{
public:
  DragPoint() {}

  void Init(IHaveDragPoints* pihdp, const float x, const float y, const float z, const bool smooth);

  // From ISelect
  virtual void OnLButtonDown(int x, int y);
  virtual void OnLButtonUp(int x, int y);
  virtual void MoveOffset(const float dx, const float dy);
  virtual void SetObjectPos();
  virtual ItemTypeEnum GetItemType() const { return eItemDragPoint; }

  // Multi-object manipulation
  virtual Vertex2D GetCenter() const;
  virtual void PutCenter(const Vertex2D& pv);

  virtual void EditMenu(CMenu& menu);
  virtual void DoCommand(int icmd, int x, int y);
  virtual void SetSelectFormat(Sur* psur);
  virtual void SetMultiSelectFormat(Sur* psur);
  virtual PinTable* GetPTable() { return m_pihdp->GetIEditable()->GetPTable(); }
  virtual const PinTable* GetPTable() const { return m_pihdp->GetIEditable()->GetPTable(); }
  virtual IEditable* GetIEditable() { return m_pihdp->GetIEditable(); }
  virtual const IEditable* GetIEditable() const { return m_pihdp->GetIEditable(); }
  virtual IDispatch* GetDispatch() { return (IDispatch*)this; }
  virtual const IDispatch* GetDispatch() const { return (const IDispatch*)this; }

  virtual int GetSelectLevel() const
  {
    return 2;
  } // So dragpoints won't be band-selected with the main objects

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

  virtual void Delete();
  virtual void Uncreate();

  virtual bool LoadToken(const int id, BiffReader* const pbr);

  // IControlPoint
public:
  STDMETHOD(get_TextureCoordinateU)(/*[out, retval]*/ float* pVal);
  STDMETHOD(put_TextureCoordinateU)(/*[in]*/ float newVal);
  STDMETHOD(get_IsAutoTextureCoordinate)(/*[out, retval]*/ VARIANT_BOOL* pVal);
  STDMETHOD(put_IsAutoTextureCoordinate)(/*[in]*/ VARIANT_BOOL newVal);
  STDMETHOD(get_Smooth)(/*[out, retval]*/ VARIANT_BOOL* pVal);
  STDMETHOD(put_Smooth)(/*[in]*/ VARIANT_BOOL newVal);
  STDMETHOD(get_X)(/*[out, retval]*/ float* pVal);
  STDMETHOD(put_X)(/*[in]*/ float newVal);
  STDMETHOD(get_Y)(/*[out, retval]*/ float* pVal);
  STDMETHOD(put_Y)(/*[in]*/ float newVal);
  STDMETHOD(get_Z)(/*[out, retval]*/ float* pVal);
  STDMETHOD(put_Z)(/*[in]*/ float newVal);
  STDMETHOD(get_CalcHeight)(/*[out, retval]*/ float* pVal);

  Vertex3Ds m_v;
  float m_calcHeight;
  float m_texturecoord;
  IHaveDragPoints* m_pihdp;
  bool m_smooth;
  bool m_slingshot;
  bool m_autoTexture;
  static Vertex3Ds m_copyPoint; // coordinates of a control point to copy
  static bool m_pointCopied;
};

#endif // !defined(AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_)
