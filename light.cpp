#include "stdafx.h"
#include "meshes/bulbLightMesh.h"
#include "meshes/bulbSocketMesh.h"
#include "Shader.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

Light::Light() : m_lightcenter(this)
{
   m_menuid = IDR_SURFACEMENU;
   m_customMoverVBuffer = nullptr;
   m_customMoverIBuffer = nullptr;
   m_bulbLightIndexBuffer = nullptr;
   m_bulbLightVBuffer = nullptr;
   m_bulbSocketIndexBuffer = nullptr;
   m_bulbSocketVBuffer = nullptr;
   m_d.m_depthBias = 0.0f;
   m_d.m_shape = ShapeCustom;
   m_d.m_visible = true;
   m_roundLight = false;
   m_propVisual = nullptr;
   m_updateBulbLightHeight = false;
   m_maxDist = 0.0f;
}

Light::~Light()
{
   SAFE_BUFFER_RELEASE(m_customMoverVBuffer);
   SAFE_BUFFER_RELEASE(m_customMoverIBuffer);
   SAFE_BUFFER_RELEASE(m_bulbLightIndexBuffer);
   SAFE_BUFFER_RELEASE(m_bulbLightVBuffer);
   SAFE_BUFFER_RELEASE(m_bulbSocketIndexBuffer);
   SAFE_BUFFER_RELEASE(m_bulbSocketVBuffer);
}

HRESULT Light::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   InitShape();

   m_lockedByLS = false;
   m_inPlayState = m_d.m_state;
   m_d.m_visible = true;

   return InitVBA(fTrue, 0, nullptr);
}

void Light::SetDefaults(bool fromMouseClick)
{
#define regKey regKey[RegName::DefaultPropsLight]

   m_duration = 0;
   m_finalState = 0;

   m_d.m_falloff = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Falloff"s, 50.f) : 50.f;
   m_d.m_falloff_power = fromMouseClick ? LoadValueFloatWithDefault(regKey, "FalloffPower"s, 2.0f) : 2.0f;
   m_d.m_state = fromMouseClick ? (LightState)LoadValueIntWithDefault(regKey, "LightState"s, LightStateOff) : LightStateOff;

   m_d.m_shape = ShapeCustom;

   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault(regKey, "TimerInterval"s, 100) : 100;
   m_d.m_color = fromMouseClick ? LoadValueIntWithDefault(regKey, "Color"s, RGB(255,255,0)) : RGB(255,255,0);
   m_d.m_color2 = fromMouseClick ? LoadValueIntWithDefault(regKey, "ColorFull"s, RGB(255,255,255)) : RGB(255,255,255);

   HRESULT hr = LoadValue(regKey, "OffImage"s, m_d.m_szImage);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage.clear();

   hr = LoadValue(regKey, "BlinkPattern"s, m_rgblinkpattern);
   if ((hr != S_OK) || !fromMouseClick)
      m_rgblinkpattern = "10";

   m_blinkinterval = fromMouseClick ? LoadValueIntWithDefault(regKey, "BlinkInterval"s, 125) : 125;
   m_d.m_intensity = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Intensity"s, 1.0f) : 1.0f;
   m_d.m_transmissionScale = fromMouseClick ? LoadValueFloatWithDefault(regKey, "TransmissionScale"s, 0.5f) : 0.f; // difference in defaults is intended

   m_d.m_intensity_scale = 1.0f;

   //m_d.m_bordercolor = fromMouseClick ? LoadValueIntWithDefault(regKey, "BorderColor"s, RGB(0,0,0)) : RGB(0,0,0);

   hr = LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface.clear();

   m_d.m_fadeSpeedUp = fromMouseClick ? LoadValueFloatWithDefault(regKey, "FadeSpeedUp"s, 0.2f) : 0.2f;
   m_d.m_fadeSpeedDown = fromMouseClick ? LoadValueFloatWithDefault(regKey, "FadeSpeedDown"s, 0.2f) : 0.2f;
   m_d.m_BulbLight = fromMouseClick ? LoadValueBoolWithDefault(regKey, "Bulb"s, false) : false;
   m_d.m_imageMode = fromMouseClick ? LoadValueBoolWithDefault(regKey, "ImageMode"s, false) : false;
   m_d.m_showBulbMesh = fromMouseClick ? LoadValueBoolWithDefault(regKey, "ShowBulbMesh"s, false) : false;
   m_d.m_staticBulbMesh = fromMouseClick ? LoadValueBoolWithDefault(regKey, "StaticBulbMesh"s, true) : true;
   m_d.m_showReflectionOnBall = fromMouseClick ? LoadValueBoolWithDefault(regKey, "ShowReflectionOnBall"s, true) : true;
   m_d.m_meshRadius = fromMouseClick ? LoadValueFloatWithDefault(regKey, "ScaleBulbMesh"s, 20.0f) : 20.0f;
   m_d.m_modulate_vs_add = fromMouseClick ? LoadValueFloatWithDefault(regKey, "BulbModulateVsAdd"s, 0.9f) : 0.9f;
   m_d.m_bulbHaloHeight = fromMouseClick ? LoadValueFloatWithDefault(regKey, "BulbHaloHeight"s, 28.0f) : 28.0f;

#undef regKey
}

void Light::WriteRegDefaults()
{
#define regKey regKey[RegName::DefaultPropsLight]

   SaveValueFloat(regKey, "Falloff"s, m_d.m_falloff);
   SaveValueFloat(regKey, "FalloffPower"s, m_d.m_falloff_power);
   SaveValueInt(regKey, "LightState"s, m_d.m_state);
   SaveValueBool(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   SaveValueInt(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   SaveValueInt(regKey, "Color"s, m_d.m_color);
   SaveValueInt(regKey, "ColorFull"s, m_d.m_color2);
   SaveValue(regKey, "OffImage"s, m_d.m_szImage);
   SaveValue(regKey, "BlinkPattern"s, m_rgblinkpattern);
   SaveValueInt(regKey, "BlinkInterval"s, m_blinkinterval);
   //SaveValueInt(regKey,"BorderColor"s, m_d.m_bordercolor);
   SaveValue(regKey, "Surface"s, m_d.m_szSurface);
   SaveValueFloat(regKey, "FadeSpeedUp"s, m_d.m_fadeSpeedUp);
   SaveValueFloat(regKey, "FadeSpeedDown"s, m_d.m_fadeSpeedDown);
   SaveValueFloat(regKey, "Intensity"s, m_d.m_intensity);
   SaveValueFloat(regKey, "TransmissionScale"s, m_d.m_transmissionScale);
   SaveValueBool(regKey, "Bulb"s, m_d.m_BulbLight);
   SaveValueBool(regKey, "ImageMode"s, m_d.m_imageMode);
   SaveValueBool(regKey, "ShowBulbMesh"s, m_d.m_showBulbMesh);
   SaveValueBool(regKey, "StaticBulbMesh"s, m_d.m_staticBulbMesh);
   SaveValueBool(regKey, "ShowReflectionOnBall"s, m_d.m_showReflectionOnBall);
   SaveValueFloat(regKey, "ScaleBulbMesh"s, m_d.m_meshRadius);
   SaveValueFloat(regKey, "BulbModulateVsAdd"s, m_d.m_modulate_vs_add);
   SaveValueFloat(regKey, "BulbHaloHeight"s, m_d.m_bulbHaloHeight);

#undef regKey
}

void Light::UIRenderPass1(Sur * const psur)
{
   psur->SetBorderColor(-1, false, 0);
   psur->SetFillColor(m_ptable->RenderSolid() ? (((m_d.m_color & 0xFEFEFE) + (m_d.m_color2 & 0xFEFEFE)) / 2) : -1);
   psur->SetObject(this);

   // workaround for the old round light object
   // after loading m_roundLight is true if an pre-VPX table was loaded
   // init the round light to the new custom one
   if (m_roundLight)
   {
      InitShape();
      m_roundLight = false;
   }

   switch (m_d.m_shape)
   {
   default:
   case ShapeCustom:
      vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);

      // Check if we should display the image in the editor.
      psur->Polygon(vvertex);

      break;
   }
}

void Light::UIRenderPass2(Sur * const psur)
{
   bool drawDragpoints = ((m_selectstate != eNotSelected) || (m_vpinball->m_alwaysDrawDragPoints));

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

   RenderOutline(psur);

   if ((m_d.m_shape == ShapeCustom) && drawDragpoints)
   {
      for (size_t i = 0; i < m_vdpoint.size(); i++)
      {
         CComObject<DragPoint> * const pdp = m_vdpoint[i];
         psur->SetFillColor(-1);
         psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(0, 0, 200), false, 0);
         psur->SetObject(pdp);

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void Light::RenderOutline(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(nullptr);

   switch (m_d.m_shape)
   {
   case ShapeCircle:
   default:
   {
      psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_falloff /*+ m_d.m_borderwidth*/);
      break;
   }

   case ShapeCustom:
   {
      vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);
      psur->SetBorderColor(RGB(255, 0, 0), false, 0);
      psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_falloff /*+ m_d.m_borderwidth*/);
      psur->SetBorderColor(RGB(0, 0, 0), false, 0);
      psur->Polygon(vvertex);

      psur->SetObject((ISelect *)&m_lightcenter);
      break;
   }
   }

   if (m_d.m_shape == ShapeCustom || m_vpinball->m_alwaysDrawLightCenters)
   {
      psur->Line(m_d.m_vCenter.x - 10.0f, m_d.m_vCenter.y, m_d.m_vCenter.x + 10.0f, m_d.m_vCenter.y);
      psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - 10.0f, m_d.m_vCenter.x, m_d.m_vCenter.y + 10.0f);
   }
}

void Light::RenderBlueprint(Sur *psur, const bool solid)
{
   RenderOutline(psur);
}

void Light::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_TimerEnabled)
      pvht.push_back(pht);
}

void Light::GetHitShapes(vector<HitObject*> &pvho)
{
}

void Light::GetHitShapesDebug(vector<HitObject*> &pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   switch (m_d.m_shape)
   {
   case ShapeCircle:
   default: {
      Hit3DPoly * const pcircle = new Hit3DPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height, m_d.m_falloff, 32);
      pvho.push_back(pcircle);

      break;
   }

   case ShapeCustom: {
      vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);

      const int cvertex = (int)vvertex.size();
      Vertex3Ds * const rgv3d = new Vertex3Ds[cvertex];

      for (int i = 0; i < cvertex; i++)
      {
         rgv3d[i].x = vvertex[i].x;
         rgv3d[i].y = vvertex[i].y;
         rgv3d[i].z = height;
      }

      Hit3DPoly * const ph3dp = new Hit3DPoly(rgv3d, cvertex);
      pvho.push_back(ph3dp);

      break;
   }
   }
}

void Light::FreeBuffers()
{
   SAFE_BUFFER_RELEASE(m_customMoverVBuffer);
   SAFE_BUFFER_RELEASE(m_customMoverIBuffer);
   SAFE_BUFFER_RELEASE(m_bulbLightIndexBuffer);
   SAFE_BUFFER_RELEASE(m_bulbLightVBuffer);
   SAFE_BUFFER_RELEASE(m_bulbSocketIndexBuffer);
   SAFE_BUFFER_RELEASE(m_bulbSocketVBuffer);
}

void Light::EndPlay()
{
   // ensure not locked just in case the player exits during a LS sequence
   m_lockedByLS = false;

   m_vvertex.clear();

   IEditable::EndPlay();
   FreeBuffers();
}

float Light::GetDepth(const Vertex3Ds& viewDir) const
{
   return !m_backglass ? (m_d.m_depthBias + viewDir.x * m_d.m_vCenter.x + viewDir.y * m_d.m_vCenter.y + viewDir.z * m_surfaceHeight) : 0.f;
}

void Light::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

void Light::RenderBulbMesh()
{
   Material mat;
   mat.m_cBase = 0x181818;
   mat.m_fWrapLighting = 0.5f;
   mat.m_bOpacityActive = false;
   mat.m_fOpacity = 1.0f;
   mat.m_cGlossy = 0xB4B4B4;
   mat.m_bIsMetal = false;
   mat.m_fEdge = 1.0f;
   mat.m_fEdgeAlpha = 1.0f;
   mat.m_fRoughness = 0.9f;
   mat.m_fGlossyImageLerp = 1.0f;
   mat.m_fThickness = 0.05f;
   mat.m_cClearcoat = 0;

   RenderDevice * const pd3dDevice = m_backglass ? g_pplayer->m_pin3d.m_pd3dSecondaryDevice : g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_without_texture, mat.m_bIsMetal);
   pd3dDevice->basicShader->SetMaterial(&mat, false);

   pd3dDevice->basicShader->Begin();
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_bulbSocketVBuffer, 0, bulbSocketNumVertices, m_bulbSocketIndexBuffer, 0, bulbSocketNumFaces);
   pd3dDevice->basicShader->End();

   mat.m_cBase = 0;
   mat.m_fWrapLighting = 0.5f;
   mat.m_bOpacityActive = true;
   mat.m_fOpacity = 0.2f;
   mat.m_cGlossy = 0xFFFFFF;
   mat.m_bIsMetal = false;
   mat.m_fEdge = 1.0f;
   mat.m_fEdgeAlpha = 1.0f;
   mat.m_fRoughness = 0.9f;
   mat.m_fGlossyImageLerp = 1.0f;
   mat.m_fThickness = 0.05f;
   mat.m_cClearcoat = 0xFFFFFF;
   pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_without_texture, mat.m_bIsMetal);
   pd3dDevice->basicShader->SetMaterial(&mat, false);

   pd3dDevice->basicShader->Begin();
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_bulbLightVBuffer, 0, bulbLightNumVertices, m_bulbLightIndexBuffer, 0, bulbLightNumFaces);
   pd3dDevice->basicShader->End();
}

void Light::RenderDynamic()
{
   RenderDevice * const pd3dDevice = m_backglass ? g_pplayer->m_pin3d.m_pd3dSecondaryDevice : g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   TRACE_FUNCTION();

   const U32 old_time_msec = (m_d.m_time_msec < g_pplayer->m_time_msec) ? m_d.m_time_msec : g_pplayer->m_time_msec;
   m_d.m_time_msec = g_pplayer->m_time_msec;
   const float diff_time_msec = (float)(g_pplayer->m_time_msec - old_time_msec);

   if ((m_duration > 0) && (m_timerDurationEndTime < m_d.m_time_msec))
   {
      m_inPlayState = (LightState)m_finalState;
      m_duration = 0;
      if (m_inPlayState == LightStateBlinking)
         RestartBlinker(g_pplayer->m_time_msec);
   }
   if (m_inPlayState == LightStateBlinking)
      UpdateBlinker(g_pplayer->m_time_msec);

   const bool isOn = (m_inPlayState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : (m_inPlayState != LightStateOff);

   if (isOn)
   {
      if (m_d.m_currentIntensity < m_d.m_intensity*m_d.m_intensity_scale)
      {
         m_d.m_currentIntensity += m_d.m_fadeSpeedUp*diff_time_msec;
         if (m_d.m_currentIntensity > m_d.m_intensity*m_d.m_intensity_scale)
            m_d.m_currentIntensity = m_d.m_intensity*m_d.m_intensity_scale;
      }
   }
   else
   {
      if (m_d.m_currentIntensity > 0.0f)
      {
         m_d.m_currentIntensity -= m_d.m_fadeSpeedDown*diff_time_msec;
         if (m_d.m_currentIntensity < 0.0f)
            m_d.m_currentIntensity = 0.0f;
      }
   }

   if (!m_d.m_visible || m_ptable->m_reflectionEnabled)
      return;

   if (m_customMoverVBuffer == nullptr) // in case of degenerate light
      return;

   if (m_backglass && !GetPTable()->GetDecalsEnabled())
      return;

   if (m_d.m_BulbLight && m_d.m_showBulbMesh && !m_d.m_staticBulbMesh && g_pplayer->m_current_renderstage == 0)
      RenderBulbMesh();

   Texture *offTexel = nullptr;

   // early out all lights with no contribution
   const vec4 lightColor2_falloff_power = convertColor(m_d.m_color2, m_d.m_falloff_power);
   vec4 lightColor_intensity = convertColor(m_d.m_color);
   if (m_d.m_BulbLight ||
      (!m_d.m_BulbLight && (m_surfaceTexture == (offTexel = m_ptable->GetImage(m_d.m_szImage))) && (offTexel != nullptr) && !m_backglass && !m_d.m_imageMode)) // assumes/requires that the light in this kind of state is basically -exactly- the same as the static/(un)lit playfield/surface and accompanying image
   {
      if (m_d.m_currentIntensity == 0.f)
         return;
      if (lightColor_intensity.x == 0.f && lightColor_intensity.y == 0.f && lightColor_intensity.z == 0.f &&
         lightColor2_falloff_power.x == 0.f && lightColor2_falloff_power.y == 0.f && lightColor2_falloff_power.z == 0.f)
         return;
   }

   if (!m_backglass)
   {
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);
      constexpr float depthbias = -1.0f;
      pd3dDevice->SetRenderStateDepthBias(depthbias);
   }
   else
   {
      pd3dDevice->SetRenderStateDepthBias(0.0f);
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   }

   if (m_backglass && (m_ptable->m_tblMirrorEnabled^m_ptable->m_reflectionEnabled))
      pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_NONE);
   else
      pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_CCW);

   Vertex2D centerHUD;
   centerHUD.x = m_d.m_vCenter.x;
   centerHUD.y = m_d.m_vCenter.y;
   if (m_backglass)
   {
      const float  mult = getBGxmult();
      const float ymult = getBGymult();

      centerHUD.x = centerHUD.x* mult - 0.5f;
      centerHUD.y = centerHUD.y*ymult - 0.5f;
   }
   const vec4 center_range(centerHUD.x, centerHUD.y, !m_backglass ? m_surfaceHeight + 0.05f : 0.0f, 1.0f / max(m_d.m_falloff, 0.1f));

   if (!m_d.m_BulbLight)
   {
      pd3dDevice->classicLightShader->SetLightData(center_range);
      pd3dDevice->classicLightShader->SetLightColor2FalloffPower(lightColor2_falloff_power);

      pd3dDevice->classicLightShader->SetLightImageBackglassMode(m_d.m_imageMode, m_backglass);
      pd3dDevice->classicLightShader->SetMaterial(m_surfaceMaterial);

      if (offTexel != nullptr)
      {
         pd3dDevice->classicLightShader->SetTechniqueMetal(SHADER_TECHNIQUE_light_with_texture, m_surfaceMaterial->m_bIsMetal);
         pd3dDevice->classicLightShader->SetTexture(SHADER_tex_light_color, offTexel, SF_TRILINEAR, SA_CLAMP, SA_CLAMP);
         // Was: if (m_ptable->m_reflectElementsOnPlayfield && g_pplayer->m_pf_refl && !m_backglass)*/
         // TOTAN and Flintstones inserts break if alpha blending is disabled here.
         // Also see below if changing again
         if (!m_backglass)
         {
            pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_TRUE);
            pd3dDevice->SetRenderState(RenderDevice::SRCBLEND, RenderDevice::ONE);
            pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::ONE);
         }
      }
      else
         pd3dDevice->classicLightShader->SetTechniqueMetal(SHADER_TECHNIQUE_light_without_texture, m_surfaceMaterial->m_bIsMetal);
   }
   else
   {
      pd3dDevice->lightShader->SetLightData(center_range);
      pd3dDevice->lightShader->SetLightColor2FalloffPower(lightColor2_falloff_power);

      pd3dDevice->lightShader->SetTechnique(SHADER_TECHNIQUE_bulb_light);

      const Pin3D * const ppin3d = &g_pplayer->m_pin3d;
      ppin3d->EnableAlphaBlend(false, false, false);
      //pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  RenderDevice::SRC_ALPHA);  // add the lightcontribution
      pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::INVSRC_COLOR); // but also modulate the light first with the underlying elements by (1+lightcontribution, e.g. a very crude approximation of real lighting)
      pd3dDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_REVSUBTRACT);

      if (m_d.m_showBulbMesh) // blend bulb mesh hull additive over "normal" bulb to approximate the emission directly reaching the camera
      {
         lightColor_intensity.w = m_d.m_currentIntensity*0.02f; //!! make configurable?
         if (g_pplayer->m_current_renderstage == 1)
            lightColor_intensity.w *= m_d.m_transmissionScale;
         pd3dDevice->lightShader->SetLightColorIntensity(lightColor_intensity);
         pd3dDevice->lightShader->SetFloat(SHADER_blend_modulate_vs_add, 0.00001f); // additive, but avoid full 0, as it disables the blend

         pd3dDevice->lightShader->Begin();
         pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_bulbLightVBuffer, 0, bulbLightNumVertices, m_bulbLightIndexBuffer, 0, bulbLightNumFaces);
         pd3dDevice->lightShader->End();
      }

      pd3dDevice->lightShader->SetFloat(SHADER_blend_modulate_vs_add, (g_pplayer->m_current_renderstage == 0) ? min(max(m_d.m_modulate_vs_add, 0.00001f), 0.9999f) : 0.00001f); // avoid 0, as it disables the blend and avoid 1 as it looks not good with day->night changes // in the separate bulb light render stage only enable additive
   }

   // (maybe) update, then render light shape
   if (m_updateBulbLightHeight)
   {
      if (m_d.m_BulbLight && !m_backglass)
         UpdateCustomMoverVBuffer();

      m_updateBulbLightHeight = false;
   }

   lightColor_intensity.w = m_d.m_currentIntensity;
   if (!m_d.m_BulbLight)
   {
      pd3dDevice->classicLightShader->SetLightColorIntensity(lightColor_intensity);
      pd3dDevice->classicLightShader->Begin();
   }
   else
   {
      if (g_pplayer->m_current_renderstage == 1)
         lightColor_intensity.w *= m_d.m_transmissionScale;
      pd3dDevice->lightShader->SetLightColorIntensity(lightColor_intensity);
      pd3dDevice->lightShader->Begin();
   }

   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, (!m_backglass) ? MY_D3DFVF_NOTEX2_VERTEX : MY_D3DTRANSFORMED_NOTEX2_VERTEX, m_customMoverVBuffer, 0, m_customMoverVertexNum, m_customMoverIBuffer, 0, m_customMoverIndexNum);

   if (!m_d.m_BulbLight)
      pd3dDevice->classicLightShader->End();
   else
      pd3dDevice->lightShader->End();

   if (!m_d.m_BulbLight && offTexel != nullptr /*&& m_ptable->m_reflectElementsOnPlayfield && g_pplayer->m_pf_refl*/ && !m_backglass) // See above: // TOTAN and Flintstones inserts break if alpha blending is disabled here.
   {
      pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_FALSE);
      pd3dDevice->SetRenderState(RenderDevice::SRCBLEND, RenderDevice::SRC_ALPHA);
      pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::INVSRC_ALPHA);
   }

   /*if ( m_d.m_BulbLight ) //!! not necessary anymore
   {
   pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_FALSE);
   pd3dDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD);
   }*/

   //if(m_backglass && (m_ptable->m_tblMirrorEnabled^m_ptable->m_reflectionEnabled))
   //	pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_CCW);
}

void Light::PrepareMoversCustom()
{
   GetRgVertex(m_vvertex);

   if (m_vvertex.empty())
      return;

   m_maxDist = 0.f;
   vector<WORD> vtri;

   {
      vector<unsigned int> vpoly(m_vvertex.size());
      const unsigned int cvertex = (unsigned int)m_vvertex.size();
      for (unsigned int i = 0; i < cvertex; i++)
      {
         vpoly[i] = i;

         const float dx = m_vvertex[i].x - m_d.m_vCenter.x;
         const float dy = m_vvertex[i].y - m_d.m_vCenter.y;
         const float dist = dx*dx + dy*dy;
         if (dist > m_maxDist)
            m_maxDist = dist;
      }

      PolygonToTriangles(m_vvertex, vpoly, vtri, true);
   }


   SAFE_BUFFER_RELEASE(m_customMoverIBuffer);
   SAFE_BUFFER_RELEASE(m_customMoverVBuffer);

   if (vtri.empty())
   {
      char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
      WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);
      char textBuffer[MAX_PATH];
      _snprintf_s(textBuffer, MAX_PATH-1, "%s has an invalid shape! It can not be rendered!", name);
      ShowError(textBuffer);
      return;
   }

   m_customMoverIndexNum = (unsigned int)vtri.size();
   IndexBuffer::CreateIndexBuffer(m_customMoverIndexNum, 0, IndexBuffer::FMT_INDEX16, &m_customMoverIBuffer, m_backglass ? SECONDARY_DEVICE : PRIMARY_DEVICE);

   WORD* bufi;
   m_customMoverIBuffer->lock(0, 0, (void**)&bufi, IndexBuffer::WRITEONLY);
   memcpy(bufi, vtri.data(), vtri.size()*sizeof(WORD));
   m_customMoverIBuffer->unlock();

   m_customMoverVertexNum = (unsigned int)m_vvertex.size();
   const DWORD vertexType = (!m_backglass) ? MY_D3DFVF_NOTEX2_VERTEX : MY_D3DTRANSFORMED_NOTEX2_VERTEX;
   m_customMoverVBuffer = new VertexBuffer(m_backglass ? g_pplayer->m_pin3d.m_pd3dSecondaryDevice : g_pplayer->m_pin3d.m_pd3dPrimaryDevice, m_customMoverVertexNum, 0, vertexType);

   UpdateCustomMoverVBuffer();
}

void Light::UpdateCustomMoverVBuffer()
{
   float height = m_initSurfaceHeight;
   if (m_d.m_BulbLight)
   {
      height += m_d.m_bulbHaloHeight*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
      m_surfaceHeight = height;
   }

   Texture* const pin = m_ptable->GetImage(m_d.m_szImage);

   const float inv_maxdist = (m_maxDist > 0.0f) ? 0.5f / sqrtf(m_maxDist) : 0.0f;
   const float inv_tablewidth = 1.0f / (m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f / (m_ptable->m_bottom - m_ptable->m_top);

   const float  mult = getBGxmult();
   const float ymult = getBGymult();

   Vertex3D_NoTex2 *buf;
   m_customMoverVBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   for (unsigned int t = 0; t < m_customMoverVertexNum; t++)
   {
      const RenderVertex * const pv0 = &m_vvertex[t];

      if (!m_backglass)
      {
         buf[t].x = pv0->x;
         buf[t].y = pv0->y;
         buf[t].z = height + 0.1f;

         // Check if we are using a custom texture.
         if (pin != nullptr)
         {
            buf[t].tu = pv0->x * inv_tablewidth;
            buf[t].tv = pv0->y * inv_tableheight;
         }
         else
         {
            // Set texture coordinates for default light.
            buf[t].tu = 0.5f + (pv0->x - m_d.m_vCenter.x) * inv_maxdist;
            buf[t].tv = 0.5f + (pv0->y - m_d.m_vCenter.y) * inv_maxdist;
         }

         buf[t].nx = 0; buf[t].ny = 0; buf[t].nz = 1.0f;
      }
      else
      {
         const float x = pv0->x* mult - 0.5f;
         const float y = pv0->y*ymult - 0.5f;

         buf[t].x = x;
         buf[t].y = y;
         buf[t].z = 0.0f;

         buf[t].nx = 1.f; //!! for backglass we use no vertex shader (D3DDECLUSAGE_POSITIONT), thus w component is actually mapped to nx, and that must be 1
         buf[t].ny = (pv0->x * (float)(1.0 / EDITOR_BG_WIDTH)); //!! abuses normal to pass tex coord via TEXCOORD1.xy to shader for non-bulbs :/
         buf[t].nz = (pv0->y * (float)(1.0 / EDITOR_BG_HEIGHT));

         //!! in backglass mode, position is also passed in via texture coords (due to D3DDECLUSAGE_POSITIONT again)
         buf[t].tu = x;
         buf[t].tv = y;
      }
   }
   m_customMoverVBuffer->unlock();
}

void Light::RenderSetup()
{
   m_iblinkframe = 0;
   m_d.m_time_msec = g_pplayer->m_time_msec;
   m_updateBulbLightHeight = false;

   m_initSurfaceHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   m_surfaceMaterial = m_ptable->GetSurfaceMaterial(m_d.m_szSurface);
   m_surfaceTexture = m_ptable->GetSurfaceImage(m_d.m_szSurface);

   m_surfaceHeight = m_initSurfaceHeight;

   if (m_inPlayState == LightStateBlinking)
      RestartBlinker(g_pplayer->m_time_msec);
   else if (m_duration > 0 && m_inPlayState == LightStateOn)
      m_timerDurationEndTime = g_pplayer->m_time_msec + m_duration;

   const bool isOn = (m_inPlayState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : (m_inPlayState != LightStateOff);
   if (isOn)
      m_d.m_currentIntensity = m_d.m_intensity*m_d.m_intensity_scale;
   else
      m_d.m_currentIntensity = 0.0f;

   if (m_d.m_BulbLight && m_d.m_showBulbMesh)
   {
      SAFE_BUFFER_RELEASE(m_bulbLightIndexBuffer);
      m_bulbLightIndexBuffer = IndexBuffer::CreateAndFillIndexBuffer(bulbLightNumFaces, bulbLightIndices, m_backglass ? SECONDARY_DEVICE : PRIMARY_DEVICE);

      SAFE_BUFFER_RELEASE(m_bulbLightVBuffer);
      m_bulbLightVBuffer = new VertexBuffer(m_backglass ? g_pplayer->m_pin3d.m_pd3dSecondaryDevice : g_pplayer->m_pin3d.m_pd3dPrimaryDevice, bulbLightNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX);

      Vertex3D_NoTex2 *buf;
      m_bulbLightVBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      for (unsigned int i = 0; i < bulbLightNumVertices; i++)
      {
         buf[i].x = bulbLight[i].x*m_d.m_meshRadius + m_d.m_vCenter.x;
         buf[i].y = bulbLight[i].y*m_d.m_meshRadius + m_d.m_vCenter.y;
         buf[i].z = bulbLight[i].z*m_d.m_meshRadius*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_surfaceHeight;
         buf[i].nx = bulbLight[i].nx;
         buf[i].ny = bulbLight[i].ny;
         buf[i].nz = bulbLight[i].nz;
         buf[i].tu = bulbLight[i].tu;
         buf[i].tv = bulbLight[i].tv;
      }
      m_bulbLightVBuffer->unlock();

      SAFE_BUFFER_RELEASE(m_bulbSocketIndexBuffer);
      m_bulbSocketIndexBuffer = IndexBuffer::CreateAndFillIndexBuffer(bulbSocketNumFaces, bulbSocketIndices, m_backglass ? SECONDARY_DEVICE : PRIMARY_DEVICE);

      SAFE_BUFFER_RELEASE(m_bulbSocketVBuffer);
      m_bulbSocketVBuffer = new VertexBuffer(m_backglass ? g_pplayer->m_pin3d.m_pd3dSecondaryDevice : g_pplayer->m_pin3d.m_pd3dPrimaryDevice, bulbSocketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX);

      m_bulbSocketVBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      for (unsigned int i = 0; i < bulbSocketNumVertices; i++)
      {
         buf[i].x = bulbSocket[i].x*m_d.m_meshRadius + m_d.m_vCenter.x;
         buf[i].y = bulbSocket[i].y*m_d.m_meshRadius + m_d.m_vCenter.y;
         buf[i].z = bulbSocket[i].z*m_d.m_meshRadius*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_surfaceHeight;
         buf[i].nx = bulbSocket[i].nx;
         buf[i].ny = bulbSocket[i].ny;
         buf[i].nz = bulbSocket[i].nz;
         buf[i].tu = bulbSocket[i].tu;
         buf[i].tv = bulbSocket[i].tv;
      }
      m_bulbSocketVBuffer->unlock();
   }

   PrepareMoversCustom();
}

void Light::RenderStatic()
{
   if (m_d.m_BulbLight && m_d.m_showBulbMesh && m_d.m_staticBulbMesh)
      RenderBulbMesh();
}

void Light::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Light::MoveOffset(const float dx, const float dy)
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

HRESULT Light::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_vCenter);
   bw.WriteFloat(FID(RADI), m_d.m_falloff);
   bw.WriteFloat(FID(FAPO), m_d.m_falloff_power);
   bw.WriteInt(FID(STAT), m_d.m_state);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteInt(FID(COL2), m_d.m_color2);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(BPAT), m_rgblinkpattern);
   bw.WriteString(FID(IMG1), m_d.m_szImage);
   bw.WriteInt(FID(BINT), m_blinkinterval);
   //bw.WriteInt(FID(BCOL), m_d.m_bordercolor);
   bw.WriteFloat(FID(BWTH), m_d.m_intensity);
   bw.WriteFloat(FID(TRMS), m_d.m_transmissionScale);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteBool(FID(BGLS), m_backglass);
   bw.WriteFloat(FID(LIDB), m_d.m_depthBias);
   bw.WriteFloat(FID(FASP), m_d.m_fadeSpeedUp);
   bw.WriteFloat(FID(FASD), m_d.m_fadeSpeedDown);
   bw.WriteBool(FID(BULT), m_d.m_BulbLight);
   bw.WriteBool(FID(IMMO), m_d.m_imageMode);
   bw.WriteBool(FID(SHBM), m_d.m_showBulbMesh);
   bw.WriteBool(FID(STBM), m_d.m_staticBulbMesh);
   bw.WriteBool(FID(SHRB), m_d.m_showReflectionOnBall);
   bw.WriteFloat(FID(BMSC), m_d.m_meshRadius);
   bw.WriteFloat(FID(BMVA), m_d.m_modulate_vs_add);
   bw.WriteFloat(FID(BHHI), m_d.m_bulbHaloHeight);

   ISelect::SaveData(pstm, hcrypthash);

   //bw.WriteTag(FID(PNTS));
   HRESULT hr;
   if (FAILED(hr = SavePointData(pstm, hcrypthash)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Light::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   m_d.m_falloff = 50.f;
   m_d.m_falloff_power = 2.0f;
   m_d.m_state = LightStateOff;
   m_d.m_shape = ShapeCustom;

   m_d.m_tdr.m_TimerEnabled = false;
   m_d.m_tdr.m_TimerInterval = 100;

   m_d.m_color = RGB(255, 255, 0);
   m_d.m_color2 = RGB(255, 255, 255);

   m_rgblinkpattern = "10";
   m_blinkinterval = 125;
   //m_d.m_borderwidth = 0;
   //m_d.m_bordercolor = RGB(0,0,0);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   m_lockedByLS = false;
   m_inPlayState = m_d.m_state;

   br.Load();
   return S_OK;
}

bool Light::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VCEN): pbr->GetVector2(m_d.m_vCenter); break;
   case FID(RADI): pbr->GetFloat(m_d.m_falloff); break;
   case FID(FAPO): pbr->GetFloat(m_d.m_falloff_power); break;
   case FID(STAT):
   {
      pbr->GetInt(&m_d.m_state);
      m_inPlayState = m_d.m_state;
      break;
   }
   case FID(COLR): pbr->GetInt(m_d.m_color); break;
   case FID(COL2): pbr->GetInt(m_d.m_color2); break;
   case FID(IMG1): pbr->GetString(m_d.m_szImage); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(SHAP): m_roundLight = true; break;
   case FID(BPAT): pbr->GetString(m_rgblinkpattern); break;
   case FID(BINT): pbr->GetInt(m_blinkinterval); break;
   //case FID(BCOL): pbr->GetInt(m_d.m_bordercolor); break;
   case FID(BWTH): pbr->GetFloat(m_d.m_intensity); break;
   case FID(TRMS): pbr->GetFloat(m_d.m_transmissionScale); break;
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   case FID(BGLS): pbr->GetBool(m_backglass); break;
   case FID(LIDB): pbr->GetFloat(m_d.m_depthBias); break;
   case FID(FASP): pbr->GetFloat(m_d.m_fadeSpeedUp); break;
   case FID(FASD): pbr->GetFloat(m_d.m_fadeSpeedDown); break;
   case FID(BULT): pbr->GetBool(m_d.m_BulbLight); break;
   case FID(IMMO): pbr->GetBool(m_d.m_imageMode); break;
   case FID(SHBM): pbr->GetBool(m_d.m_showBulbMesh); break;
   case FID(STBM): pbr->GetBool(m_d.m_staticBulbMesh); break;
   case FID(SHRB): pbr->GetBool(m_d.m_showReflectionOnBall); break;
   case FID(BMSC): pbr->GetFloat(m_d.m_meshRadius); break;
   case FID(BMVA): pbr->GetFloat(m_d.m_modulate_vs_add); break;
   case FID(BHHI): pbr->GetFloat(m_d.m_bulbHaloHeight); break;
   default:
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
      break;
   }
   }
   return true;
}

HRESULT Light::InitPostLoad()
{
   return S_OK;
}

Vertex2D Light::GetPointCenter() const
{
   return m_d.m_vCenter;
}

void Light::PutPointCenter(const Vertex2D& pv)
{
   m_d.m_vCenter = pv;
}

void Light::EditMenu(CMenu &menu)
{
#ifndef __STANDALONE__
    menu.EnableMenuItem(ID_WALLMENU_FLIP, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
    menu.EnableMenuItem(ID_WALLMENU_MIRROR, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
    menu.EnableMenuItem(ID_WALLMENU_ROTATE, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
    menu.EnableMenuItem(ID_WALLMENU_SCALE, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
    menu.EnableMenuItem(ID_WALLMENU_ADDPOINT, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
#endif
}

void Light::AddPoint(int x, int y, const bool smooth)
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
      pdp->Init(this, vOut.x, vOut.y, 0.f, smooth);
      m_vdpoint.insert(m_vdpoint.begin() + icp, pdp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
   }

   STOPUNDO
}

void Light::DoCommand(int icmd, int x, int y)
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
      AddPoint(x, y, true);
      break;
   }
}

STDMETHODIMP Light::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_ILight,
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Light::get_Falloff(float *pVal)
{
   *pVal = m_d.m_falloff;

   return S_OK;
}

STDMETHODIMP Light::put_Falloff(float newVal)
{
   if (newVal < 0.f)
      return E_FAIL;

   m_d.m_falloff = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_FalloffPower(float *pVal)
{
   *pVal = m_d.m_falloff_power;

   return S_OK;
}

STDMETHODIMP Light::put_FalloffPower(float newVal)
{
   m_d.m_falloff_power = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_State(LightState *pVal)
{
   if (g_pplayer && !m_lockedByLS)
      *pVal = m_inPlayState;
   else
      *pVal = m_d.m_state; //the LS needs the old m_d.m_state and not the current one, m_fLockedByLS is true if under the light is under control of the LS
   return S_OK;
}

STDMETHODIMP Light::put_State(LightState newVal)
{
   if (!m_lockedByLS)
      setInPlayState(newVal);

   m_d.m_state = newVal;
   return S_OK;
}

void Light::FlipY(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Light::FlipX(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Light::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter, useElementCenter);
}

void Light::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter, useElementCenter);
}

void Light::Translate(const Vertex2D &pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

STDMETHODIMP Light::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Light::put_Color(OLE_COLOR newVal)
{
   m_d.m_color = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_ColorFull(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color2;

   return S_OK;
}

STDMETHODIMP Light::put_ColorFull(OLE_COLOR newVal)
{
   m_d.m_color2 = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;
   m_vpinball->SetStatusBarUnitInfo(string(), true);

   return S_OK;
}

STDMETHODIMP Light::put_X(float newVal)
{
   m_d.m_vCenter.x = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Light::put_Y(float newVal)
{
   m_d.m_vCenter.y = newVal;

   return S_OK;
}

void Light::InitShape()
{
   if (m_vdpoint.empty())
   {
      // First time shape has been set to custom - set up some points
      const float x = m_d.m_vCenter.x;
      const float y = m_d.m_vCenter.y;

      for (int i = 8; i > 0; i--)
      {
         const float angle = (float)(M_PI*2.0 / 8.0)*(float)i;
         const float xx = x + sinf(angle)*m_d.m_falloff;
         const float yy = y - cosf(angle)*m_d.m_falloff;
         CComObject<DragPoint> *pdp;
         CComObject<DragPoint>::CreateInstance(&pdp);
         if (pdp)
         {
            pdp->AddRef();
            pdp->Init(this, xx, yy, 0.f, true);
            m_vdpoint.push_back(pdp);
         }
      }
   }
}

STDMETHODIMP Light::get_BlinkPattern(BSTR *pVal)
{
   WCHAR wz[NUM_RGB_BLINK_PATTERN];
   MultiByteToWideCharNull(CP_ACP, 0, m_rgblinkpattern.c_str(), -1, wz, NUM_RGB_BLINK_PATTERN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Light::put_BlinkPattern(BSTR newVal)
{
   char sz[NUM_RGB_BLINK_PATTERN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, sz, NUM_RGB_BLINK_PATTERN, nullptr, nullptr);
   m_rgblinkpattern = sz;

   if (m_rgblinkpattern.empty())
      m_rgblinkpattern = "0"; // "10" ?

   if (g_pplayer)
      RestartBlinker(g_pplayer->m_time_msec);

   return S_OK;
}

STDMETHODIMP Light::get_BlinkInterval(long *pVal)
{
   *pVal = m_blinkinterval;

   return S_OK;
}

STDMETHODIMP Light::put_BlinkInterval(long newVal)
{
   m_blinkinterval = newVal;
   if (g_pplayer)
      m_timenextblink = g_pplayer->m_time_msec + m_blinkinterval;

   return S_OK;
}


STDMETHODIMP Light::Duration(long startState, long newVal, long endState)
{
    m_inPlayState = (LightState)startState;
    m_duration = newVal;
    m_finalState = endState;
    if (g_pplayer)
    {
        m_timerDurationEndTime = g_pplayer->m_time_msec + m_duration;
        if (m_inPlayState == LightStateBlinking)
        {
            m_iblinkframe = 0;
            m_timenextblink = g_pplayer->m_time_msec + m_blinkinterval;
        }
    }

    return S_OK;
}


STDMETHODIMP Light::get_Intensity(float *pVal)
{
   *pVal = m_d.m_intensity;

   return S_OK;
}

STDMETHODIMP Light::put_Intensity(float newVal)
{
   m_d.m_intensity = max(0.f, newVal);
   const bool isOn = (m_inPlayState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : (m_inPlayState != LightStateOff);
   if (isOn)
      m_d.m_currentIntensity = m_d.m_intensity*m_d.m_intensity_scale;

   return S_OK;
}

STDMETHODIMP Light::get_TransmissionScale(float *pVal)
{
   *pVal = m_d.m_transmissionScale;

   return S_OK;
}

STDMETHODIMP Light::put_TransmissionScale(float newVal)
{
   m_d.m_transmissionScale = max(0.f, newVal);

   return S_OK;
}

STDMETHODIMP Light::get_IntensityScale(float *pVal)
{
   *pVal = m_d.m_intensity_scale;

   return S_OK;
}

STDMETHODIMP Light::put_IntensityScale(float newVal)
{
   m_d.m_intensity_scale = newVal;
   const bool isOn = (m_inPlayState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : (m_inPlayState != LightStateOff);
   if (isOn)
      m_d.m_currentIntensity = m_d.m_intensity*m_d.m_intensity_scale;

   return S_OK;
}

STDMETHODIMP Light::get_Surface(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSurface.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Light::put_Surface(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_d.m_szSurface = buf;

   return S_OK;
}


STDMETHODIMP Light::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Light::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   m_d.m_szImage = szImage;

   return S_OK;
}

STDMETHODIMP Light::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Light::put_DepthBias(float newVal)
{
   m_d.m_depthBias = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_FadeSpeedUp(float *pVal)
{
   *pVal = m_d.m_fadeSpeedUp;

   return S_OK;
}

STDMETHODIMP Light::put_FadeSpeedUp(float newVal)
{
   m_d.m_fadeSpeedUp = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_FadeSpeedDown(float *pVal)
{
   *pVal = m_d.m_fadeSpeedDown;

   return S_OK;
}

STDMETHODIMP Light::put_FadeSpeedDown(float newVal)
{
   m_d.m_fadeSpeedDown = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_Bulb(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_BulbLight);

   return S_OK;
}

STDMETHODIMP Light::put_Bulb(VARIANT_BOOL newVal)
{
   m_d.m_BulbLight = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Light::get_ImageMode(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_imageMode);

   return S_OK;
}

STDMETHODIMP Light::put_ImageMode(VARIANT_BOOL newVal)
{
   m_d.m_imageMode = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Light::get_ShowBulbMesh(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_showBulbMesh);

   return S_OK;
}

STDMETHODIMP Light::put_ShowBulbMesh(VARIANT_BOOL newVal)
{
   m_d.m_showBulbMesh = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Light::get_StaticBulbMesh(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_staticBulbMesh);

   return S_OK;
}

STDMETHODIMP Light::put_StaticBulbMesh(VARIANT_BOOL newVal)
{
   m_d.m_staticBulbMesh = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Light::get_ShowReflectionOnBall(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_showReflectionOnBall);

   return S_OK;
}

STDMETHODIMP Light::put_ShowReflectionOnBall(VARIANT_BOOL newVal)
{
   m_d.m_showReflectionOnBall = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Light::get_ScaleBulbMesh(float *pVal)
{
   *pVal = m_d.m_meshRadius;

   return S_OK;
}

STDMETHODIMP Light::put_ScaleBulbMesh(float newVal)
{
   m_d.m_meshRadius = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_BulbModulateVsAdd(float *pVal)
{
   *pVal = m_d.m_modulate_vs_add;

   return S_OK;
}

STDMETHODIMP Light::put_BulbModulateVsAdd(float newVal)
{
   m_d.m_modulate_vs_add = newVal;

   return S_OK;
}

STDMETHODIMP Light::get_BulbHaloHeight(float *pVal)
{
   *pVal = m_d.m_bulbHaloHeight;

   return S_OK;
}

STDMETHODIMP Light::put_BulbHaloHeight(float newVal)
{
   if(m_d.m_bulbHaloHeight != newVal)
   {
      m_d.m_bulbHaloHeight = newVal;
      m_updateBulbLightHeight = true;
   }

   return S_OK;
}

void Light::setInPlayState(const LightState newVal)
{
   if (newVal != m_inPlayState) // state changed???
   {
      m_inPlayState = newVal;

      if (g_pplayer)
      {
         if (m_inPlayState == LightStateBlinking)
         {
            m_timenextblink = g_pplayer->m_time_msec; // Start pattern right away // + m_d.m_blinkinterval;
            m_iblinkframe = 0; // reset pattern
         }
         if (m_duration > 0)
            m_duration = 0; // disable duration if a state was set this way
      }
   }
}

STDMETHODIMP Light::GetInPlayState(LightState* pVal)
{
    *pVal = m_inPlayState;
    return S_OK;
}

STDMETHODIMP Light::GetInPlayStateBool(VARIANT_BOOL* pVal)
{
    const bool isOn = (m_inPlayState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : (m_inPlayState != LightStateOff);

    *pVal = FTOVB(isOn);
    return S_OK;
}

STDMETHODIMP Light::GetInPlayIntensity(float *pVal)
{
   *pVal = m_d.m_currentIntensity;

   return S_OK;
}

void Light::setLightState(const LightState newVal)
{
   if (!m_lockedByLS)
      setInPlayState(newVal);

   m_d.m_state = newVal;
}

LightState Light::getLightState() const
{
   return m_d.m_state;
}

STDMETHODIMP Light::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_visible);

   return S_OK;
}

STDMETHODIMP Light::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_visible = VBTOb(newVal);

   return S_OK;
}
