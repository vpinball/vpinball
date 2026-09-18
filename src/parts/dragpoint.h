// license:GPLv3+

// Definition of the DragPoint class

#pragma once

#include "core/ieditable.h"
#include "math/MeshUtils.h"
#include "ui/win/resource.h"

class DragPointCurve;

class DragPoint : public IDispatchImpl<IControlPoint, &IID_IControlPoint, &LIBID_VPinballLib>,
                  public ISupportErrorInfo,
                  public CComObjectRoot,
                  public CComCoClass<DragPoint, &CLSID_DragPoint>
{
public:
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(MEMBERID index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
#endif
   DragPoint() { }

   void Init(DragPointCurve *pcurve, const float x, const float y, const float z, const bool smooth);

   static inline constexpr ItemTypeEnum ItemType = eItemDragPoint;
   ItemTypeEnum GetItemType() const { return eItemDragPoint; }

   // Single point manipulation, used by the Win32 editor (moves only this point, not the parent part)
   void Translate(const Vertex2D &offset);
   Vertex2D GetCenter() const;

   IEditable *GetIEditable();
   const IEditable *GetIEditable() const;

   void Copy();
   void Paste();

   void ToggleSmooth();
   void ToggleSlingshot();

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

   // Can't allow less points than the user can recover from
   bool CanDelete() const;

   void Delete();

   bool LoadToken(const int id, IObjectReader& reader);

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

   bool m_uiLocked = false; // Can not be dragged in the editor
   bool m_uiVisible = true; // UI visibility (not the same as rendering visibility which is a member of part data)

private:
   DragPointCurve *m_pcurve;
   static Vertex3Ds m_copyPoint;   // coordinates of a control point to copy
   static bool      m_pointCopied;
};

// A curve made of DragPoint control points
class DragPointCurve final
{
public:
   // 'center' optionally points to a part-owned position (e.g. LightData::m_vCenter) which is used
   // as the element center by the point transforms and updated by them. When null, the element center
   // is derived from the points and PutPointCenter has no effect.
   DragPointCurve(IEditable *owner, Vertex2D *const center = nullptr, const int minPoints = 3)
      : m_owner(owner)
      , m_pCenter(center)
      , m_minPoints(minPoints)
   {
   }

   ~DragPointCurve();

   IEditable *GetIEditable() { return m_owner; }
   const IEditable *GetIEditable() const { return m_owner; }
   PinTable *GetPTable() { return m_owner->GetPTable(); }
   const PinTable *GetPTable() const { return m_owner->GetPTable(); }

   int GetMinimumPoints() const { return m_minPoints; }

   void SavePoints(IObjectWriter& writer) const;
   void LoadPointToken(IObjectReader& reader);

   void ClearPointsForOverwrite();

   Vertex2D GetPointCenter() const;
   void PutPointCenter(const Vertex2D &pv)
   {
      if (m_pCenter != nullptr)
         *m_pCenter = pv;
   }

   void FlipPointY(const Vertex2D& pvCenter);
   void FlipPointX(const Vertex2D& pvCenter);
   void RotatePoints(const float ang, const Vertex2D& pvCenter, const bool useElementCenter);
   void ScalePoints(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter);
   void TranslatePoints(const Vertex2D &offset);
   void ReverseOrder();

   void GetTextureCoords(const vector<RenderVertex> & vv, float **ppcoords) const;

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

private:
   IEditable *const m_owner;
   Vertex2D *const m_pCenter;
   const int m_minPoints;
};
