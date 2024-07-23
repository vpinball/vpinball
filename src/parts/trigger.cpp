#include "stdafx.h"
#include "objloader.h"
#include "meshes/triggerSimpleMesh.h"
#include "meshes/triggerStarMesh.h"
#include "meshes/triggerButtonMesh.h"
#include "meshes/triggerWireDMesh.h"
#include "meshes/triggerInderMesh.h"
#include "renderer/Shader.h"

Trigger::Trigger()
{
   m_ptriggerhitcircle = nullptr;

   m_hitEvent = false;
   m_unhitEvent = false;
   m_doAnimation = false;
   m_moveDown = false;
   m_animHeightOffset = 0.0f;
   m_vertexBuffer_animHeightOffset = -FLT_MAX;

   m_hitEnabled = true;
   m_menuid = IDR_SURFACEMENU;
   m_propVisual = nullptr;
}

Trigger::~Trigger()
{
   assert(m_rd == nullptr);
}

Trigger *Trigger::CopyForPlay(PinTable *live_table)
{
   STANDARD_EDITABLE_WITH_DRAGPOINT_COPY_FOR_PLAY_IMPL(Trigger, live_table, m_vdpoint)
   dst->m_hitEnabled = m_hitEnabled;
   return dst;
}

void Trigger::UpdateStatusBarInfo()
{
   if(g_pplayer)
       return;

   if (m_d.m_shape != TriggerNone)
   {
      const Vertex3D_NoTex2 *meshVertices;
      switch(m_d.m_shape)
      {
      case TriggerWireA:
      case TriggerWireB:
      case TriggerWireC:
      {
         m_numVertices = triggerSimpleNumVertices;
         m_numIndices = triggerSimpleNumIndices;
         m_faceIndices = triggerSimpleIndices;
         meshVertices = triggerSimple;
         break;
      }
      case TriggerWireD:
      {
         m_numVertices = triggerDWireNumVertices;
         m_numIndices = triggerDWireNumIndices;
         m_faceIndices = triggerDWireIndices;
         meshVertices = triggerDWireMesh;
         break;
      }
      case TriggerInder:
      {
         m_numVertices = triggerInderNumVertices;
         m_numIndices = triggerInderNumIndices;
         m_faceIndices = triggerInderIndices;
         meshVertices = triggerInderMesh;
         break;
      }
      case TriggerButton:
      {
         m_numVertices = triggerButtonNumVertices;
         m_numIndices = triggerButtonNumIndices;
         m_faceIndices = triggerButtonIndices;
         meshVertices = triggerButtonMesh;
         break;
      }
      case TriggerStar:
      {
         m_numVertices = triggerStarNumVertices;
         m_numIndices = triggerStarNumIndices;
         m_faceIndices = triggerStarIndices;
         meshVertices = triggerStar;
         break;
      }
      default:
         assert(!"Unhandled Trigger case");
         break;
      }

      m_vertices.resize(m_numVertices);
      Matrix3D fullMatrix;
      fullMatrix.SetRotateZ(ANGTORAD(m_d.m_rotation));
      for (int i = 0; i < m_numVertices; i++)
      {
         Vertex3Ds vert(meshVertices[i].x, meshVertices[i].y, meshVertices[i].z);
         fullMatrix.MultiplyVector(vert, m_vertices[i]);
         if (m_d.m_shape != TriggerStar && m_d.m_shape != TriggerButton)
         {
            m_vertices[i].x *= m_d.m_scaleX;
            m_vertices[i].y *= m_d.m_scaleY;
         }
         else
         {
            m_vertices[i].x *= m_d.m_radius;
            m_vertices[i].y *= m_d.m_radius;
         }
         m_vertices[i].x += m_d.m_vCenter.x;
         m_vertices[i].y += m_d.m_vCenter.y;
      }
   }
}

void Trigger::InitShape(float x, float y)
{
   constexpr float lengthX = 30.0f;
   constexpr float lengthY = 30.0f;
   UpdateStatusBarInfo();
   for (size_t i = 0; i < m_vdpoint.size(); i++)
      m_vdpoint[i]->Release();
   m_vdpoint.clear();

   // First time shape has been set to custom - set up some points
   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x - lengthX, y - lengthY, 0.f, false);
      m_vdpoint.push_back(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x - lengthX, y + lengthY, 0.f, false);
      m_vdpoint.push_back(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x + lengthX, y + lengthY, 0.f, false);
      m_vdpoint.push_back(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x + lengthX, y - lengthY, 0.f, false);
      m_vdpoint.push_back(pdp);
   }
}

HRESULT Trigger::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;
   if (m_vdpoint.empty())
      InitShape(x, y);
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

void Trigger::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsTrigger

   m_d.m_radius = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Radius"s, 25.0f) : 25.0f;
   m_d.m_rotation = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Rotation"s, 0.f) : 0.f;
   m_d.m_wireThickness = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "WireThickness"s, 0.f) : 0.f;
   m_d.m_scaleX = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ScaleX"s, 1.f) : 1.f;
   m_d.m_scaleY = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ScaleY"s, 1.f) : 1.f;
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerInterval"s, 100) : 100;
   m_d.m_enabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Enabled"s, true) : true;
   m_d.m_visible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Visible"s, true) : true;
   m_d.m_hit_height = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "HitHeight"s, 50.f) : 50.f;
   m_d.m_shape = fromMouseClick ? (TriggerShape)g_pvp->m_settings.LoadValueWithDefault(regKey, "Shape"s, (int)TriggerWireA) : TriggerWireA;

   const bool hr = g_pvp->m_settings.LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if (!hr || !fromMouseClick)
      m_d.m_szSurface.clear();

   m_d.m_animSpeed = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "AnimSpeed"s, 1.f) : 1.f;
   m_d.m_reflectionEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ReflectionEnabled"s, true) : true;

#undef regKey
}

void Trigger::UIRenderPass1(Sur * const psur)
{
   if (m_vdpoint.empty())
      InitShape(m_d.m_vCenter.x, m_d.m_vCenter.y);

   psur->SetBorderColor(-1, false, 0);
   psur->SetObject(this);

   if (m_d.m_shape != TriggerStar && m_d.m_shape != TriggerButton)
   {
      psur->SetFillColor(m_ptable->RenderSolid() ? RGB(200, 220, 200) : -1);

      vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);

      psur->Polygon(vvertex);
   }
   else
   {
      psur->SetFillColor(-1);
      psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
   }
}

void Trigger::UIRenderPass2(Sur * const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);
   psur->SetFillColor(-1);

   if (m_d.m_shape != TriggerStar && m_d.m_shape != TriggerButton)
   {
      vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);

      psur->SetObject(nullptr);
      psur->SetBorderColor(RGB(0, 180, 0), false, 1);

      psur->Polygon(vvertex);

      bool drawDragpoints = (m_selectstate != eNotSelected) || (m_vpinball->m_alwaysDrawDragPoints);
      // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
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
      {
         for (size_t i = 0; i < m_vdpoint.size(); i++)
         {
            CComObject<DragPoint> * const pdp = m_vdpoint[i];
            psur->SetFillColor(-1);
            psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(0, 180, 0), false, 0);
            psur->SetObject(pdp);

            psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
         }
      }
   }
   else
   {
      psur->SetObject(nullptr);
      psur->SetBorderColor(RGB(0, 180, 0), false, 1);

      psur->Line(m_d.m_vCenter.x - m_d.m_radius, m_d.m_vCenter.y, m_d.m_vCenter.x + m_d.m_radius, m_d.m_vCenter.y);
      psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - m_d.m_radius, m_d.m_vCenter.x, m_d.m_vCenter.y + m_d.m_radius);

      const float r2 = m_d.m_radius * (float)sin(M_PI / 4.0);

      psur->Line(m_d.m_vCenter.x - r2, m_d.m_vCenter.y - r2, m_d.m_vCenter.x + r2, m_d.m_vCenter.y + r2);
      psur->Line(m_d.m_vCenter.x - r2, m_d.m_vCenter.y + r2, m_d.m_vCenter.x + r2, m_d.m_vCenter.y - r2);
   }

   if (m_d.m_shape == TriggerWireA || m_d.m_shape == TriggerWireB || m_d.m_shape == TriggerWireC || m_d.m_shape == TriggerWireD || m_d.m_shape == TriggerInder)
   {
      if (m_numIndices > 0)
      {
         const size_t numPts = m_numIndices / 3 + 1;
         vector<Vertex2D> drawVertices(numPts);

         const Vertex3Ds& A = m_vertices[m_faceIndices[0]];
         drawVertices[0] = Vertex2D(A.x, A.y);

         size_t o = 1;
         for (int i = 0; i < m_numIndices; i += 3, ++o)
         {
            const Vertex3Ds& B = m_vertices[m_faceIndices[i + 1]];
            drawVertices[o] = Vertex2D(B.x, B.y);
         }

         psur->Polyline(drawVertices.data(), (int)drawVertices.size());
      }
   }
}

void Trigger::RenderBlueprint(Sur *psur, const bool solid)
{
   if (solid)
      psur->SetFillColor(BLUEPRINT_SOLID_COLOR);
   else
      psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
}

void Trigger::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();
   m_phittimer = new HitTimer(GetName(), m_d.m_tdr.m_TimerInterval, this);
   if (m_d.m_tdr.m_TimerEnabled)
      pvht.push_back(m_phittimer);
}

#pragma region Physics

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Trigger/TriggerHitGenerator.cs
//

void Trigger::GetHitShapes(vector<HitObject*> &pvho)
{
   m_hitEnabled = m_d.m_enabled;

   if (m_d.m_shape == TriggerStar || m_d.m_shape == TriggerButton)
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

      m_ptriggerhitcircle = new TriggerHitCircle(m_d.m_vCenter, m_d.m_radius, height, height + m_d.m_hit_height);

      m_ptriggerhitcircle->m_enabled = m_d.m_enabled;
      m_ptriggerhitcircle->m_ObjType = eTrigger;
      m_ptriggerhitcircle->m_obj = (IFireEvents*)this;

      m_ptriggerhitcircle->m_ptrigger = this;

      pvho.push_back(m_ptriggerhitcircle);
   }
   else
      CurvesToShapes(pvho);
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Trigger::GetHitShapesDebug(vector<HitObject*> &pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   m_hitEnabled = m_d.m_enabled;
   switch (m_d.m_shape)
   {
   case TriggerButton:
   case TriggerStar:
   {
      Hit3DPoly * const pcircle = new Hit3DPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height + 10, m_d.m_radius, 32);
      pcircle->m_ObjType = eTrigger;
      pcircle->m_obj = (IFireEvents*)this;

      pvho.push_back(pcircle);
      break;
   }

   default:
   case TriggerWireA:
   case TriggerWireB:
   case TriggerWireC:
   case TriggerWireD:
   case TriggerInder:
   {
      vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);

      const int cvertex = (int)vvertex.size();
      Vertex3Ds * const rgv3d = new Vertex3Ds[cvertex];

      for (int i = 0; i < cvertex; i++)
      {
         rgv3d[i].x = vvertex[i].x;
         rgv3d[i].y = vvertex[i].y;
         rgv3d[i].z = height + (float)(PHYS_SKIN*2.0);
      }

      Hit3DPoly * const ph3dp = new Hit3DPoly(rgv3d, cvertex);
      ph3dp->m_ObjType = eTrigger;
      ph3dp->m_obj = (IFireEvents*)this;

      pvho.push_back(ph3dp);
      //ph3dp->m_enabled = false;	//!! disable hit process on polygon body, only trigger edges 
      break;
   }
   }
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Trigger/TriggerHitGenerator.cs
//

void Trigger::CurvesToShapes(vector<HitObject*> &pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   const int count = (int)vvertex.size();
   RenderVertex * const rgv = new RenderVertex[count];
   Vertex3Ds * const rgv3D = new Vertex3Ds[count];

   for (int i = 0; i < count; i++)
   {
      rgv[i] = vvertex[i];
      rgv3D[i].x = rgv[i].x;
      rgv3D[i].y = rgv[i].y;
      rgv3D[i].z = height + (float)(PHYS_SKIN*2.0);
   }
#if 1	
   for (int i = 0; i < count; i++)
   {
      const RenderVertex &pv2 = rgv[(i < count - 1) ? (i + 1) : 0];
      const RenderVertex &pv3 = rgv[(i < count - 2) ? (i + 2) : (i + 2 - count)];

      AddLine(pvho, pv2, pv3, height);
   }
#endif

#if 1	
   Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D, count);
   ph3dpoly->m_ObjType = eTrigger;
   ph3dpoly->m_obj = (IFireEvents*)this;

   pvho.push_back(ph3dpoly);
#else
   delete[] rgv3D;
#endif

   delete[] rgv;
}

void Trigger::AddLine(vector<HitObject*> &pvho, const RenderVertex &pv1, const RenderVertex &pv2, const float height)
{
   TriggerLineSeg * const plineseg = new TriggerLineSeg();

   plineseg->m_ptrigger = this;
   plineseg->m_ObjType = eTrigger;
   plineseg->m_obj = (IFireEvents*)this;

   plineseg->m_hitBBox.zlow  = height;
   plineseg->m_hitBBox.zhigh = height + max(m_d.m_hit_height - 8.0f, 0.f); //adjust for same hit height as circular

   plineseg->v1.x = pv1.x;
   plineseg->v1.y = pv1.y;
   plineseg->v2.x = pv2.x;
   plineseg->v2.y = pv2.y;

   pvho.push_back(plineseg);

   plineseg->CalcNormal();
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Trigger::EndPlay()
{
   IEditable::EndPlay();
   m_ptriggerhitcircle = nullptr;
}

void Trigger::TriggerAnimationHit()
{
   m_hitEvent = true;
}

void Trigger::TriggerAnimationUnhit()
{
   m_unhitEvent = true;
}

#pragma endregion


#pragma region Rendering

void Trigger::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;
   m_hitEvent = false;
   m_unhitEvent = false;
   m_doAnimation = false;
   m_moveDown = false;
   m_animHeightOffset = 0.0f;
   m_vertexBuffer_animHeightOffset = -FLT_MAX;

   if (!m_d.m_visible || m_d.m_shape == TriggerNone)
      return;

   const WORD* indices;
   switch(m_d.m_shape)
   {
   case TriggerWireA:
   case TriggerWireB:
   case TriggerWireC:
   {
      m_numVertices = triggerSimpleNumVertices;
      m_numIndices = triggerSimpleNumIndices;
      indices = triggerSimpleIndices;
      break;
   }
   case TriggerWireD:
   {
      m_numVertices = triggerDWireNumVertices;
      m_numIndices = triggerDWireNumIndices;
      indices = triggerDWireIndices;
      break;
   }
   case TriggerInder:
   {
      m_numVertices = triggerInderNumVertices;
      m_numIndices = triggerInderNumIndices;
      indices = triggerInderIndices;
      break;
   }
   case TriggerButton:
   {
      m_numVertices = triggerButtonNumVertices;
      m_numIndices = triggerButtonNumIndices;
      indices = triggerButtonIndices;
      break;
   }
   case TriggerStar:
   {
      m_numVertices = triggerStarNumVertices;
      m_numIndices = triggerStarNumIndices;
      indices = triggerStarIndices;
      break;
   }
   default:
   {
      assert(!"Unknown Trigger");
      break;
   }
   }

   GenerateMesh();
   IndexBuffer *triggerIndexBuffer = new IndexBuffer(m_rd, m_numIndices, indices);
   VertexBuffer *vertexBuffer = new VertexBuffer(m_rd, m_numVertices, (float*) m_triggerVertices, true);
   m_meshBuffer = new MeshBuffer(m_wzName, vertexBuffer, triggerIndexBuffer, true);
}

void Trigger::RenderRelease()
{
   assert(m_rd != nullptr);
   m_rd = nullptr;
   delete m_meshBuffer;
   delete[] m_triggerVertices;
   m_meshBuffer = nullptr;
   m_triggerVertices = nullptr;
}

//
// license:GPLv3+
// Ported at: VisualPinball.Unity/VisualPinball.Unity/VPT/Trigger/TriggerAnimationSystem.cs
//

void Trigger::UpdateAnimation(const float diff_time_msec)
{
   assert(m_rd != nullptr);
   float animLimit;
   switch (m_d.m_shape)
   {
      case TriggerStar:   animLimit = m_d.m_radius * (float)(1.0 /  5.0); break;
      case TriggerButton: animLimit = m_d.m_radius * (float)(1.0 / 10.0); break;
      case TriggerWireC:  animLimit = 60.0f; break;
      case TriggerWireD:  animLimit = 25.0f; break;
      case TriggerInder:  animLimit = 25.0f; break;
      default:            animLimit = 32.0f; break;
   }

   if (m_hitEvent)
   {
      m_doAnimation = true;
      m_hitEvent = false;
      // unhitEvent = false;   // Bugfix: If HitEvent and unhitEvent happen at the same time, you want to favor the unhit, otherwise the switch gets stuck down.
      m_animHeightOffset = 0.0f;
      m_moveDown = true;
   }
   if (m_unhitEvent)
   {
      m_doAnimation = true;
      m_unhitEvent = false;
      m_hitEvent = false;
      m_animHeightOffset = animLimit;
      m_moveDown = false;
   }

   if (m_doAnimation)
   {
      float step = diff_time_msec*m_d.m_animSpeed;
      if (m_moveDown)
         step = -step;
      m_animHeightOffset += step;

      if (m_moveDown)
      {
         if (m_animHeightOffset <= -animLimit)
         {
            m_animHeightOffset = -animLimit;
            m_doAnimation = false;
            m_moveDown = false;
         }
      }
      else
      {
         if (m_animHeightOffset >= 0.0f)
         {
            m_animHeightOffset = 0.0f;
            m_doAnimation = false;
            m_moveDown = true;
         }
      }
      FireGroupEvent(DISPID_AnimateEvents_Animate);
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Trigger::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Player::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Player::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Player::REFLECTION_PASS;
   TRACE_FUNCTION();

   if (!m_d.m_visible 
   || m_d.m_shape == TriggerNone 
   || isStaticOnly 
   || (isReflectionPass && !m_d.m_reflectionEnabled))
      return;
      
   if (m_animHeightOffset != m_vertexBuffer_animHeightOffset)
   {
      m_vertexBuffer_animHeightOffset = m_animHeightOffset;
      Vertex3D_NoTex2 *buf;
      m_meshBuffer->m_vb->lock(0, 0, (void **)&buf, VertexBuffer::DISCARDCONTENTS);
      for (int i = 0; i < m_numVertices; i++)
      {
         buf[i].x = m_triggerVertices[i].x;
         buf[i].y = m_triggerVertices[i].y;
         buf[i].z = m_triggerVertices[i].z + m_animHeightOffset;
         buf[i].nx = m_triggerVertices[i].nx;
         buf[i].ny = m_triggerVertices[i].ny;
         buf[i].nz = m_triggerVertices[i].nz;
         buf[i].tu = m_triggerVertices[i].tu;
         buf[i].tv = m_triggerVertices[i].tv;
      }
      m_meshBuffer->m_vb->unlock();
   }

   m_rd->ResetRenderState();
   if (m_d.m_shape == TriggerWireA || m_d.m_shape == TriggerWireB || m_d.m_shape == TriggerWireC || m_d.m_shape == TriggerWireD || m_d.m_shape == TriggerInder)
      m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_rd->basicShader->SetBasic(m_ptable->GetMaterial(m_d.m_szMaterial), nullptr);
   m_rd->DrawMesh(m_rd->basicShader, false, m_boundingSphereCenter, 0.f, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_numIndices);
}

#pragma endregion



void Trigger::ExportMesh(ObjLoader& loader)
{
   if (!m_d.m_visible || m_d.m_shape == TriggerNone)
      return;

   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);
   GenerateMesh();
   loader.WriteObjectName(name);
   loader.WriteVertexInfo(m_triggerVertices, m_numVertices);
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   loader.WriteMaterial(m_d.m_szMaterial, string(), mat);
   loader.UseTexture(m_d.m_szMaterial);

   const WORD* indices;
   switch(m_d.m_shape)
   {
   case TriggerWireA:
   case TriggerWireB:
   case TriggerWireC:
   {
      indices = triggerSimpleIndices;
      break;
   }
   case TriggerWireD:
   {
      indices = triggerDWireIndices;
      break;
   }
   case TriggerInder:
   {
      indices = triggerInderIndices;
      break;
   }
   case TriggerButton:
   {
      indices = triggerButtonIndices;
      break;
   }
   case TriggerStar:
   {
      indices = triggerStarIndices;
      break;
   }
   default:
   {
      assert(!"Unknown Trigger");
      break;
   }
   }

   loader.WriteFaceInfoList(indices, m_numIndices);
   loader.UpdateFaceOffset(m_numVertices);
   
   delete[] m_triggerVertices;
   m_triggerVertices = nullptr;
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Trigger/TriggerMeshGenerator.cs
//

void Trigger::GenerateMesh()
{
   // This create m_triggerVertices which must be disposed by caller
   assert(m_triggerVertices == nullptr);
   const float baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const Vertex3D_NoTex2 *verts;
   float zoffset = (m_d.m_shape == TriggerButton) ? 5.0f : 0.0f;
   if (m_d.m_shape == TriggerWireC) zoffset = -19.0f;

   m_boundingSphereCenter.Set(m_d.m_vCenter.x, m_d.m_vCenter.y, baseHeight);

   switch(m_d.m_shape)
   {
   case TriggerWireA:
   case TriggerWireB:
   case TriggerWireC:
   {
      m_numVertices = triggerSimpleNumVertices;
      m_numIndices = triggerSimpleNumIndices;
      verts = triggerSimple;
      break;
   }
   case TriggerWireD:
   {
      m_numVertices = triggerDWireNumVertices;
      m_numIndices = triggerDWireNumIndices;
      verts = triggerDWireMesh;
      break;
   }
   case TriggerInder:
   {
      m_numVertices = triggerInderNumVertices;
      m_numIndices = triggerInderNumIndices;
      verts = triggerInderMesh;
      break;
   }
   case TriggerButton:
   {
      m_numVertices = triggerButtonNumVertices;
      m_numIndices = triggerButtonNumIndices;
      verts = triggerButtonMesh;
      break;
   }
   case TriggerStar:
   {
      m_numVertices = triggerStarNumVertices;
      m_numIndices = triggerStarNumIndices;
      verts = triggerStar;
      break;
   }
   default:
   {
      ShowError("Unknown Trigger type");
      return;
   }
   }

   m_triggerVertices = new Vertex3D_NoTex2[m_numVertices];

   Matrix3D fullMatrix;
   if (m_d.m_shape == TriggerWireB)
   {
      Matrix3D tempMatrix;
      fullMatrix.SetRotateX(ANGTORAD(-23.f));
      tempMatrix.SetRotateZ(ANGTORAD(m_d.m_rotation));
      tempMatrix.Multiply(fullMatrix, fullMatrix);
   }
   else if (m_d.m_shape == TriggerWireC)
   {
      Matrix3D tempMatrix;
      fullMatrix.SetRotateX(ANGTORAD(140.f));
      tempMatrix.SetRotateZ(ANGTORAD(m_d.m_rotation));
      tempMatrix.Multiply(fullMatrix, fullMatrix);
   }
   else
      fullMatrix.SetRotateZ(ANGTORAD(m_d.m_rotation));

   for (int i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(verts[i].x, verts[i].y, verts[i].z);
      vert = fullMatrix.MultiplyVector(vert);

      if (m_d.m_shape == TriggerButton || m_d.m_shape == TriggerStar)
      {
         m_triggerVertices[i].x = (vert.x*m_d.m_radius) + m_d.m_vCenter.x;
         m_triggerVertices[i].y = (vert.y*m_d.m_radius) + m_d.m_vCenter.y;
         m_triggerVertices[i].z = (vert.z*m_d.m_radius) + baseHeight+zoffset;
      }
      else 
      {
         m_triggerVertices[i].x = (vert.x*m_d.m_scaleX) + m_d.m_vCenter.x;
         m_triggerVertices[i].y = (vert.y*m_d.m_scaleY) + m_d.m_vCenter.y;
         m_triggerVertices[i].z = (vert.z*1.0f) + baseHeight+zoffset;
      }

      vert = Vertex3Ds(verts[i].nx, verts[i].ny, verts[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      m_triggerVertices[i].nx = vert.x;
      m_triggerVertices[i].ny = vert.y;
      m_triggerVertices[i].nz = vert.z;
      m_triggerVertices[i].tu = verts[i].tu;
      m_triggerVertices[i].tv = verts[i].tv;

      if (m_d.m_shape == TriggerWireA || m_d.m_shape == TriggerWireB || m_d.m_shape == TriggerWireC || m_d.m_shape == TriggerWireD || m_d.m_shape == TriggerInder)
      {
         m_triggerVertices[i].x += m_triggerVertices[i].nx*m_d.m_wireThickness;
         m_triggerVertices[i].y += m_triggerVertices[i].ny*m_d.m_wireThickness;
         m_triggerVertices[i].z += m_triggerVertices[i].nz*m_d.m_wireThickness;
      }
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Trigger::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Trigger::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint[i];

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }

   UpdateStatusBarInfo();
}

Vertex2D Trigger::GetPointCenter() const
{
   return m_d.m_vCenter;
}

void Trigger::PutPointCenter(const Vertex2D& pv)
{
   m_d.m_vCenter = pv;
}

void Trigger::EditMenu(CMenu &menu)
{
#ifndef __STANDALONE__
   menu.EnableMenuItem(ID_WALLMENU_FLIP, MF_BYCOMMAND | MF_ENABLED);
   menu.EnableMenuItem(ID_WALLMENU_MIRROR, MF_BYCOMMAND | MF_ENABLED);
   menu.EnableMenuItem(ID_WALLMENU_ROTATE, MF_BYCOMMAND | MF_ENABLED);
   menu.EnableMenuItem(ID_WALLMENU_SCALE, MF_BYCOMMAND | MF_ENABLED);
   menu.EnableMenuItem(ID_WALLMENU_ADDPOINT, MF_BYCOMMAND | MF_ENABLED);
#endif
}

void Trigger::DoCommand(int icmd, int x, int y)
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
   {
      STARTUNDO

      const Vertex2D v = m_ptable->TransformPoint(x, y);

      vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);

      int iSeg;
      Vertex2D vOut;
      ClosestPointOnPolygon(vvertex, v, vOut, iSeg, true);

      // Go through vertices (including iSeg itself) counting control points until iSeg
      int icp = 0;
      for (int i = 0; i < (iSeg + 1); i++)
         if (vvertex[i].controlPoint)
            icp++;

      //if (icp == 0) // need to add point after the last point
      //icp = m_vdpoint.size();

      CComObject<DragPoint> *pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, vOut.x, vOut.y, 0.f, false);
         m_vdpoint.insert(m_vdpoint.begin() + icp, pdp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
      }

      STOPUNDO
   }
   break;
   }
}

void Trigger::FlipY(const Vertex2D& pvCenter)
{
   if (m_d.m_shape == TriggerNone)
      IHaveDragPoints::FlipPointY(pvCenter);
}

void Trigger::FlipX(const Vertex2D& pvCenter)
{
   if (m_d.m_shape == TriggerNone)
      IHaveDragPoints::FlipPointX(pvCenter);
}

void Trigger::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   if (m_d.m_shape == TriggerNone)
      IHaveDragPoints::RotatePoints(ang, pvCenter, useElementCenter);
   else
   {
      STARTUNDOSELECT
      m_d.m_rotation = ang;
      STOPUNDOSELECT
      UpdateStatusBarInfo();
   }
}

void Trigger::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   if (m_d.m_shape == TriggerNone)
      IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter, useElementCenter);
   else
   {
      STARTUNDOSELECT
      m_d.m_scaleX = scalex;
      m_d.m_scaleY = scaley;
      STOPUNDOSELECT
      UpdateStatusBarInfo();
   }
}

void Trigger::Translate(const Vertex2D &pvOffset)
{
   if (m_d.m_shape == TriggerNone)
      IHaveDragPoints::TranslatePoints(pvOffset);
   else
   {
      STARTUNDOSELECT
      MoveOffset(pvOffset.x, pvOffset.y);
      STOPUNDOSELECT
   }
}

HRESULT Trigger::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_vCenter);
   bw.WriteFloat(FID(RADI), m_d.m_radius);
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteFloat(FID(WITI), m_d.m_wireThickness);
   bw.WriteFloat(FID(SCAX), m_d.m_scaleX);
   bw.WriteFloat(FID(SCAY), m_d.m_scaleY);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(EBLD), m_d.m_enabled);
   bw.WriteBool(FID(VSBL), m_d.m_visible);
   bw.WriteFloat(FID(THOT), m_d.m_hit_height);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteInt(FID(SHAP), m_d.m_shape);
   bw.WriteFloat(FID(ANSP), m_d.m_animSpeed);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);

   ISelect::SaveData(pstm, hcrypthash);

   HRESULT hr;
   if (FAILED(hr = SavePointData(pstm, hcrypthash)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

void Trigger::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

void Trigger::WriteRegDefaults()
{
#define regKey Settings::DefaultPropsTrigger

   g_pvp->m_settings.SaveValue(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   g_pvp->m_settings.SaveValue(regKey, "Enabled"s, m_d.m_enabled);
   g_pvp->m_settings.SaveValue(regKey, "Visible"s, m_d.m_visible);
   g_pvp->m_settings.SaveValue(regKey, "HitHeight"s, m_d.m_hit_height);
   g_pvp->m_settings.SaveValue(regKey, "Radius"s, m_d.m_radius);
   g_pvp->m_settings.SaveValue(regKey, "Rotation"s, m_d.m_rotation);
   g_pvp->m_settings.SaveValue(regKey, "WireThickness"s, m_d.m_wireThickness);
   g_pvp->m_settings.SaveValue(regKey, "ScaleX"s, m_d.m_scaleX);
   g_pvp->m_settings.SaveValue(regKey, "ScaleY"s, m_d.m_scaleY);
   g_pvp->m_settings.SaveValue(regKey, "Shape"s, m_d.m_shape);
   g_pvp->m_settings.SaveValue(regKey, "Surface"s, m_d.m_szSurface);
   g_pvp->m_settings.SaveValue(regKey, "AnimSpeed"s, m_d.m_animSpeed);
   g_pvp->m_settings.SaveValue(regKey, "ReflectionEnabled"s, m_d.m_reflectionEnabled);

#undef regKey
}

HRESULT Trigger::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   UpdateStatusBarInfo();
   return S_OK;
}

bool Trigger::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VCEN): pbr->GetVector2(m_d.m_vCenter); break;
   case FID(RADI): pbr->GetFloat(m_d.m_radius); break;
   case FID(ROTA): pbr->GetFloat(m_d.m_rotation); break;
   case FID(WITI): pbr->GetFloat(m_d.m_wireThickness); break;
   case FID(SCAX): pbr->GetFloat(m_d.m_scaleX); break;
   case FID(SCAY): pbr->GetFloat(m_d.m_scaleY); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(EBLD): pbr->GetBool(m_d.m_enabled); break;
   case FID(THOT): pbr->GetFloat(m_d.m_hit_height); break;
   case FID(VSBL): pbr->GetBool(m_d.m_visible); break;
   case FID(REEN): pbr->GetBool(m_d.m_reflectionEnabled); break;
   case FID(SHAP): pbr->GetInt(&m_d.m_shape); break;
   case FID(ANSP): pbr->GetFloat(m_d.m_animSpeed); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   default:
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
      break;
   }
   }
   return true;
}

HRESULT Trigger::InitPostLoad()
{
   UpdateStatusBarInfo();
   return S_OK;
}

STDMETHODIMP Trigger::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_ITrigger,
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Trigger::get_Radius(float *pVal)
{
   *pVal = m_d.m_radius;

   return S_OK;
}

STDMETHODIMP Trigger::put_Radius(float newVal)
{
   m_d.m_radius = newVal;

   return S_OK;
}

STDMETHODIMP Trigger::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;
   m_vpinball->SetStatusBarUnitInfo(string(), true);

   return S_OK;
}

STDMETHODIMP Trigger::put_X(float newVal)
{
   m_d.m_vCenter.x = newVal;

   return S_OK;
}

STDMETHODIMP Trigger::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Trigger::put_Y(float newVal)
{
   m_d.m_vCenter.y = newVal;

   return S_OK;
}

STDMETHODIMP Trigger::get_Surface(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSurface.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Trigger::put_Surface(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_d.m_szSurface = buf;

   return S_OK;
}

STDMETHODIMP Trigger::get_Enabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB((g_pplayer) ? m_hitEnabled : m_d.m_enabled);

   return S_OK;
}

STDMETHODIMP Trigger::put_Enabled(VARIANT_BOOL newVal)
{
   if (g_pplayer)
   {
      m_hitEnabled = VBTOb(newVal);

      if (m_ptriggerhitcircle) m_ptriggerhitcircle->m_enabled = m_hitEnabled;
   }
   else
   {
      m_d.m_enabled = VBTOb(newVal);
      m_hitEnabled = m_d.m_enabled;
   }

   return S_OK;
}

STDMETHODIMP Trigger::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_visible);

   return S_OK;
}

STDMETHODIMP Trigger::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_visible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Trigger::BallCntOver(int *pVal)
{
   int cnt = 0;

   if (g_pplayer)
   {
      for (size_t i = 0; i < g_pplayer->m_vball.size(); i++)
      {
         Ball * const pball = g_pplayer->m_vball[i];

         if (pball->m_d.m_vpVolObjs && FindIndexOf(*(pball->m_d.m_vpVolObjs), (IFireEvents*)this) >= 0) // cast to IFireEvents necessary, as it is stored like this in HitObject.m_obj
         {
            g_pplayer->m_pactiveball = pball; // set active ball for scriptor
            ++cnt;
         }
      }
   }

   *pVal = cnt;

   return S_OK;
}

STDMETHODIMP Trigger::DestroyBall(int *pVal)
{
   int cnt = 0;

   if (g_pplayer)
   {
      for (size_t i = 0; i < g_pplayer->m_vball.size(); i++)
      {
         Ball * const pball = g_pplayer->m_vball[i];

         int j;
         if (pball->m_d.m_vpVolObjs && (j = FindIndexOf(*(pball->m_d.m_vpVolObjs), (IFireEvents*)this)) >= 0) // cast to IFireEvents necessary, as it is stored like this in HitObject.m_obj
         {
            ++cnt;
            pball->m_d.m_vpVolObjs->erase(pball->m_d.m_vpVolObjs->begin() + j);
            g_pplayer->DestroyBall(pball); // inside trigger volume?
         }
      }
   }

   if (pVal) *pVal = cnt;

   return S_OK;
}

STDMETHODIMP Trigger::get_HitHeight(float *pVal)
{
   *pVal = m_d.m_hit_height;

   return S_OK;
}

STDMETHODIMP Trigger::put_HitHeight(float newVal)
{
   m_d.m_hit_height = newVal;

   return S_OK;
}

STDMETHODIMP Trigger::get_Rotation(float *pVal)
{
   *pVal = m_d.m_rotation;

   return S_OK;
}

STDMETHODIMP Trigger::put_Rotation(float newVal)
{
   m_d.m_rotation = newVal;
   UpdateStatusBarInfo();

   return S_OK;
}

STDMETHODIMP Trigger::get_WireThickness(float *pVal)
{
   *pVal = m_d.m_wireThickness;

   return S_OK;
}

STDMETHODIMP Trigger::put_WireThickness(float newVal)
{
   m_d.m_wireThickness = newVal;

   return S_OK;
}

STDMETHODIMP Trigger::get_AnimSpeed(float *pVal)
{
   *pVal = m_d.m_animSpeed;

   return S_OK;
}

STDMETHODIMP Trigger::put_AnimSpeed(float newVal)
{
   m_d.m_animSpeed = newVal;

   return S_OK;
}

STDMETHODIMP Trigger::get_CurrentAnimOffset(float *pVal)
{
   *pVal = m_animHeightOffset;

   return S_OK;
}

STDMETHODIMP Trigger::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Trigger::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szMaterial = buf;

   return S_OK;
}

STDMETHODIMP Trigger::get_TriggerShape(TriggerShape *pVal)
{
   *pVal = m_d.m_shape;

   return S_OK;
}

STDMETHODIMP Trigger::put_TriggerShape(TriggerShape newVal)
{
   m_d.m_shape = newVal;
   UpdateStatusBarInfo();

   return S_OK;
}

STDMETHODIMP Trigger::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP Trigger::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);

   return S_OK;
}
