// license:GPLv3+

#include "core/stdafx.h"
#include "dragpoint.h"

Vertex3Ds DragPoint::m_copyPoint;
bool      DragPoint::m_pointCopied = false;

IHaveDragPoints::~IHaveDragPoints()
{
   for (size_t i = 0; i < m_vdpoint.size(); i++)
      m_vdpoint[i]->Release();
}

Vertex2D IHaveDragPoints::GetPointCenter() const
{
   float minx = FLT_MAX;
   float maxx = -FLT_MAX;
   float miny = FLT_MAX;
   float maxy = -FLT_MAX;

   for (const auto& v : m_vdpoint)
   {
      minx = min(minx, v->m_v.x);
      maxx = max(maxx, v->m_v.x);
      miny = min(miny, v->m_v.y);
      maxy = max(maxy, v->m_v.y);
   }

   return {(maxx + minx)*0.5f, (maxy + miny)*0.5f};
}

void IHaveDragPoints::FlipPointY(const Vertex2D& pvCenter)
{
   STARTUNDOSELECT

   Vertex2D newcenter = GetPointCenter();

   for (const auto& v : m_vdpoint)
   {
      const float deltay = v->m_v.y - pvCenter.y;

      v->m_v.y -= deltay*2.0f;
   }

   const float deltay = newcenter.y - pvCenter.y;
   newcenter.y -= deltay*2.0f;
   PutPointCenter(newcenter);

   ReverseOrder();

   STOPUNDOSELECT
}

void IHaveDragPoints::FlipPointX(const Vertex2D& pvCenter)
{
   STARTUNDOSELECT

   Vertex2D newcenter = GetPointCenter();

   for (const auto& v : m_vdpoint)
   {
      const float deltax = v->m_v.x - pvCenter.x;

      v->m_v.x -= deltax*2.0f;
   }

   const float deltax = newcenter.x - pvCenter.x;
   newcenter.x -= deltax*2.0f;
   PutPointCenter(newcenter);

   ReverseOrder();

   STOPUNDOSELECT
}

void IHaveDragPoints::RotatePoints(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   STARTUNDOSELECT

   Vertex2D newcenter = GetPointCenter();

   const float centerx = useElementCenter ? newcenter.x : pvCenter.x;
   const float centery = useElementCenter ? newcenter.y : pvCenter.y;

   const float sn = sinf(ANGTORAD(ang));
   const float cs = cosf(ANGTORAD(ang));

   for (const auto& v : m_vdpoint)
   {
      const float dx = v->m_v.x - centerx;
      const float dy = v->m_v.y - centery;
      const float dx2 = cs*dx - sn*dy;
      const float dy2 = cs*dy + sn*dx;
      v->m_v.x = centerx + dx2;
      v->m_v.y = centery + dy2;
   }

   // Move object center as well (if rotating around object center, this would have no effect)
   if (!useElementCenter)
   {
      const float dx = newcenter.x - centerx;
      const float dy = newcenter.y - centery;
      const float dx2 = cs*dx - sn*dy;
      const float dy2 = cs*dy + sn*dx;
      newcenter.x = centerx + dx2;
      newcenter.y = centery + dy2;
      PutPointCenter(newcenter);
   }

   STOPUNDOSELECT
}

void IHaveDragPoints::ScalePoints(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   STARTUNDOSELECT

   Vertex2D newcenter = GetPointCenter();

   const float centerx = useElementCenter ? newcenter.x : pvCenter.x;
   const float centery = useElementCenter ? newcenter.y : pvCenter.y;

   for (const auto& v : m_vdpoint)
   {
      const float dx = (v->m_v.x - centerx) * scalex;
      const float dy = (v->m_v.y - centery) * scaley;
      v->m_v.x = centerx + dx;
      v->m_v.y = centery + dy;
   }

   // Move object center as well (if scaling from object center, this would have no effect)
   if (!useElementCenter)
   {
      const float dx = (newcenter.x - centerx) * scalex;
      const float dy = (newcenter.y - centery) * scaley;
      newcenter.x = centerx + dx;
      newcenter.y = centery + dy;
      PutPointCenter(newcenter);
   }

   STOPUNDOSELECT
}

void IHaveDragPoints::TranslatePoints(const Vertex2D &pvOffset)
{
   STARTUNDOSELECT

   for (const auto& v : m_vdpoint)
   {
      v->m_v.x += pvOffset.x;
      v->m_v.y += pvOffset.y;
   }

   PutPointCenter(GetPointCenter());

   STOPUNDOSELECT
}

void IHaveDragPoints::ReverseOrder()
{
   if (m_vdpoint.empty())
      return;

   // Reverse order of points (switches winding, reverses inside/outside)
   std::ranges::reverse(m_vdpoint.begin(), m_vdpoint.end());

   const bool slingshotTemp = m_vdpoint[0]->m_slingshot;

   for (size_t i = 0; i < m_vdpoint.size() - 1; i++)
   {
      DragPoint * const pdp1 = m_vdpoint[i];
      const DragPoint * const pdp2 = m_vdpoint[i + 1];

      pdp1->m_slingshot = pdp2->m_slingshot;
   }

   m_vdpoint[m_vdpoint.size() - 1]->m_slingshot = slingshotTemp;
}

// Ported at: VisualPinball.Engine/Math/DragPoint.cs

void IHaveDragPoints::GetTextureCoords(const vector<RenderVertex> & vv, float **ppcoords) const
{
   vector<int> vitexpoints;
   vector<int> virenderpoints;
   bool noCoords = false;

   const int cpoints = (int)vv.size();
   int icontrolpoint = 0;

   *ppcoords = new float[cpoints];
   memset(*ppcoords, 0, sizeof(float)*cpoints);

   for (int i = 0; i < cpoints; ++i)
   {
      const RenderVertex * const prv = &vv[i];
      if (prv->controlPoint)
      {
         if (!m_vdpoint[icontrolpoint]->m_autoTexture)
         {
            vitexpoints.push_back(icontrolpoint);
            virenderpoints.push_back(i);
         }
         ++icontrolpoint;
      }
   }

   if (vitexpoints.empty())
   {
      // Special case - no texture coordinates were specified
      // Make them up starting at point 0
      vitexpoints.push_back(0);
      virenderpoints.push_back(0);

      noCoords = true;
   }

   // Wrap the array around so we cover the last section
   vitexpoints.push_back(vitexpoints[0] + (int)m_vdpoint.size());
   virenderpoints.push_back(virenderpoints[0] + cpoints);

   for (int i = 0; i < (int)vitexpoints.size() - 1; ++i)
   {
      const int startrenderpoint = virenderpoints[i] % cpoints;
      int endrenderpoint = virenderpoints[(i < cpoints - 1) ? (i + 1) : 0] % cpoints;

      float starttexcoord;
      float endtexcoord;
      if (noCoords)
      {
         starttexcoord = 0.0f;
         endtexcoord = 1.0f;
      }
      else
      {
         starttexcoord = m_vdpoint[vitexpoints[i] % m_vdpoint.size()]->m_texturecoord;
         endtexcoord = m_vdpoint[vitexpoints[i + 1] % m_vdpoint.size()]->m_texturecoord;
      }

      const float deltacoord = endtexcoord - starttexcoord;

      if (endrenderpoint <= startrenderpoint)
         endrenderpoint += cpoints;

      float totallength = 0.0f;
      for (int l = startrenderpoint; l < endrenderpoint; ++l)
      {
         const Vertex2D * const pv1 = &vv[l % cpoints];
         const Vertex2D * const pv2 = &vv[(l + 1) % cpoints];

         const float dx = pv1->x - pv2->x;
         const float dy = pv1->y - pv2->y;
         const float length = sqrtf(dx*dx + dy*dy);

         totallength += length;
      }

      float partiallength = 0.0f;
      for (int l = startrenderpoint; l < endrenderpoint; ++l)
      {
         const Vertex2D * const pv1 = &vv[l % cpoints];
         const Vertex2D * const pv2 = &vv[(l + 1) % cpoints];

         const float dx = pv1->x - pv2->x;
         const float dy = pv1->y - pv2->y;
         const float length = sqrtf(dx*dx + dy*dy);
         if (totallength == 0.0f)
            totallength = 1.0f;
         const float texcoord = partiallength / totallength;

         (*ppcoords)[l % cpoints] = (texcoord * deltacoord) + starttexcoord;

         partiallength += length;
      }
   }
}

void IHaveDragPoints::ClearPointsForOverwrite()
{
   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      if (m_vdpoint[i]->m_selectstate != ISelect::SelectState::NotSelected /*GetPTable()->m_pselcur == m_vdpoint[i]*/)
      {
         //GetPTable()->SetSel(GetPTable());
         GetPTable()->AddMultiSel(GetPTable(), false, true, false);
      }

      m_vdpoint[i]->Release();
   }

   m_vdpoint.clear();
}

void IHaveDragPoints::SavePoints(IObjectWriter &writer) const
{
   for (const auto pdp : m_vdpoint)
   {
      writer.BeginObject(FID(DPNT), true, false);
      writer.WriteVector2(FID(VCEN), Vertex2D(pdp->m_v.x, pdp->m_v.y));
      writer.WriteFloat(FID(POSZ), pdp->m_v.z);
      writer.WriteBool(FID(SMTH), pdp->m_smooth);
      writer.WriteBool(FID(SLNG), pdp->m_slingshot);
      writer.WriteBool(FID(ATEX), pdp->m_autoTexture);
      writer.WriteFloat(FID(TEXC), pdp->m_texturecoord);
      writer.WriteBool(FID(LOCK), pdp->m_uiLocked);
      writer.WriteBool(FID(LVIS), pdp->m_uiVisible);
      writer.EndObject();
   }
}

void IHaveDragPoints::LoadPointToken(IObjectReader& reader)
{
   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp == nullptr)
      return;

   pdp->AddRef();
   pdp->Init(this, 0.f, 0.f, 0.f, false);
   reader.AsObject(
      [pdp](int tag, IObjectReader &reader)
      {
         switch (tag)
         {
         case FID(VCEN):
         {
            auto v = reader.AsVector2();
            pdp->m_v.x = v.x;
            pdp->m_v.y = v.y; 
            break;
         }
         case FID(POSZ): pdp->m_v.z = reader.AsFloat(); break;
         case FID(SMTH): pdp->m_smooth = reader.AsBool(); break;
         case FID(SLNG): pdp->m_slingshot = reader.AsBool(); break;
         case FID(ATEX): pdp->m_autoTexture = reader.AsBool(); break;
         case FID(TEXC): pdp->m_texturecoord = reader.AsFloat(); break;
         case FID(LOCK): pdp->m_uiLocked = reader.AsBool(); break;
         case FID(LVIS): pdp->m_uiVisible = reader.AsBool(); break;
         // Old save would wrongly save these fields which do not apply to dragpoint
         case FID(LAYR): reader.AsInt(); break;
         case FID(LANR): reader.AsString(); break;
         }
         return true;
      });
   m_vdpoint.push_back(pdp);
}

void DragPoint::Init(IHaveDragPoints *pihdp, const float x, const float y, const float z, const bool smooth)
{
   m_pihdp = pihdp;
   m_smooth = smooth;

   m_slingshot = false;
   m_v.x = x;
   m_v.y = y;
   m_v.z = z;
   m_calcHeight = 0.0f;
   m_autoTexture = true;
   m_texturecoord = 0.0f;

   m_menuid = (pihdp->GetIEditable()->GetItemType() == eItemRubber) ? IDR_POINTMENU_SMOOTH : IDR_POINTMENU;
}

IEditable *DragPoint::GetIEditable()
{
   return M_PIHDP->GetIEditable();
}

const IEditable *DragPoint::GetIEditable() const
{
   return M_PIHDP->GetIEditable();
}

void DragPoint::OnLButtonDown(int x, int y)
{
   ISelect::OnLButtonDown(x, y);
   GetPTable()->SetDirtyDraw();
}

void DragPoint::OnLButtonUp(int x, int y)
{
   ISelect::OnLButtonUp(x, y);
   GetPTable()->SetDirtyDraw();
}

void DragPoint::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_v.x, m_v.y);
}

void DragPoint::MoveOffset(const float dx, const float dy)
{
   m_v.x += dx;
   m_v.y += dy;
}

Vertex2D DragPoint::GetCenter() const
{
   return {m_v.x, m_v.y};
}

void DragPoint::PutCenter(const Vertex2D& pv)
{
   m_v.x = pv.x;
   m_v.y = pv.y;
}

void DragPoint::Delete()
{
   if ((int)M_PIHDP->m_vdpoint.size() > M_PIHDP->GetMinimumPoints()) // Can't allow less points than the user can recover from
   {
      STARTUNDOSELECT
      RemoveFromVectorSingle(M_PIHDP->m_vdpoint, (CComObject<DragPoint> *)this);
      STOPUNDOSELECT
      Release();
   }
}

void DragPoint::Uncreate()
{
   RemoveFromVectorSingle(M_PIHDP->m_vdpoint, (CComObject<DragPoint> *)this);
   Release();
}

#ifndef __STANDALONE__
void DragPoint::EditMenu(CMenu &menu)
{
   menu.CheckMenuItem(ID_POINTMENU_SMOOTH, MF_BYCOMMAND | (m_smooth ? MF_CHECKED : MF_UNCHECKED));
   //EnableMenuItem(hmenu, ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | (m_fSmooth ? MF_GRAYED : MF_ENABLED));
   menu.CheckMenuItem(ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | ((m_slingshot && !m_smooth) ? MF_CHECKED : MF_UNCHECKED));
}

void DragPoint::DoCommand(int icmd, int x, int y)
{
   ISelect::DoCommand(icmd, x, y);
   switch (icmd)
   {
   case ID_POINTMENU_SMOOTH:
   {
      STARTUNDOSELECT
      m_smooth = !m_smooth;
      const int index2 = (FindIndexOf(M_PIHDP->m_vdpoint, (CComObject<DragPoint> *)this) - 1 + (int)M_PIHDP->m_vdpoint.size()) % (int)M_PIHDP->m_vdpoint.size();
      if (m_smooth && m_slingshot)
      {
         m_slingshot = false;
      }
      if (m_smooth && M_PIHDP->m_vdpoint[index2]->m_slingshot)
      {
         M_PIHDP->m_vdpoint[index2]->m_slingshot = false;
      }
      STOPUNDOSELECT
      break;
   }
   case ID_POINTMENU_SLINGSHOT:
   {
      STARTUNDOSELECT
      m_slingshot = !m_slingshot;
      if (m_slingshot)
      {
         m_smooth = false;
         const int index2 = (FindIndexOf(M_PIHDP->m_vdpoint, (CComObject<DragPoint> *)this) + 1) % M_PIHDP->m_vdpoint.size();
         M_PIHDP->m_vdpoint[index2]->m_smooth = false;
      }
      STOPUNDOSELECT
      break;
   }
   }
}
#endif

void DragPoint::SetSelectFormat(Sur *psur)
{
   psur->SetFillColor(RGB(150, 200, 255));
}

void DragPoint::SetMultiSelectFormat(Sur *psur)
{
   psur->SetFillColor(RGB(200, 225, 255));
}

STDMETHODIMP DragPoint::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IControlPoint,
   };

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

   return S_FALSE;
}

void DragPoint::Copy()
{
    m_copyPoint = m_v;
    m_pointCopied = true;
}

void DragPoint::Paste()
{
    if (m_pointCopied)
        m_v = m_copyPoint;
}

STDMETHODIMP DragPoint::get_X(float *pVal)
{
   *pVal = m_v.x;
   return S_OK;
}

STDMETHODIMP DragPoint::put_X(float newVal)
{
   STARTUNDOSELECT
   m_v.x = newVal;
   STOPUNDOSELECT

   return S_OK;
}

STDMETHODIMP DragPoint::get_Y(float *pVal)
{
   *pVal = m_v.y;
   return S_OK;
}

STDMETHODIMP DragPoint::put_Y(float newVal)
{
   STARTUNDOSELECT
   m_v.y = newVal;
   STOPUNDOSELECT

   return S_OK;
}

STDMETHODIMP DragPoint::get_Z(float *pVal)
{
   *pVal = m_v.z;
   return S_OK;
}

STDMETHODIMP DragPoint::put_Z(float newVal)
{
   STARTUNDOSELECT
   m_v.z = newVal;
   STOPUNDOSELECT

   return S_OK;
}

STDMETHODIMP DragPoint::get_CalcHeight(float *pVal)
{
   *pVal = m_calcHeight;
   return S_OK;
}

STDMETHODIMP DragPoint::get_Smooth(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_smooth);
   return S_OK;
}

STDMETHODIMP DragPoint::put_Smooth(VARIANT_BOOL newVal)
{
   STARTUNDOSELECT
   m_smooth = VBTOb(newVal);
   STOPUNDOSELECT

   return S_OK;
}

STDMETHODIMP DragPoint::get_IsAutoTextureCoordinate(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_autoTexture);
   return S_OK;
}

STDMETHODIMP DragPoint::put_IsAutoTextureCoordinate(VARIANT_BOOL newVal)
{
   STARTUNDOSELECT
   m_autoTexture = VBTOb(newVal);
   STOPUNDOSELECT

   return S_OK;
}

STDMETHODIMP DragPoint::get_TextureCoordinateU(float *pVal)
{
   *pVal = m_texturecoord;
   return S_OK;
}

STDMETHODIMP DragPoint::put_TextureCoordinateU(float newVal)
{
   STARTUNDOSELECT
   m_texturecoord = newVal;
   STOPUNDOSELECT

   return S_OK;
}
