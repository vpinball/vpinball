#include "stdafx.h"
#include "renderer/Shader.h"
#include "renderer/IndexBuffer.h"
#include "renderer/VertexBuffer.h"
#ifndef __STANDALONE__
#include "captureExt.h"
#endif

Flasher::Flasher()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_isVisible = true;
   m_d.m_depthBias = 0.0f;
   m_dynamicVertexBufferRegenerate = true;
   m_vertices = nullptr;
   m_propVisual = nullptr;
   m_ptable = nullptr;
   m_numVertices = 0;
   m_numPolys = 0;
   m_minx = FLT_MAX;
   m_maxx = -FLT_MAX;
   m_miny = FLT_MAX;
   m_maxy = -FLT_MAX;
   m_lockedByLS = false;
}

Flasher::~Flasher()
{
   assert(m_rd == nullptr); // RenderRelease must be explicitely called before deleting this object
}

Flasher *Flasher::CopyForPlay(PinTable *live_table)
{
   STANDARD_EDITABLE_WITH_DRAGPOINT_COPY_FOR_PLAY_IMPL(Flasher, live_table, m_vdpoint)
   return dst;
}

void Flasher::InitShape()
{
   if (m_vdpoint.empty())
   {
      // First time shape has been set to custom - set up some points
      const float x = m_d.m_vCenter.x;
      const float y = m_d.m_vCenter.y;
      constexpr float size = 100.0f;

      CComObject<DragPoint> *pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x - size*0.5f, y - size*0.5f, 0.f, false);
         m_vdpoint.push_back(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x - size*0.5f, y + size*0.5f, 0.f, false);
         m_vdpoint.push_back(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x + size*0.5f, y + size*0.5f, 0.f, false);
         m_vdpoint.push_back(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x + size*0.5f, y - size*0.5f, 0.f, false);
         m_vdpoint.push_back(pdp);
      }
   }
}

HRESULT Flasher::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_isVisible = true;
   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;
   m_d.m_rotX = 0.0f;
   m_d.m_rotY = 0.0f;
   m_d.m_rotZ = 0.0f;
   InitShape();
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

void Flasher::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsFlasher

   m_d.m_height = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Height"s, 50.f) : 50.f;
   m_d.m_rotX = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "RotX"s, 0.f) : 0.f;
   m_d.m_rotY = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "RotY"s, 0.f) : 0.f;
   m_d.m_rotZ = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "RotZ"s, 0.f) : 0.f;
   m_d.m_color = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Color"s, (int)RGB(50,200,50)) : RGB(50,200,50);
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerInterval"s, 100) : 100;

   bool hr = g_pvp->m_settings.LoadValue(regKey, "ImageA"s, m_d.m_szImageA);
   if (!hr || !fromMouseClick)
      m_d.m_szImageA.clear();

   hr = g_pvp->m_settings.LoadValue(regKey, "ImageB"s, m_d.m_szImageB);
   if (!hr || !fromMouseClick)
      m_d.m_szImageB.clear();

   m_d.m_alpha = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Opacity"s, 100) : 100;

   m_d.m_intensity_scale = 1.0f;

   m_d.m_modulate_vs_add = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ModulateVsAdd"s, 0.9f) : 0.9f;
   m_d.m_filterAmount = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "FilterAmount"s, 100) : 100;
   m_d.m_isVisible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Visible"s, true) : true;
   m_inPlayState = m_d.m_isVisible;
   m_d.m_addBlend = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "AddBlend"s, false) : false;
   m_d.m_isDMD = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "DMD"s, false) : false;
   m_d.m_displayTexture = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "DisplayTexture"s, false) : false;
   m_d.m_imagealignment = fromMouseClick ? (RampImageAlignment)g_pvp->m_settings.LoadValueWithDefault(regKey, "ImageMode"s, (int)ImageModeWrap) : ImageModeWrap;
   m_d.m_filter = fromMouseClick ? (Filters)g_pvp->m_settings.LoadValueWithDefault(regKey, "Filter"s, (int)Filter_Overlay) : Filter_Overlay;

#undef regKey
}

void Flasher::WriteRegDefaults()
{
#define regKey Settings::DefaultPropsFlasher

   g_pvp->m_settings.SaveValue(regKey, "Height"s, m_d.m_height);
   g_pvp->m_settings.SaveValue(regKey, "RotX"s, m_d.m_rotX);
   g_pvp->m_settings.SaveValue(regKey, "RotY"s, m_d.m_rotY);
   g_pvp->m_settings.SaveValue(regKey, "RotZ"s, m_d.m_rotZ);
   g_pvp->m_settings.SaveValue(regKey, "Color"s, (int)m_d.m_color);
   g_pvp->m_settings.SaveValue(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   g_pvp->m_settings.SaveValue(regKey, "ImageA"s, m_d.m_szImageA);
   g_pvp->m_settings.SaveValue(regKey, "ImageB"s, m_d.m_szImageB);
   g_pvp->m_settings.SaveValue(regKey, "Alpha"s, m_d.m_alpha);
   g_pvp->m_settings.SaveValue(regKey, "ModulateVsAdd"s, m_d.m_modulate_vs_add);
   g_pvp->m_settings.SaveValue(regKey, "Visible"s, m_d.m_isVisible);
   g_pvp->m_settings.SaveValue(regKey, "DisplayTexture"s, m_d.m_displayTexture);
   g_pvp->m_settings.SaveValue(regKey, "AddBlend"s, m_d.m_addBlend);
   g_pvp->m_settings.SaveValue(regKey, "DMD"s, m_d.m_isDMD);
   g_pvp->m_settings.SaveValue(regKey, "ImageMode"s, (int)m_d.m_imagealignment);
   g_pvp->m_settings.SaveValue(regKey, "Filter"s, m_d.m_filter);
   g_pvp->m_settings.SaveValue(regKey, "FilterAmount"s, (int)m_d.m_filterAmount);

#undef regKey
}

void Flasher::UIRenderPass1(Sur * const psur)
{
   if (m_vdpoint.empty())
      InitShape();

   psur->SetFillColor(m_ptable->RenderSolid() ? m_vpinball->m_fillColor: -1);
   psur->SetObject(this);
   // Don't want border color to be over-ridden when selected - that will be drawn later
   psur->SetBorderColor(-1, false, 0);

   vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);
   Texture *ppi;
   if (m_ptable->RenderSolid() && m_d.m_displayTexture && (ppi = m_ptable->GetImage(m_d.m_szImageA)))
   {
      ppi->CreateGDIVersion();
      if (m_d.m_imagealignment == ImageModeWrap)
      {
         float _minx = FLT_MAX;
         float _miny = FLT_MAX;
         float _maxx = -FLT_MAX;
         float _maxy = -FLT_MAX;
         for (size_t i = 0; i < vvertex.size(); i++)
         {
            if (vvertex[i].x < _minx) _minx = vvertex[i].x;
            if (vvertex[i].x > _maxx) _maxx = vvertex[i].x;
            if (vvertex[i].y < _miny) _miny = vvertex[i].y;
            if (vvertex[i].y > _maxy) _maxy = vvertex[i].y;
         }

         if (ppi->m_hbmGDIVersion)
            psur->PolygonImage(vvertex, ppi->m_hbmGDIVersion, _minx, _miny, _minx + (_maxx - _minx), _miny + (_maxy - _miny), ppi->m_width, ppi->m_height);
      }
      else
      {
         if (ppi->m_hbmGDIVersion)
            psur->PolygonImage(vvertex, ppi->m_hbmGDIVersion, m_ptable->m_left, m_ptable->m_top, m_ptable->m_right, m_ptable->m_bottom, ppi->m_width, ppi->m_height);
         else
         {
            // Do nothing for now to indicate to user that there is a problem
         }
      }
   }
   else
      psur->Polygon(vvertex);
}

void Flasher::UIRenderPass2(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this); // For selected formatting
   psur->SetObject(nullptr);

   {
      vector<RenderVertex> vvertex; //!! check/reuse from UIRenderPass1
      GetRgVertex(vvertex);
      psur->Polygon(vvertex);
   }

   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   bool drawDragpoints = ((m_selectstate != eNotSelected) || m_vpinball->m_alwaysDrawDragPoints);

   if (!drawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (size_t i = 0; i < m_vdpoint.size(); i++)
      {
         const CComObject<DragPoint> * const pdp = m_vdpoint[i];
         if (pdp->m_selectstate != eNotSelected)
         {
            drawDragpoints = true;
            break;
         }
      }
   }

   if (drawDragpoints)
   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint[i];
      psur->SetFillColor(-1);
      psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(255, 0, 0), false, 0);
      psur->SetObject(pdp);

      psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
   }
}


void Flasher::RenderBlueprint(Sur *psur, const bool solid)
{
}

void Flasher::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();
   m_phittimer = new HitTimer(GetName(), m_d.m_tdr.m_TimerInterval, this);
   if (m_d.m_tdr.m_TimerEnabled)
   pvht.push_back(m_phittimer);
}

void Flasher::GetHitShapes(vector<HitObject*> &pvho)
{
}

void Flasher::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}


void Flasher::EndPlay()
{
   IEditable::EndPlay();
   // ensure not locked just in case the player exits during a LS sequence
   m_lockedByLS = false;
}

void Flasher::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(0, 0);
}

void Flasher::FlipY(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Flasher::FlipX(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Flasher::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter, useElementCenter);
}

void Flasher::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter, useElementCenter);
}

void Flasher::Translate(const Vertex2D &pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

void Flasher::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;
   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint[i];

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }
}

void Flasher::DoCommand(int icmd, int x, int y)
{
   ISelect::DoCommand(icmd, x, y);

   switch (icmd)
   {
   case ID_WALLMENU_FLIP:
      FlipPointY(GetPointCenter());
      break;

   case ID_WALLMENU_MIRROR:
      FlipPointX(GetPointCenter());
      break;

   case ID_WALLMENU_ROTATE:
      RotateDialog();
      break;

   case ID_WALLMENU_SCALE:
      ScaleDialog();
      break;

   case ID_WALLMENU_TRANSLATE:
      TranslateDialog();
      break;

   case ID_WALLMENU_ADDPOINT:
      AddPoint(x, y, false);
      break;
   }
}

void Flasher::AddPoint(int x, int y, const bool smooth)
{
      STARTUNDO
      const Vertex2D v = m_ptable->TransformPoint(x, y);

      vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);

      Vertex2D vOut;
      int iSeg;
      ClosestPointOnPolygon(vvertex, v, vOut, iSeg, true);

      // Go through vertices (including iSeg itself) counting control points until iSeg
      int icp = 0;
      for (int i = 0; i < (iSeg + 1); i++)
         if (vvertex[i].controlPoint)
            icp++;

      CComObject<DragPoint> *pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, vOut.x, vOut.y, 0.f, smooth);
         m_vdpoint.insert(m_vdpoint.begin() + icp, pdp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
      }

      STOPUNDO
}

#ifdef __STANDALONE__
void Flasher::UpdatePoint(int index, int x, int y)
{
     CComObject<DragPoint> *pdp = m_vdpoint[index];
     pdp->m_v.x = x;
     pdp->m_v.y = y;

}
#endif

HRESULT Flasher::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteFloat(FID(FHEI), m_d.m_height);
   bw.WriteFloat(FID(FLAX), m_d.m_vCenter.x);
   bw.WriteFloat(FID(FLAY), m_d.m_vCenter.y);
   bw.WriteFloat(FID(FROX), m_d.m_rotX);
   bw.WriteFloat(FID(FROY), m_d.m_rotY);
   bw.WriteFloat(FID(FROZ), m_d.m_rotZ);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteString(FID(IMAG), m_d.m_szImageA);
   bw.WriteString(FID(IMAB), m_d.m_szImageB);
   bw.WriteInt(FID(FALP), m_d.m_alpha);
   bw.WriteFloat(FID(MOVA), m_d.m_modulate_vs_add);
   bw.WriteBool(FID(FVIS), m_d.m_isVisible);
   bw.WriteBool(FID(DSPT), m_d.m_displayTexture);
   bw.WriteBool(FID(ADDB), m_d.m_addBlend);
   bw.WriteBool(FID(IDMD), m_d.m_isDMD);
   bw.WriteFloat(FID(FLDB), m_d.m_depthBias);
   bw.WriteInt(FID(ALGN), m_d.m_imagealignment);
   bw.WriteInt(FID(FILT), m_d.m_filter);
   bw.WriteInt(FID(FIAM), m_d.m_filterAmount);
   bw.WriteString(FID(LMAP), m_d.m_szLightmap);
   ISelect::SaveData(pstm, hcrypthash);

   HRESULT hr;
   if (FAILED(hr = SavePointData(pstm, hcrypthash)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

void Flasher::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}


HRESULT Flasher::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();

   m_inPlayState = m_d.m_isVisible;

   return S_OK;
}

bool Flasher::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(FHEI): pbr->GetFloat(m_d.m_height); break;
   case FID(FLAX): pbr->GetFloat(m_d.m_vCenter.x); break;
   case FID(FLAY): pbr->GetFloat(m_d.m_vCenter.y); break;
   case FID(FROX): pbr->GetFloat(m_d.m_rotX); break;
   case FID(FROY): pbr->GetFloat(m_d.m_rotY); break;
   case FID(FROZ): pbr->GetFloat(m_d.m_rotZ); break;
   case FID(COLR): pbr->GetInt(m_d.m_color); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImageA); break;
   case FID(IMAB): pbr->GetString(m_d.m_szImageB); break;
   case FID(FALP):
   {
      int iTmp;
      pbr->GetInt(iTmp);
      //if (iTmp>100) iTmp=100;
      if (iTmp < 0) iTmp = 0;
      m_d.m_alpha = iTmp;
      break;
   }
   case FID(MOVA): pbr->GetFloat(m_d.m_modulate_vs_add); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   case FID(FVIS): pbr->GetBool(m_d.m_isVisible); break;
   case FID(ADDB): pbr->GetBool(m_d.m_addBlend); break;
   case FID(IDMD): pbr->GetBool(m_d.m_isDMD); break;
   case FID(DSPT): pbr->GetBool(m_d.m_displayTexture); break;
   case FID(FLDB): pbr->GetFloat(m_d.m_depthBias); break;
   case FID(ALGN): pbr->GetInt(&m_d.m_imagealignment); break;
   case FID(FILT): pbr->GetInt(&m_d.m_filter); break;
   case FID(FIAM): pbr->GetInt(m_d.m_filterAmount); break;
   case FID(LMAP): pbr->GetString(m_d.m_szLightmap); break;
   default:
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
      break;
   }
   }
   return true;
}

HRESULT Flasher::InitPostLoad()
{
   return S_OK;
}

STDMETHODIMP Flasher::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IFlasher
   };

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

   return S_FALSE;
}

STDMETHODIMP Flasher::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;
   m_vpinball->SetStatusBarUnitInfo(string(), true);

   return S_OK;
}

STDMETHODIMP Flasher::put_X(float newVal)
{
   if (m_d.m_vCenter.x != newVal)
   {
      m_d.m_vCenter.x = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Flasher::put_Y(float newVal)
{
   if (m_d.m_vCenter.y != newVal)
   {
      m_d.m_vCenter.y = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_RotX(float *pVal)
{
   *pVal = m_d.m_rotX;

   return S_OK;
}

STDMETHODIMP Flasher::put_RotX(float newVal)
{
   if (m_d.m_rotX != newVal)
   {
      m_d.m_rotX = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_RotY(float *pVal)
{
   *pVal = m_d.m_rotY;

   return S_OK;
}

STDMETHODIMP Flasher::put_RotY(float newVal)
{
   if (m_d.m_rotY != newVal)
   {
      m_d.m_rotY = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_RotZ(float *pVal)
{
   *pVal = m_d.m_rotZ;

   return S_OK;
}

STDMETHODIMP Flasher::put_RotZ(float newVal)
{
   if (m_d.m_rotZ != newVal)
   {
      m_d.m_rotZ = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Height(float *pVal)
{
   *pVal = m_d.m_height;

   return S_OK;
}

STDMETHODIMP Flasher::put_Height(float newVal)
{
   if (m_d.m_height != newVal)
   {
      m_d.m_height = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Flasher::put_Color(OLE_COLOR newVal)
{
   m_d.m_color = newVal;

   return S_OK;
}

STDMETHODIMP Flasher::get_ImageA(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImageA.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_ImageA(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   m_d.m_szImageA = szImage;

   return S_OK;
}

STDMETHODIMP Flasher::get_ImageB(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImageB.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_ImageB(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   m_d.m_szImageB = szImage;

   return S_OK;
}

STDMETHODIMP Flasher::get_Filter(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];

   switch (m_d.m_filter)
   {
   case Filter_Additive:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "Additive", -1, wz, MAXNAMEBUFFER);
      break;
   }
   case Filter_Multiply:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "Multiply", -1, wz, MAXNAMEBUFFER);
      break;
   }
   case Filter_Overlay:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "Overlay", -1, wz, MAXNAMEBUFFER);
      break;
   }
   case Filter_Screen:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "Screen", -1, wz, MAXNAMEBUFFER);
      break;
   }
   default:
      assert(!"Invalid Flasher Filter");
   case Filter_None:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "None", -1, wz, MAXNAMEBUFFER);
      break;
   }
   }
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_Filter(BSTR newVal)
{
   char m_szFilter[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, m_szFilter, MAXNAMEBUFFER, nullptr, nullptr);

   if (strcmp(m_szFilter, "Additive") == 0 && m_d.m_filter != Filter_Additive)
      m_d.m_filter = Filter_Additive;
   else if (strcmp(m_szFilter, "Multiply") == 0 && m_d.m_filter != Filter_Multiply)
      m_d.m_filter = Filter_Multiply;
   else if (strcmp(m_szFilter, "Overlay") == 0 && m_d.m_filter != Filter_Overlay)
      m_d.m_filter = Filter_Overlay;
   else if (strcmp(m_szFilter, "Screen") == 0 && m_d.m_filter != Filter_Screen)
      m_d.m_filter = Filter_Screen;
   else if (strcmp(m_szFilter, "None") == 0 && m_d.m_filter != Filter_None)
      m_d.m_filter = Filter_None;

   return S_OK;
}

STDMETHODIMP Flasher::get_Opacity(long *pVal)
{
   *pVal = m_d.m_alpha;
   return S_OK;
}

STDMETHODIMP Flasher::put_Opacity(long newVal)
{
   SetAlpha(newVal);
   return S_OK;
}

STDMETHODIMP Flasher::get_IntensityScale(float *pVal)
{
   *pVal = m_d.m_intensity_scale;
   return S_OK;
}

STDMETHODIMP Flasher::put_IntensityScale(float newVal)
{
   m_d.m_intensity_scale = newVal;
   return S_OK;
}

STDMETHODIMP Flasher::get_ModulateVsAdd(float *pVal)
{
   *pVal = m_d.m_modulate_vs_add;
   return S_OK;
}

STDMETHODIMP Flasher::put_ModulateVsAdd(float newVal)
{
   m_d.m_modulate_vs_add = newVal;
   return S_OK;
}

STDMETHODIMP Flasher::get_Amount(long *pVal)
{
   *pVal = m_d.m_filterAmount;
   return S_OK;
}

STDMETHODIMP Flasher::put_Amount(long newVal)
{
   SetFilterAmount(newVal);
   return S_OK;
}

STDMETHODIMP Flasher::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_isVisible);

   return S_OK;
}

STDMETHODIMP Flasher::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_isVisible = VBTOb(newVal); // set visibility
   return S_OK;
}

STDMETHODIMP Flasher::get_DisplayTexture(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_displayTexture);

   return S_OK;
}

STDMETHODIMP Flasher::put_DisplayTexture(VARIANT_BOOL newVal)
{
   m_d.m_displayTexture = VBTOb(newVal); // set visibility
   return S_OK;
}

STDMETHODIMP Flasher::get_AddBlend(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_addBlend);

   return S_OK;
}

STDMETHODIMP Flasher::put_AddBlend(VARIANT_BOOL newVal)
{
   m_d.m_addBlend = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Flasher::get_DMD(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_isDMD);

   return S_OK;
}

STDMETHODIMP Flasher::put_DMD(VARIANT_BOOL newVal)
{
   m_d.m_isDMD = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Flasher::put_DMDWidth(int pVal)
{
   m_dmdSize.x = pVal;
   return S_OK;
}

STDMETHODIMP Flasher::put_DMDHeight(int pVal)
{
   m_dmdSize.y = pVal;
   return S_OK;
}

// Implementation included in pintable.cpp
void upscale(DWORD *const data, const int2 &res, const bool is_brightness_data);

STDMETHODIMP Flasher::put_DMDPixels(VARIANT pVal) // assumes VT_UI1 as input //!! use 64bit instead of 8bit to reduce overhead??
{
   SAFEARRAY *psa = V_ARRAY(&pVal);
   if (m_rd == nullptr)
      return E_FAIL;

   if (psa && m_dmdSize.x > 0 && m_dmdSize.y > 0)
   {
      const int size = m_dmdSize.x * m_dmdSize.y;
      if (!m_texdmd
#ifdef DMD_UPSCALE
         || (m_texdmd->width() * m_texdmd->height() != size * (3 * 3)))
#else
         || (m_texdmd->width() * m_texdmd->height() != size))
#endif
      {
         if (m_texdmd)
         {
            m_rd->DMDShader->SetTextureNull(SHADER_tex_dmd);
            m_rd->m_texMan.UnloadTexture(m_texdmd);
            delete m_texdmd;
         }
#ifdef DMD_UPSCALE
         m_texdmd = new BaseTexture(m_dmdSize.x * 3, m_dmdSize.y * 3, BaseTexture::RGBA);
#else
         m_texdmd = new BaseTexture(m_dmdSize.x, m_dmdSize.y, BaseTexture::RGBA);
#endif
      }

      DWORD *const data = (DWORD *)m_texdmd->data(); //!! assumes tex data to be always 32bit

      VARIANT *p;
      SafeArrayAccessData(psa, (void **)&p);
      for (int ofs = 0; ofs < size; ++ofs)
         data[ofs] = V_UI4(&p[ofs]); // store raw values (0..100), let shader do the rest
      SafeArrayUnaccessData(psa);

      if (g_pplayer->m_scaleFX_DMD)
         upscale(data, m_dmdSize, true);

      m_rd->m_texMan.SetDirty(m_texdmd);
   }

   return S_OK;
}

STDMETHODIMP Flasher::put_DMDColoredPixels(VARIANT pVal) //!! assumes VT_UI4 as input //!! use 64bit instead of 32bit to reduce overhead??
{
   SAFEARRAY *psa = V_ARRAY(&pVal);

   if (psa && m_dmdSize.x > 0 && m_dmdSize.y > 0)
   {
      const int size = m_dmdSize.x * m_dmdSize.y;
      if (!m_texdmd
#ifdef DMD_UPSCALE
         || (m_texdmd->width() * m_texdmd->height() != size * (3 * 3)))
#else
         || (m_texdmd->width() * m_texdmd->height() != size))
#endif
      {
         if (m_texdmd)
         {
            m_rd->DMDShader->SetTextureNull(SHADER_tex_dmd);
            m_rd->m_texMan.UnloadTexture(m_texdmd);
            delete m_texdmd;
         }
#ifdef DMD_UPSCALE
         m_texdmd = new BaseTexture(m_dmdSize.x * 3, m_dmdSize.y * 3, BaseTexture::RGBA);
#else
         m_texdmd = new BaseTexture(m_dmdSize.x, m_dmdSize.y, BaseTexture::RGBA);
#endif
      }

      DWORD *const data = (DWORD *)m_texdmd->data(); //!! assumes tex data to be always 32bit

      VARIANT *p;
      SafeArrayAccessData(psa, (void **)&p);
      for (int ofs = 0; ofs < size; ++ofs)
         data[ofs] = V_UI4(&p[ofs]) | 0xFF000000u; // store RGB values and let shader do the rest (set alpha to let shader know that this is RGB and not just brightness)
      SafeArrayUnaccessData(psa);

      if (g_pplayer->m_scaleFX_DMD)
         upscale(data, m_dmdSize, false);

      m_rd->m_texMan.SetDirty(m_texdmd);
   }

   return S_OK;
}

STDMETHODIMP Flasher::put_VideoCapWidth(long cWidth)
{
    if (m_videoCapWidth != cWidth) ResetVideoCap(); //resets capture
    m_videoCapWidth = cWidth;

    return S_OK;
}

STDMETHODIMP Flasher::put_VideoCapHeight(long cHeight)
{
    if (m_videoCapHeight != cHeight) ResetVideoCap(); //resets capture
    m_videoCapHeight = cHeight;

    return S_OK;
}

void Flasher::ResetVideoCap()
{
   m_isVideoCap = false;
   if (m_videoCapTex)
   {
      //  m_rd->flasherShader->SetTextureNull(SHADER_tex_flasher_A); //!! ??
      m_rd->m_texMan.UnloadTexture(m_videoCapTex);
      delete m_videoCapTex;
      m_videoCapTex = nullptr;
   }
}

//if PASSED a blank title then we treat this as STOP capture and free resources.
STDMETHODIMP Flasher::put_VideoCapUpdate(BSTR cWinTitle)
{
#ifndef __STANDALONE__
    if (m_videoCapWidth == 0 || m_videoCapHeight == 0) return S_FALSE; //safety.  VideoCapWidth/Height needs to be set prior to this call

    char szWinTitle[MAXNAMEBUFFER];
    WideCharToMultiByteNull(CP_ACP, 0, cWinTitle, -1, szWinTitle, MAXNAMEBUFFER, nullptr, nullptr);

    //if PASS blank title then we treat as STOP capture and free resources.  Should be called on table1_exit
    if (szWinTitle[0] == '\0')
    {
        ResetVideoCap();
        return S_OK;
    }

    if (m_isVideoCap == false) {  // VideoCap has not started because no sourcewin found
        m_videoCapHwnd = ::FindWindow(0, szWinTitle);
        if (m_videoCapHwnd == nullptr)
            return S_FALSE;

        //source videocap found.  lets start!
        GetClientRect(m_videoCapHwnd, &m_videoSourceRect);
        ResetVideoCap();
        try
        {
           m_videoCapTex = new BaseTexture(m_videoCapWidth, m_videoCapHeight, BaseTexture::SRGBA);
        }
        catch (...)
        {
           delete m_videoCapTex;
           m_videoCapTex = nullptr;
           return S_FAIL;
        }
    }

    // Retrieve the handle to a display device context for the client
    // area of the window.

    const HDC hdcWindow = GetDC(m_videoCapHwnd);

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    const HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

    // Get the client area for size calculation.
    const int pWidth = m_videoCapWidth;
    const int pHeight = m_videoCapHeight;

    // Create a compatible bitmap from the Window DC.
    const HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, pWidth, pHeight);

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC, hbmScreen);
    SetStretchBltMode(hdcMemDC, HALFTONE);
    // Bit block transfer into our compatible memory DC.
    m_isVideoCap = StretchBlt(hdcMemDC, 0, 0, pWidth, pHeight, hdcWindow, 0, 0, m_videoSourceRect.right - m_videoSourceRect.left, m_videoSourceRect.bottom - m_videoSourceRect.top, SRCCOPY);
    if (m_isVideoCap)
    {
        // Get the BITMAP from the HBITMAP.
        BITMAP bmpScreen;
        GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

        BITMAPINFOHEADER bi;
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmpScreen.bmWidth;
        bi.biHeight = -bmpScreen.bmHeight;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        const DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

        const HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
        char* lpbitmap = (char*)GlobalLock(hDIB);

        // Gets the "bits" from the bitmap, and copies them into a buffer 
        // that's pointed to by lpbitmap.
        GetDIBits(hdcWindow, hbmScreen, 0, (UINT)bmpScreen.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        // copy bitmap pixels to texture, reversing BGR to RGB and adding an opaque alpha channel
        copy_bgra_rgba<true>((unsigned int*)(m_videoCapTex->data()), (const unsigned int*)lpbitmap, pWidth * pHeight);

        GlobalUnlock(hDIB);
        GlobalFree(hDIB);

        m_rd->m_texMan.SetDirty(m_videoCapTex);
    }

    ReleaseDC(m_videoCapHwnd, hdcWindow);
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
#endif

    return S_OK;
}

STDMETHODIMP Flasher::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Flasher::put_DepthBias(float newVal)
{
   m_d.m_depthBias = newVal;

   return S_OK;
}

STDMETHODIMP Flasher::get_ImageAlignment(RampImageAlignment *pVal)
{
   *pVal = m_d.m_imagealignment;

   return S_OK;
}

STDMETHODIMP Flasher::put_ImageAlignment(RampImageAlignment newVal)
{
   m_d.m_imagealignment = newVal;

   return S_OK;
}

//Sets the in play state for light sequencing rendering
void Flasher::setInPlayState(const bool newVal)
{
   m_inPlayState = newVal;
}

#pragma region Rendering

void Flasher::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   m_texdmd = nullptr;
   m_dmdSize = int2(0, 0);

   m_lightmap = m_ptable->GetLight(m_d.m_szLightmap);

   vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   m_numVertices = (unsigned int)vvertex.size();
   if (m_numVertices == 0)
   {
       // no polys to render leave vertex buffer undefined 
       m_numPolys = 0;
       return;
   }

   vector<WORD> vtri;
   
   {
   vector<unsigned int> vpoly(m_numVertices);
   for (unsigned int i = 0; i < m_numVertices; i++)
      vpoly[i] = i;

   PolygonToTriangles(vvertex, vpoly, vtri, false);
   }

   m_numPolys = (int)(vtri.size()/3);
   if (m_numPolys == 0)
   {
      // no polys to render leave vertex buffer undefined 
      return;
   }

   IndexBuffer* dynamicIndexBuffer = new IndexBuffer(m_rd, m_numPolys * 3, 0, IndexBuffer::FMT_INDEX16);

   WORD* bufi;
   dynamicIndexBuffer->lock(0, 0, (void**)&bufi, IndexBuffer::WRITEONLY);
   memcpy(bufi, vtri.data(), vtri.size()*sizeof(WORD));
   dynamicIndexBuffer->unlock();

   VertexBuffer* dynamicVertexBuffer = new VertexBuffer(m_rd, m_numVertices, nullptr, true);

   delete m_meshBuffer;
   m_meshBuffer = new MeshBuffer(m_wzName, dynamicVertexBuffer, dynamicIndexBuffer, true);

   m_vertices = new Vertex3D_NoTex2[m_numVertices];

   m_minx = FLT_MAX;
   m_miny = FLT_MAX;
   m_maxx = -FLT_MAX;
   m_maxy = -FLT_MAX;

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      const RenderVertex * const pv0 = &vvertex[i];

      m_vertices[i].x = pv0->x;
      m_vertices[i].y = pv0->y;
      m_vertices[i].z = 0;

      if (pv0->x > m_maxx) m_maxx = pv0->x;
      if (pv0->x < m_minx) m_minx = pv0->x;
      if (pv0->y > m_maxy) m_maxy = pv0->y;
      if (pv0->y < m_miny) m_miny = pv0->y;
   }
   
   const float inv_width = 1.0f / (m_maxx - m_minx);
   const float inv_height = 1.0f / (m_maxy - m_miny);
   const float inv_tablewidth = 1.0f / (m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f / (m_ptable->m_bottom - m_ptable->m_top);
   m_d.m_vCenter.x = m_minx + ((m_maxx - m_minx)*0.5f);
   m_d.m_vCenter.y = m_miny + ((m_maxy - m_miny)*0.5f);

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      if (m_d.m_imagealignment == ImageModeWrap)
      {
         m_vertices[i].tu = (m_vertices[i].x - m_minx)*inv_width;
         m_vertices[i].tv = (m_vertices[i].y - m_miny)*inv_height;
      }
      else
      {
         m_vertices[i].tu = m_vertices[i].x*inv_tablewidth;
         m_vertices[i].tv = m_vertices[i].y*inv_tableheight;
      }
   }
}

void Flasher::RenderRelease()
{
   assert(m_rd != nullptr);
   ResetVideoCap();
   delete m_meshBuffer;
   delete[] m_vertices;
   delete m_texdmd;
   m_meshBuffer = nullptr;
   m_vertices = nullptr;
   m_texdmd = nullptr;
   m_dmdSize = int2(0, 0);
   m_lightmap = nullptr;
   m_rd = nullptr;
}

void Flasher::UpdateAnimation(const float diff_time_msec)
{
}

void Flasher::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Player::STATIC_ONLY;
   const bool isReflectionPass = renderMask & Player::REFLECTION_PASS;
   TRACE_FUNCTION();

   // Flashers are always dynamic parts
   if (isStaticOnly)
      return;

   // Don't render if invisible, degenerated or in reflection pass
   // TODO shouldn't we handle flashers like lights and therefore process them according to disable lightmap flag ?
   if (!m_d.m_isVisible || m_meshBuffer == nullptr || isReflectionPass)
      return;

   // Don't render if LightSequence in play and state is off
   if (m_lockedByLS && !m_inPlayState)
      return;
   
   // Don't render DMD if DMD connection not set
   BaseTexture *texdmd = m_texdmd != nullptr ? m_texdmd : g_pplayer->m_texdmd;
   if (m_d.m_isDMD && !texdmd)
      return;

   // Update lightmap before checking anything that uses alpha
   float alpha = (float) m_d.m_alpha;
   if (m_lightmap)
      alpha *= m_lightmap->m_currentIntensity / (m_lightmap->m_d.m_intensity * m_lightmap->m_d.m_intensity_scale);

   if (m_d.m_color == 0 || alpha == 0.0f || m_d.m_intensity_scale == 0.0f)
      return;

   if (m_dynamicVertexBufferRegenerate)
   {
      m_dynamicVertexBufferRegenerate = false;
      const float height = m_d.m_height;
      const float movx = m_minx + (m_maxx - m_minx)*0.5f;
      const float movy = m_miny + (m_maxy - m_miny)*0.5f;

      Matrix3D tempMatrix, TMatrix;
      TMatrix.SetRotateZ(ANGTORAD(m_d.m_rotZ));
      tempMatrix.SetRotateY(ANGTORAD(m_d.m_rotY));
      tempMatrix.Multiply(TMatrix, TMatrix);
      tempMatrix.SetRotateX(ANGTORAD(m_d.m_rotX));
      tempMatrix.Multiply(TMatrix, TMatrix);

      tempMatrix.SetTranslation(m_d.m_vCenter.x,m_d.m_vCenter.y,height);
      tempMatrix.Multiply(TMatrix, tempMatrix);

      TMatrix.SetTranslation(
          -movx, //-m_d.m_vCenter.x,
          -movy, //-m_d.m_vCenter.y,
          0.f);
      tempMatrix.Multiply(TMatrix, tempMatrix);

      Vertex3D_NoTex2 *buf;
      m_meshBuffer->m_vb->lock(0, 0, (void **)&buf, VertexBuffer::DISCARDCONTENTS);
      for (unsigned int i = 0; i < m_numVertices; i++)
      {
         Vertex3D_NoTex2 vert = m_vertices[i];
         tempMatrix.MultiplyVector(vert, vert);
         buf[i] = vert;
      }
      m_meshBuffer->m_vb->unlock();
   }

   m_rd->ResetRenderState();
   m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);

   const vec4 color = convertColor(m_d.m_color, (float)alpha*m_d.m_intensity_scale / 100.0f);
   if (m_d.m_isDMD)
   {
      if (m_d.m_modulate_vs_add < 1.f)
         m_rd->EnableAlphaBlend(m_d.m_addBlend);
      else
         m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);

      /*const unsigned int alphamode = 1; //!! make configurable?
      // add
      if (alphamode == 1) {
         m_rd->EnableAlphaBlend(true);
      // max
      } else if (alphamode == 2) {
         m_rd->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_TRUE);
         m_rd->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_MAX);
         m_rd->SetRenderState(RenderDevice::SRCBLEND, RenderDevice::SRC_ALPHA);
         m_rd->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::INVSRC_COLOR);
      //subtract
      } else if (alphamode == 3) {
         m_rd->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_TRUE);
         m_rd->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_SUBTRACT);
         m_rd->SetRenderState(RenderDevice::SRCBLEND, RenderDevice::ZERO);
         m_rd->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::INVSRC_COLOR);
      // normal
      } else if (alphamode == 4) {
         m_rd->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_TRUE);
         m_rd->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD);
         m_rd->SetRenderState(RenderDevice::SRCBLEND, RenderDevice::SRC_ALPHA);
         m_rd->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::INVSRC_ALPHA);
      }

      const int alphatest = 0; //!!
      if (alphatest)
         g_pplayer->m_pin3d.EnableAlphaTestReference(0x80);*/

      m_rd->DMDShader->SetTechnique(SHADER_TECHNIQUE_basic_DMD_world); //!! DMD_UPSCALE ?? -> should just work

      m_rd->DMDShader->SetVector(SHADER_vColor_Intensity, &color);

      const int2 &dmdSize = m_texdmd != nullptr ? m_dmdSize : g_pplayer->m_dmd;

      #ifdef DMD_UPSCALE
      const vec4 r((float)(dmdSize.x * 3), (float)(dmdSize.y * 3), m_d.m_modulate_vs_add, (float)(g_pplayer->m_overall_frames % 2048)); //(float)(0.5 / m_width), (float)(0.5 / m_height));
      #else
      const vec4 r((float)dmdSize.x, (float)dmdSize.y, m_d.m_modulate_vs_add, (float)(g_pplayer->m_overall_frames % 2048)); //(float)(0.5 / m_width), (float)(0.5 / m_height));
      #endif
      m_rd->DMDShader->SetVector(SHADER_vRes_Alpha_time, &r);

#ifndef __STANDALONE__
      // If we're capturing Freezy DMD switch to ext technique to avoid incorrect colorization
      if (HasDMDCapture())
         m_rd->DMDShader->SetTechnique(SHADER_TECHNIQUE_basic_DMD_world_ext);
#endif

      if (texdmd != nullptr)
         m_rd->DMDShader->SetTexture(SHADER_tex_dmd, texdmd, SF_NONE, SA_CLAMP, SA_CLAMP);

      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_height);
      // DMD flasher are rendered transparent. They used to be drawn as a separate pass after opaque parts and before other transparents.
      // There we shift the depthbias to reproduce this behavior.
      m_rd->DrawMesh(m_rd->DMDShader, true, pos, m_d.m_depthBias - 10000.f, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_numPolys * 3);
   }
   else
   {
      Texture * const pinA = m_ptable->GetImage(m_d.m_szImageA);
      Texture * const pinB = m_ptable->GetImage(m_d.m_szImageB);

      m_rd->flasherShader->SetVector(SHADER_staticColor_Alpha, &color);

      vec4 flasherData(-1.f, -1.f, (float)m_d.m_filter, m_d.m_addBlend ? 1.f : 0.f);
      m_rd->flasherShader->SetTechnique(SHADER_TECHNIQUE_basic_noLight);

      float flasherMode;
      if ((pinA || m_isVideoCap) && !pinB)
      {
         flasherMode = 0.f;
         if (m_isVideoCap)
            m_rd->flasherShader->SetTexture(SHADER_tex_flasher_A, m_videoCapTex);
         else
            m_rd->flasherShader->SetTexture(SHADER_tex_flasher_A, pinA);

         if (!m_d.m_addBlend)
            flasherData.x = !m_isVideoCap ? pinA->m_alphaTestValue : 0.f;
      }
      else if (!(pinA || m_isVideoCap) && pinB)
      {
         flasherMode = 0.f;
         m_rd->flasherShader->SetTexture(SHADER_tex_flasher_A, pinB);

         if (!m_d.m_addBlend)
            flasherData.x = pinB->m_alphaTestValue;
      }
      else if ((pinA || m_isVideoCap) && pinB)
      {
         flasherMode = 1.f;
         if (m_isVideoCap)
            m_rd->flasherShader->SetTexture(SHADER_tex_flasher_A, m_videoCapTex);
         else
            m_rd->flasherShader->SetTexture(SHADER_tex_flasher_A, pinA);
         m_rd->flasherShader->SetTexture(SHADER_tex_flasher_B, pinB);

         if (!m_d.m_addBlend)
         {
            flasherData.x = !m_isVideoCap ? pinA->m_alphaTestValue : 0.f;
            flasherData.y = pinB->m_alphaTestValue;
         }
      }
      else
         flasherMode = 2.f;

      const vec4 flasherData2((float)m_d.m_filterAmount / 100.0f, min(max(m_d.m_modulate_vs_add, 0.00001f), 0.9999f), // avoid 0, as it disables the blend and avoid 1 as it looks not good with day->night changes
         flasherMode, 0.f);
      m_rd->flasherShader->SetVector(SHADER_alphaTestValueAB_filterMode_addBlend, &flasherData);
      m_rd->flasherShader->SetVector(SHADER_amount_blend_modulate_vs_add_flasherMode, &flasherData2);

      // Check if this flasher is used as a lightmap and should be convoluted with the light shadows
      if (m_lightmap != nullptr && m_lightmap->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS)
         m_rd->flasherShader->SetVector(SHADER_lightCenter_doShadow, m_lightmap->m_d.m_vCenter.x, m_lightmap->m_d.m_vCenter.y, m_lightmap->GetCurrentHeight(), 1.0f);

      m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
      m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
      m_rd->SetRenderState(RenderState::DESTBLEND, m_d.m_addBlend ? RenderState::INVSRC_COLOR : RenderState::INVSRC_ALPHA);
      m_rd->SetRenderState(RenderState::BLENDOP, m_d.m_addBlend ? RenderState::BLENDOP_REVSUBTRACT : RenderState::BLENDOP_ADD);

      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_height);
      m_rd->DrawMesh(m_rd->flasherShader, true, pos, m_d.m_depthBias, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_numPolys * 3);

      m_rd->flasherShader->SetVector(SHADER_lightCenter_doShadow, 0.0f, 0.0f, 0.0f, 0.0f);
   }
}

#pragma endregion