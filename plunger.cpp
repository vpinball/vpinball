#include "StdAfx.h"

Plunger::Plunger()
{
   m_phitplunger = NULL;
   vertexBuffer = NULL;
   indexBuffer = NULL;
   memset(m_d.m_szImage, 0, MAXTOKEN);
   memset(m_d.m_szMaterial, 0, 32);
   memset(m_d.m_szSurface, 0, MAXTOKEN);
}

Plunger::~Plunger()
{
   if (vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = NULL;
   }
   if (indexBuffer)
   {
      indexBuffer->release();
      indexBuffer = NULL;
   }
}

HRESULT Plunger::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_v.x = x;
   m_d.m_v.y = y;

   SetDefaults(fromMouseClick);
   return InitVBA(fTrue, 0, NULL);
}

void Plunger::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   SetDefaultPhysics(fromMouseClick);

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "Height", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_height = fTmp;
   else
      m_d.m_height = 20;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "Width", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_width = fTmp;
   else
      m_d.m_width = 25;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "ZAdjust", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_zAdjust = fTmp;
   else
      m_d.m_zAdjust = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "Stroke", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_stroke = fTmp;
   else
      m_d.m_stroke = m_d.m_height * 4;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "PullSpeed", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_speedPull = fTmp;
   else
      m_d.m_speedPull = 5;

   hr = GetRegInt("DefaultProps\\Plunger", "PlungerType", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_type = (enum PlungerType)iTmp;
   else
      m_d.m_type = PlungerTypeModern;

   hr = GetRegInt("DefaultProps\\Plunger", "Color", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_color = iTmp;
   else
      m_d.m_color = RGB(76, 76, 76);

   hr = GetRegString("DefaultProps\\Plunger", "Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegInt("DefaultProps\\Plunger", "AnimFrames", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_animFrames = iTmp;
   else
      m_d.m_animFrames = 1;


   hr = GetRegInt("DefaultProps\\Plunger", "TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\Plunger", "TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegString("DefaultProps\\Plunger", "Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   hr = GetRegInt("DefaultProps\\Plunger", "MechPlunger", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_mechPlunger = iTmp == 0 ? false : true;
   else
      m_d.m_mechPlunger = fFalse;             // plungers require selection for mechanical input

   hr = GetRegInt("DefaultProps\\Plunger", "AutoPlunger", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_autoPlunger = iTmp == 0 ? false : true;
   else
      m_d.m_autoPlunger = fFalse;


   hr = GetRegInt("DefaultProps\\Plunger", "Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = true;


   hr = GetRegString("DefaultProps\\Plunger", "CustomTipShape", m_d.m_szTipShape, MAXTIPSHAPE);
   if ((hr != S_OK) || !fromMouseClick)
      strcpy_s(m_d.m_szTipShape, MAXTIPSHAPE,
      "0 .34; 2 .6; 3 .64; 5 .7; 7 .84; 8 .88; 9 .9; 11 .92; 14 .92; 39 .84");

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "CustomRodDiam", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rodDiam = fTmp;
   else
      m_d.m_rodDiam = 0.60f;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "CustomRingGap", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_ringGap = fTmp;
   else
      m_d.m_ringGap = 2.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "CustomRingDiam", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_ringDiam = fTmp;
   else
      m_d.m_ringDiam = 0.94f;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "CustomRingWidth", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_ringWidth = fTmp;
   else
      m_d.m_ringWidth = 3.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "CustomSpringDiam", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_springDiam = fTmp;
   else
      m_d.m_springDiam = 0.77f;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "CustomSpringGauge", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_springGauge = fTmp;
   else
      m_d.m_springGauge = 1.38f;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "CustomSpringLoops", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_springLoops = fTmp;
   else
      m_d.m_springLoops = 8.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "CustomSpringEndLoops", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_springEndLoops = fTmp;
   else
      m_d.m_springEndLoops = 2.5f;

   hr = GetRegInt("DefaultProps\\Plunger", "ReflectionEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fReflectionEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_fReflectionEnabled = true;

}

void Plunger::WriteRegDefaults()
{
   SetRegValueFloat("DefaultProps\\Plunger", "Height", m_d.m_height);
   SetRegValueFloat("DefaultProps\\Plunger", "Width", m_d.m_width);
   SetRegValueFloat("DefaultProps\\Plunger", "ZAdjust", m_d.m_zAdjust);
   SetRegValueFloat("DefaultProps\\Plunger", "Stroke", m_d.m_stroke);
   SetRegValueFloat("DefaultProps\\Plunger", "PullSpeed", m_d.m_speedPull);
   SetRegValueFloat("DefaultProps\\Plunger", "ReleaseSpeed", m_d.m_speedFire);
   SetRegValue("DefaultProps\\Plunger", "PlungerType", REG_DWORD, &m_d.m_type, 4);
   SetRegValue("DefaultProps\\Plunger", "AnimFrames", REG_DWORD, &m_d.m_animFrames, 4);
   SetRegValue("DefaultProps\\Plunger", "Color", REG_DWORD, &m_d.m_color, 4);
   SetRegValue("DefaultProps\\Plunger", "Image", REG_SZ, &m_d.m_szImage, lstrlen(m_d.m_szImage));
   SetRegValueBool("DefaultProps\\Plunger", "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValue("DefaultProps\\Plunger", "TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Plunger", "Surface", REG_SZ, &m_d.m_szSurface, lstrlen(m_d.m_szSurface));
   SetRegValue("DefaultProps\\Plunger", "MechPlunger", REG_DWORD, &m_d.m_mechPlunger, 4);
   SetRegValue("DefaultProps\\Plunger", "AutoPlunger", REG_DWORD, &m_d.m_autoPlunger, 4);
   SetRegValueFloat("DefaultProps\\Plunger", "MechStrength", m_d.m_mechStrength);
   SetRegValueFloat("DefaultProps\\Plunger", "ParkPosition", m_d.m_parkPosition);
   SetRegValueBool("DefaultProps\\Plunger", "Visible", m_d.m_fVisible);
   SetRegValueFloat("DefaultProps\\Plunger", "ScatterVelocity", m_d.m_scatterVelocity);
   SetRegValueFloat("DefaultProps\\Plunger", "MomentumXfer", m_d.m_momentumXfer);
   SetRegValue("DefaultProps\\Plunger", "CustomTipShape", REG_SZ, &m_d.m_szTipShape, lstrlen(m_d.m_szTipShape));
   SetRegValueFloat("DefaultProps\\Plunger", "CustomRodDiam", m_d.m_rodDiam);
   SetRegValueFloat("DefaultProps\\Plunger", "CustomRingGap", m_d.m_ringGap);
   SetRegValueFloat("DefaultProps\\Plunger", "CustomRingDiam", m_d.m_ringDiam);
   SetRegValueFloat("DefaultProps\\Plunger", "CustomRingWidth", m_d.m_ringWidth);
   SetRegValueFloat("DefaultProps\\Plunger", "CustomSpringDiam", m_d.m_springDiam);
   SetRegValueFloat("DefaultProps\\Plunger", "CustomSpringGauge", m_d.m_springGauge);
   SetRegValueFloat("DefaultProps\\Plunger", "CustomSpringLoops", m_d.m_springLoops);
   SetRegValueFloat("DefaultProps\\Plunger", "CustomSpringEndLoops", m_d.m_springEndLoops);
   SetRegValueBool("DefaultProps\\Plunger", "ReflectionEnabled", m_d.m_fReflectionEnabled);
}

void Plunger::PreRender(Sur * const psur)
{
}

void Plunger::Render(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);

   psur->Rectangle(m_d.m_v.x - m_d.m_width, m_d.m_v.y - m_d.m_stroke,
      m_d.m_v.x + m_d.m_width, m_d.m_v.y + m_d.m_height);

   // draw a dotted line at the park position, if appropriate
   if (m_d.m_parkPosition > 0.0f && m_d.m_parkPosition < 1.0f)
   {
      const float park = m_d.m_parkPosition * m_d.m_stroke;
      psur->SetLineColor(RGB(0x80, 0x80, 0x80), true, 1);
      psur->Line(m_d.m_v.x - m_d.m_width, m_d.m_v.y - m_d.m_stroke + park,
         m_d.m_v.x + m_d.m_width, m_d.m_v.y - m_d.m_stroke + park);
   }
}

void Plunger::GetHitShapes(Vector<HitObject> * const pvho)
{
   const float zheight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_v.x, m_d.m_v.y);

   HitPlunger * const php = new HitPlunger(m_d.m_v.x - m_d.m_width,
      m_d.m_v.y + m_d.m_height, m_d.m_v.x + m_d.m_width,
      zheight, m_d.m_v.y - m_d.m_stroke, m_d.m_v.y,
      this);

   php->m_pfe = NULL;

   pvho->AddElement(php);
   php->m_pplunger = this;
   m_phitplunger = php;
}

void Plunger::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Plunger::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_fTimerEnabled)
   {
      pvht->AddElement(pht);
   }
}

void Plunger::EndPlay()
{
   m_phitplunger = NULL;       // possible memory leak here?

   IEditable::EndPlay();
   if (vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = NULL;
   }
   if (indexBuffer)
   {
      indexBuffer->release();
      indexBuffer = NULL;
   }
}

void Plunger::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_v.x, m_d.m_v.y);
}

void Plunger::MoveOffset(const float dx, const float dy)
{
   m_d.m_v.x += dx;
   m_d.m_v.y += dy;
   m_ptable->SetDirtyDraw();
}

void Plunger::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_v;
}

void Plunger::PutCenter(const Vertex2D * const pv)
{
   m_d.m_v = *pv;

   m_ptable->SetDirtyDraw();
}

void Plunger::SetDefaultPhysics(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "ReleaseSpeed", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_speedFire = fTmp;
   else
      m_d.m_speedFire = 80;
   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "MechStrength", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_mechStrength = fTmp;
   else
      m_d.m_mechStrength = 85;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "ParkPosition", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_parkPosition = fTmp;
   else
      m_d.m_parkPosition = (float)(0.5 / 3.0); // typical mechanical plunger has 3 inch stroke and 0.5 inch rest position //!! 0.01f better for some HW-plungers, but this seems to be rather a firmware/config issue
   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "ScatterVelocity", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_scatterVelocity = fTmp;
   else
      m_d.m_scatterVelocity = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Plunger", "MomentumXfer", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_momentumXfer = fTmp;
   else
      m_d.m_momentumXfer = 1.0f;

}

void Plunger::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   // TODO: get rid of frame stuff
   if (!m_d.m_fVisible)
      return;

   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

   _ASSERTE(m_phitplunger);

   const PlungerAnimObject& pa = m_phitplunger->m_plungeranim;
   const int frame0 = (int)((pa.m_pos - pa.m_frameStart) / (pa.m_frameEnd - pa.m_frameStart) * (cframes - 1) + 0.5f);
   const int frame = (frame0 < 0 ? 0 : frame0 >= cframes ? cframes - 1 : frame0);

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   Texture *pin = m_ptable->GetImage(m_d.m_szImage);
   if (pin)
   {
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
      pd3dDevice->basicShader->SetTexture("Texture0", pin);
      pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));
   }
   else
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer,
      frame*vtsPerFrame, vtsPerFrame,
      indexBuffer, 0, indicesPerFrame);
   pd3dDevice->basicShader->End();
}

// Modern Plunger - added by rascal
const static PlungerCoord modernCoords[] =
{
   { 0.20f, 0.0f, 0.00f, 1.0f, 0.0f },  // tip
   { 0.30f, 3.0f, 0.11f, 1.0f, 0.0f },  // tip
   { 0.35f, 5.0f, 0.14f, 1.0f, 0.0f },  // tip
   { 0.35f, 23.0f, 0.19f, 1.0f, 0.0f },  // tip
   { 0.45f, 23.0f, 0.21f, 0.8f, 0.0f },  // ring
   { 0.25f, 24.0f, 0.25f, 0.3f, 0.0f },  // shaft
   { 0.25f, 100.0f, 1.00f, 0.3f, 0.0f }   // shaft
};
const static PlungerDesc modernDesc = {
   sizeof(modernCoords) / sizeof(modernCoords[0]), modernCoords
};

// Flat Plunger.  This is a special case with no "lathe" entries;
// instead we define a simple rectangle covering the stroke length
// by 2x the nominal width (1x the width on each side of the centerline).
// The pre-render code knows to set up the flat rendering when there are
// no lathe coordinates.
const static PlungerDesc flatDesc = { 0, 0 };


// Find and skip the next token in a TipShape string.  Finds the
// next non-whitespace character and returns that a pointer to
// that position in the string.  Also advances p past the token.
// If the token ends with ';' or '\0', leaves p pointing to
// that delimiter.
static const char *nextTipToken(const char* &p)
{
   // skip whitespace
   for (; isspace(*p); ++p);

   // this is the start of the token, which will be our return value
   const char *tok = p;

   // skip ahead to the next delimiter
   for (; *p != ';' && *p != ',' && !isspace(*p) && *p != '\0'; ++p);

   // skip whitespace at/after the delimiter
   for (; isspace(*p); ++p);

   // return the start of the token
   return tok;
}

#define PLUNGER_FRAME_COUNT 25   //frame per 80 units distance

void Plunger::RenderSetup(RenderDevice* pd3dDevice)
{
   const float zheight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_v.x, m_d.m_v.y) + m_d.m_zAdjust;
   const float stroke = m_d.m_stroke;
   const float beginy = m_d.m_v.y;
   const float endy = m_d.m_v.y - stroke;
   cframes = (int)((float)PLUNGER_FRAME_COUNT * (stroke*(float)(1.0 / 80.0))) + 1; // 25 frames per 80 units travel
   const float inv_scale = (cframes > 1) ? (1.0f / (float)(cframes - 1)) : 0.0f;
   const float dyPerFrame = (endy - beginy) * inv_scale;
   int circlePoints = 0;
   float springLoops = 0.0f, springEndLoops = 0.0f;
   float springGauge = 0.0f;
   float springRadius = 0.0f;
   const float springMinSpacing = 2.2f;
   float rody = beginy + m_d.m_height;
   const float zScale = m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   // note the number of cells in the source image
   int srcCells = m_d.m_animFrames;
   if (srcCells < 1)
      srcCells = 1;

   // figure the width in relative units (0..1) of each cell
   float cellWid = 1.0f / float(srcCells);

   // figure which plunger descriptor we're using
   const PlungerDesc *desc;
   PlungerDesc *customDesc = 0;
   switch (m_d.m_type)
   {
   case PlungerTypeModern:
   default:
      desc = &modernDesc;
      break;

   case PlungerTypeFlat:
      desc = &flatDesc;
      break;

   case PlungerTypeCustom:
      // custom - create a private desc from the custom properties
   {
      // Several of the entries are fixed:
      //   shaft x 2 (top, bottom)
      //   ring x 6 (inner top, outer top x 2, outer bottom x 2, inner bottom)
      //   ring gap x 2 (top, bottom)
      //   tip bottom inner x 1
      //   first entry in custom tip list (there's always at least one;
      //      even if it's blank, we read the empty entry as "0,0" )
      int nn = 2 + 6 + 2 + 1 + 1;

      // Count entries in the tip list.  Entries are separated
      // by semicolons.
      int nTip = 1;
      for (const char *p = m_d.m_szTipShape; *p != '\0'; ++p)
      {
         if (*p == ';')
            ++nTip, ++nn;
      }

      // allocate the descriptor and the coordinate array
      desc = customDesc = new PlungerDesc;
      customDesc->n = nn;
      PlungerCoord *cc = new PlungerCoord[nn];
      customDesc->c = cc;

      // figure the tip lathe descriptor from the shape point list
      PlungerCoord c0 = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
      PlungerCoord *c = cc, *cprv = &c0;
      float tiplen = 0;
      for (const char *p = m_d.m_szTipShape; *p != '\0'; cprv = c++)
      {
         // Parse the entry: "yOffset, diam".  yOffset is the
         // offset (in table distance units) from the previous
         // point.  "diam" is the diameter (relative to the
         // nominal width of the plunger, as given by the width
         // property) of the tip at this point.  1.0 means that
         // the diameter is the same as the nominal width; 0.5
         // is half the width.
         c->y = float(atof(nextTipToken(p)));
         c->r = float(atof(nextTipToken(p))) / 2.0f;

         // each entry has to have a higher y value than the last
         if (c->y < tiplen)
            c->y = tiplen;

         // update the tip length so far
         tiplen = c->y;

         // skip to the next entry after the ';' delimiter
         for (; *p != ';' && *p != '\0'; ++p);
         if (*p == ';')
            ++p;
      }

      // Figure the normals and the texture coordinates
      c = cc;
      cprv = &c0;
      for (int i = 0; i < nTip; ++i, cprv = c++)
      {
         // Figure the texture coordinate.  The tip is always
         // the top 25% of the overall texture; interpolate the
         // current lathe point's position within that 25%.
         c->tv = 0.24f * c->y / tiplen;

         // Figure the normal as the average of the surrounding
         // surface normals.
         PlungerCoord *cnxt = (i + 1 < nTip ? c + 1 : c);
         float x0 = cprv->r, y0 = cprv->y;
         float x1 = cnxt->r, y1 = cnxt->y;
         float th = atan2f(y1 - y0, (x1 - x0) * m_d.m_width);
         c->nx = sinf(th);
         c->ny = -cosf(th);
      }

      // add the inner edge of the tip (abutting the rod)
      float rRod = m_d.m_rodDiam / 2.0f;
      float y = tiplen;
      (c++)->set(rRod, y, 0.24f, 1.0f, 0.0f);

      // add the gap between tip and ring (texture is in the rod
      // quadrant of overall texture, 50%-75%)
      (c++)->set(rRod, y, 0.51f, 1.0f, 0.0f);
      y += m_d.m_ringGap;
      (c++)->set(rRod, y, 0.55f, 1.0f, 0.0f);

      // add the ring (texture is in the ring quadrant, 25%-50%)
      float rRing = m_d.m_ringDiam / 2.0f;
      (c++)->set(rRod, y, 0.26f, 0.0f, -1.0f);
      (c++)->set(rRing, y, 0.33f, 0.0f, -1.0f);
      (c++)->set(rRing, y, 0.33f, 1.0f, 0.0f);
      y += m_d.m_ringWidth;
      (c++)->set(rRing, y, 0.42f, 1.0f, 0.0f);
      (c++)->set(rRing, y, 0.42f, 0.0f, 1.0f);
      (c++)->set(rRod, y, 0.49f, 0.0f, 1.0f);

      // set the spring values from the properties
      springRadius = m_d.m_springDiam / 2.0f;
      springGauge = m_d.m_springGauge;
      springLoops = m_d.m_springLoops;
      springEndLoops = m_d.m_springEndLoops;

      // add the top of the shaft (texture is in the 50%-75% quadrant)
      (c++)->set(rRod, y, 0.51f, 1.0f, 0.0f);

      // Figure the fully compressed spring length.  This is
      // the lower bound for the rod length.
      float springMin = (springLoops + springEndLoops)*springMinSpacing;

      // Figure the rod bottom position (rody).  This is the fully
      // retracted tip position (beginy), plus the length of the parts
      // at the end that don't compress with the spring (y), plus the
      // fully retracted spring length.
      rody = beginy + y + springMin;
      (c++)->set(rRod, rody, 0.74f, 1.0f, 0.0f);
   }
   break;
   }

   // get the number of lathe points from the descriptor
   int lathePoints = desc->n;

   // calculate the frame rendering details
   int latheVts = 0, springVts = 0;
   int latheIndices = 0, springIndices = 0;
   if (m_d.m_type == PlungerTypeFlat)
   {
      // For the flat plunger, we render every frame as a simple
      // flat rectangle.  This requires four vertices for the corners,
      // and two triangles -> 6 indices.
      vtsPerFrame = 4;
      indicesPerFrame = 6;
   }
   else
   {
      // For all other plungers, we render one circle per lathe
      // point.  Each circle has 'circlePoints' vertices.  We
      // also need to render the spring:  this consists of 3
      // spirals, where each sprial has 'springLoops' loops
      // times 'circlePoints' vertices.
      circlePoints = 24;
      latheVts = lathePoints * circlePoints;
      springVts = int((springLoops + springEndLoops) * circlePoints) * 3;
      vtsPerFrame = latheVts + springVts;

      // For the lathed section, we need two triangles == 6
      // indices for every point on every lathe circle past
      // the first.  (We connect pairs of lathe circles, so
      // the first one doesn't count: two circles -> one set
      // of triangles, three circles -> two sets, etc).
      latheIndices = 6 * circlePoints * (lathePoints - 1);

      // For the spring, we need 4 triangles == 12 indices
      // for every matching set of three vertices on the
      // three spirals, not counting the first set (as above,
      // we're connecting adjacent sets, so the first doesn't
      // count).  We already counted the total number of
      // vertices, so divide that by 3 to get the number
      // of sets.  12*vts/3 = 4*vts.
      // 
      // The spring only applies to the custom plunger.
      if (m_d.m_type == PlungerTypeCustom)
      {
         if ((springIndices = (4 * springVts) - 12) < 0)
            springIndices = 0;
      }

      // the total number of indices is simply the sum of the
      // lathe and spring indices
      indicesPerFrame = latheIndices + springIndices;
   }

   // figure the relative spring gauge, in terms of the overall width
   float springGaugeRel = springGauge / m_d.m_width;

   if (vertexBuffer)
      vertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(cframes*vtsPerFrame, 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer);

   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);

   Vertex3D_NoTex2 *ptr = buf;

   // Build the animation frames.  We have 'cframes' frames total.  The 0th frame
   // shows the plunger in the maximum retracted position; the cframes-1'th frame
   // is the maximum forward position.
   for (int i = 0; i < cframes; i++, ptr += vtsPerFrame)
   {
      const float ytip = beginy + dyPerFrame*(float)i;

      if(m_d.m_type != PlungerTypeFlat)
      {
      // Go around in a circle starting at the top, stepping through
      // 'circlePoints' angles along the circle.  Start the texture
      // mapping in the middle, so that the centerline of the texture
      // maps to the centerline of the top of the cylinder surface.
      // Work outwards on the texture to wrap it around the cylinder.
      float tu = 0.51f;

      const float stepU = 1.0f / (float)circlePoints;
      for (int l = 0, offset = 0; l < circlePoints; l++, offset += lathePoints, tu += stepU)
      {
         // Go down the long axis, adding a vertex for each point
         // in the descriptor list at the current lathe angle.
         if (tu > 1.0f) tu -= 1.0f;
         const float angle = (float)(M_PI*2.0 / circlePoints)*(float)l;
         const float sn = sinf(angle);
         const float cs = cosf(angle);
         const PlungerCoord *c = desc->c;
         Vertex3D_NoTex2 *pm = &ptr[offset];
         for (int m = 0; m < lathePoints; m++, ++c, ++pm)
         {
            // get the current point's coordinates
            float y = c->y + ytip;
            float r = c->r;
            float tv = c->tv;

            // the last coordinate is always the bottom of the rod
            if (m + 1 == lathePoints)
            {
               // set the end point
               y = rody;

               // Figure the texture mapping for the rod position.  This is
               // important because we draw the rod with varying length -
               // the part that's pulled back beyond the 'rody' point is
               // hidden.  We want the texture to maintain the same apparent
               // position and scale in each frame, so we need to figure the
               // proportional point of the texture at our cut-off point on
               // the object surface.
               float ratio = (float(i) * inv_scale);
               tv = (pm - 1)->tv + (tv - (pm - 1)->tv)*ratio;
            }

            // figure the point coordinates
            pm->x = r * (sn * m_d.m_width) + m_d.m_v.x;
            pm->y = y;
            pm->z = (r * (cs * m_d.m_width) + m_d.m_width + zheight) * zScale;
            pm->nx = c->nx * sn;
            pm->ny = c->ny;
            pm->nz = -c->nx * cs;
            pm->tu = tu;
            pm->tv = tv;
         }
      }
      }

      // Build the flat plunger rectangle, if desired
      if (m_d.m_type == PlungerTypeFlat)
      {
         // Flat plunger - overlay the alpha image on a rectangular surface.

         // Figure the corner coordinates.
         //
         // The tip of the plunger for this frame is at 'height', which is the
         // nominal y position (m_d.m_v.y) plus the portion of the stroke length
         // for the current frame.  (The 0th frame is the most retracted position;
         // the cframe-1'th frame is the most forward position.)  The base is at
         // the nominal y position plus m_d.m_height.
         const float xLt = m_d.m_v.x - m_d.m_width;
         const float xRt = m_d.m_v.x + m_d.m_width;
         const float yTop = ytip;
         const float yBot = beginy + m_d.m_height;

         // Figure the z coordinate.
         //
         // For the shaped plungers, the vertical extent is determined by placing
         // the long axis at the plunger's nominal width (m_d.m_width) above the
         // playfield (or whatever the base surface is).  Since those are modeled
         // roughly as cylinders with the main shaft radius at about 1/4 the nominal
         // width, the top point is at about 1.25 the nominal width and the bulk
         // is between 1x and 1.25x the nominal width above the base surface.  To
         // get approximately the same effect, we place our rectangular surface at
         // 1.25x the width above the base surface.  The table author can tweak this
         // using the ZAdjust property, which is added to the zheight base level.
         //
         const float z = (zheight + m_d.m_width*1.25f) * zScale;

         // Figure out which animation cell we're using.  The source image might not
         // (and probably does not) have the same number of cells as the frame list
         // we're generating, since our frame count depends on the stroke length.
         // So we need to interpolate between the image cells and the generated frames.
         // 
         // The source image is arranged with the fully extended image in the leftmost
         // cell and the fully retracted image in the rightmost cell.  Our frame
         // numbering is just the reverse, so figure the cell number in right-to-left
         // order to simplify the texture mapping calculations.
         int cellIdx = srcCells - 1 - int((i * float(srcCells) / float(cframes)) + 0.5f);
         if (cellIdx < 0) cellIdx = 0;

         // Figure the texture coordinates.
         //
         // The y extent (tv) maps to the top portion of the image with height
         // proportional to the current frame's height relative to the overall height.
         // Our frames vary in height to display the motion of the plunger.  The
         // animation cells are all the same height, so we need to map to the
         // proportional vertical share of the cell.  The images in the cells are
         // top-justified, so we always start at the top of the cell.
         //
         // The x extent is the full width of the current cell.
         const float tu_local = cellWid * float(cellIdx);
         const float tv_local = (yBot - yTop) / (beginy + m_d.m_height - endy);

         // Fill in the four corner vertices.
         // Vertices are (in order): bottom left, top left, top right, bottom right.
         ptr[0].x = xLt;        ptr[0].nx = 0.0f;          ptr[0].tu = tu_local;           // left
         ptr[0].y = yBot;       ptr[0].ny = 0.0f;          ptr[0].tv = tv_local;           // bottom
         ptr[0].z = z;          ptr[0].nz = -1.0f;
         ptr[1].x = xLt;        ptr[1].nx = 0.0f;          ptr[1].tu = tu_local;           // left
         ptr[1].y = yTop;       ptr[1].ny = 0.0f;          ptr[1].tv = 0.0f;				  // top
         ptr[1].z = z;          ptr[1].nz = -1.0f;
         ptr[2].x = xRt;        ptr[2].nx = 0.0f;          ptr[2].tu = tu_local + cellWid; // right
         ptr[2].y = yTop;       ptr[2].ny = 0.0f;          ptr[2].tv = 0.0f;			      // top
         ptr[2].z = z;          ptr[2].nz = -1.0f;
         ptr[3].x = xRt;        ptr[3].nx = 0.0f;          ptr[3].tu = tu_local + cellWid; // right
         ptr[3].y = yBot;       ptr[3].ny = 0.0f;          ptr[3].tv = tv_local;           // bottom
         ptr[3].z = z;          ptr[3].nz = -1.0f;
      }
      else
      {
         // Build the spring.  We build this as wedge shape wrapped around a spiral.
         // So we actually have three spirals: the front edge, the top edge, and the
         // back edge.  The y extent is the length of the rod; the rod starts at the
         // second-to-last entry and ends at the last entry.  But the actual base
         // position of the spring is fixed at the end of the shaft, which might
         // differ from the on-screen position of the last point in that the rod can
         // be visually cut off by the length adjustment.  So use the true rod base
         // (rody) position to figure the spring length.
         const int offset = circlePoints * lathePoints;
         const float y0 = ptr[offset - 2].y;
         float y1 = rody;
         int n = int((springLoops + springEndLoops) * circlePoints);
         const int nEnd = int(springEndLoops * circlePoints);
         const int nMain = n - nEnd;
         const float yEnd = springEndLoops * springGauge * springMinSpacing;
         const float dyMain = (y1 - y0 - yEnd) / (float)(nMain - 1);
         const float dyEnd = yEnd / (float)(nEnd - 1);
         float dy = dyEnd;
         Vertex3D_NoTex2 * pm = &ptr[offset];
         const float TWOPI = float(M_PI*2.0);
         const float dtheta = TWOPI / (float)(circlePoints - 1) + (float)M_PI / (float)(n - 1);
         for (float theta = (float)M_PI, y = y0; n != 0; --n, theta += dtheta, y += dy)
         {
            if (n == nMain) dy = dyMain;
            if (theta >= TWOPI) theta -= TWOPI;
            const float sn = sinf(theta);
            const float cs = cosf(theta);

            // set the point on the front spiral
            pm->x = springRadius * (sn * m_d.m_width) + m_d.m_v.x;
            pm->y = y - springGauge;
            pm->z = (springRadius * (cs * m_d.m_width) + m_d.m_width + zheight) * zScale;
            pm->nx = 0.0f;
            pm->ny = -1.0f;
            pm->nz = -0.0f;
            pm->tu = (sn + 1.0f) * 0.5f;
            pm->tv = 0.76f;
            ++pm;

            // set the point on the top spiral
            pm->x = (springRadius + springGaugeRel / 1.5f) * (sn * m_d.m_width) + m_d.m_v.x;
            pm->y = y;
            pm->z = ((springRadius + springGaugeRel / 1.5f) * (cs * m_d.m_width) + m_d.m_width + zheight) * zScale;
            pm->nx = sn;
            pm->ny = 0.0f;
            pm->nz = -cs;
            pm->tu = (sn + 1.0f) * 0.5f;
            pm->tv = 0.85f;
            ++pm;

            // set the point on the back spiral
            pm->x = springRadius * (sn * m_d.m_width) + m_d.m_v.x;
            pm->y = y + springGauge;
            pm->z = (springRadius * (cs * m_d.m_width) + m_d.m_width + zheight) * zScale;
            pm->nx = 0.0f;
            pm->ny = 1.0f;
            pm->nz = 0.0f;
            pm->tu = (sn + 1.0f) * 0.5f;
            pm->tv = 0.98f;
            ++pm;
         }

         y1 += 0;
      }
   }

   // set up the vertex index list
   WORD * const indices = new WORD[indicesPerFrame];
   int k = 0;

   // set up the vertex list for the lathe circles
   for (int l = 0, offset = 0; l < circlePoints; l++, offset += lathePoints)
   {
      for (int m = 0; m < lathePoints - 1; m++)
      {
         indices[k++] = (m + offset) % latheVts;
         indices[k++] = (m + offset + lathePoints) % latheVts;
         indices[k++] = (m + offset + 1 + lathePoints) % latheVts;

         indices[k++] = (m + offset + 1 + lathePoints) % latheVts;
         indices[k++] = (m + offset + 1) % latheVts;
         indices[k++] = (m + offset) % latheVts;
      }
   }

   // set up the vertex list for the spring
   Vertex3D_NoTex2 *v = &buf[latheVts + 1];
   for (int l = 0, offset = latheVts; l < springIndices; l += 12, offset += 3, v += 3)
   {
      // Direct3D only renders faces if the vertices are in clockwise
      // order.  We want to render the spring all the way around, so
      // we need to use different vertex ordering for faces that are
      // above and below the vertical midpoint on the spring.  We
      // can use the z normal from the center spiral to determine
      // whether we're at a top or bottom face.  Note that all of
      // the springs in all frames have the same relative position
      // on the spiral, so we can use the first spiral as a proxy
      // for all of them - the only thing about the spring that
      // varies from frame to frame is the length of the spiral.
      if (v->nz <= 0.0f)
      {
         // top half vertices
         indices[k++] = offset + 0;
         indices[k++] = offset + 3;
         indices[k++] = offset + 1;

         indices[k++] = offset + 1;
         indices[k++] = offset + 3;
         indices[k++] = offset + 4;

         indices[k++] = offset + 4;
         indices[k++] = offset + 5;
         indices[k++] = offset + 2;

         indices[k++] = offset + 2;
         indices[k++] = offset + 1;
         indices[k++] = offset + 4;
      }
      else
      {
         // bottom half vertices
         indices[k++] = offset + 3;
         indices[k++] = offset + 0;
         indices[k++] = offset + 4;

         indices[k++] = offset + 4;
         indices[k++] = offset + 0;
         indices[k++] = offset + 1;

         indices[k++] = offset + 1;
         indices[k++] = offset + 2;
         indices[k++] = offset + 5;

         indices[k++] = offset + 5;
         indices[k++] = offset + 1;
         indices[k++] = offset + 2;
      }
   }

   // if applicable, set up the vertex list for the flat plunger
   if (m_d.m_type == PlungerTypeFlat)
   {
      // for the flat rectangle, we just need two triangles:
      // bottom left - top left - top right
      // and top right - bottom right - bottom left
      indices[0] = 0;
      indices[1] = 1;
      indices[2] = 2;

      indices[3] = 2;
      indices[4] = 3;
      indices[5] = 0;

      k = 6;
   }

   // create the new index buffer
   if (indexBuffer)
      indexBuffer->release();
   indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(k, indices);

   // done with the index scratch pad
   delete[] indices;

   // done with the vertex buffer
   vertexBuffer->unlock();

   // delete our custom descriptor, if we created one
   if (customDesc != 0)
   {
      delete[] customDesc->c;
      delete customDesc;
   }
}

void Plunger::RenderStatic(RenderDevice* pd3dDevice)
{
}

STDMETHODIMP Plunger::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IPlunger,
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

HRESULT Plunger::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_v, sizeof(Vertex2D));
   bw.WriteFloat(FID(WDTH), m_d.m_width);
   bw.WriteFloat(FID(HIGH), m_d.m_height);
   bw.WriteFloat(FID(ZADJ), m_d.m_zAdjust);
   bw.WriteFloat(FID(HPSL), m_d.m_stroke);
   bw.WriteFloat(FID(SPDP), m_d.m_speedPull);
   bw.WriteFloat(FID(SPDF), m_d.m_speedFire);
   bw.WriteInt(FID(TYPE), m_d.m_type);
   bw.WriteInt(FID(ANFR), m_d.m_animFrames);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteString(FID(IMAG), m_d.m_szImage);

   bw.WriteFloat(FID(MESTH), m_d.m_mechStrength);
   bw.WriteBool(FID(MECH), m_d.m_mechPlunger);
   bw.WriteBool(FID(APLG), m_d.m_autoPlunger);

   bw.WriteFloat(FID(MPRK), m_d.m_parkPosition);
   bw.WriteFloat(FID(PSCV), m_d.m_scatterVelocity);
   bw.WriteFloat(FID(MOMX), m_d.m_momentumXfer);

   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteBool(FID(VSBL), m_d.m_fVisible);
   bw.WriteBool(FID(REEN), m_d.m_fReflectionEnabled);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

   bw.WriteString(FID(TIPS), m_d.m_szTipShape);
   bw.WriteFloat(FID(RODD), m_d.m_rodDiam);
   bw.WriteFloat(FID(RNGG), m_d.m_ringGap);
   bw.WriteFloat(FID(RNGD), m_d.m_ringDiam);
   bw.WriteFloat(FID(RNGW), m_d.m_ringWidth);
   bw.WriteFloat(FID(SPRD), m_d.m_springDiam);
   bw.WriteFloat(FID(SPRG), m_d.m_springGauge);
   bw.WriteFloat(FID(SPRL), m_d.m_springLoops);
   bw.WriteFloat(FID(SPRE), m_d.m_springEndLoops);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Plunger::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   m_d.m_color = RGB(76, 76, 76); //initialize color for new plunger
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL Plunger::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_d.m_v, sizeof(Vertex2D));
   }
   else if (id == FID(WDTH))
   {
      pbr->GetFloat(&m_d.m_width);
   }
   else if (id == FID(ZADJ))
   {
      pbr->GetFloat(&m_d.m_zAdjust);
   }
   else if (id == FID(HIGH))
   {
      pbr->GetFloat(&m_d.m_height);
   }
   else if (id == FID(HPSL))
   {
      pbr->GetFloat(&m_d.m_stroke);
   }
   else if (id == FID(SPDP))
   {
      pbr->GetFloat(&m_d.m_speedPull);
   }
   else if (id == FID(SPDF))
   {
      pbr->GetFloat(&m_d.m_speedFire);
   }
   else if (id == FID(MESTH))
   {
      pbr->GetFloat(&m_d.m_mechStrength);
   }
   else if (id == FID(MPRK))
   {
      pbr->GetFloat(&m_d.m_parkPosition);
   }
   else if (id == FID(PSCV))
   {
      pbr->GetFloat(&m_d.m_scatterVelocity);
   }
   else if (id == FID(MOMX))
   {
      pbr->GetFloat(&m_d.m_momentumXfer);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(MECH))
   {
      pbr->GetBool(&m_d.m_mechPlunger);
   }
   else if (id == FID(APLG))
   {
      pbr->GetBool(&m_d.m_autoPlunger);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(TYPE))
   {
      pbr->GetInt(&m_d.m_type);
   }
   else if (id == FID(ANFR))
   {
      pbr->GetInt(&m_d.m_animFrames);
   }
   else if (id == FID(MATR))
   {
      pbr->GetString(m_d.m_szMaterial);
   }
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(VSBL))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(REEN))
   {
      pbr->GetBool(&m_d.m_fReflectionEnabled);
   }
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(TIPS))
   {
      pbr->GetString(m_d.m_szTipShape);
   }
   else if (id == FID(RODD))
   {
      pbr->GetFloat(&m_d.m_rodDiam);
   }
   else if (id == FID(RNGG))
   {
      pbr->GetFloat(&m_d.m_ringGap);
   }
   else if (id == FID(RNGD))
   {
      pbr->GetFloat(&m_d.m_ringDiam);
   }
   else if (id == FID(RNGW))
   {
      pbr->GetFloat(&m_d.m_ringWidth);
   }
   else if (id == FID(SPRD))
   {
      pbr->GetFloat(&m_d.m_springDiam);
   }
   else if (id == FID(SPRG))
   {
      pbr->GetFloat(&m_d.m_springGauge);
   }
   else if (id == FID(SPRL))
   {
      pbr->GetFloat(&m_d.m_springLoops);
   }
   else if (id == FID(SPRE))
   {
      pbr->GetFloat(&m_d.m_springEndLoops);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }
   return fTrue;
}

HRESULT Plunger::InitPostLoad()
{
   return S_OK;
}

STDMETHODIMP Plunger::PullBack()
{
   // initiate a pull; the speed is set by our pull speed property
   if (m_phitplunger)
      m_phitplunger->m_plungeranim.PullBack(m_d.m_speedPull);

   return S_OK;
}

STDMETHODIMP Plunger::MotionDevice(int *pVal)
{
   *pVal = g_pplayer->m_pininput.uShockType;

   return S_OK;
}

STDMETHODIMP Plunger::Position(int *pVal)
{
   //      *pVal=m_curMechPlungerPos;
   if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_PBWIZARD)
   {
      const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
      float tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
      tmp = tmp / range + m_d.m_parkPosition;           //scale and offset
      *pVal = (int)(tmp*(float)(1.0 / 0.04));
   }

   if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_ULTRACADE)
   {
      const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
      float tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
      tmp = tmp / range + m_d.m_parkPosition;           //scale and offset
      *pVal = (int)(tmp*(float)(1.0 / 0.04));
   }

   if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_SIDEWINDER)
   {
      const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
      float tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
      tmp = tmp / range + m_d.m_parkPosition;           //scale and offset
      *pVal = (int)(tmp*(float)(1.0 / 0.04));
   }

   if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_VIRTUAPIN)
   {
      const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
      float tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
      tmp = tmp / range + m_d.m_parkPosition;           //scale and offset
      *pVal = (int)(tmp*(float)(1.0 / 0.04));
   }

   if (g_pplayer->m_pininput.uShockType == USHOCKTYPE_GENERIC)
   {
      float tmp;
      if (g_pplayer->m_pininput.m_linearPlunger)
      {
         // Use a single linear scaling function.  Constrain the line to the physical
         // plunger calibration, which we define as follows: the rest position is at 0,
         // the fully retracted position is at JOYRANGEMX.  The fully compressed position
         // is *not* part of the calibration, since that would over-constrain the
         // calibration.  Instead, assume that the response on the negative (compression)
         // side is a linear extension of the positive (retraction) side.  Calculate
         // the scaling function as mx+b - the calibration constraints give us the following
         // parameters:
         const float m = (1.0f - m_d.m_parkPosition)*(float)(1.0 / JOYRANGEMX), b = m_d.m_parkPosition;

         // calculate the rescaled value
         tmp = m*g_pplayer->m_curMechPlungerPos + b;

         // Because we don't have a calibration constraint on the negative side of the
         // axis, the physical plunger could report a negative value that goes beyond
         // the minimum on the virtual plunger.  Force it into range.
         if (tmp < 0.0f)
            tmp = 0.0f;
      }
      else
      {
         tmp = (g_pplayer->m_curMechPlungerPos < 0.f) ? g_pplayer->m_curMechPlungerPos*m_d.m_parkPosition : (g_pplayer->m_curMechPlungerPos*(1.0f - m_d.m_parkPosition));
         const float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
         tmp = tmp / range + m_d.m_parkPosition;           //scale and offset
      }
      *pVal = (int)(tmp*(float)(1.0 / 0.04));
   }

   //      return tmp;

   //      float range = (float)JOYRANGEMX * (1.0f - m_d.m_parkPosition) - (float)JOYRANGEMN *m_d.m_parkPosition; // final range limit
   //      float tmp = ((float)(JOYRANGEMN-1) < 0) ? (float)(JOYRANGEMN-1)*m_d.m_parkPosition : (float)(JOYRANGEMN-1)*(1.0f - m_d.m_parkPosition);
   //      tmp = tmp/range + m_d.m_parkPosition;           //scale and offset
   //      *pVal = tmp;
   return S_OK;
}

STDMETHODIMP Plunger::Fire()
{
   if (m_phitplunger)
   {
      // check for an auto plunger
      if (m_d.m_autoPlunger)
      {
         // Auto Plunger - this models a "Launch Ball" button or a
         // ROM-controlled launcher, rather than a player-operated
         // spring plunger.  In a physical machine, this would be
         // implemented as a solenoid kicker, so the amount of force
         // is constant (modulo some mechanical randomness).  Simulate
         // this by triggering a release from the maximum retracted
         // position.
         m_phitplunger->m_plungeranim.Fire(1.0f);
      }
      else
      {
         // Regular plunger - trigger a release from the current
         // position, using the keyboard firing strength.
         m_phitplunger->m_plungeranim.Fire();
      }
   }

#ifdef LOG
   const int i = g_pplayer->m_vmover.IndexOf(m_phitplunger);
   fprintf(g_pplayer->m_flog, "Plunger Release %d\n", i);
#endif

   return S_OK;
}

STDMETHODIMP Plunger::get_PullSpeed(float *pVal)
{
   *pVal = m_d.m_speedPull;

   return S_OK;
}

STDMETHODIMP Plunger::put_PullSpeed(float newVal)
{
   STARTUNDO

      m_d.m_speedPull = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_FireSpeed(float *pVal)
{
   *pVal = m_d.m_speedFire;

   return S_OK;
}

STDMETHODIMP Plunger::put_FireSpeed(float newVal)
{
   STARTUNDO

      m_d.m_speedFire = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_Type(PlungerType *pVal)
{
   *pVal = m_d.m_type;

   return S_OK;
}

STDMETHODIMP Plunger::put_Type(PlungerType newVal)
{
   STARTUNDO

      m_d.m_type = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);


   return S_OK;
}

STDMETHODIMP Plunger::put_Material(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Plunger::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if(tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO

   strcpy_s(m_d.m_szImage,szImage);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Plunger::get_AnimFrames(int *pVal)
{
   *pVal = m_d.m_animFrames;
   return S_OK;
}

STDMETHODIMP Plunger::put_AnimFrames(int newVal)
{
   STARTUNDO
      m_d.m_animFrames = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_TipShape(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szTipShape, -1, wz, MAXTIPSHAPE);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Plunger::put_TipShape(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szTipShape, MAXTOKEN, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_RodDiam(float *pVal)
{
   *pVal = m_d.m_rodDiam;

   return S_OK;
}

STDMETHODIMP Plunger::put_RodDiam(float newVal)
{
   STARTUNDO

      m_d.m_rodDiam = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_RingGap(float *pVal)
{
   *pVal = m_d.m_ringGap;

   return S_OK;
}

STDMETHODIMP Plunger::put_RingGap(float newVal)
{
   STARTUNDO

      m_d.m_ringGap = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_RingDiam(float *pVal)
{
   *pVal = m_d.m_ringDiam;

   return S_OK;
}

STDMETHODIMP Plunger::put_RingDiam(float newVal)
{
   STARTUNDO

      m_d.m_ringDiam = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_RingWidth(float *pVal)
{
   *pVal = m_d.m_ringWidth;

   return S_OK;
}

STDMETHODIMP Plunger::put_RingWidth(float newVal)
{
   STARTUNDO

      m_d.m_ringWidth = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_SpringDiam(float *pVal)
{
   *pVal = m_d.m_springDiam;

   return S_OK;
}

STDMETHODIMP Plunger::put_SpringDiam(float newVal)
{
   STARTUNDO

      m_d.m_springDiam = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_SpringGauge(float *pVal)
{
   *pVal = m_d.m_springGauge;

   return S_OK;
}

STDMETHODIMP Plunger::put_SpringGauge(float newVal)
{
   STARTUNDO

      m_d.m_springGauge = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_SpringLoops(float *pVal)
{
   *pVal = m_d.m_springLoops;

   return S_OK;
}

STDMETHODIMP Plunger::put_SpringLoops(float newVal)
{
   STARTUNDO

      m_d.m_springLoops = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_SpringEndLoops(float *pVal)
{
   *pVal = m_d.m_springEndLoops;

   return S_OK;
}

STDMETHODIMP Plunger::put_SpringEndLoops(float newVal)
{
   STARTUNDO

      m_d.m_springEndLoops = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::CreateBall(IBall **pBallEx)
{
   if (m_phitplunger)
   {
      const float x = (m_phitplunger->m_plungeranim.m_x + m_phitplunger->m_plungeranim.m_x2) * 0.5f;
      const float y = m_phitplunger->m_plungeranim.m_pos - (25.0f + 0.01f); //!! assumes ball radius 25

      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, x, y);

      Ball * const pball = g_pplayer->CreateBall(x, y, height, 0.1f, 0, 0);

      *pBallEx = pball->m_pballex;
      pball->m_pballex->AddRef();
   }

   return S_OK;
}

STDMETHODIMP Plunger::get_X(float *pVal)
{
   *pVal = m_d.m_v.x;

   return S_OK;
}

STDMETHODIMP Plunger::put_X(float newVal)
{
   STARTUNDO

      m_d.m_v.x = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_Y(float *pVal)
{
   *pVal = m_d.m_v.y;

   return S_OK;
}

STDMETHODIMP Plunger::put_Y(float newVal)
{
   STARTUNDO

      m_d.m_v.y = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_Width(float *pVal)
{
   *pVal = m_d.m_width;

   return S_OK;
}

STDMETHODIMP Plunger::put_Width(float newVal)
{
   STARTUNDO

      m_d.m_width = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_ZAdjust(float *pVal)
{
   *pVal = m_d.m_zAdjust;

   return S_OK;
}

STDMETHODIMP Plunger::put_ZAdjust(float newVal)
{
   STARTUNDO

      m_d.m_zAdjust = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_Surface(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Plunger::put_Surface(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_MechStrength(float *pVal)
{
   *pVal = m_d.m_mechStrength;

   return S_OK;
}

STDMETHODIMP Plunger::put_MechStrength(float newVal)
{
   STARTUNDO
      m_d.m_mechStrength = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_MechPlunger(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_mechPlunger);

   return S_OK;
}

STDMETHODIMP Plunger::put_MechPlunger(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_mechPlunger = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_AutoPlunger(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_autoPlunger);

   return S_OK;
}

STDMETHODIMP Plunger::put_AutoPlunger(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_autoPlunger = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Plunger::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fVisible = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_ParkPosition(float *pVal)
{
   *pVal = m_d.m_parkPosition;
   return S_OK;
}

STDMETHODIMP Plunger::put_ParkPosition(float newVal)
{
   STARTUNDO
      m_d.m_parkPosition = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_Stroke(float *pVal)
{
   *pVal = m_d.m_stroke;

   return S_OK;
}

STDMETHODIMP Plunger::put_Stroke(float newVal)
{
   STARTUNDO

      if (newVal < 16.5f) newVal = 16.5f;
   m_d.m_stroke = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_ScatterVelocity(float *pVal)
{
   *pVal = m_d.m_scatterVelocity;

   return S_OK;
}

STDMETHODIMP Plunger::put_ScatterVelocity(float newVal)
{
   STARTUNDO
      m_d.m_scatterVelocity = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_MomentumXfer(float *pVal)
{
   *pVal = m_d.m_momentumXfer;

   return S_OK;
}

STDMETHODIMP Plunger::put_MomentumXfer(float newVal)
{
   STARTUNDO
      m_d.m_momentumXfer = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Plunger::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fReflectionEnabled);

   return S_OK;
}

STDMETHODIMP Plunger::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fReflectionEnabled = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

void Plunger::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPLUNGER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPLUNGER_PHYSICS, IDS_STATE);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}
