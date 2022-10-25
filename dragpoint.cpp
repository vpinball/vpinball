#include "stdafx.h"

Vertex3Ds DragPoint::m_copyPoint;
bool      DragPoint::m_pointCopied = false;

IHaveDragPoints::IHaveDragPoints()
{
    m_propVisuals = nullptr;
    m_propPosition = nullptr;
}

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

   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      const Vertex3Ds& v = m_vdpoint[i]->m_v;
      minx = min(minx, v.x);
      maxx = max(maxx, v.x);
      miny = min(miny, v.y);
      maxy = max(maxy, v.y);
   }

   return Vertex2D((maxx + minx)*0.5f, (maxy + miny)*0.5f);
}

void IHaveDragPoints::PutPointCenter(const Vertex2D& pv)
{
}

void IHaveDragPoints::FlipPointY(const Vertex2D& pvCenter)
{
   STARTUNDOSELECT

   Vertex2D newcenter = GetPointCenter();

   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      const float deltay = m_vdpoint[i]->m_v.y - pvCenter.y;

      m_vdpoint[i]->m_v.y -= deltay*2.0f;
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

   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      const float deltax = m_vdpoint[i]->m_v.x - pvCenter.x;

      m_vdpoint[i]->m_v.x -= deltax*2.0f;
   }

   const float deltax = newcenter.x - pvCenter.x;
   newcenter.x -= deltax*2.0f;
   PutPointCenter(newcenter);

   ReverseOrder();

   STOPUNDOSELECT
}

void IHaveDragPoints::RotateDialog()
{
#ifndef __STANDALONE__
   DialogBoxParam(g_pvp->theInstance, MAKEINTRESOURCE(IDD_ROTATE),
       g_pvp->GetHwnd(), RotateProc, (size_t)this->GetIEditable()->GetISelect());//(long)this);
#endif
}

void IHaveDragPoints::ScaleDialog()
{
#ifndef __STANDALONE__
   DialogBoxParam(g_pvp->theInstance, MAKEINTRESOURCE(IDD_SCALE),
       g_pvp->GetHwnd(), ScaleProc, (size_t)this->GetIEditable()->GetISelect());
#endif
}

void IHaveDragPoints::TranslateDialog()
{
#ifndef __STANDALONE__
   DialogBoxParam(g_pvp->theInstance, MAKEINTRESOURCE(IDD_TRANSLATE),
       g_pvp->GetHwnd(), TranslateProc, (size_t)this->GetIEditable()->GetISelect());
#endif
}

void IHaveDragPoints::RotatePoints(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   Vertex2D newcenter = GetPointCenter();

   STARTUNDOSELECT

   if (useElementCenter)
   {
      /* Don't use the pvCenter anymore! pvCenter is the mouse position when rotating is activated.
      Because the mouse position (rotation center) isn't shown in the editor use the element's center returned by GetPointCenter() */
      const float centerx = newcenter.x;
      const float centery = newcenter.y;

      const float sn = sinf(ANGTORAD(ang));
      const float cs = cosf(ANGTORAD(ang));

      for (size_t i = 0; i < m_vdpoint.size(); i++)
      {
         DragPoint * const pdp1 = m_vdpoint[i];
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
      const float centerx = pvCenter.x;
      const float centery = pvCenter.y;

      const float sn = sinf(ANGTORAD(ang));
      const float cs = cosf(ANGTORAD(ang));

      for (size_t i = 0; i < m_vdpoint.size(); i++)
      {
         DragPoint * const pdp1 = m_vdpoint[i];
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
         PutPointCenter(newcenter);
      }
   }

   STOPUNDOSELECT
}

void IHaveDragPoints::ScalePoints(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   Vertex2D newcenter = GetPointCenter();

   STARTUNDOSELECT

   if (useElementCenter)
   {
      /* Don't use the pvCenter anymore! pvCenter is the mouse position when scaling is activated.
      Because the mouse position (scaling center) isn't shown in the editor use the element's center returned by GetPointCenter() */
      const float centerx = newcenter.x;
      const float centery = newcenter.y;

      for (size_t i = 0; i < m_vdpoint.size(); i++)
      {
         DragPoint * const pdp1 = m_vdpoint[i];
         const float dx = (pdp1->m_v.x - centerx) * scalex;
         const float dy = (pdp1->m_v.y - centery) * scaley;
         pdp1->m_v.x = centerx + dx;
         pdp1->m_v.y = centery + dy;
      }
   }
   else
   {
      const float centerx = pvCenter.x;
      const float centery = pvCenter.y;

      for (size_t i = 0; i < m_vdpoint.size(); i++)
      {
         DragPoint * const pdp1 = m_vdpoint[i];
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
         PutPointCenter(newcenter);
      }
   }

   STOPUNDOSELECT
}

void IHaveDragPoints::TranslatePoints(const Vertex2D &pvOffset)
{
   STARTUNDOSELECT

   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      DragPoint * const pdp1 = m_vdpoint[i];
      pdp1->m_v.x += pvOffset.x;
      pdp1->m_v.y += pvOffset.y;
   }

   Vertex2D newcenter = GetPointCenter();

   newcenter.x += pvOffset.x;
   newcenter.y += pvOffset.y;

   PutPointCenter(newcenter);

   STOPUNDOSELECT
}

void IHaveDragPoints::ReverseOrder()
{
   if (m_vdpoint.empty())
      return;

   // Reverse order of points (switches winding, reverses inside/outside)
   std::reverse(m_vdpoint.begin(), m_vdpoint.end());

   const bool slingshotTemp = m_vdpoint[0]->m_slingshot;

   for (size_t i = 0; i < m_vdpoint.size() - 1; i++)
   {
      DragPoint * const pdp1 = m_vdpoint[i];
      const DragPoint * const pdp2 = m_vdpoint[i + 1];

      pdp1->m_slingshot = pdp2->m_slingshot;
   }

   m_vdpoint[m_vdpoint.size() - 1]->m_slingshot = slingshotTemp;
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/Math/DragPoint.cs
//

void IHaveDragPoints::GetTextureCoords(const vector<RenderVertex> & vv, float **ppcoords)
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

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void IHaveDragPoints::ClearPointsForOverwrite()
{
   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      if (m_vdpoint[i]->m_selectstate != eNotSelected/*GetPTable()->m_pselcur == m_vdpoint[i]*/)
      {
         //GetPTable()->SetSel(GetPTable());
         GetPTable()->AddMultiSel(GetPTable(), false, true, false);
      }

      m_vdpoint[i]->Release();
   }

   m_vdpoint.clear();
}

HRESULT IHaveDragPoints::SavePointData(IStream *pstm, HCRYPTHASH hcrypthash)
{
   BiffWriter bw(pstm, hcrypthash);

   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      bw.WriteTag(FID(DPNT));
      CComObject<DragPoint> * const pdp = m_vdpoint[i];
      bw.WriteStruct(FID(VCEN), &(pdp->m_v), sizeof(Vertex2D));
      bw.WriteFloat(FID(POSZ), pdp->m_v.z);
      bw.WriteBool(FID(SMTH), pdp->m_smooth);
      bw.WriteBool(FID(SLNG), pdp->m_slingshot);
      bw.WriteBool(FID(ATEX), pdp->m_autoTexture);
      bw.WriteFloat(FID(TEXC), pdp->m_texturecoord);

      ((ISelect *)pdp)->SaveData(pstm, hcrypthash);

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
         pdp->Init(this, 0.f, 0.f, 0.f, false);
         m_vdpoint.push_back(pdp);
         BiffReader br(pbr->m_pistream, pdp, nullptr, version, pbr->m_hcrypthash, pbr->m_hcryptkey);
         br.Load();
      }
   }
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
   GetIEditable()->SetDirtyDraw();
}

void DragPoint::OnLButtonUp(int x, int y)
{
   ISelect::OnLButtonUp(x, y);
   GetIEditable()->SetDirtyDraw();
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
   return Vertex2D(m_v.x, m_v.y);
}

void DragPoint::PutCenter(const Vertex2D& pv)
{
   m_v.x = pv.x;
   m_v.y = pv.y;
}

void DragPoint::EditMenu(CMenu &menu)
{
#ifndef __STANDALONE__
   menu.CheckMenuItem(ID_POINTMENU_SMOOTH, MF_BYCOMMAND | (m_smooth ? MF_CHECKED : MF_UNCHECKED));
   //EnableMenuItem(hmenu, ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | (m_fSmooth ? MF_GRAYED : MF_ENABLED));
   menu.CheckMenuItem(ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | ((m_slingshot && !m_smooth) ? MF_CHECKED : MF_UNCHECKED));
#endif
}

void DragPoint::Delete()
{
   if ((int)M_PIHDP->m_vdpoint.size() > M_PIHDP->GetMinimumPoints()) // Can't allow less points than the user can recover from
   {
      GetIEditable()->BeginUndo();
      GetIEditable()->MarkForUndo();
      RemoveFromVectorSingle(M_PIHDP->m_vdpoint, (CComObject<DragPoint> *)this);
      GetIEditable()->EndUndo();
      GetIEditable()->SetDirtyDraw();
      Release();
   }
}

void DragPoint::Uncreate()
{
   RemoveFromVectorSingle(M_PIHDP->m_vdpoint, (CComObject<DragPoint> *)this);
   Release();
}

void DragPoint::DoCommand(int icmd, int x, int y)
{
   ISelect::DoCommand(icmd, x, y);
   switch (icmd)
   {
   case ID_POINTMENU_SMOOTH:
   {
      IEditable * const pedit = GetIEditable();
      pedit->BeginUndo();
      pedit->MarkForUndo();

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

      pedit->EndUndo();
      pedit->SetDirtyDraw();
      break;
   }
   case ID_POINTMENU_SLINGSHOT:
   {
      IEditable * const pedit = GetIEditable();
      pedit->BeginUndo();
      pedit->MarkForUndo();

      m_slingshot = !m_slingshot;
      if (m_slingshot)
      {
         m_smooth = false;
         const int index2 = (FindIndexOf(M_PIHDP->m_vdpoint, (CComObject<DragPoint> *)this) + 1) % M_PIHDP->m_vdpoint.size();
         M_PIHDP->m_vdpoint[index2]->m_smooth = false;
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

bool DragPoint::LoadToken(const int id, BiffReader * const pbr)
{
   switch (id)
   {
   case FID(VCEN): pbr->GetStruct(&m_v, sizeof(Vertex2D)); break;
   case FID(POSZ): pbr->GetFloat(m_v.z); break;
   case FID(SMTH): pbr->GetBool(m_smooth); break;
   case FID(SLNG): pbr->GetBool(m_slingshot); break;
   case FID(ATEX): pbr->GetBool(m_autoTexture); break;
   case FID(TEXC): pbr->GetFloat(m_texturecoord); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
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

int rotateApplyCount = 0;
INT_PTR CALLBACK RotateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
   ISelect *psel;

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      rotateApplyCount = 0;
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      const float angle = psel->GetRotate();

      SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_SETCHECK, BST_CHECKED, 0);

      SetDlgItemText(hwndDlg, IDC_ROTATEBY, f2sz(angle).c_str());
      const Vertex2D v = psel->GetCenter();
      SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(v.x).c_str());
      SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(v.y).c_str());
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
               if (!(SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED))
               {
                  SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(g_pvp->m_mouseCursorPosition.x).c_str());
                  SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(g_pvp->m_mouseCursorPosition.y).c_str());
               }
               else
               {
                  const Vertex2D v = psel->GetCenter();
                  SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(v.x).c_str());
                  SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(v.y).c_str());
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

               const bool useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);
               GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
               Vertex2D v;
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
               v.y = sz2f(szT);

               psel->Rotate(f, v, useElementCenter);
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

            const bool useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_ROTATE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);
            GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
            Vertex2D v;
            v.x = sz2f(szT);
            GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
            v.y = sz2f(szT);

            psel->Rotate(f, v, useElementCenter);
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

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      scaleApplyCount = 0;
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

      Vertex2D v = psel->GetScale();

      SetDlgItemText(hwndDlg, IDC_SCALEFACTOR, f2sz(v.x).c_str());
      SetDlgItemText(hwndDlg, IDC_SCALEY, f2sz(v.y).c_str());
      v = psel->GetCenter();

      SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_SETCHECK, BST_CHECKED, 0);

      SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(v.x).c_str());
      SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(v.y).c_str());

      SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_SETCHECK, TRUE, 0);

      EnableWindow(GetDlgItem(hwndDlg, IDC_SCALEY), FALSE);
      EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_SCALEY), FALSE);
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
               if (!(SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED))
               {
                  SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(g_pvp->m_mouseCursorPosition.x).c_str());
                  SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(g_pvp->m_mouseCursorPosition.y).c_str());
               }
               else
               {
                  const Vertex2D v = psel->GetCenter();
                  SetDlgItemText(hwndDlg, IDC_CENTERX, f2sz(v.x).c_str());
                  SetDlgItemText(hwndDlg, IDC_CENTERY, f2sz(v.y).c_str());
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

               const bool useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);
               //pihdp->ScalePoints(fx, fy, &v);
               psel->Scale(fx, fy, v, useElementCenter);
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

            const bool useElementCenter = (SendDlgItemMessage(hwndDlg, IDC_CHECK_SCALE_CENTER, BM_GETCHECK, 0, 0) == BST_CHECKED);

            //pihdp->ScalePoints(fx, fy, &v);
            psel->Scale(fx, fy, v, useElementCenter);
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
            EnableWindow(GetDlgItem(hwndDlg, IDC_SCALEY), !(checked == BST_CHECKED));
            EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_SCALEY), !(checked == BST_CHECKED));
         }
         break;
         }
         break;
      }
      break;
   }
#endif

   return FALSE;
}

static int translateApplyCount = 0;
INT_PTR CALLBACK TranslateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
   ISelect *psel;

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      translateApplyCount = 0;
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      SetDlgItemText(hwndDlg, IDC_OFFSETX, f2sz(0.f).c_str());
      SetDlgItemText(hwndDlg, IDC_OFFSETY, f2sz(0.f).c_str());
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

               psel->Translate(v);
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

            psel->Translate(v);
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
#endif

   return FALSE;
}
