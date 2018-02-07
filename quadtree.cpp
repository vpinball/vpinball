#include "stdafx.h"
#include "quadtree.h"

HitQuadtree::~HitQuadtree()
{
   if (lefts != 0)
   {
      _aligned_free(lefts);
      _aligned_free(rights);
      _aligned_free(tops);
      _aligned_free(bottoms);
      _aligned_free(zlows);
      _aligned_free(zhighs);
   }

   if (!m_fLeaf)
   {
      for (int i = 0; i < 4; i++)
      {
         delete m_children[i];
      }
   }
}

void HitQuadtree::Initialize()
{
   FRect3D bounds;
   bounds.Clear();

   for (unsigned i = 0; i < m_vho.size(); ++i)
      bounds.Extend(m_vho[i]->m_hitBBox);

#ifdef _DEBUGPHYSICS
   g_pplayer->c_quadObjects = (U32)m_vho.size();
#endif

   CreateNextLevel(bounds, 0, 0);
}

void HitQuadtree::Initialize(const FRect3D& bounds)
{
#ifdef _DEBUGPHYSICS
   g_pplayer->c_quadObjects = (U32)m_vho.size();
#endif

   CreateNextLevel(bounds, 0, 0);
}

void HitQuadtree::CreateNextLevel(const FRect3D& bounds, const unsigned int level, unsigned int level_empty)
{
   if (m_vho.size() <= 4) //!! magic
      return;

#ifdef _DEBUGPHYSICS
   g_pplayer->c_quadNextlevels++;
#endif

   m_fLeaf = false;

   m_vcenter.x = (bounds.left + bounds.right)*0.5f;
   m_vcenter.y = (bounds.top + bounds.bottom)*0.5f;
   m_vcenter.z = (bounds.zlow + bounds.zhigh)*0.5f;

   for (int i = 0; i < 4; i++)
      m_children[i] = new HitQuadtree();

   std::vector<HitObject*> vRemain; // hit objects which did not go to a quadrant

   m_unique = m_vho[0]->m_e ? (Primitive *)(m_vho[0]->m_obj) : NULL;

   // sort items into appropriate child nodes
   for (unsigned i = 0; i < m_vho.size(); i++)
   {
      int oct;
      HitObject * const pho = m_vho[i];

      if ((pho->m_e ? (Primitive *)(pho->m_obj) : NULL) != m_unique) // are all objects in current node unique/belong to the same primitive?
         m_unique = NULL;

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
         m_children[oct]->m_vho.push_back(pho);
      else
         vRemain.push_back(pho);
   }

   m_vho.swap(vRemain);

   // check if at least two nodes feature objects, otherwise don't bother subdividing further
   unsigned int count_empty = (m_vho.size() == 0) ? 1 : 0;
   for (int i = 0; i < 4; ++i)
      if (m_children[i]->m_vho.size() == 0)
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
         FRect3D childBounds;

         childBounds.left = (i & 1) ? m_vcenter.x : bounds.left;
         childBounds.top = (i & 2) ? m_vcenter.y : bounds.top;
         childBounds.zlow = bounds.zlow;

         childBounds.right = (i & 1) ? bounds.right : m_vcenter.x;
         childBounds.bottom = (i & 2) ? bounds.bottom : m_vcenter.y;
         childBounds.zhigh = bounds.zhigh;

         m_children[i]->CreateNextLevel(childBounds, level + 1, level_empty);
      }

   InitSseArrays();
   for (int i = 0; i < 4; ++i)
      m_children[i]->InitSseArrays();
}

void HitQuadtree::InitSseArrays()
{
   // build SSE boundary arrays of the local hit-object list
   // (don't init twice)
   const size_t padded = ((m_vho.size() + 3) / 4) * 4;
   const size_t ssebytes = sizeof(float) * padded;
   if (ssebytes > 0 && lefts == 0)
   {
      lefts = (float*)_aligned_malloc(ssebytes, 16);
      rights = (float*)_aligned_malloc(ssebytes, 16);
      tops = (float*)_aligned_malloc(ssebytes, 16);
      bottoms = (float*)_aligned_malloc(ssebytes, 16);
      zlows = (float*)_aligned_malloc(ssebytes, 16);
      zhighs = (float*)_aligned_malloc(ssebytes, 16);

      for (size_t j = 0; j < m_vho.size(); j++)
      {
         const FRect3D r = m_vho[j]->m_hitBBox;
         lefts[j] = r.left;
         rights[j] = r.right;
         tops[j] = r.top;
         bottoms[j] = r.bottom;
         zlows[j] = r.zlow;
         zhighs[j] = r.zhigh;
      }

      for (size_t j = m_vho.size(); j < padded; j++)
      {
         lefts[j] = FLT_MAX;
         rights[j] = -FLT_MAX;
         tops[j] = FLT_MAX;
         bottoms[j] = -FLT_MAX;
         zlows[j] = FLT_MAX;
         zhighs[j] = -FLT_MAX;
      }
   }
}



/*  RLC

    Hit logic needs to be expanded, during static and pseudo-static conditions, multiple hits (multi-face contacts)
    are possible and should be handled, with embedding (pentrations) some contacts persist for long periods
    and may cause others not to be seen (masked because of their position in the object list).

    A short term solution might be to rotate the object list on each collision round. Currently, its a linear array.
    and some subscript magic might be needed, where the actually collision counts are used to cycle the starting position
    for the next search. This could become a Ball property ... i.e my last hit object index, start at the next
    and cycle around until the last hit object is the last to be tested ... this could be made complex due to
    scripts removing objects .... i.e. balls ... better study well before I start

    The most effective would be to sort the search results, always moving the last hit to the end of it's grouping

    At this instance, I'm reporting static contacts as random hitimes during the specific physics frame; the zero time
    slot is not in the random time generator algorithm, it is offset by STATICTIME so not to compete with the fast moving
    collisions

    */

void HitQuadtree::HitTestBall(Ball * const pball, CollisionEvent& coll) const
{
#if 1   /// with SSE optimizations //////////////////////////

   HitTestBallSse(pball, coll);

#else   /// without SSE optimization ////////////////////////

   for (unsigned i=0; i<m_vho.size(); i++)
   {
#ifdef _DEBUGPHYSICS
      g_pplayer->c_tested++;
#endif
      if ((pball != m_vho[i]) // ball can not hit itself
         && fRectIntersect3D(pball->m_hitBBox, m_vho[i]->m_hitBBox)
		 && fRectIntersect3D(pball->m_pos, pball->m_rcHitRadiusSqr, m_vho[i]->m_hitBBox))
      {
         DoHitTest(pball, m_vho[i], coll);
      }
   }//end for loop

   if (!m_fLeaf)
   {
      const bool fLeft = (pball->m_hitBBox.left <= m_vcenter.x);
      const bool fRight = (pball->m_hitBBox.right >= m_vcenter.x);

#ifdef _DEBUGPHYSICS
      g_pplayer->c_tested++;
#endif
      if (pball->m_hitBBox.top <= m_vcenter.y) // Top
      {
         if (fLeft)  m_children[0]->HitTestBall(pball, coll);
         if (fRight) m_children[1]->HitTestBall(pball, coll);
      }
      if (pball->m_hitBBox.bottom >= m_vcenter.y) // Bottom
      {
         if (fLeft)  m_children[2]->HitTestBall(pball, coll);
         if (fRight) m_children[3]->HitTestBall(pball, coll);
      }
   }
#endif
}

void HitQuadtree::HitTestBallSse(Ball * const pball, CollisionEvent& coll) const
{
   const HitQuadtree* stack[128]; //!! should be enough, but better implement test in construction to not exceed this
   unsigned int stackpos = 0;
   stack[0] = NULL; // sentinel

   const HitQuadtree* __restrict current = this;

   // init SSE registers with ball bbox
   const __m128 bleft = _mm_set1_ps(pball->m_hitBBox.left);
   const __m128 bright = _mm_set1_ps(pball->m_hitBBox.right);
   const __m128 btop = _mm_set1_ps(pball->m_hitBBox.top);
   const __m128 bbottom = _mm_set1_ps(pball->m_hitBBox.bottom);
   const __m128 bzlow = _mm_set1_ps(pball->m_hitBBox.zlow);
   const __m128 bzhigh = _mm_set1_ps(pball->m_hitBBox.zhigh);

   const __m128 posx = _mm_set1_ps(pball->m_pos.x);
   const __m128 posy = _mm_set1_ps(pball->m_pos.y);
   const __m128 posz = _mm_set1_ps(pball->m_pos.z);
   const __m128 rsqr = _mm_set1_ps(pball->m_rcHitRadiusSqr);

   const bool traversal_order = (rand_mt_01() < 0.5f); // swaps test order in leafs randomly
   const size_t dt = traversal_order ? 1 : -1;

   do
   {
      if (current->m_unique == NULL || current->m_unique->m_d.m_fCollidable) // early out if only one unique primitive stored inside all of the subtree/current node that is also not collidable (at the moment)
      {
         if (current->lefts != 0) // does node contain hitables?
         {
            const __m128* const pL = (__m128*)current->lefts;
            const __m128* const pR = (__m128*)current->rights;
            const __m128* const pT = (__m128*)current->tops;
            const __m128* const pB = (__m128*)current->bottoms;
            const __m128* const pZl = (__m128*)current->zlows;
            const __m128* const pZh = (__m128*)current->zhighs;

            // loop implements 4 collision checks at once
            // (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow)
            const size_t size = (current->m_vho.size() + 3) / 4;
            const size_t start = traversal_order ? 0 : (size - 1);
            const size_t end = traversal_order ? size : -1;
            for (size_t i = start; i != end; i += dt)
            {
#ifdef _DEBUGPHYSICS
               g_pplayer->c_tested++; //!! +=4? or is this more fair?
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

               cmp = _mm_cmpge_ps(bzhigh, pZl[i]);
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;

               cmp = _mm_cmple_ps(bzlow, pZh[i]);
               mask &= _mm_movemask_ps(cmp);
               if (mask == 0) continue;

			   // test actual sphere against box(es)
			   const __m128 zero = _mm_setzero_ps();
			   __m128 ex = _mm_add_ps(_mm_max_ps(_mm_sub_ps(pL[i],  posx), zero), _mm_max_ps(_mm_sub_ps(posx, pR[i] ), zero));
			   __m128 ey = _mm_add_ps(_mm_max_ps(_mm_sub_ps(pT[i],  posy), zero), _mm_max_ps(_mm_sub_ps(posy, pB[i] ), zero));
			   __m128 ez = _mm_add_ps(_mm_max_ps(_mm_sub_ps(pZl[i], posz), zero), _mm_max_ps(_mm_sub_ps(posz, pZh[i]), zero));
			   ex = _mm_mul_ps(ex, ex);
			   ey = _mm_mul_ps(ey, ey);
			   ez = _mm_mul_ps(ez, ez);
			   const __m128 d = _mm_add_ps(_mm_add_ps(ex,ey),ez);
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

         if (!current->m_fLeaf)
         {
#ifdef _DEBUGPHYSICS
            g_pplayer->c_traversed++;
#endif
            const bool fLeft = (pball->m_hitBBox.left <= current->m_vcenter.x);
            const bool fRight = (pball->m_hitBBox.right >= current->m_vcenter.x);

            if (pball->m_hitBBox.top <= current->m_vcenter.y) // Top
            {
               if (fLeft)  stack[++stackpos] = current->m_children[0];
               if (fRight) stack[++stackpos] = current->m_children[1];
            }
            if (pball->m_hitBBox.bottom >= current->m_vcenter.y) // Bottom
            {
               if (fLeft)  stack[++stackpos] = current->m_children[2];
               if (fRight) stack[++stackpos] = current->m_children[3];
            }
         }
      }

      //current = stack[stackpos];
      //if (stackpos > 0)
      //    stackpos--;
      current = stack[stackpos--]; // above test not needed due to sentinel in stack[0]=NULL

   } while (current);
}

void HitQuadtree::HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit, CollisionEvent& coll) const
{
   for (unsigned i = 0; i < m_vho.size(); i++)
   {
#ifdef _DEBUGPHYSICS
      g_pplayer->c_tested++;
#endif
      if ((pball != m_vho[i]) && fRectIntersect3D(pball->m_hitBBox, m_vho[i]->m_hitBBox) && fRectIntersect3D(pball->m_pos, pball->m_rcHitRadiusSqr, m_vho[i]->m_hitBBox))
      {
#ifdef _DEBUGPHYSICS
         g_pplayer->c_deepTested++;
#endif
         const float newtime = m_vho[i]->HitTest(pball, coll.m_hittime, coll);
         if (newtime >= 0)
         {
            pvhoHit->AddElement(m_vho[i]);
         }
      }
   }

   if (!m_fLeaf)
   {
      const bool fLeft = (pball->m_hitBBox.left <= m_vcenter.x);
      const bool fRight = (pball->m_hitBBox.right >= m_vcenter.x);

#ifdef _DEBUGPHYSICS
      g_pplayer->c_tested++;
#endif
      if (pball->m_hitBBox.top <= m_vcenter.y) // Top
      {
         if (fLeft)  m_children[0]->HitTestXRay(pball, pvhoHit, coll);
         if (fRight) m_children[1]->HitTestXRay(pball, pvhoHit, coll);
      }
      if (pball->m_hitBBox.bottom >= m_vcenter.y) // Bottom
      {
         if (fLeft)  m_children[2]->HitTestXRay(pball, pvhoHit, coll);
         if (fRight) m_children[3]->HitTestXRay(pball, pvhoHit, coll);
      }
   }
}
