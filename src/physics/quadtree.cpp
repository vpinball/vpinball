// license:GPLv3+

#include "core/stdafx.h"
#include "quadtree.h"
#include "ThreadPool.h"

#ifdef ENABLE_SSE_OPTIMIZATIONS
   #define QUADTREE_SSE_LEAFTEST
#else
   #pragma message ("Warning: No SSE quadtree tests")
#endif

#define MAX_LEVEL 128

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



HitQuadtree::HitQuadtree()
{
   m_bounds.Clear();
#ifdef USE_EMBREE
   m_embree_device = rtcNewDevice(nullptr);
   m_scene = nullptr;
#endif
}
   
HitQuadtree::~HitQuadtree()
{
#ifndef USE_EMBREE
   #ifdef QUADTREE_SSE_LEAFTEST
      if (l_r_t_b_zl_zh)
         _aligned_free(l_r_t_b_zl_zh);
   #endif
#else
   if (m_scene)
      rtcReleaseScene(m_scene);
   rtcReleaseDevice(m_embree_device);
   m_embree_device = rtcNewDevice(nullptr);
   m_scene = nullptr;
#endif
}

void HitQuadtree::Reset(const vector<HitObject*>& vho)
{
   m_vho = vho;
   Update();
}

void HitQuadtree::Insert(HitObject* ho)
{
   m_vho.push_back(ho);
   Initialize();
}

void HitQuadtree::Remove(HitObject* ho)
{
   vector<HitObject*>::const_iterator it = std::find(m_vho.begin(), m_vho.end(), ho);
   if (it != m_vho.end())
      m_vho.erase(it);
   Initialize();
}

void HitQuadtree::Update()
{
   // need to update here, as only done lazily for balls
   for (size_t i = 0; i < m_vho.size(); ++i)
      m_vho[i]->CalcHitBBox();
   Initialize();
}

void HitQuadtree::Initialize()
{
#ifndef USE_EMBREE
   if (m_vho.size() > m_maxItems)
   {
      m_maxItems = m_vho.size();

      m_tmp1.resize(m_maxItems);
      m_tmp2.resize(m_maxItems);

      #ifdef QUADTREE_SSE_LEAFTEST
         if (l_r_t_b_zl_zh)
            _aligned_free(l_r_t_b_zl_zh);
         l_r_t_b_zl_zh = (float*)_aligned_malloc(sizeof(float) * ((m_maxItems + 3) & 0xFFFFFFFC) * 6, 16);
      #endif

      m_nodes.clear();
      // Unfortunately, there is no a priori bound on the number of nodes in the tree. We just make
      // an educated guess on the maximum and truncate the subdivision if we run out of nodes.
      m_nodes.resize((m_vho.size() * 2 + 1) & ~3u); // Nodes are allocated 4 by 4, so round up accordingly
   }

   // did somebody call finalize inbetween?
   if (m_tmp1.empty())
      m_tmp1.resize(m_maxItems);
   if (m_tmp2.empty())
      m_tmp2.resize(m_maxItems);

   FRect bounds(m_bounds);
   if (bounds.left == FLT_MAX)
   {
      bounds.Clear();
      for (size_t i = 0; i < m_vho.size(); ++i)
         bounds.Extend(m_vho[i]->m_hitBBox);
   }

   m_nLevels = 0;
   m_numNodes = 0;
   m_rootNode.Reset();   
   m_rootNode.m_start = 0;
   m_rootNode.m_items = static_cast<unsigned int>(m_vho.size());
   m_rootNode.CreateNextLevel(this, bounds, 0, 0);

   if (m_threadPool)
   {
      m_threadPool->wait_until_empty();
      m_threadPool->wait_until_nothing_in_flight();
   }

   InitSseArrays();

#else
   if (m_scene)
       rtcReleaseScene(m_scene);

   m_scene = rtcNewScene(m_embree_device);
   rtcSetSceneBuildQuality(m_scene, RTC_BUILD_QUALITY_HIGH);
   rtcSetSceneFlags(m_scene, RTC_SCENE_FLAG_ROBUST);

   const RTCGeometry geom = rtcNewGeometry(m_embree_device, RTC_GEOMETRY_TYPE_USER);
   rtcSetGeometryUserPrimitiveCount(geom, m_vho.size());

   rtcSetGeometryUserData(geom, &m_vho);
   rtcSetGeometryBoundsFunction(geom, &EmbreeBoundsFunc, &m_vho);
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
#endif
}

void HitQuadtree::Finalize()
{
#ifndef USE_EMBREE
   m_tmp1.clear();
   m_tmp2.clear();
   delete m_threadPool;
   m_threadPool = nullptr;
#endif
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// Base implementation (not using Embree, with optional SSE path)

#ifndef USE_EMBREE
HitQuadtreeNode* HitQuadtree::AllocFourNodes()
{
   const std::lock_guard<std::mutex> lock(m_nodePoolMutex);
   if ((m_numNodes + 3) >= m_nodes.size()) // space for 4 more nodes?
      return nullptr;
   else
   {
      m_nodes[m_numNodes    ].Reset();
      m_nodes[m_numNodes + 1].Reset();
      m_nodes[m_numNodes + 2].Reset();
      m_nodes[m_numNodes + 3].Reset();
      m_numNodes += 4;
      return &m_nodes[m_numNodes - 4];
   }
}
#endif

// build SSE boundary arrays of the local hit-object/m_vho HitRect list, generated for -full- list completely in the end!
void HitQuadtree::InitSseArrays()
{
#ifdef QUADTREE_SSE_LEAFTEST
   const unsigned int padded = (m_vho.size() + 3) & 0xFFFFFFFC;
   if (l_r_t_b_zl_zh == nullptr)
      l_r_t_b_zl_zh = (float*)_aligned_malloc(padded * 6 * sizeof(float), 16);

   for (size_t j = 0; j < m_vho.size(); ++j)
   {
      const FRect3D& r = m_vho[j]->m_hitBBox;
      l_r_t_b_zl_zh[j             ] = r.left;
      l_r_t_b_zl_zh[j + padded    ] = r.right;
      l_r_t_b_zl_zh[j + padded * 2] = r.top;
      l_r_t_b_zl_zh[j + padded * 3] = r.bottom;
      l_r_t_b_zl_zh[j + padded * 4] = r.zlow;
      l_r_t_b_zl_zh[j + padded * 5] = r.zhigh;
   }

   for (size_t j = m_vho.size(); j < padded; ++j)
   {
      l_r_t_b_zl_zh[j             ] =  FLT_MAX;
      l_r_t_b_zl_zh[j + padded    ] = -FLT_MAX;
      l_r_t_b_zl_zh[j + padded * 2] =  FLT_MAX;
      l_r_t_b_zl_zh[j + padded * 3] = -FLT_MAX;
      l_r_t_b_zl_zh[j + padded * 4] =  FLT_MAX;
      l_r_t_b_zl_zh[j + padded * 5] = -FLT_MAX;
   }
#endif
}

HitQuadtreeNode::HitQuadtreeNode()
{
}

HitQuadtreeNode::~HitQuadtreeNode()
{
   Reset();
}

void HitQuadtreeNode::Reset()
{
   m_children = nullptr;
   m_unique = nullptr;
   m_start = 0;
   m_items = 0;
}

void HitQuadtreeNode::CreateNextLevel(HitQuadtree* const quadTree, const FRect& bounds, const unsigned int level, unsigned int level_empty)
{
   if (level > quadTree->m_nLevels)
      quadTree->m_nLevels = level;

   if ((m_items <= 4) //!! magic
      || (level == MAX_LEVEL - 1) // bottom of tree
      || ((m_children = quadTree->AllocFourNodes()) == nullptr)) // Ran out of nodes
      return;

   m_vcenter.x = (bounds.left + bounds.right) * 0.5f;
   m_vcenter.y = (bounds.top + bounds.bottom) * 0.5f;
   //m_vcenter.z = (bounds.zlow + bounds.zhigh)*0.5f;
   #ifdef QUADTREE_SSE_LEAFTEST // With SSE optimization
      const __m128 center = _mm_set_ps(m_vcenter.y, m_vcenter.y, m_vcenter.x, m_vcenter.x);
      static constexpr int quadrant[] { 0, 1, 128, 1, 2, 3, 128, 3, 128, 128, 128, 128, 2, 3, 128, 3 };
   #endif

   m_unique = quadTree->m_vho[m_start]->m_editable->GetIHitable();

   // sort items into appropriate child nodes
   HitObject **ppTmp0, **ppTmp0x, **ppTmp1, **ppTmp1x, **ppTmp2, **ppTmp2x, **ppTmp3, **ppTmp3x, **ppTmp4, **ppTmp4x, **ppEnd;
   ppTmp0 = ppTmp0x = &quadTree->m_vho[m_start];
   ppTmp1 = ppTmp1x = &quadTree->m_tmp1[m_start];
   ppTmp2 = ppTmp2x = &quadTree->m_tmp2[m_start];
   ppTmp3 = ppTmp3x = &quadTree->m_tmp1[m_start + m_items - 1];
   ppTmp4 = ppTmp4x = &quadTree->m_tmp2[m_start + m_items - 1];
   ppEnd = &quadTree->m_vho[m_start + m_items - 1];
   for (HitObject** ppho = ppTmp0; ppho <= ppEnd; ppho++)
   {
      HitObject* const pho = *ppho;
      if (m_unique != pho->m_editable->GetIHitable()) // are all objects in current node unique/belong to the same hitable ?
         m_unique = nullptr;
      #ifdef QUADTREE_SSE_LEAFTEST // With SSE optimization
         const __m128 hb = _mm_loadu_ps(&pho->m_hitBBox.left); // L.R.T.B
         const __m128 cmp = _mm_cmpgt_ps(hb, center);
         const int oct = quadrant[_mm_movemask_ps(cmp)];
      #else
         int oct;
         if (pho->m_hitBBox.left > m_vcenter.x)
            oct = 1;
         else if (pho->m_hitBBox.right < m_vcenter.x)
            oct = 0;
         else
            oct = 128;
         if (pho->m_hitBBox.top > m_vcenter.y)
            oct |= 2;
         else if (pho->m_hitBBox.bottom < m_vcenter.y)
            oct |= 0;
         else
            oct |= 128;
      #endif
      switch (oct)
      {
      case 0: *ppTmp1++ = pho; break;
      case 1: *ppTmp2++ = pho; break;
      case 2: *ppTmp3-- = pho; break;
      case 3: *ppTmp4-- = pho; break;
      default: *ppTmp0++ = pho; break;
      }
   }
   m_items = static_cast<unsigned int>(ppTmp0 - ppTmp0x);
   m_children[0].m_items = static_cast<unsigned int>(ppTmp1 - ppTmp1x);
   m_children[1].m_items = static_cast<unsigned int>(ppTmp2 - ppTmp2x);
   m_children[2].m_items = static_cast<unsigned int>(ppTmp3x - ppTmp3);
   m_children[3].m_items = static_cast<unsigned int>(ppTmp4x - ppTmp4);
   m_children[0].m_start = m_start + m_items;
   m_children[1].m_start = m_children[0].m_start + m_children[0].m_items;
   m_children[2].m_start = m_children[1].m_start + m_children[1].m_items;
   m_children[3].m_start = m_children[2].m_start + m_children[2].m_items;
   if (m_children[0].m_items > 0)
      memcpy(&quadTree->m_vho[m_children[0].m_start], ppTmp1x, m_children[0].m_items * sizeof(HitObject*));
   if (m_children[1].m_items > 0)
      memcpy(&quadTree->m_vho[m_children[1].m_start], ppTmp2x, m_children[1].m_items * sizeof(HitObject*));
   if (m_children[2].m_items > 0)
      memcpy(&quadTree->m_vho[m_children[2].m_start], ppTmp3 + 1, m_children[2].m_items * sizeof(HitObject*));
   if (m_children[3].m_items > 0)
      memcpy(&quadTree->m_vho[m_children[3].m_start], ppTmp4 + 1, m_children[3].m_items * sizeof(HitObject*));

   // We only early out for Primitive and HitTarget objects
   if ((m_unique != nullptr) && (m_unique->HitableGetItemType() != eItemPrimitive) && (m_unique->HitableGetItemType() != eItemHitTarget))
      m_unique = nullptr;

   // check if at least two nodes feature objects, otherwise don't bother subdividing further
   unsigned int count_empty = m_items == 0 ? 1 : 0;
   for (int i = 0; i < 4; ++i)
      if (m_children[i].m_items == 0)
         ++count_empty;

   if (count_empty >= 4)
      ++level_empty;
   else
      level_empty = 0;

   if (m_vcenter.x - bounds.left > 0.0001f && //!! magic
      level_empty <= 8 && // If 8 levels were all just subdividing the same objects without luck, exit & Free the nodes again (but at least empty space was cut off)
      level + 1 < 128 / 3)
   {
      bool shouldDispatch = false;
      for (int i = 0; i < 4; ++i)
      {
         FRect childBounds;

         childBounds.left = (i & 1) ? m_vcenter.x : bounds.left;
         childBounds.top = (i & 2) ? m_vcenter.y : bounds.top;
         //childBounds.zlow = bounds.zlow;

         childBounds.right = (i & 1) ? bounds.right : m_vcenter.x;
         childBounds.bottom = (i & 2) ? bounds.bottom : m_vcenter.y;
         //childBounds.zhigh = bounds.zhigh;

         HitQuadtreeNode* child = &m_children[i];
         if (child->m_items > 1000) // Magic threshold above which we dispatch to a worker thread
         {
            if (shouldDispatch)
            {
               if (quadTree->m_threadPool == nullptr)
                  quadTree->m_threadPool = new ThreadPool(g_pvp->GetLogicalNumberOfProcessors());
               quadTree->m_threadPool->enqueue([child, quadTree, childBounds, level, level_empty] { child->CreateNextLevel(quadTree, childBounds, level + 1, level_empty); });
               continue;
            }
            shouldDispatch = true;
         }
         child->CreateNextLevel(quadTree, childBounds, level + 1, level_empty);
      }
   }
}

// OUTDATED INFO?!
// Hit logic needs to be expanded, during static and pseudo-static conditions, multiple hits (multi-face contacts)
// are possible and should be handled, with embedding (penetrations) some contacts persist for long periods
// and may cause others not to be seen (masked because of their position in the object list).

// A short term solution might be to rotate the object list on each collision round. Currently, it's a linear array.
// and some subscript magic might be needed, where the actual collision counts are used to cycle the starting position
// for the next search. This could become a Ball property ... i.e. my last hit object index, start at the next
// and cycle around until the last hit object is the last to be tested ... this could be made complex due to
// scripts removing objects ... i.e. balls ...

// The most effective would be to sort the search results, always moving the last hit to the end of it's grouping

// At this instance, its reporting static contacts as random hittimes during the specific physics frame; the zero time
// slot is not in the random time generator algorithm, it is offset by STATICTIME so not to compete with the fast moving
// collisions

#ifndef QUADTREE_SSE_LEAFTEST // Without SSE optimization
void HitQuadtree::HitTestBall(const HitBall* const pball, CollisionEvent& coll) const
{
   m_rootNode.HitTestBall(this, pball, coll);
}

#else // With SSE optimization
void HitQuadtree::HitTestBall(const HitBall* const pball, CollisionEvent& coll) const
{
   const HitQuadtreeNode* stack[MAX_LEVEL];
   unsigned int stackpos = 0;
   stack[0] = nullptr; // sentinel

   const HitQuadtreeNode* __restrict current = &m_rootNode;

   const unsigned int padded = (m_vho.size() + 3) & 0xFFFFFFFC;

   const __m128* __restrict const pL = (__m128*)l_r_t_b_zl_zh;
   const __m128* __restrict const pR = (__m128*)(l_r_t_b_zl_zh + padded);
   const __m128* __restrict const pT = (__m128*)(l_r_t_b_zl_zh + padded * 2);
   const __m128* __restrict const pB = (__m128*)(l_r_t_b_zl_zh + padded * 3);
   #ifndef DISABLE_ZTEST
      const __m128* __restrict const pZl = (__m128*)(l_r_t_b_zl_zh + padded * 4);
      const __m128* __restrict const pZh = (__m128*)(l_r_t_b_zl_zh + padded * 5);
   #endif

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
   const int dt = traversal_order ? 1 : -1;

   do
   {
      if (current->m_unique == nullptr
          || (current->m_unique->HitableGetItemType() == eItemPrimitive && static_cast<Primitive*>(current->m_unique)->m_d.m_collidable)
          || (current->m_unique->HitableGetItemType() == eItemHitTarget && !static_cast<HitTarget*>(current->m_unique)->m_d.m_isDropped)) // early out if only one unique primitive/hittarget stored inside all of the subtree/current node that is also not collidable (at the moment)
      {
         if (current->m_items != 0) // does node contain hitables?
         {
            const unsigned int size = (current->m_start + current->m_items + 3) / 4;
            const unsigned int start = traversal_order ? current->m_start / 4 : (size - 1);
            const unsigned int end = traversal_order ? size : (current->m_start / 4 - 1);

            // loop implements 4 collision checks at once
            // (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow)
            for (unsigned int i = start; i != end; i += dt)
            {
               #ifdef DEBUGPHYSICS
                  g_pplayer->m_physics->c_tested++; //!! +=4? or is this more fair?
               #endif

               // comparisons set bits if bounds miss. if all bits are set, there is no collision. otherwise continue comparisons
               // bits set, there is a bounding box collision
               __m128 cmp = _mm_cmpge_ps(bright, pL[i]);
               int mask = _mm_movemask_ps(cmp);
               if (mask == 0) continue;

               cmp = _mm_cmple_ps(bleft, pR[i]);
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;

               cmp = _mm_cmpge_ps(bbottom, pT[i]);
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;

               cmp = _mm_cmple_ps(btop, pB[i]);
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;

               #ifndef DISABLE_ZTEST
                  cmp = _mm_cmpge_ps(bzhigh, pZl[i]);
                  mask &= _mm_movemask_ps(cmp);
                  if (mask == 0) continue;

                  cmp = _mm_cmple_ps(bzlow, pZh[i]);
                  mask &= _mm_movemask_ps(cmp);
                  if (mask == 0) continue;
               #endif

               // test actual sphere against box(es)
               const __m128 zero = _mm_setzero_ps();
               __m128 ex = _mm_add_ps(_mm_max_ps(_mm_sub_ps(pL[i], posx), zero), _mm_max_ps(_mm_sub_ps(posx, pR[i] ), zero));
               __m128 ey = _mm_add_ps(_mm_max_ps(_mm_sub_ps(pT[i], posy), zero), _mm_max_ps(_mm_sub_ps(posy, pB[i] ), zero));
               #ifndef DISABLE_ZTEST
                  __m128 ez = _mm_add_ps(_mm_max_ps(_mm_sub_ps(pZl[i], posz), zero), _mm_max_ps(_mm_sub_ps(posz, pZh[i]), zero));
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
               if ((mask2 & 1) != 0)
               {
                  HitObject* const pho = m_vho[i * 4];
                  if (pball != pho) // ball can not hit itself
                     DoHitTest(pball, pho, coll);
               }
               // array boundary checks for the rest not necessary as non-valid entries were initialized to keep these maskbits 0
               if ((mask2 & 2) != 0 /*&& (i*4+1)<m_hitoct->m_num_items*/)
               {
                  HitObject* const pho = m_vho[i * 4 + 1];
                  if (pball != pho) // ball can not hit itself
                     DoHitTest(pball, pho, coll);
               }
               if ((mask2 & 4) != 0 /*&& (i*4+2)<m_hitoct->m_num_items*/)
               {
                  HitObject* const pho = m_vho[i * 4 + 2];
                  if (pball != pho) // ball can not hit itself
                     DoHitTest(pball, pho, coll);
               }
               if ((mask2 & 8) != 0 /*&& (i*4+3)<m_hitoct->m_num_items*/)
               {
                  HitObject* const pho = m_vho[i * 4 + 3];
                  if (pball != pho) // ball can not hit itself
                     DoHitTest(pball, pho, coll);
               }
            }
         }

         if (current->m_children != nullptr)
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

      current = stack[stackpos--];
   } while (current); // stops when stackpos is 0 since we defined stack[0] to nullptr
}
#endif // SSE vs non SSE implementation

void HitQuadtreeNode::HitTestBall(const HitQuadtree* const quadTree, const HitBall* const pball, CollisionEvent& coll) const
{
   const float rcHitRadiusSqr = pball->HitRadiusSqr();

   for (unsigned int i = m_start; i < m_start + m_items; i++)
   {
      #ifdef DEBUGPHYSICS
         g_pplayer->m_physics->c_tested++;
      #endif
      HitObject* pho = quadTree->m_vho[i];
      if ((pball != pho) // ball can not hit itself
         && fRectIntersect3D(pball->m_hitBBox, pho->m_hitBBox)
         && fRectIntersect3D(pball->m_d.m_pos, rcHitRadiusSqr, pho->m_hitBBox))
      {
         DoHitTest(pball, pho, coll);
      }
   }

   if (m_children != nullptr)
   {
      #ifdef DEBUGPHYSICS
         g_pplayer->m_physics->c_tested++;
      #endif
      const bool left = (pball->m_hitBBox.left <= m_vcenter.x);
      const bool right = (pball->m_hitBBox.right >= m_vcenter.x);
      if (pball->m_hitBBox.top <= m_vcenter.y) // Top
      {
         if (left)  m_children[0].HitTestBall(quadTree, pball, coll);
         if (right) m_children[1].HitTestBall(quadTree, pball, coll);
      }
      if (pball->m_hitBBox.bottom >= m_vcenter.y) // Bottom
      {
         if (left)  m_children[2].HitTestBall(quadTree, pball, coll);
         if (right) m_children[3].HitTestBall(quadTree, pball, coll);
      }
   }
}

void HitQuadtreeNode::HitTestXRay(const HitQuadtree* const quadTree, const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const
{
   const float rcHitRadiusSqr = pball->HitRadiusSqr();

   for (unsigned int i = m_start; i < m_start + m_items; i++)
   {
      #ifdef DEBUGPHYSICS
         g_pplayer->m_physics->c_tested++;
      #endif
      HitObject* pho = quadTree->m_vho[i];
      if ((pball != pho) // ball can not hit itself
         && fRectIntersect3D(pball->m_hitBBox, pho->m_hitBBox)
         && fRectIntersect3D(pball->m_d.m_pos, rcHitRadiusSqr, pho->m_hitBBox))
      {
         #ifdef DEBUGPHYSICS
            g_pplayer->m_physics->c_deepTested++;
         #endif
         const float newtime = pho->HitTest(pball->m_d, coll.m_hittime, coll);
         if (newtime >= 0.f)
            pvhoHit.push_back({ pho, newtime });
      }
   }

   if (m_children != nullptr)
   {
      #ifdef DEBUGPHYSICS
         g_pplayer->m_physics->c_tested++;
      #endif
      const bool left = (pball->m_hitBBox.left <= m_vcenter.x);
      const bool right = (pball->m_hitBBox.right >= m_vcenter.x);
      if (pball->m_hitBBox.top <= m_vcenter.y) // Top
      {
         if (left) m_children[0].HitTestXRay(quadTree, pball, pvhoHit, coll);
         if (right) m_children[1].HitTestXRay(quadTree, pball, pvhoHit, coll);
      }
      if (pball->m_hitBBox.bottom >= m_vcenter.y) // Bottom
      {
         if (left)  m_children[2].HitTestXRay(quadTree, pball, pvhoHit, coll);
         if (right) m_children[3].HitTestXRay(quadTree, pball, pvhoHit, coll);
      }
   }
}

void HitQuadtreeNode::DumpTree(const int indentLevel)
{
   #if !defined(NDEBUG) && defined(PRINT_DEBUG_COLLISION_TREE)
      char indent[256];
      for (int i = 0; i <= indentLevel; ++i)
         indent[i] = (i == indentLevel) ? '\0' : ' ';
      char msg[256];
      sprintf_s(msg, sizeof(msg), "[%f %f], items=%u", m_vcenter.x, m_vcenter.y, m_vho.size());
      strncat_s(indent, msg, sizeof(indent)-strnlen_s(indent, sizeof(indent))-1);
      OutputDebugString(indent);
      if (m_children != nullptr)
      {
         m_children[0].DumpTree(indentLevel + 1);
         m_children[1].DumpTree(indentLevel + 1);
         m_children[2].DumpTree(indentLevel + 1);
         m_children[3].DumpTree(indentLevel + 1);
      }
   #endif
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// Embree implementation (prototype missing UI picking)

#ifdef USE_EMBREE
void HitQuadtree::HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const
{
   ShowError("HitTestXRay not implemented yet");
}

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
   vpc.vho = &m_vho;
   vpc.ball = &ball;
   rtcCollide(m_scene, scene, &EmbreeCollideBalls, &vpc);

   rtcReleaseScene(scene);

   CHECK_EMBREE(m_embree_device);
}
#endif
