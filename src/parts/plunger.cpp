// license:GPLv3+

#include "core/stdafx.h"
#include "renderer/Shader.h"

Plunger::~Plunger()
{
   assert(m_rd == nullptr);
}

Plunger *Plunger::CopyForPlay(PinTable *live_table) const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Plunger, live_table)
   return dst;
}

HRESULT Plunger::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_v.x = x;
   m_d.m_v.y = y;
   return forPlay ? S_OK : InitVBA(true, nullptr);
}

#define LinkProp(field, prop) field = fromMouseClick ? g_pvp->m_settings.GetDefaultPropsPlunger_##prop() : Settings::GetDefaultPropsPlunger_##prop##_Default()
void Plunger::SetDefaults(const bool fromMouseClick)
{
   LinkProp(m_d.m_height, Height);
   LinkProp(m_d.m_width, Width);
   LinkProp(m_d.m_zAdjust, ZAdjust);
   LinkProp(m_d.m_stroke, Stroke);
   LinkProp(m_d.m_speedPull, PullSpeed);
   LinkProp(m_d.m_type, PlungerType);
   LinkProp(m_d.m_animFrames, AnimFrames);
   LinkProp(m_d.m_color, Color);
   LinkProp(m_d.m_szImage, Image);
   LinkProp(m_d.m_szSurface, Surface);
   LinkProp(m_d.m_mechPlunger, MechPlunger);
   LinkProp(m_d.m_autoPlunger, AutoPlunger);
   LinkProp(m_d.m_visible, Visible);
   LinkProp(m_d.m_szTipShape, CustomTipShape);
   LinkProp(m_d.m_rodDiam, CustomRodDiam);
   LinkProp(m_d.m_ringGap, CustomRingGap);
   LinkProp(m_d.m_ringDiam, CustomRingDiam);
   LinkProp(m_d.m_ringWidth, CustomRingWidth);
   LinkProp(m_d.m_springDiam, CustomSpringDiam);
   LinkProp(m_d.m_springGauge, CustomSpringGauge);
   LinkProp(m_d.m_springLoops, CustomSpringLoops);
   LinkProp(m_d.m_springEndLoops, CustomSpringEndLoops);
   LinkProp(m_d.m_reflectionEnabled, ReflectionEnabled);
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);
   SetDefaultPhysics(fromMouseClick);
}

void Plunger::SetDefaultPhysics(const bool fromMouseClick)
{
   LinkProp(m_d.m_speedFire, ReleaseSpeed);
   LinkProp(m_d.m_mechStrength, MechStrength);
   LinkProp(m_d.m_parkPosition, ParkPosition);
   LinkProp(m_d.m_scatterVelocity, ScatterVelocity);
   LinkProp(m_d.m_momentumXfer, MomentumXfer);
}
#undef LinkProp

void Plunger::WriteRegDefaults()
{
#define LinkProp(field, prop) g_pvp->m_settings.SetDefaultPropsPlunger_##prop(field, false)
   LinkProp(m_d.m_height, Height);
   LinkProp(m_d.m_width, Width);
   LinkProp(m_d.m_zAdjust, ZAdjust);
   LinkProp(m_d.m_stroke, Stroke);
   LinkProp(m_d.m_speedPull, PullSpeed);
   LinkProp(m_d.m_type, PlungerType);
   LinkProp(m_d.m_animFrames, AnimFrames);
   LinkProp(m_d.m_color, Color);
   LinkProp(m_d.m_szImage, Image);
   LinkProp(m_d.m_szSurface, Surface);
   LinkProp(m_d.m_mechPlunger, MechPlunger);
   LinkProp(m_d.m_autoPlunger, AutoPlunger);
   LinkProp(m_d.m_visible, Visible);
   LinkProp(m_d.m_szTipShape, CustomTipShape);
   LinkProp(m_d.m_rodDiam, CustomRodDiam);
   LinkProp(m_d.m_ringGap, CustomRingGap);
   LinkProp(m_d.m_ringDiam, CustomRingDiam);
   LinkProp(m_d.m_ringWidth, CustomRingWidth);
   LinkProp(m_d.m_springDiam, CustomSpringDiam);
   LinkProp(m_d.m_springGauge, CustomSpringGauge);
   LinkProp(m_d.m_springLoops, CustomSpringLoops);
   LinkProp(m_d.m_springEndLoops, CustomSpringEndLoops);
   LinkProp(m_d.m_speedFire, ReleaseSpeed);
   LinkProp(m_d.m_mechStrength, MechStrength);
   LinkProp(m_d.m_parkPosition, ParkPosition);
   LinkProp(m_d.m_scatterVelocity, ScatterVelocity);
   LinkProp(m_d.m_momentumXfer, MomentumXfer);
   LinkProp(m_d.m_reflectionEnabled, ReflectionEnabled);
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);
#undef LinkProp
}

void Plunger::UIRenderPass1(Sur * const psur)
{
}

void Plunger::UIRenderPass2(Sur * const psur)
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


#pragma region Physics

void Plunger::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI && GetPartGroup() != nullptr && GetPartGroup()->GetReferenceSpace() != PartGroupData::SpaceReference::SR_PLAYFIELD)
      return;

   if (isUI)
   {
      // FIXME implement UI picking
   }
   else
   {
      const float zheight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_v.x, m_d.m_v.y);

      HitPlunger *const php = new HitPlunger(m_d.m_v.x - m_d.m_width, m_d.m_v.y + m_d.m_height, m_d.m_v.x + m_d.m_width, zheight, m_d.m_v.y - m_d.m_stroke, m_d.m_v.y, this);

      physics->AddCollider(php, isUI);
      php->m_pplunger = this;
      m_phitplunger = php;
   }
}

void Plunger::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI)
      m_phitplunger = nullptr; // possible memory leak here?
}

#pragma endregion


void Plunger::SetObjectPos()
{
   m_vpinball->SetObjectPosCur(m_d.m_v.x, m_d.m_v.y);
}

void Plunger::MoveOffset(const float dx, const float dy)
{
   m_d.m_v.x += dx;
   m_d.m_v.y += dy;
}

Vertex2D Plunger::GetCenter() const
{
   return m_d.m_v;
}

void Plunger::PutCenter(const Vertex2D& pv)
{
   m_d.m_v = pv;
}


#pragma region Rendering

// Ported at: VisualPinball.Engine/VPT/Plunger/PlungerCoord.cs

// Modern Plunger - added by rascal
static constexpr PlungerCoord modernCoords[] =
{
   { 0.20f, 0.0f, 0.00f, 1.0f, 0.0f },  // tip
   { 0.30f, 3.0f, 0.11f, 1.0f, 0.0f },  // tip
   { 0.35f, 5.0f, 0.14f, 1.0f, 0.0f },  // tip
   { 0.35f, 23.0f, 0.19f, 1.0f, 0.0f }, // tip
   { 0.45f, 23.0f, 0.21f, 0.8f, 0.0f }, // ring
   { 0.25f, 24.0f, 0.25f, 0.3f, 0.0f }, // shaft
   { 0.25f, 100.0f, 1.00f, 0.3f, 0.0f } // shaft
};
static const PlungerDesc modernDesc = {
   std::size(modernCoords), (PlungerCoord*)modernCoords
};

// Flat Plunger.  This is a special case with no "lathe" entries;
// instead we define a simple rectangle covering the stroke length
// by 2x the nominal width (1x the width on each side of the centerline).
// The pre-render code knows to set up the flat rendering when there are
// no lathe coordinates.
constexpr static PlungerDesc flatDesc = { 0, 0 };


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

// Ported at: VisualPinball.Engine/VPT/Plunger/PlungerDesc.cs
//            VisualPinball.Engine/VPT/Plunger/PlungerMeshGenerator.cs

void Plunger::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;
   const float zheight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_v.x, m_d.m_v.y) + m_d.m_zAdjust;
   const float stroke = m_d.m_stroke;
   const float beginy = m_d.m_v.y;
   const float endy = m_d.m_v.y - stroke;
   m_cframes = (int)(stroke*(float)(PLUNGER_FRAME_COUNT / 80.0)) + 1; // 25 frames per 80 units travel
   const float inv_scale = (m_cframes > 1) ? (1.0f / (float)(m_cframes - 1)) : 0.0f;
   const float dyPerFrame = (endy - beginy) * inv_scale;
   const int circlePoints = (m_d.m_type == PlungerTypeFlat) ? 0 : 24;
   float springLoops = 0.0f, springEndLoops = 0.0f;
   float springGauge = 0.0f;
   float springRadius = 0.0f;
   constexpr float springMinSpacing = 2.2f;
   float rody = beginy + m_d.m_height;
   constexpr float zScale = 1.f;

   // note the number of cells in the source image
   int srcCells = m_d.m_animFrames;
   if (srcCells < 1)
      srcCells = 1;

   // figure the width in relative units (0..1) of each cell
   const float cellWid = 1.0f / float(srcCells);

   // figure which plunger descriptor we're using
   const PlungerDesc *desc;
   PlungerDesc *customDesc = nullptr;
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
      for (const char *p = m_d.m_szTipShape.c_str(); *p != '\0'; ++p)
      {
         if (*p == ';')
         {
            ++nTip;
            ++nn;
         }
      }

      // allocate the descriptor and the coordinate array
      desc = customDesc = new PlungerDesc;
      customDesc->n = nn;
      customDesc->c = new PlungerCoord[nn];

      // figure the tip lathe descriptor from the shape point list
      PlungerCoord *c = customDesc->c;
      float tiplen = 0;
      for (const char *p = m_d.m_szTipShape.c_str(); *p != '\0'; c++)
      {
         // Parse the entry: "yOffset, diam".  yOffset is the
         // offset (in table distance units) from the previous
         // point.  "diam" is the diameter (relative to the
         // nominal width of the plunger, as given by the width
         // property) of the tip at this point.  1.0 means that
         // the diameter is the same as the nominal width; 0.5
         // is half the width.
         c->y = float(atof(nextTipToken(p)));
         c->r = float(atof(nextTipToken(p))) * 0.5f;

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
      c = customDesc->c;
      constexpr PlungerCoord c0 = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
      const PlungerCoord *cprv = &c0;
      for (int i = 0; i < nTip; ++i, cprv = c++)
      {
         // Figure the texture coordinate.  The tip is always
         // the top 25% of the overall texture; interpolate the
         // current lathe point's position within that 25%.
         c->tv = 0.24f * c->y / tiplen;

         // Figure the normal as the average of the surrounding
         // surface normals.
         const PlungerCoord * const cnxt = (i + 1 < nTip ? c + 1 : c);
         const float x0 = cprv->r, y0 = cprv->y;
         const float x1 = cnxt->r, y1 = cnxt->y;
         const float yd = y1 - y0;
         const float xd = (x1 - x0) * m_d.m_width;	 
         //const float th = atan2f(yd, xd);
         //c->nx = sinf(th);
         //c->ny = -cosf(th);
         const float r = sqrtf(xd*xd + yd*yd);
         c->nx = yd / r;
         c->ny = -xd / r;
      }

      // add the inner edge of the tip (abutting the rod)
      const float rRod = m_d.m_rodDiam / 2.0f;
      float y = tiplen;
      (c++)->set(rRod, y, 0.24f, 1.0f, 0.0f);

      // add the gap between tip and ring (texture is in the rod
      // quadrant of overall texture, 50%-75%)
      (c++)->set(rRod, y, 0.51f, 1.0f, 0.0f);
      y += m_d.m_ringGap;
      (c++)->set(rRod, y, 0.55f, 1.0f, 0.0f);

      // add the ring (texture is in the ring quadrant, 25%-50%)
      const float rRing = m_d.m_ringDiam / 2.0f;
      (c++)->set(rRod, y, 0.26f, 0.0f, -1.0f);
      (c++)->set(rRing, y, 0.33f, 0.0f, -1.0f);
      (c++)->set(rRing, y, 0.33f, 1.0f, 0.0f);
      y += m_d.m_ringWidth;
      (c++)->set(rRing, y, 0.42f, 1.0f, 0.0f);
      (c++)->set(rRing, y, 0.42f, 0.0f, 1.0f);
      (c++)->set(rRod, y, 0.49f, 0.0f, 1.0f);

      // set the spring values from the properties
      springRadius = m_d.m_springDiam * 0.5f;
      springGauge = m_d.m_springGauge;
      springLoops = m_d.m_springLoops;
      springEndLoops = m_d.m_springEndLoops;

      // add the top of the shaft (texture is in the 50%-75% quadrant)
      (c++)->set(rRod, y, 0.51f, 1.0f, 0.0f);

      // Figure the fully compressed spring length.  This is
      // the lower bound for the rod length.
      const float springMin = (springLoops + springEndLoops)*springMinSpacing;

      // Figure the rod bottom position (rody).  This is the fully
      // retracted tip position (beginy), plus the length of the parts
      // at the end that don't compress with the spring (y), plus the
      // fully retracted spring length.
      rody = beginy + y + springMin;
      (c++)->set(rRod, rody, 0.74f, 1.0f, 0.0f);
   }
   break;
   }//switch end

   // get the number of lathe points from the descriptor
   const int lathePoints = desc->n;

   // calculate the frame rendering details
   int latheVts = 0;
   int springIndices = 0;
   if (m_d.m_type == PlungerTypeFlat)
   {
      // For the flat plunger, we render every frame as a simple
      // flat rectangle.  This requires four vertices for the corners,
      // and two triangles -> 6 indices.
      m_vtsPerFrame = 4;
      m_indicesPerFrame = 6;
   }
   else
   {
      // For all other plungers, we render one circle per lathe
      // point.  Each circle has 'circlePoints' vertices.  We
      // also need to render the spring:  this consists of 3
      // spirals, where each sprial has 'springLoops' loops
      // times 'circlePoints' vertices.
      latheVts = lathePoints * circlePoints;
      const int springVts = int((springLoops + springEndLoops) * (float)circlePoints) * 3;
      m_vtsPerFrame = latheVts + springVts;

      // For the lathed section, we need two triangles == 6
      // indices for every point on every lathe circle past
      // the first.  (We connect pairs of lathe circles, so
      // the first one doesn't count: two circles -> one set
      // of triangles, three circles -> two sets, etc).
      const int latheIndices = 6 * circlePoints * (lathePoints - 1);

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
      m_indicesPerFrame = latheIndices + springIndices;
   }

   // figure the relative spring gauge, in terms of the overall width
   const float springGaugeRel = springGauge / m_d.m_width;

   std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(m_rd, m_cframes * m_vtsPerFrame);

   Vertex3D_NoTex2 *buf;
   vertexBuffer->Lock(buf);

   Vertex3D_NoTex2 *ptr = buf;

   // Build the animation frames.  We have 'm_cframes' frames total.  The 0th frame
   // shows the plunger in the maximum retracted position; the m_cframes-1'th frame
   // is the maximum forward position.
   for (int i = 0; i < m_cframes; i++, ptr += m_vtsPerFrame)
   {
      const float ytip = beginy + dyPerFrame*(float)i;

      if (m_d.m_type != PlungerTypeFlat)
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
         const float angle = ((float)(M_PI*2.0) / (float)circlePoints)*(float)l;
         const float sn = sinf(angle);
         const float cs = cosf(angle);
         const PlungerCoord *c = desc->c;
         Vertex3D_NoTex2 *pm = &ptr[offset];
         float pmm1tv = 0.f;
         for (int m = 0; m < lathePoints; m++, ++c, ++pm)
         {
            // get the current point's coordinates
            float y = c->y + ytip;
            const float r = c->r;
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
               const float ratio = float(i) * inv_scale;
               tv = pmm1tv + (tv - pmm1tv)*ratio;
            }

            // figure the point coordinates
            pm->x = r * (sn * m_d.m_width) + m_d.m_v.x;
            pm->y = y;
            pm->z = (r * (cs * m_d.m_width) + m_d.m_width + zheight) * zScale;
            pm->nx = c->nx * sn;
            pm->ny = c->ny;
            pm->nz = -c->nx * cs;
            pm->tu = tu;
            pm->tv = pmm1tv = tv;
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
         int cellIdx = srcCells - 1 - int(((float)i * float(srcCells) / float(m_cframes)) + 0.5f);
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
         ptr[1].y = yTop;       ptr[1].ny = 0.0f;          ptr[1].tv = 0.0f;               // top
         ptr[1].z = z;          ptr[1].nz = -1.0f;
         ptr[2].x = xRt;        ptr[2].nx = 0.0f;          ptr[2].tu = tu_local + cellWid; // right
         ptr[2].y = yTop;       ptr[2].ny = 0.0f;          ptr[2].tv = 0.0f;               // top
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
         const float y1 = rody;
         int n = int((springLoops + springEndLoops) * (float)circlePoints);
         const int nEnd = int(springEndLoops * (float)circlePoints);
         const int nMain = n - nEnd;
         const float yEnd = springEndLoops * springGauge * springMinSpacing;
         const float dyMain = (y1 - y0 - yEnd) / (float)(nMain - 1);
         const float dyEnd = yEnd / (float)(nEnd - 1);
         float dy = dyEnd;
         Vertex3D_NoTex2 * pm = &ptr[offset];
         const float dtheta = float(M_PI*2.0) / (float)(circlePoints - 1) + (float)M_PI / (float)(n - 1);
         for (float theta = (float)M_PI, y = y0; n != 0; --n, theta += dtheta, y += dy)
         {
            if (n == nMain) dy = dyMain;
            if (theta >= float(M_PI*2.0)) theta -= float(M_PI*2.0);
            const float sn = sinf(theta);
            const float cs = cosf(theta);

            // set the point on the front spiral
            pm->x = springRadius * (sn * m_d.m_width) + m_d.m_v.x;
            pm->y = y - springGauge;
            pm->z = (springRadius * (cs * m_d.m_width) + m_d.m_width + zheight) * zScale;
            pm->nx = 0.0f;
            pm->ny = -1.0f;
            pm->nz = 0.0f;
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

         //y1 += 0;
      }
   }

   // for sorting and bounding
   m_boundingSphereRadius = 0.5f * (m_d.m_stroke + m_d.m_height);
   m_boundingSphereCenter.Set(m_d.m_v.x, 0.5f * (m_d.m_v.y - m_d.m_stroke + m_d.m_v.y + m_d.m_height), zScale * (zheight + m_d.m_width * (m_d.m_type == PlungerTypeFlat ? 1.25f : 1.f)));

   // set up the vertex index list
   unsigned int *const indices = new unsigned int[m_indicesPerFrame * m_cframes];
   int k = 0;

   for (int f = 0; f < m_cframes; f++)
   {
      const int f_offset = f * m_vtsPerFrame;

      // if applicable, set up the vertex list for the flat plunger
      if (m_d.m_type == PlungerTypeFlat)
      {
         // for the flat rectangle, we just need two triangles:
         // bottom left - top left - top right
         // and top right - bottom right - bottom left
         indices[k++] = f_offset + 0;
         indices[k++] = f_offset + 1;
         indices[k++] = f_offset + 2;

         indices[k++] = f_offset + 2;
         indices[k++] = f_offset + 3;
         indices[k++] = f_offset + 0;
         continue;
      }

      // set up the vertex list for the lathe circles
      for (int l = 0, offset = 0; l < circlePoints; l++, offset += lathePoints)
      {
         for (int m = 0; m < lathePoints - 1; m++)
         {
            indices[k++] = f_offset + (m + offset) % latheVts;
            indices[k++] = f_offset + (m + offset + lathePoints) % latheVts;
            indices[k++] = f_offset + (m + offset + 1 + lathePoints) % latheVts;

            indices[k++] = f_offset + (m + offset + 1 + lathePoints) % latheVts;
            indices[k++] = f_offset + (m + offset + 1) % latheVts;
            indices[k++] = f_offset + (m + offset) % latheVts;
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
            indices[k++] = f_offset + offset + 0;
            indices[k++] = f_offset + offset + 3;
            indices[k++] = f_offset + offset + 1;

            indices[k++] = f_offset + offset + 1;
            indices[k++] = f_offset + offset + 3;
            indices[k++] = f_offset + offset + 4;

            indices[k++] = f_offset + offset + 4;
            indices[k++] = f_offset + offset + 5;
            indices[k++] = f_offset + offset + 2;

            indices[k++] = f_offset + offset + 2;
            indices[k++] = f_offset + offset + 1;
            indices[k++] = f_offset + offset + 4;
         }
         else
         {
            // bottom half vertices
            indices[k++] = f_offset + offset + 3;
            indices[k++] = f_offset + offset + 0;
            indices[k++] = f_offset + offset + 4;

            indices[k++] = f_offset + offset + 4;
            indices[k++] = f_offset + offset + 0;
            indices[k++] = f_offset + offset + 1;

            indices[k++] = f_offset + offset + 1;
            indices[k++] = f_offset + offset + 2;
            indices[k++] = f_offset + offset + 5;

            indices[k++] = f_offset + offset + 5;
            indices[k++] = f_offset + offset + 1;
            indices[k++] = f_offset + offset + 2;
         }
      }
   }
   vertexBuffer->Unlock();

   // create the new index buffer
   std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(m_rd, k, indices);
   delete[] indices;

   // Create the mesh buffer
   m_meshBuffer = std::make_shared<MeshBuffer>(GetName(), vertexBuffer, indexBuffer, true);

   // delete our custom descriptor, if we created one
   if (customDesc != 0)
   {
      delete[] customDesc->c;
      delete customDesc;
   }
}

void Plunger::RenderRelease()
{
   assert(m_rd != nullptr);
   m_meshBuffer = nullptr;
   m_rd = nullptr;
}

void Plunger::UpdateAnimation(const float diff_time_msec)
{
   // Animation is updated by physics engine through a MoverObject. No additional visual animation here
}

void Plunger::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   assert(!m_backglass);
   const bool isStaticOnly = renderMask & Renderer::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Renderer::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Renderer::REFLECTION_PASS;
   TRACE_FUNCTION();

   if (isStaticOnly || !m_d.m_visible || (isReflectionPass && !m_d.m_reflectionEnabled))
      return;

   // TODO: get rid of frame stuff
   assert(m_phitplunger != nullptr);
   const PlungerMoverObject& pa = m_phitplunger->m_plungerMover;
   const int frame0 = (int)((pa.m_pos - pa.m_frameStart) / (pa.m_frameEnd - pa.m_frameStart) * (float)(m_cframes - 1) + 0.5f);
   const int frame = (frame0 < 0 ? 0 : frame0 >= m_cframes ? m_cframes - 1 : frame0);

   m_rd->ResetRenderState();
   m_rd->m_basicShader->SetBasic(m_ptable->GetMaterial(m_d.m_szMaterial), m_ptable->GetImage(m_d.m_szImage));
   m_rd->DrawMesh(m_rd->m_basicShader, false, m_boundingSphereCenter, 0.f /*m_boundingSphereRadius*/, m_meshBuffer, 
      RenderDevice::TRIANGLELIST, frame * m_indicesPerFrame, m_indicesPerFrame);
}

#pragma endregion


STDMETHODIMP Plunger::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IPlunger,
   };

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

   return S_FALSE;
}

HRESULT Plunger::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_v);
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

   bw.WriteFloat(FID(MEST), m_d.m_mechStrength);
   bw.WriteBool(FID(MECH), m_d.m_mechPlunger);
   bw.WriteBool(FID(APLG), m_d.m_autoPlunger);

   bw.WriteFloat(FID(MPRK), m_d.m_parkPosition);
   bw.WriteFloat(FID(PSCV), m_d.m_scatterVelocity);
   bw.WriteFloat(FID(MOMX), m_d.m_momentumXfer);

   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteBool(FID(VSBL), m_d.m_visible);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), m_wzName);

   bw.WriteString(FID(TIPS), m_d.m_szTipShape);
   bw.WriteFloat(FID(RODD), m_d.m_rodDiam);
   bw.WriteFloat(FID(RNGG), m_d.m_ringGap);
   bw.WriteFloat(FID(RNGD), m_d.m_ringDiam);
   bw.WriteFloat(FID(RNGW), m_d.m_ringWidth);
   bw.WriteFloat(FID(SPRD), m_d.m_springDiam);
   bw.WriteFloat(FID(SPRG), m_d.m_springGauge);
   bw.WriteFloat(FID(SPRL), m_d.m_springLoops);
   bw.WriteFloat(FID(SPRE), m_d.m_springEndLoops);

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Plunger::InitLoad(IStream *pstm, PinTable *ptable, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   m_d.m_color = RGB(76, 76, 76); //initialize color for new plunger
   SetDefaults(false);

   BiffReader br(pstm, this, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

bool Plunger::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): { int pid; pbr->GetInt(&pid); } break;
   case FID(VCEN): pbr->GetVector2(m_d.m_v); break;
   case FID(WDTH): pbr->GetFloat(m_d.m_width); break;
   case FID(ZADJ): pbr->GetFloat(m_d.m_zAdjust); break;
   case FID(HIGH): pbr->GetFloat(m_d.m_height); break;
   case FID(HPSL): pbr->GetFloat(m_d.m_stroke); break;
   case FID(SPDP): pbr->GetFloat(m_d.m_speedPull); break;
   case FID(SPDF): pbr->GetFloat(m_d.m_speedFire); break;
   case FID(MEST): pbr->GetFloat(m_d.m_mechStrength); break;
   case FID(MPRK): pbr->GetFloat(m_d.m_parkPosition); break;
   case FID(PSCV): pbr->GetFloat(m_d.m_scatterVelocity); break;
   case FID(MOMX): pbr->GetFloat(m_d.m_momentumXfer); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(MECH): pbr->GetBool(m_d.m_mechPlunger); break;
   case FID(APLG): pbr->GetBool(m_d.m_autoPlunger); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(NAME): pbr->GetWideString(m_wzName, std::size(m_wzName)); break;
   case FID(TYPE): pbr->GetInt(&m_d.m_type); break;
   case FID(ANFR): pbr->GetInt(m_d.m_animFrames); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(VSBL): pbr->GetBool(m_d.m_visible); break;
   case FID(REEN): pbr->GetBool(m_d.m_reflectionEnabled); break;
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(TIPS): pbr->GetString(m_d.m_szTipShape); break;
   case FID(RODD): pbr->GetFloat(m_d.m_rodDiam); break;
   case FID(RNGG): pbr->GetFloat(m_d.m_ringGap); break;
   case FID(RNGD): pbr->GetFloat(m_d.m_ringDiam); break;
   case FID(RNGW): pbr->GetFloat(m_d.m_ringWidth); break;
   case FID(SPRD): pbr->GetFloat(m_d.m_springDiam); break;
   case FID(SPRG): pbr->GetFloat(m_d.m_springGauge); break;
   case FID(SPRL): pbr->GetFloat(m_d.m_springLoops); break;
   case FID(SPRE): pbr->GetFloat(m_d.m_springEndLoops); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT Plunger::InitPostLoad()
{
   return S_OK;
}

STDMETHODIMP Plunger::PullBack()
{
   // initiate a pull; the speed is set by our pull speed property
   if (m_phitplunger)
   {
     if(g_pplayer->m_pininput.m_plunger_retract)
        m_phitplunger->m_plungerMover.PullBackandRetract(m_d.m_speedPull);
      else
        m_phitplunger->m_plungerMover.PullBack(m_d.m_speedPull);
   }

   return S_OK;
}

STDMETHODIMP Plunger::PullBackandRetract()
{
   // initiate a pull; the speed is set by our pull speed property
   if (m_phitplunger)
      m_phitplunger->m_plungerMover.PullBackandRetract(m_d.m_speedPull);

   return S_OK;
}

STDMETHODIMP Plunger::MotionDevice(int *pVal)
{
   *pVal = 6; // Deprecated: Always returns InputLayout::Generic
return S_OK;
}

// Returns the position of the plunger as a value between 0 and 25
// Note that g_pplayer->m_curMechPlungerPos is 0 at park position, which usually correspond to something like 4 or 5 here,
// leading to value from 5 to 25 when pulling the plunger, with value below 5 being when the plunger pass the park position.
STDMETHODIMP Plunger::Position(float *pVal)
{
   const PlungerMoverObject &pa = m_phitplunger->m_plungerMover;
   const float frame = (pa.m_frameEnd - pa.m_pos) / (pa.m_frameEnd - pa.m_frameStart);
   *pVal = 25.f * saturate(frame);
   return S_OK;
}

// Ported at: VisualPinball.Unity/VisualPinball.Unity/VPT/Plunger/PlungerApi.cs

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
         m_phitplunger->m_plungerMover.Fire(1.0f);
      }
      else
      {
         // Regular plunger - trigger a release from the current
         // position, using the keyboard firing strength.
         m_phitplunger->m_plungerMover.Fire();
      }
   }

   #ifdef LOG
   PLOGD << "Plunger Release " << GetName();
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
   m_d.m_speedPull = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_FireSpeed(float *pVal)
{
   *pVal = m_d.m_speedFire;
   return S_OK;
}

STDMETHODIMP Plunger::put_FireSpeed(float newVal)
{
   m_d.m_speedFire = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_Type(PlungerType *pVal)
{
   *pVal = m_d.m_type;
   return S_OK;
}

STDMETHODIMP Plunger::put_Type(PlungerType newVal)
{
   m_d.m_type = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_Material(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_d.m_szMaterial);
   return S_OK;
}

STDMETHODIMP Plunger::put_Material(BSTR newVal)
{
   m_d.m_szMaterial = MakeString(newVal);
   return S_OK;
}

STDMETHODIMP Plunger::get_Image(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_d.m_szImage);
   return S_OK;
}

STDMETHODIMP Plunger::put_Image(BSTR newVal)
{
   const string szImage = MakeString(newVal);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }
   m_d.m_szImage = szImage;

   return S_OK;
}

STDMETHODIMP Plunger::get_AnimFrames(int *pVal)
{
   *pVal = m_d.m_animFrames;
   return S_OK;
}

STDMETHODIMP Plunger::put_AnimFrames(int newVal)
{
   m_d.m_animFrames = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_TipShape(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_d.m_szTipShape);
   return S_OK;
}

STDMETHODIMP Plunger::put_TipShape(BSTR newVal)
{
   m_d.m_szTipShape = MakeString(newVal);
   return S_OK;
}

STDMETHODIMP Plunger::get_RodDiam(float *pVal)
{
   *pVal = m_d.m_rodDiam;
   return S_OK;
}

STDMETHODIMP Plunger::put_RodDiam(float newVal)
{
   m_d.m_rodDiam = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_RingGap(float *pVal)
{
   *pVal = m_d.m_ringGap;
   return S_OK;
}

STDMETHODIMP Plunger::put_RingGap(float newVal)
{
   m_d.m_ringGap = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_RingDiam(float *pVal)
{
   *pVal = m_d.m_ringDiam;
   return S_OK;
}

STDMETHODIMP Plunger::put_RingDiam(float newVal)
{
   m_d.m_ringDiam = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_RingWidth(float *pVal)
{
   *pVal = m_d.m_ringWidth;
   return S_OK;
}

STDMETHODIMP Plunger::put_RingWidth(float newVal)
{
   m_d.m_ringWidth = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_SpringDiam(float *pVal)
{
   *pVal = m_d.m_springDiam;
   return S_OK;
}

STDMETHODIMP Plunger::put_SpringDiam(float newVal)
{
   m_d.m_springDiam = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_SpringGauge(float *pVal)
{
   *pVal = m_d.m_springGauge;
   return S_OK;
}

STDMETHODIMP Plunger::put_SpringGauge(float newVal)
{
   m_d.m_springGauge = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_SpringLoops(float *pVal)
{
   *pVal = m_d.m_springLoops;
   return S_OK;
}

STDMETHODIMP Plunger::put_SpringLoops(float newVal)
{
   m_d.m_springLoops = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_SpringEndLoops(float *pVal)
{
   *pVal = m_d.m_springEndLoops;
   return S_OK;
}

STDMETHODIMP Plunger::put_SpringEndLoops(float newVal)
{
   m_d.m_springEndLoops = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::CreateBall(IBall **pResult)
{
   if (m_phitplunger)
   {
      const float x = (m_phitplunger->m_plungerMover.m_x + m_phitplunger->m_plungerMover.m_x2) * 0.5f;
      const float y = m_phitplunger->m_plungerMover.m_pos - (float)(PHYS_SKIN + 0.01); //!! assumes ball radius 25

      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, x, y);

      Ball *const pball = g_pplayer->CreateBall(x, y, height, 0.1f, 0, 0, 25.f, 1.f);

      *pResult = pball;
      pball->AddRef();
   }

   return S_OK;
}

STDMETHODIMP Plunger::get_X(float *pVal)
{
   *pVal = m_d.m_v.x;
   m_vpinball->SetStatusBarUnitInfo(string(), true);

   return S_OK;
}

STDMETHODIMP Plunger::put_X(float newVal)
{
   m_d.m_v.x = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_Y(float *pVal)
{
   *pVal = m_d.m_v.y;
   return S_OK;
}

STDMETHODIMP Plunger::put_Y(float newVal)
{
   m_d.m_v.y = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_Width(float *pVal)
{
   *pVal = m_d.m_width;
   return S_OK;
}

STDMETHODIMP Plunger::put_Width(float newVal)
{
   m_d.m_width = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_ZAdjust(float *pVal)
{
   *pVal = m_d.m_zAdjust;
   return S_OK;
}

STDMETHODIMP Plunger::put_ZAdjust(float newVal)
{
   m_d.m_zAdjust = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_Surface(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_d.m_szSurface);
   return S_OK;
}

STDMETHODIMP Plunger::put_Surface(BSTR newVal)
{
   m_d.m_szSurface = MakeString(newVal);
   return S_OK;
}

STDMETHODIMP Plunger::get_MechStrength(float *pVal)
{
   *pVal = m_d.m_mechStrength;
   return S_OK;
}

STDMETHODIMP Plunger::put_MechStrength(float newVal)
{
   m_d.m_mechStrength = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_MechPlunger(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_mechPlunger);
   return S_OK;
}

STDMETHODIMP Plunger::put_MechPlunger(VARIANT_BOOL newVal)
{
   m_d.m_mechPlunger = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Plunger::get_AutoPlunger(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_autoPlunger);
   return S_OK;
}

STDMETHODIMP Plunger::put_AutoPlunger(VARIANT_BOOL newVal)
{
   m_d.m_autoPlunger = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Plunger::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_visible);
   return S_OK;
}

STDMETHODIMP Plunger::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_visible = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Plunger::get_ParkPosition(float *pVal)
{
   *pVal = m_d.m_parkPosition;
   return S_OK;
}

STDMETHODIMP Plunger::put_ParkPosition(float newVal)
{
   m_d.m_parkPosition = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_Stroke(float *pVal)
{
   *pVal = m_d.m_stroke;
   return S_OK;
}

STDMETHODIMP Plunger::put_Stroke(float newVal)
{
   if (newVal < 16.5f) newVal = 16.5f;
   m_d.m_stroke = newVal;

   return S_OK;
}

STDMETHODIMP Plunger::get_ScatterVelocity(float *pVal)
{
   *pVal = m_d.m_scatterVelocity;
   return S_OK;
}

STDMETHODIMP Plunger::put_ScatterVelocity(float newVal)
{
   m_d.m_scatterVelocity = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_MomentumXfer(float *pVal)
{
   *pVal = m_d.m_momentumXfer;
   return S_OK;
}

STDMETHODIMP Plunger::put_MomentumXfer(float newVal)
{
   m_d.m_momentumXfer = newVal;
   return S_OK;
}

STDMETHODIMP Plunger::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);
   return S_OK;
}

STDMETHODIMP Plunger::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);
   return S_OK;
}
