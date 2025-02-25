// license:GPLv3+

#include "core/stdafx.h"
#include "quadtree.h"

#ifdef ENABLE_SSE_OPTIMIZATIONS
#define QUADTREE_SSE_LEAFTEST
#else
#pragma message ("Warning: No SSE quadtree tests")
#endif

HitQuadtree::~HitQuadtree()
{
   Reset(vector<HitObject*>());
}

void HitQuadtree::Reset(const vector<HitObject*>& vho)
{
#ifndef USE_EMBREE
   if (lefts_rights_tops_bottoms_zlows_zhighs != nullptr)
      _aligned_free(lefts_rights_tops_bottoms_zlows_zhighs);
   lefts_rights_tops_bottoms_zlows_zhighs = 0;

   if (!m_leaf)
      delete[] m_children;
   m_leaf = true;
   m_children = nullptr;
   m_unique = nullptr;

#else
   rtcReleaseScene(m_scene);
   rtcReleaseDevice(m_embree_device);
   m_embree_device = rtcNewDevice(nullptr);
   m_scene = nullptr;
#endif

   m_vho = vho;
}

#ifdef USE_EMBREE
#include <mutex>
static std::mutex mtx;

#define CHECK_EMBREE(dev) \
   { const RTCError rc = rtcGetDeviceError(dev); \
   switch (rc) { \
      case RTC_ERROR_NONE: break; \
      default: { char error[256]; sprintf_s(error, sizeof(error), "%u %s %d",rc,__FILE__,__LINE__); ShowError(error); break; }; \
   }}

void EmbreeBoundsFunc(const struct RTCBoundsFunctionArguments* const args)
{
   const HitObject * const ho = (*((const vector<HitObject*> *)args->geometryUserPtr))[args->primID];

   args->bounds_o->lower_x = ho->m_hitBBox.left;
   args->bounds_o->lower_y = ho->m_hitBBox.top;
   args->bounds_o->lower_z = ho->m_hitBBox.zlow;
   args->bounds_o->upper_x = ho->m_hitBBox.right;
   args->bounds_o->upper_y = ho->m_hitBBox.bottom;
   args->bounds_o->upper_z = ho->m_hitBBox.zhigh;
}
#endif

void HitQuadtree::Initialize()
{
   m_nLevels = 0;
#ifdef USE_EMBREE
   if (m_scene)
       rtcReleaseScene(m_scene);

   m_scene = rtcNewScene(m_embree_device);
   rtcSetSceneBuildQuality(m_scene, RTC_BUILD_QUALITY_HIGH);
   rtcSetSceneFlags(m_scene, RTC_SCENE_FLAG_ROBUST);

   const RTCGeometry geom = rtcNewGeometry(m_embree_device, RTC_GEOMETRY_TYPE_USER);
   rtcSetGeometryUserPrimitiveCount(geom, (*m_pvho).size());

   rtcSetGeometryUserData(geom, m_pvho);
   rtcSetGeometryBoundsFunction(geom, &EmbreeBoundsFunc, m_pvho);
   rtcSetGeometryIntersectFunction(geom, nullptr); // no ray tracing
   rtcSetGeometryOccludedFunction(geom, nullptr); // no shadow ray tracing
   rtcSetGeometryIntersectFilterFunction(geom, nullptr); // no trace filter

   rtcCommitGeometry(geom);
   // attach geometry to scene
   const unsigned int l_geom = rtcAttachGeometry(m_scene, geom); // no need to rtcDeleteGeometry(l_geom), as we throw away complete scene later-on
   rtcReleaseGeometry(geom);

   rtcCommitScene(m_scene);

   //RTCBounds b;
   //rtcGetSceneBounds(m_scene, &b);

   CHECK_EMBREE(m_embree_device);
#else
   FRect bounds; // FRect3D for an octree
   bounds.Clear();

   for (size_t i = 0; i < m_vho.size(); ++i)
      bounds.Extend(m_vho[i]->m_hitBBox);

   Initialize(bounds);
#endif
}

// Ported at: VisualPinball.Engine/Physics/HitQuadTree.cs

void HitQuadtree::Initialize(const FRect& bounds)
{
   m_nLevels = 0;
#ifdef USE_EMBREE
   m_pvho = &m_vho;
   Initialize();
#else
   CreateNextLevel(bounds, 0, 0);
#endif
}

#ifdef USE_EMBREE
void HitQuadtree::FillFromVector(vector<HitObject*>& vho)
{
   m_pvho = &vho;
   for (size_t i = 0; i < vho.size(); ++i)
      vho[i]->CalcHitBBox(); // need to update here, as only done lazily for some objects (i.e. balls!)

   Initialize();
}

void HitQuadtree::Update()
{
   FillFromVector(*m_pvho);
}

#else

// Ported at: VisualPinball.Engine/Physics/HitQuadTree.cs

void HitQuadtree::CreateNextLevel(const FRect& bounds, const unsigned int level, unsigned int level_empty)
{
   if (m_vho.size() <= 4) //!! magic
      return;

   m_nLevels++;

   m_leaf = false;

   m_vcenter.x = (bounds.left + bounds.right)*0.5f;
   m_vcenter.y = (bounds.top + bounds.bottom)*0.5f;
   //m_vcenter.z = (bounds.zlow + bounds.zhigh)*0.5f;

   m_children = new HitQuadtree[4];

   vector<HitObject*> vRemain; // hit objects which did not go to a quadrant

   m_unique = (m_vho[0]->m_e != 0) ? m_vho[0]->m_obj : nullptr;

   // sort items into appropriate child nodes
   for (size_t i = 0; i < m_vho.size(); i++)
   {
      HitObject * const pho = m_vho[i];

      if (((pho->m_e != 0) ? pho->m_obj : nullptr) != m_unique) // are all objects in current node unique/belong to the same primitive?
         m_unique = nullptr;

      int oct;
      if (pho->m_hitBBox.right < m_vcenter.x)
         oct = 0;
      else if (pho->m_hitBBox.left > m_vcenter.x)
         oct = 1;
      else
         oct = 128;

      if (pho->m_hitBBox.bottom < m_vcenter.y)
         oct |= 0;
      else if (pho->m_hitBBox.top > m_vcenter.y)
         oct |= 2;
      else
         oct |= 128;

      if ((oct & 128) == 0)
         m_children[oct].m_vho.push_back(pho);
      else
         vRemain.push_back(pho);
   }

   m_ObjType = (m_unique != nullptr) ? m_vho[0]->m_ObjType : eNull;

   m_vho.swap(vRemain);

   // check if at least two nodes feature objects, otherwise don't bother subdividing further
   unsigned int count_empty = m_vho.empty() ? 1 : 0;
   for (int i = 0; i < 4; ++i)
      if (m_children[i].m_vho.empty())
         ++count_empty;

   if (count_empty >= 4)
      ++level_empty;
   else
      level_empty = 0;

   if (m_vcenter.x - bounds.left > 0.0001f && //!! magic
      level_empty <= 8 && // If 8 levels were all just subdividing the same objects without luck, exit & Free the nodes again (but at least empty space was cut off)
      level + 1 < 128 / 3)
      for (int i = 0; i < 4; ++i)
      {
         FRect childBounds;

         childBounds.left = (i & 1) ? m_vcenter.x : bounds.left;
         childBounds.top = (i & 2) ? m_vcenter.y : bounds.top;
         //childBounds.zlow = bounds.zlow;

         childBounds.right = (i & 1) ? bounds.right : m_vcenter.x;
         childBounds.bottom = (i & 2) ? bounds.bottom : m_vcenter.y;
         //childBounds.zhigh = bounds.zhigh;

         m_children[i].CreateNextLevel(childBounds, level + 1, level_empty);
      }

   InitSseArrays();
   for (int i = 0; i < 4; ++i)
      m_children[i].InitSseArrays();
}

void HitQuadtree::InitSseArrays()
{
   // build SSE boundary arrays of the local hit-object list
   // (don't init twice)
   const size_t padded = ((m_vho.size() + 3) / 4) * 4;
   if (lefts_rights_tops_bottoms_zlows_zhighs == nullptr && padded > 0)
   {
#ifdef DISABLE_ZTEST
      constexpr size_t mul = 4;
#else
      constexpr size_t mul = 6;
#endif
      lefts_rights_tops_bottoms_zlows_zhighs = (float*)_aligned_malloc(padded * (mul * sizeof(float)), 16);

      // fill array in chunks of 4xSIMD data: 4xleft,4xright,4xtop,4xbottom,4xzlow,4xzhigh, 4xleft ... ... ...

      const unsigned int end = (unsigned int)m_vho.size() & 0xFFFFFFFCu;
#ifdef DISABLE_ZTEST
      constexpr unsigned int offs = 16;
#else
      constexpr unsigned int offs = 24;
#endif
      for (unsigned int j = 0,j2 = 0; j < end; j+=4,j2+=offs)
      {
         const FRect3D& r0 = m_vho[j  ]->m_hitBBox;
         const FRect3D& r1 = m_vho[j+1]->m_hitBBox;
         const FRect3D& r2 = m_vho[j+2]->m_hitBBox;
         const FRect3D& r3 = m_vho[j+3]->m_hitBBox;
         lefts_rights_tops_bottoms_zlows_zhighs[j2   ] = r0.left; 
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 1] = r1.left;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 2] = r2.left;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 3] = r3.left; 
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 4] = r0.right;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 5] = r1.right;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 6] = r2.right;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 7] = r3.right;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 8] = r0.top;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 9] = r1.top;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+10] = r2.top;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+11] = r3.top;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+12] = r0.bottom;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+13] = r1.bottom;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+14] = r2.bottom;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+15] = r3.bottom;
#ifndef DISABLE_ZTEST
         lefts_rights_tops_bottoms_zlows_zhighs[j2+16] = r0.zlow;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+17] = r1.zlow;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+18] = r2.zlow;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+19] = r3.zlow;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+20] = r0.zhigh;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+21] = r1.zhigh;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+22] = r2.zhigh;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+23] = r3.zhigh;
#endif
      }

      // fill the remainder of the array with the remaining data and invalid BBoxes for padding

      if (end != m_vho.size())
      {
         const FRect3D& r0 = m_vho[end]->m_hitBBox;
         const FRect3D r1 = end + 1 < m_vho.size() ? m_vho[end + 1]->m_hitBBox : FRect3D(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);
         const FRect3D r2 = end + 2 < m_vho.size() ? m_vho[end + 2]->m_hitBBox : FRect3D(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);
         const FRect3D r3 = FRect3D(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);
#ifdef DISABLE_ZTEST
         const unsigned int j2 = end * 4;
#else
         const unsigned int j2 = end * 6;
#endif
         lefts_rights_tops_bottoms_zlows_zhighs[j2   ] = r0.left; 
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 1] = r1.left;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 2] = r2.left;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 3] = r3.left; 
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 4] = r0.right;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 5] = r1.right;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 6] = r2.right;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 7] = r3.right;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 8] = r0.top;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+ 9] = r1.top;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+10] = r2.top;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+11] = r3.top;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+12] = r0.bottom;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+13] = r1.bottom;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+14] = r2.bottom;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+15] = r3.bottom;
#ifndef DISABLE_ZTEST
         lefts_rights_tops_bottoms_zlows_zhighs[j2+16] = r0.zlow;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+17] = r1.zlow;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+18] = r2.zlow;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+19] = r3.zlow;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+20] = r0.zhigh;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+21] = r1.zhigh;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+22] = r2.zhigh;
         lefts_rights_tops_bottoms_zlows_zhighs[j2+23] = r3.zhigh;
#endif
      }
   }
}
#endif

// OUTDATED INFO?!
// Hit logic needs to be expanded, during static and pseudo-static conditions, multiple hits (multi-face contacts)
// are possible and should be handled, with embedding (penetrations) some contacts persist for long periods
// and may cause others not to be seen (masked because of their position in the object list).

// A short term solution might be to rotate the object list on each collision round. Currently, its a linear array.
// and some subscript magic might be needed, where the actually collision counts are used to cycle the starting position
// for the next search. This could become a Ball property ... i.e. my last hit object index, start at the next
// and cycle around until the last hit object is the last to be tested ... this could be made complex due to
// scripts removing objects ... i.e. balls ...

// The most effective would be to sort the search results, always moving the last hit to the end of it's grouping

// At this instance, its reporting static contacts as random hittimes during the specific physics frame; the zero time
// slot is not in the random time generator algorithm, it is offset by STATICTIME so not to compete with the fast moving
// collisions

#ifndef USE_EMBREE
void HitQuadtree::HitTestBall(const HitBall* const pball, CollisionEvent& coll) const
{
#ifdef QUADTREE_SSE_LEAFTEST

   HitTestBallSse(pball, coll);

#else // without SSE optimization

// Ported at: VisualPinball.Engine/Physics/HitQuadTree.cs

   const float rcHitRadiusSqr = pball->HitRadiusSqr();

   for (unsigned i=0; i<m_vho.size(); i++)
   {
#ifdef DEBUGPHYSICS
      g_pplayer->c_tested++;
#endif
      if ((pball != m_vho[i]) // ball can not hit itself
         && fRectIntersect3D(pball->m_hitBBox, m_vho[i]->m_hitBBox)
         && fRectIntersect3D(pball->m_d.m_pos, rcHitRadiusSqr, m_vho[i]->m_hitBBox))
      {
         DoHitTest(pball, m_vho[i], coll);
      }
   }//end for loop

   if (!m_leaf)
   {
      const bool left = (pball->m_hitBBox.left <= m_vcenter.x);
      const bool right = (pball->m_hitBBox.right >= m_vcenter.x);

#ifdef DEBUGPHYSICS
      g_pplayer->c_tested++;
#endif
      if (pball->m_hitBBox.top <= m_vcenter.y) // Top
      {
         if (left)  m_children[0].HitTestBall(pball, coll);
         if (right) m_children[1].HitTestBall(pball, coll);
      }
      if (pball->m_hitBBox.bottom >= m_vcenter.y) // Bottom
      {
         if (left)  m_children[2].HitTestBall(pball, coll);
         if (right) m_children[3].HitTestBall(pball, coll);
      }
   }
#endif
}

#ifdef QUADTREE_SSE_LEAFTEST
void HitQuadtree::HitTestBallSse(const HitBall* const pball, CollisionEvent& coll) const
{
   const HitQuadtree* stack[128]; //!! should be enough, but better implement test in construction to not exceed this
   unsigned int stackpos = 0;
   stack[0] = nullptr; // sentinel

   const HitQuadtree* __restrict current = this;

   // init SSE registers with ball bbox
   const __m128 bleft = _mm_set1_ps(pball->m_hitBBox.left);
   const __m128 bright = _mm_set1_ps(pball->m_hitBBox.right);
   const __m128 btop = _mm_set1_ps(pball->m_hitBBox.top);
   const __m128 bbottom = _mm_set1_ps(pball->m_hitBBox.bottom);
#ifndef DISABLE_ZTEST
   const __m128 bzlow = _mm_set1_ps(pball->m_hitBBox.zlow);
   const __m128 bzhigh = _mm_set1_ps(pball->m_hitBBox.zhigh);
#endif
   const __m128 posx = _mm_set1_ps(pball->m_d.m_pos.x);
   const __m128 posy = _mm_set1_ps(pball->m_d.m_pos.y);
#ifndef DISABLE_ZTEST
   const __m128 posz = _mm_set1_ps(pball->m_d.m_pos.z);
#endif
   const __m128 rsqr = _mm_set1_ps(pball->HitRadiusSqr());

   const bool traversal_order = (rand_mt_01() < 0.5f); // swaps test order in leafs randomly
   const size_t dt = traversal_order ? 1 : -1;

   do
   {
      if (current->m_unique == nullptr
          || (current->m_ObjType == ePrimitive && ((Primitive*)current->m_unique)->m_d.m_collidable)
          || (current->m_ObjType == eHitTarget && ((HitTarget*)current->m_unique)->m_d.m_isDropped == false)) // early out if only one unique primitive/hittarget stored inside all of the subtree/current node that is also not collidable (at the moment)
      {
         if (current->lefts_rights_tops_bottoms_zlows_zhighs != nullptr) // does node contain hitables?
         {
            const size_t size = (current->m_vho.size() + 3) / 4;

            const __m128* const __restrict p = (__m128*)current->lefts_rights_tops_bottoms_zlows_zhighs;

            // loop implements 4 collision checks at once
            // (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow)
            const size_t start = traversal_order ? 0 : (size - 1);
#ifdef DISABLE_ZTEST
            const size_t dt2 = dt * 4;
            constexpr size_t mul = 4;
#else
            const size_t dt2 = dt * 6;
            constexpr size_t mul = 6;
#endif
            const size_t end = traversal_order ? size : -1;
            for (size_t i = start, i2 = start*mul; i != end; i += dt, i2 += dt2)
            {
#ifdef DEBUGPHYSICS
               g_pplayer->m_physics->c_tested++; //!! +=4? or is this more fair?
#endif
               // comparisons set bits if bounds miss. if all bits are set, there is no collision. otherwise continue comparisons
               // bits set, there is a bounding box collision
               __m128 cmp = _mm_cmpge_ps(bright, p[i2]); // right vs left
               int mask = _mm_movemask_ps(cmp);
               if (mask == 0) continue;

               cmp = _mm_cmple_ps(bleft, p[i2+1]); // left vs right
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;

               cmp = _mm_cmpge_ps(bbottom, p[i2+2]); // bottom vs top
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;

               cmp = _mm_cmple_ps(btop, p[i2+3]); // top vs bottom
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;

#ifndef DISABLE_ZTEST
               cmp = _mm_cmpge_ps(bzhigh, p[i2+4]); // zhigh vs zlow
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;

               cmp = _mm_cmple_ps(bzlow, p[i2+5]); // zlow vs zhigh
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;
#endif
               // test actual sphere against box(es)
               const __m128 zero = _mm_setzero_ps();
               __m128 ex = _mm_add_ps(_mm_max_ps(_mm_sub_ps(p[i2  ]/*left*/, posx), zero), _mm_max_ps(_mm_sub_ps(posx, p[i2+1]/*right */), zero));
               __m128 ey = _mm_add_ps(_mm_max_ps(_mm_sub_ps(p[i2+2]/*top */, posy), zero), _mm_max_ps(_mm_sub_ps(posy, p[i2+3]/*bottom*/), zero));
#ifndef DISABLE_ZTEST
               __m128 ez = _mm_add_ps(_mm_max_ps(_mm_sub_ps(p[i2+4]/*zlow*/, posz), zero), _mm_max_ps(_mm_sub_ps(posz, p[i2+5]/*zhigh */), zero));
#endif
               ex = _mm_mul_ps(ex, ex);
               ey = _mm_mul_ps(ey, ey);
#ifndef DISABLE_ZTEST
               ez = _mm_mul_ps(ez, ez);
               const __m128 d = _mm_add_ps(_mm_add_ps(ex, ey), ez);
#else
               const __m128 d = _mm_add_ps(ex, ey);
#endif
               const __m128 cmp2 = _mm_cmple_ps(d, rsqr);
               const int mask2 = _mm_movemask_ps(cmp2);
               if (mask2 == 0) continue;

               // now there is at least one bbox collision
               if ((mask2 & 1) != 0 && (pball != current->m_vho[i * 4])) // ball can not hit itself
                  DoHitTest(pball, current->m_vho[i * 4], coll);
               // array boundary checks for the rest not necessary as non-valid entries were initialized to keep these maskbits 0
               if ((mask2 & 2) != 0 /*&& (i*4+1)<m_vho.size()*/ && (pball != current->m_vho[i * 4 + 1])) // ball can not hit itself
                  DoHitTest(pball, current->m_vho[i * 4 + 1], coll);
               if ((mask2 & 4) != 0 /*&& (i*4+2)<m_vho.size()*/ && (pball != current->m_vho[i * 4 + 2])) // ball can not hit itself
                  DoHitTest(pball, current->m_vho[i * 4 + 2], coll);
               if ((mask2 & 8) != 0 /*&& (i*4+3)<m_vho.size()*/ && (pball != current->m_vho[i * 4 + 3])) // ball can not hit itself
                  DoHitTest(pball, current->m_vho[i * 4 + 3], coll);
            }
         }

         //if (stackpos >= 127)
         //	ShowError("Quadtree stack size to be exceeded");

         if (!current->m_leaf)
         {
#ifdef DEBUGPHYSICS
            g_pplayer->m_physics->c_traversed++;
#endif
            const bool left = (pball->m_hitBBox.left <= current->m_vcenter.x);
            const bool right = (pball->m_hitBBox.right >= current->m_vcenter.x);

            if (pball->m_hitBBox.top <= current->m_vcenter.y) // Top
            {
               if (left)  stack[++stackpos] = current->m_children;
               if (right) stack[++stackpos] = current->m_children+1;
            }
            if (pball->m_hitBBox.bottom >= current->m_vcenter.y) // Bottom
            {
               if (left)  stack[++stackpos] = current->m_children+2;
               if (right) stack[++stackpos] = current->m_children+3;
            }
         }
      }

      //current = stack[stackpos];
      //if (stackpos > 0)
      //    stackpos--;
      current = stack[stackpos--]; // above test not needed due to sentinel in stack[0]=nullptr

   } while (current);
}
#endif
#endif

void HitQuadtree::HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const
{
#ifdef USE_EMBREE
   ShowError("HitTestXRay not implemented yet");
#else
   const float rcHitRadiusSqr = pball->HitRadiusSqr();

   for (size_t i = 0; i < m_vho.size(); i++)
   {
#ifdef DEBUGPHYSICS
      g_pplayer->m_physics->c_tested++;
#endif
      if ((pball != m_vho[i]) && fRectIntersect3D(pball->m_hitBBox, m_vho[i]->m_hitBBox) && fRectIntersect3D(pball->m_d.m_pos, rcHitRadiusSqr, m_vho[i]->m_hitBBox))
      {
#ifdef DEBUGPHYSICS
         g_pplayer->m_physics->c_deepTested++;
#endif
         const float newtime = m_vho[i]->HitTest(pball->m_d, coll.m_hittime, coll);
         if (newtime >= 0.f)
         {
            const HitTestResult r { m_vho[i], newtime };
            pvhoHit.push_back(r);
         }
      }
   }

   if (!m_leaf)
   {
      const bool left = (pball->m_hitBBox.left <= m_vcenter.x);
      const bool right = (pball->m_hitBBox.right >= m_vcenter.x);

#ifdef DEBUGPHYSICS
      g_pplayer->m_physics->c_tested++;
#endif
      if (pball->m_hitBBox.top <= m_vcenter.y) // Top
      {
         if (left)  m_children[0].HitTestXRay(pball, pvhoHit, coll);
         if (right) m_children[1].HitTestXRay(pball, pvhoHit, coll);
      }
      if (pball->m_hitBBox.bottom >= m_vcenter.y) // Bottom
      {
         if (left)  m_children[2].HitTestXRay(pball, pvhoHit, coll);
         if (right) m_children[3].HitTestXRay(pball, pvhoHit, coll);
      }
   }
#endif
}

#ifdef USE_EMBREE
void EmbreeBoundsFuncBalls(const struct RTCBoundsFunctionArguments* const args)
{
   const HitBall* const ball = (*((const vector<HitBall*>*)args->geometryUserPtr))[args->primID];

   args->bounds_o->lower_x = ball->m_hitBBox.left;
   args->bounds_o->lower_y = ball->m_hitBBox.top;
   args->bounds_o->lower_z = ball->m_hitBBox.zlow;
   args->bounds_o->upper_x = ball->m_hitBBox.right;
   args->bounds_o->upper_y = ball->m_hitBBox.bottom;
   args->bounds_o->upper_z = ball->m_hitBBox.zhigh;
}

struct VPCollisions
{
   const vector<HitObject*> *vho;
   const vector<HitBall*> *ball;
};


void EmbreeCollideBalls(void* const userPtr, RTCCollision* const collisions, const unsigned int num_collisions)
{
   const VPCollisions * const vpc = (VPCollisions*)userPtr;

   for (unsigned int i = 0; i < num_collisions; ++i)
   {
      HitBall* const ball = (*vpc->ball)[collisions[i].primID1];
      const HitObject* const ho = (*vpc->vho)[collisions[i].primID0];

      if (!ball->m_d.m_lockedInKicker
#ifdef C_DYNAMIC
          && ball->m_dynamic > 0
#endif
         ) // don't play with frozen balls
      if (ball != ho
          //&& fRectIntersect3D(ball->m_hitBBox, ho->m_hitBBox)
          && fRectIntersect3D(ball->m_d.m_pos, ball->HitRadiusSqr(), ho->m_hitBBox))
      {
         CollisionEvent coll_local;
         coll_local.m_obj = nullptr;
         mtx.lock();
         coll_local.m_hittime = ball->m_coll.m_hittime;
         mtx.unlock();
         DoHitTest(ball, ho, coll_local);
         mtx.lock();
         if (coll_local.m_hittime < ball->m_coll.m_hittime)
            ball->m_coll = coll_local;
         mtx.unlock();
      }
   }
}

void HitQuadtree::HitTestBall(vector<HitBall*> ball) const
{
   RTCScene scene = rtcNewScene(m_embree_device);
   rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);
   rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);

   const RTCGeometry geom = rtcNewGeometry(m_embree_device, RTC_GEOMETRY_TYPE_USER);
   rtcSetGeometryUserPrimitiveCount(geom, ball.size());

   rtcSetGeometryUserData(geom, &ball);
   rtcSetGeometryBoundsFunction(geom, &EmbreeBoundsFuncBalls, &ball);
   rtcSetGeometryIntersectFunction(geom, nullptr); // no ray tracing
   rtcSetGeometryOccludedFunction(geom, nullptr); // no shadow ray tracing
   rtcSetGeometryIntersectFilterFunction(geom, nullptr); // no trace filter

   rtcCommitGeometry(geom);
   // attach geometry to scene
   const unsigned int l_geom = rtcAttachGeometry(scene, geom); // no need to rtcDeleteGeometry(l_geom), as we throw away complete scene later-on
   rtcReleaseGeometry(geom);

   rtcCommitScene(scene);

   //RTCBounds b;
   //rtcGetSceneBounds(scene, &b);

   CHECK_EMBREE(m_embree_device);

   VPCollisions vpc;
   vpc.vho = m_pvho;
   vpc.ball = &ball;
   rtcCollide(m_scene, scene, &EmbreeCollideBalls, &vpc);

   rtcReleaseScene(scene);

   CHECK_EMBREE(m_embree_device);
}
#endif
