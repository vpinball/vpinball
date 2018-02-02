#include "stdafx.h"

Vertex3Ds DragPoint::m_copyPoint;
bool      DragPoint::m_pointCopied=false;

IHaveDragPoints::IHaveDragPoints()
{
    m_propVisuals = NULL;
    m_propPosition = NULL;
}

IHaveDragPoints::~IHaveDragPoints()
{
   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      m_vdpoint.ElementAt(i)->Release();
   }
}

void IHaveDragPoints::GetPointCenter(Vertex2D * const pv) const
{
   float minx = FLT_MAX;
   float maxx = -FLT_MAX;
   float miny = FLT_MAX;
   float maxy = -FLT_MAX;

   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      const Vertex3Ds& v = m_vdpoint.ElementAt(i)->m_v;
      minx = min(minx, v.x);
      maxx = max(maxx, v.x);
      miny = min(miny, v.y);
      maxy = max(maxy, v.y);
   }

   pv->x = (maxx + minx)*0.5f;
   pv->y = (maxy + miny)*0.5f;
}

void IHaveDragPoints::PutPointCenter(const Vertex2D * const pv)
{
}

void IHaveDragPoints::FlipPointY(Vertex2D *pvCenter)
{
   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   const float ycenter = pvCenter->y;

   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      const float deltay = m_vdpoint.ElementAt(i)->m_v.y - ycenter;

      m_vdpoint.ElementAt(i)->m_v.y -= deltay*2.0f;
   }

   const float deltay = newcenter.y - ycenter;
   newcenter.y -= deltay*2.0f;
   PutPointCenter(&newcenter);

   ReverseOrder();

   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::FlipPointX(Vertex2D *pvCenter)
{
   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   const float xcenter = pvCenter->x;

   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      const float deltax = m_vdpoint.ElementAt(i)->m_v.x - xcenter;

      m_vdpoint.ElementAt(i)->m_v.x -= deltax*2.0f;
   }

   const float deltax = newcenter.x - xcenter;
   newcenter.x -= deltax*2.0f;
   PutPointCenter(&newcenter);

   ReverseOrder();

   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::RotateDialog()
{
   DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_ROTATE),
      g_pvp->m_hwnd, RotateProc, (size_t)this->GetIEditable()->GetISelect());//(long)this);
}

void IHaveDragPoints::ScaleDialog()
{
   DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_SCALE),
      g_pvp->m_hwnd, ScaleProc, (size_t)this->GetIEditable()->GetISelect());
}

void IHaveDragPoints::TranslateDialog()
{
   DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_TRANSLATE),
      g_pvp->m_hwnd, TranslateProc, (size_t)this->GetIEditable()->GetISelect());
}

void IHaveDragPoints::RotatePoints(float ang, Vertex2D *pvCenter, const bool useElementCenter)
{
   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   if (useElementCenter)
   {
      /* Don't use the pvCenter anymore! pvCenter is the mouse position when rotating is activated.
      Because the mouse position (rotation center) isn't shown in the editor use the element's center returned by GetPointCenter() */
      const float centerx = newcenter.x;
      const float centery = newcenter.y;

      const float sn = sinf(ANGTORAD(ang));
      const float cs = cosf(ANGTORAD(ang));

      for (int i = 0; i < m_vdpoint.Size(); i++)
      {
         DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
         const float dx = pdp1->m_v.x - centerx;
         const float dy = pdp1->m_v.y - centery;
         const float dx2 = cs*dx - sn*dy;
         const float dy2 = cs*dy + sn*dx;
         pdp1->m_v.x = centerx + dx2;
         pdp1->m_v.y = centery + dy2;
      }
   }
   else
   {
      const float centerx = pvCenter->x;
      const float centery = pvCenter->y;

      const float sn = sinf(ANGTORAD(ang));
      const float cs = cosf(ANGTORAD(ang));

      for (int i = 0; i < m_vdpoint.Size(); i++)
      {
         DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
         const float dx = pdp1->m_v.x - centerx;
         const float dy = pdp1->m_v.y - centery;
         const float dx2 = cs*dx - sn*dy;
         const float dy2 = cs*dy + sn*dx;
         pdp1->m_v.x = centerx + dx2;
         pdp1->m_v.y = centery + dy2;
      }

      // Move object center as well (if rotating around object center,
      // this will have no effect)
      {
         const float dx = newcenter.x - centerx;
         const float dy = newcenter.y - centery;
         const float dx2 = cs*dx - sn*dy;
         const float dy2 = cs*dy + sn*dx;
         newcenter.x = centerx + dx2;
         newcenter.y = centery + dy2;
         PutPointCenter(&newcenter);
      }
   }
   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::ScalePoints(float scalex, float scaley, Vertex2D *pvCenter, const bool useElementsCenter)
{
   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   if (useElementsCenter)
   {
      /* Don't use the pvCenter anymore! pvCenter is the mouse position when scaling is activated.
      Because the mouse position (scaling center) isn't shown in the editor use the element's center returned by GetPointCenter() */
      const float centerx = newcenter.x;
      const float centery = newcenter.y;

      for (int i = 0; i < m_vdpoint.Size(); i++)
      {
         DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
         const float dx = (pdp1->m_v.x - centerx) * scalex;
         const float dy = (pdp1->m_v.y - centery) * scaley;
         pdp1->m_v.x = centerx + dx;
         pdp1->m_v.y = centery + dy;
      }
   }
   else
   {
      const float centerx = pvCenter->x;
      const float centery = pvCenter->y;

      for (int i = 0; i < m_vdpoint.Size(); i++)
      {
         DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
         const float dx = (pdp1->m_v.x - centerx) * scalex;
         const float dy = (pdp1->m_v.y - centery) * scaley;
         pdp1->m_v.x = centerx + dx;
         pdp1->m_v.y = centery + dy;
      }

      // Move object center as well (if scaling from object center,
      // this will have no effect)
      {
         const float dx = (newcenter.x - centerx) * scalex;
         const float dy = (newcenter.y - centery) * scaley;
         newcenter.x = centerx + dx;
         newcenter.y = centery + dy;
         PutPointCenter(&newcenter);
      }
   }

   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::TranslatePoints(Vertex2D *pvOffset)
{
   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
      pdp1->m_v.x += pvOffset->x;
      pdp1->m_v.y += pvOffset->y;
   }

   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   newcenter.x += pvOffset->x;
   newcenter.y += pvOffset->y;

   PutPointCenter(&newcenter);

   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::ReverseOrder()
{
   if (m_vdpoint.Size() == 0)
   {
      return;
   }

   // Reverse order of points (switches winding, reverses inside/outside)
   for (int i = 0; i < m_vdpoint.Size() / 2; i++)
   {
      DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
      DragPoint * const pdp2 = m_vdpoint.ElementAt(m_vdpoint.Size() - 1 - i);
      m_vdpoint.ReplaceElementAt(pdp2, i);
      m_vdpoint.ReplaceElementAt(pdp1, m_vdpoint.Size() - 1 - i);
   }

   const bool fSlingshotTemp = m_vdpoint.ElementAt(0)->m_fSlingshot;

   for (int i = 0; i < m_vdpoint.Size() - 1; i++)
   {
      DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
      const DragPoint * const pdp2 = m_vdpoint.ElementAt(i + 1);

      pdp1->m_fSlingshot = pdp2->m_fSlingshot;
   }

   m_vdpoint.ElementAt(m_vdpoint.Size() - 1)->m_fSlingshot = fSlingshotTemp;
}

void IHaveDragPoints::GetPointDialogPanes(Vector<PropertyPane> *pvproppane)
{
   m_propVisuals = new PropertyPane(IDD_PROPPOINT_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(m_propVisuals);

   m_propPosition = new PropertyPane(IDD_PROPPOINT_POSITION, IDS_POSITION);
   pvproppane->AddElement(m_propPosition);
}

void IHaveDragPoints::GetTextureCoords(const std::vector<RenderVertex> & vv, float **ppcoords)
{
   std::vector<int> vitexpoints;
   std::vector<int> virenderpoints;
   bool m_fNoCoords = false;

   const int cpoints = (int)vv.size();
   int icontrolpoint = 0;

   *ppcoords = new float[cpoints];
   memset(*ppcoords, 0, sizeof(float)*cpoints);

   for (int i = 0; i < cpoints; ++i)
   {
      const RenderVertex * const prv = &vv[i];
      if (prv->fControlPoint)
      {
         if (!m_vdpoint.ElementAt(icontrolpoint)->m_fAutoTexture)
         {
            vitexpoints.push_back(icontrolpoint);
            virenderpoints.push_back(i);
         }
         ++icontrolpoint;
      }
   }

   if (vitexpoints.size() == 0)
   {
      // Special case - no texture coordinates were specified
      // Make them up starting at point 0
      vitexpoints.push_back(0);
      virenderpoints.push_back(0);

      m_fNoCoords = true;
   }

   // Wrap the array around so we cover the last section
   vitexpoints.push_back(vitexpoints[0] + m_vdpoint.size());
   virenderpoints.push_back(virenderpoints[0] + cpoints);

   for (int i = 0; i < (int)vitexpoints.size() - 1; ++i)
   {
      const int startrenderpoint = virenderpoints[i] % cpoints;
      int endrenderpoint = virenderpoints[(i < cpoints - 1) ? (i + 1) : 0] % cpoints;

      float starttexcoord;
      float endtexcoord;
      if (m_fNoCoords)
      {
         starttexcoord = 0.0f;
         endtexcoord = 1.0f;
      }
      else
      {
         starttexcoord = m_vdpoint.ElementAt(vitexpoints[i] % m_vdpoint.Size())->m_texturecoord;
         endtexcoord = m_vdpoint.ElementAt(vitexpoints[i + 1] % m_vdpoint.Size())->m_texturecoord;
      }

      const float deltacoord = endtexcoord - starttexcoord;

      if (endrenderpoint <= startrenderpoint)
      {
         endrenderpoint += cpoints;
      }

      float totallength = 0.0f;
      for (int l = startrenderpoint; l < endrenderpoint; ++l)
      {
         const Vertex2D * const pv1 = (Vertex2D *)&vv[l % cpoints];
         const Vertex2D * const pv2 = (Vertex2D *)&vv[(l + 1) % cpoints];

         const float dx = pv1->x - pv2->x;
         const float dy = pv1->y - pv2->y;
         const float length = sqrtf(dx*dx + dy*dy);

         totallength += length;
      }

      float partiallength = 0.0f;
      for (int l = startrenderpoint; l < endrenderpoint; ++l)
      {
         const Vertex2D * const pv1 = (Vertex2D *)&vv[l % cpoints];
         const Vertex2D * const pv2 = (Vertex2D *)&vv[(l + 1) % cpoints];

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
   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      if (m_vdpoint.ElementAt(i)->m_selectstate != eNotSelected/*GetPTable()->m_pselcur == m_vdpoint.ElementAt(i)*/)
      {
         //GetPTable()->SetSel(GetPTable());
         GetPTable()->AddMultiSel(GetPTable(), fFalse, fTrue);
      }

      m_vdpoint.ElementAt(i)->Release();
   }

   m_vdpoint.RemoveAllElements();
}

HRESULT IHaveDragPoints::SavePointData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      bw.WriteTag(FID(DPNT));
      CComObject<DragPoint> *pdp = m_vdpoint.ElementAt(i);
      bw.WriteStruct(FID(VCEN), &(pdp->m_v), sizeof(Vertex2D));
      bw.WriteFloat(FID(POSZ), pdp->m_v.z);
      bw.WriteBool(FID(SMTH), pdp->m_fSmooth);
      bw.WriteBool(FID(SLNG), pdp->m_fSlingshot);
      bw.WriteBool(FID(ATEX), pdp->m_fAutoTexture);
      bw.WriteFloat(FID(TEXC), pdp->m_texturecoord);

      ((ISelect *)pdp)->SaveData(pstm, hcrypthash, hcryptkey);

      bw.WriteTag(FID(ENDB));
   }

   return S_OK;
}


void IHaveDragPoints::LoadPointToken(int id, BiffReader *pbr, int version)
{
   if (id == FID(DPNT))
   {
      CComObject<DragPoint> *pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, 0, 0);
         m_vdpoint.AddElement(pdp);
         BiffReader br(pbr->m_pistream, pdp, NULL, version, pbr->m_hcrypthash, pbr->m_hcryptkey);
         br.Load();
      }
   }
}

void DragPoint::Init(IHaveDragPoints *pihdp, const float x, const float y, const float z)
{
   m_pihdp = pihdp;
   m_fSmooth = false;

   m_fSlingshot = false;
   m_v.x = x;
   m_v.y = y;
   m_v.z = z;
   m_calcHeight = 0.0f;
   m_fAutoTexture = true;
   m_texturecoord = 0.0f;
   if (pihdp->GetIEditable()->GetItemType() == eItemRubber)
      m_menuid = IDR_POINTMENU_SMOOTH;
   else
      m_menuid = IDR_POINTMENU;
}

void DragPoint::OnLButtonDown(int x, int y)
{
   ISelect::OnLButtonDown(x, y);
   m_pihdp->GetIEditable()->SetDirtyDraw();
}

void DragPoint::OnLButtonUp(int x, int y)
{
   ISelect::OnLButtonUp(x, y);
   m_pihdp->GetIEditable()->SetDirtyDraw();
}

void DragPoint::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_v.x, m_v.y);
}

void DragPoint::MoveOffset(const float dx, const float dy)
{
   m_v.x += dx;
   m_v.y += dy;

   m_pihdp->GetIEditable()->SetDirtyDraw();
}

void DragPoint::GetCenter(Vertex2D * const pv) const
{
   pv->x = m_v.x;
   pv->y = m_v.y;
}

void DragPoint::PutCenter(const Vertex2D * const pv)
{
   m_v.x = pv->x;
   m_v.y = pv->y;

   m_pihdp->GetIEditable()->SetDirtyDraw();
}

void DragPoint::EditMenu(HMENU hmenu)
{
   CheckMenuItem(hmenu, ID_POINTMENU_SMOOTH, MF_BYCOMMAND | (m_fSmooth ? MF_CHECKED : MF_UNCHECKED));
   //EnableMenuItem(hmenu, ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | (m_fSmooth ? MF_GRAYED : MF_ENABLED));
   CheckMenuItem(hmenu, ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | ((m_fSlingshot && !m_fSmooth) ? MF_CHECKED : MF_UNCHECKED));
}

void DragPoint::Delete()
{
   if (m_pihdp->m_vdpoint.Size() > m_pihdp->GetMinimumPoints()) // Can't allow less points than the user can recover from
   {
      m_pihdp->GetIEditable()->BeginUndo();
      m_pihdp->GetIEditable()->MarkForUndo();
      m_pihdp->m_vdpoint.RemoveElement(this);
      m_pihdp->GetIEditable()->EndUndo();
      m_pihdp->GetIEditable()->SetDirtyDraw();
      Release();
   }
}

void DragPoint::Uncreate()
{
   m_pihdp->m_vdpoint.RemoveElement(this);
   Release();
}

void DragPoint::DoCommand(int icmd, int x, int y)
{
   int index2;
   ISelect::DoCommand(icmd, x, y);
   switch (icmd)
   {
   case ID_POINTMENU_SMOOTH:
   {
      IEditable *pedit = m_pihdp->GetIEditable();
      pedit->BeginUndo();
      pedit->MarkForUndo();

      m_fSmooth = !m_fSmooth;
      index2 = (m_pihdp->m_vdpoint.IndexOf(this) - 1 + m_pihdp->m_vdpoint.Size()) % m_pihdp->m_vdpoint.Size();
      if (m_fSmooth && m_fSlingshot)
      {
         m_fSlingshot = false;
      }
      if (m_fSmooth && m_pihdp->m_vdpoint.ElementAt(index2)->m_fSlingshot)
      {
         m_pihdp->m_vdpoint.ElementAt(index2)->m_fSlingshot = false;
      }

      pedit->EndUndo();
      pedit->SetDirtyDraw();
      break;
   }
   case ID_POINTMENU_SLINGSHOT:
   {
      IEditable *pedit = m_pihdp->GetIEditable();
      pedit->BeginUndo();
      pedit->MarkForUndo();

      m_fSlingshot = !m_fSlingshot;
      if (m_fSlingshot)
      {
         m_fSmooth = false;
         index2 = (m_pihdp->m_vdpoint.IndexOf(this) + 1) % m_pihdp->m_vdpoint.Size();
         m_pihdp->m_vdpoint.ElementAt(index2)->m_fSmooth = false;
      }

      pedit->EndUndo();
      pedit->SetDirtyDraw();
      break;
   }
   }
}

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

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

BOOL DragPoint::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_v, sizeof(Vertex2D));
   }
   else if (id == FID(POSZ))
   {
      pbr->GetFloat(&m_v.z);
   }
   else if (id == FID(SMTH))
   {
      pbr->GetBool(&m_fSmooth);
   }
   else if (id == FID(SLNG))
   {
      pbr->GetBool(&m_fSlingshot);
   }
   else if (id == FID(ATEX))
   {
      pbr->GetBool(&m_fAutoTexture);
   }
   else if (id == FID(TEXC))
   {
      pbr->GetFloat(&m_texturecoord);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

IDispatch *DragPoint::GetDispatch()
{
   return (IDispatch *)this;
}

void DragPoint::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   m_pihdp->GetPointDialogPanes(pvproppane);
}

void DragPoint::Copy()
{
    m_copyPoint = m_v;
    m_pointCopied = true;
}

void DragPoint::Paste()
{
    if(m_pointCopied)
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
   *pVal = (VARIANT_BOOL)FTOVB(m_fSmooth);

   return S_OK;
}

STDMETHODIMP DragPoint::put_Smooth(VARIANT_BOOL newVal)
{
   STARTUNDOSELECT

      m_fSmooth = VBTOF(newVal);

   STOPUNDOSELECT

      return S_OK;
}

STDMETHODIMP DragPoint::get_IsAutoTextureCoordinate(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fAutoTexture);

   return S_OK;
}

STDMETHODIMP DragPoint::put_IsAutoTextureCoordinate(VARIANT_BOOL newVal)
{
   STARTUNDOSELECT

      m_fAutoTexture = VBTOF(newVal);

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

int rotateApplyCount = 0;
INT_PTR CALLBACK RotateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ISelect *psel;
   bool useElementCenter = false;

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      rotateApplyCount = 0;
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      const float angle = psel->GetRotate();

      SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_SETCHECK, BST_CHECKED, 0);

      char szT[256];
      f2sz(angle, szT);
      SetDlgItemText(hwndDlg, IDC_ROTATEBY, szT);
      Vertex2D v;
      psel->GetCenter(&v);
      f2sz(v.x, szT);
      SetDlgItemText(hwndDlg, IDC_CENTERX, szT);
      f2sz(v.y, szT);
      SetDlgItemText(hwndDlg, IDC_CENTERY, szT);
   }
   return TRUE;
   break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      switch (LOWORD(wParam))
      {
      case IDC_CHECK_ROTATE_CENTER:
      {
         switch (HIWORD(wParam))
         {
            case BN_CLICKED:
            {
               char szT[256];
               if (!(SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED))
               {
                  f2sz(g_pvp->m_mouseCursorPosition.x, szT);
                  SetDlgItemText(hwndDlg, IDC_CENTERX, szT);
                  f2sz(g_pvp->m_mouseCursorPosition.y, szT);
                  SetDlgItemText(hwndDlg, IDC_CENTERY, szT);
               }
               else
               {
                  Vertex2D v;
                  psel->GetCenter(&v);
                  f2sz(v.x, szT);
                  SetDlgItemText(hwndDlg, IDC_CENTERX, szT);
                  f2sz(v.y, szT);
                  SetDlgItemText(hwndDlg, IDC_CENTERY, szT);
               }
               break;
            }
               default:
               break;
          }
      }
      default:
         break;
      }


      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
         {
            if (rotateApplyCount == 0)
            {
               char szT[256];
               GetDlgItemText(hwndDlg, IDC_ROTATEBY, szT, 255);
               const float f = sz2f(szT);

               useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);
               GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
               Vertex2D v;
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
               v.y = sz2f(szT);

               psel->Rotate(f, &v, useElementCenter);
            }
            EndDialog(hwndDlg, TRUE);
            break;
         }
         case IDC_ROTATE_APPLY_BUTTON:
         {
            rotateApplyCount++;
            char szT[256];
            GetDlgItemText(hwndDlg, IDC_ROTATEBY, szT, 255);
            const float f = sz2f(szT);

            useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);
            GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
            Vertex2D v;
            v.x = sz2f(szT);
            GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
            v.y = sz2f(szT);

            psel->Rotate(f, &v, useElementCenter);
            psel->GetPTable()->SetDirtyDraw();
            break;
         }
         case IDC_ROTATE_UNDO_BUTTON:
         {
            if (rotateApplyCount > 0)
            {
               rotateApplyCount--;
               psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            break;
         }
         case IDCANCEL:
            if (rotateApplyCount > 0)
            {
               for (int i = 0; i < rotateApplyCount; i++) psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            EndDialog(hwndDlg, FALSE);
            break;
         }
         break;
      }
      break;
   }

   return FALSE;
}

int scaleApplyCount = 0;
INT_PTR CALLBACK ScaleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ISelect *psel;
   bool useElementCenter = false;

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      scaleApplyCount = 0;
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

      Vertex2D v;
      psel->GetScale(&v.x, &v.y);

      char szT[256];
      f2sz(v.x, szT);
      SetDlgItemText(hwndDlg, IDC_SCALEFACTOR, szT);
      f2sz(v.y, szT);
      SetDlgItemText(hwndDlg, IDC_SCALEY, szT);
      psel->GetCenter(&v);

      SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_SETCHECK, BST_CHECKED, 0);

      f2sz(v.x, szT);
      SetDlgItemText(hwndDlg, IDC_CENTERX, szT);
      f2sz(v.y, szT);
      SetDlgItemText(hwndDlg, IDC_CENTERY, szT);

      SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_SETCHECK, TRUE, 0);

      const HWND hwndEdit = GetDlgItem(hwndDlg, IDC_SCALEY);
      const HWND hwndText = GetDlgItem(hwndDlg, IDC_STATIC_SCALEY);
      EnableWindow(hwndEdit, FALSE);
      EnableWindow(hwndText, FALSE);
   }
   return TRUE;
   break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      switch (LOWORD(wParam))
      {
         case IDC_CHECK_SCALE_CENTER:
         {
            switch (HIWORD(wParam))
            {
            case BN_CLICKED:
            {
               char szT[256];
               if (!(SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED))
               {
                  f2sz(g_pvp->m_mouseCursorPosition.x, szT);
                  SetDlgItemText(hwndDlg, IDC_CENTERX, szT);
                  f2sz(g_pvp->m_mouseCursorPosition.y, szT);
                  SetDlgItemText(hwndDlg, IDC_CENTERY, szT);
               }
               else
               {
                  Vertex2D v;
                  psel->GetCenter(&v);
                  f2sz(v.x, szT);
                  SetDlgItemText(hwndDlg, IDC_CENTERX, szT);
                  f2sz(v.y, szT);
                  SetDlgItemText(hwndDlg, IDC_CENTERY, szT);
               }
               break;
            }
            default:
               break;
            }
         }
      default:
         break;
      }

      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
         {
            if (scaleApplyCount == 0)
            {
               char szT[256];
               GetDlgItemText(hwndDlg, IDC_SCALEFACTOR, szT, 255);
               const float fx = sz2f(szT);
               const size_t checked = SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_GETCHECK, 0, 0);
               float fy;
               if (checked)
               {
                  fy = fx;
               }
               else
               {
                  GetDlgItemText(hwndDlg, IDC_SCALEY, szT, 255);
                  fy = sz2f(szT);
               }

               GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
               Vertex2D v;
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
               v.y = sz2f(szT);

               useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);
               //pihdp->ScalePoints(fx, fy, &v);
               psel->Scale(fx, fy, &v, useElementCenter);
            }
            EndDialog(hwndDlg, TRUE);
            break;
         }
         case IDC_SCALE_APPLY_BUTTON:
         {
            scaleApplyCount++;
            char szT[256];
            GetDlgItemText(hwndDlg, IDC_SCALEFACTOR, szT, 255);
            const float fx = sz2f(szT);
            const size_t checked = SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_GETCHECK, 0, 0);
            float fy;
            if (checked)
            {
               fy = fx;
            }
            else
            {
               GetDlgItemText(hwndDlg, IDC_SCALEY, szT, 255);
               fy = sz2f(szT);
            }

            GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
            Vertex2D v;
            v.x = sz2f(szT);
            GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
            v.y = sz2f(szT);

            useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);

            //pihdp->ScalePoints(fx, fy, &v);
            psel->Scale(fx, fy, &v, useElementCenter);
            psel->GetPTable()->SetDirtyDraw();
            break;
         }
         case IDC_SCALE_UNDO_BUTTON:
         {
            if (scaleApplyCount > 0)
            {
               scaleApplyCount--;
               psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            break;
         }
         case IDCANCEL:
            if (scaleApplyCount > 0)
            {
               for (int i = 0; i < scaleApplyCount; i++) psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            EndDialog(hwndDlg, FALSE);
            break;

         case IDC_SQUARE:
         {
            const size_t checked = SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_GETCHECK, 0, 0);
            const HWND hwndEdit = GetDlgItem(hwndDlg, IDC_SCALEY);
            const HWND hwndText = GetDlgItem(hwndDlg, IDC_STATIC_SCALEY);

            EnableWindow(hwndEdit, !(checked == BST_CHECKED));
            EnableWindow(hwndText, !(checked == BST_CHECKED));
         }
         break;
         }
         break;
      }
      break;
   }

   return FALSE;
}

int translateApplyCount = 0;
INT_PTR CALLBACK TranslateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ISelect *psel;

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      translateApplyCount = 0;
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      char szT[256];
      f2sz(0, szT);
      SetDlgItemText(hwndDlg, IDC_OFFSETX, szT);
      f2sz(0, szT);
      SetDlgItemText(hwndDlg, IDC_OFFSETY, szT);
   }
   return TRUE;
   break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
         {
            if (translateApplyCount == 0)
            {
               char szT[256];
               GetDlgItemText(hwndDlg, IDC_OFFSETX, szT, 255);
               Vertex2D v;
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_OFFSETY, szT, 255);
               v.y = sz2f(szT);

               psel->Translate(&v);
            }
            EndDialog(hwndDlg, TRUE);
            break;
         }
         case IDC_TRANSLATE_APPLY_BUTTON:
         {
            translateApplyCount++;
            char szT[256];
            GetDlgItemText(hwndDlg, IDC_OFFSETX, szT, 255);
            Vertex2D v;
            v.x = sz2f(szT);
            GetDlgItemText(hwndDlg, IDC_OFFSETY, szT, 255);
            v.y = sz2f(szT);

            psel->Translate(&v);
            psel->GetPTable()->SetDirtyDraw();
            break;
         }
         case IDC_TRANSLATE_UNDO_BUTTON:
         {
            if (translateApplyCount > 0)
            {
               translateApplyCount--;
               psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            break;
         }
         case IDCANCEL:
            if (translateApplyCount > 0)
            {
               for (int i = 0; i < translateApplyCount; i++) psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            EndDialog(hwndDlg, FALSE);
            break;
         }
         break;
      }
      break;
   }

   return FALSE;
}
