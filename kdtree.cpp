#include "stdafx.h"
#include "kdtree.h"

HitKD::HitKD()
{
   m_num_items = 0;
   m_max_items = 0;
   m_num_nodes = 0;
   m_rootNode.m_hitoct = this;
#ifdef KDTREE_SSE_LEAFTEST
   l_r_t_b_zl_zh = NULL;
#endif
}

HitKD::~HitKD()
{
#ifdef KDTREE_SSE_LEAFTEST
   if (l_r_t_b_zl_zh)
      _aligned_free(l_r_t_b_zl_zh);
#endif
}

void HitKD::Init(Vector<HitObject> *vho, const unsigned int num_items)
{
   m_org_vho = vho;
   m_num_items = num_items;

   if (m_num_items > m_max_items)
   {
#ifdef KDTREE_SSE_LEAFTEST
      if (l_r_t_b_zl_zh)
         _aligned_free(l_r_t_b_zl_zh);
      l_r_t_b_zl_zh = (float*)_aligned_malloc(sizeof(float) * ((m_num_items + 3) & 0xFFFFFFFC) * 6, 16);
#endif
      m_max_items = m_num_items;

      m_org_idx.clear();
      m_org_idx.resize(m_num_items);

      tmp.clear();
      tmp.resize(m_num_items);

      m_nodes.clear();
      /* NB:
      * Unfortunately, there is no a priori bound on the number of nodes in the tree. We just make
      * an educated guess on the maximum and truncate the subdivision if we run out of nodes.
      */
      m_nodes.resize((m_num_items * 2 + 1) & ~1u);      // always allocate an even number of nodes, rounded up
   }

   if (tmp.empty()) // did somebody call finalize inbetween?
      tmp.resize(m_max_items);

   m_num_nodes = 0;

   m_rootNode.Reset();
   m_rootNode.m_hitoct = this;
}

void HitKD::Finalize()
{
   tmp.clear();
}

HitKDNode* HitKD::AllocTwoNodes()
{
   if ((m_num_nodes + 1) >= m_nodes.size())        // space for two more nodes?
      return NULL;
   else
   {
      m_num_nodes += 2;
      return &m_nodes[m_num_nodes - 2];
   }
}


// build SSE boundary arrays of the local hit-object/m_vho HitRect list, generated for -full- list completely in the end!
void HitKD::InitSseArrays()
{
#ifdef KDTREE_SSE_LEAFTEST
   const unsigned int padded = (m_num_items + 3) & 0xFFFFFFFC;
   if (!l_r_t_b_zl_zh)
      l_r_t_b_zl_zh = (float*)_aligned_malloc(sizeof(float) * padded * 6, 16);

   for (unsigned int j = 0; j < m_num_items; ++j)
   {
      const FRect3D& r = GetItemAt(j)->m_hitBBox;
      l_r_t_b_zl_zh[j] = r.left;
      l_r_t_b_zl_zh[j + padded] = r.right;
      l_r_t_b_zl_zh[j + padded * 2] = r.top;
      l_r_t_b_zl_zh[j + padded * 3] = r.bottom;
      l_r_t_b_zl_zh[j + padded * 4] = r.zlow;
      l_r_t_b_zl_zh[j + padded * 5] = r.zhigh;
   }

   for (unsigned int j = m_num_items; j < padded; ++j)
   {
      l_r_t_b_zl_zh[j] = FLT_MAX;
      l_r_t_b_zl_zh[j + padded] = -FLT_MAX;
      l_r_t_b_zl_zh[j + padded * 2] = FLT_MAX;
      l_r_t_b_zl_zh[j + padded * 3] = -FLT_MAX;
      l_r_t_b_zl_zh[j + padded * 4] = FLT_MAX;
      l_r_t_b_zl_zh[j + padded * 5] = -FLT_MAX;
   }
#endif
}

void HitKD::FillFromVector(Vector<HitObject>& vho)
{
   Init(&vho, vho.Size());

   m_rootNode.m_rectbounds.Clear();

   m_rootNode.m_start = 0;
   m_rootNode.m_items = m_num_items;

   for (unsigned i = 0; i < m_num_items; ++i)
   {
      HitObject * const pho = vho.ElementAt(i);
      pho->CalcHitBBox(); //!! omit, as already calced?!
      m_rootNode.m_rectbounds.Extend(pho->m_hitBBox);

      m_org_idx[i] = i;
   }

#ifdef _DEBUGPHYSICS
   g_pplayer->c_kDObjects = vho.size();
#endif

   m_rootNode.CreateNextLevel(0, 0);
   InitSseArrays();
}

void HitKD::FillFromIndices()
{
   m_rootNode.m_rectbounds.Clear();

   m_rootNode.m_start = 0;
   m_rootNode.m_items = m_num_items;

   for (unsigned i = 0; i < m_num_items; ++i)
   {
      HitObject * const pho = GetItemAt(i);
      pho->CalcHitBBox(); //!! omit, as already calced?!
      m_rootNode.m_rectbounds.Extend(pho->m_hitBBox);
   }

#ifdef _DEBUGPHYSICS
   g_pplayer->c_kDObjects = m_num_items;
#endif

   m_rootNode.CreateNextLevel(0, 0);
   InitSseArrays();
}

void HitKD::FillFromIndices(const FRect3D& initialBounds)
{
   m_rootNode.m_rectbounds = initialBounds;

   m_rootNode.m_start = 0;
   m_rootNode.m_items = m_num_items;

   // assume that CalcHitBBox() was already called on the hit objects

#ifdef _DEBUGPHYSICS
   g_pplayer->c_kDObjects = m_num_items;
#endif

   m_rootNode.CreateNextLevel(0, 0);
   InitSseArrays();
}

void HitKD::Update()
{
   FillFromVector(*m_org_vho);
}


void HitKDNode::CreateNextLevel(const unsigned int level, unsigned int level_empty)
{
   const unsigned int org_items = (m_items & 0x3FFFFFFF);

   if (org_items <= 4 || //!! magic
      level >= 128 / 2)
      return;

   const Vertex3Ds vdiag(m_rectbounds.right - m_rectbounds.left, m_rectbounds.bottom - m_rectbounds.top, m_rectbounds.zhigh - m_rectbounds.zlow);

   unsigned int axis;
   if ((vdiag.x > vdiag.y) && (vdiag.x > vdiag.z))
   {
      if (vdiag.x < 0.0001f) //!! magic
         return;
      axis = 0;
   }
   else if (vdiag.y > vdiag.z)
   {
      if (vdiag.y < 0.0001f) //!!
         return;
      axis = 1;
   }
   else
   {
      if (vdiag.z < 0.0001f) //!!
         return;
      axis = 2;
   }

   //!! weight this with ratio of elements going to middle vs left&right! (avoids volume split that goes directly through object)

   // create children, calc bboxes
   m_children = m_hitoct->AllocTwoNodes();
   if (!m_children)
   {
      // ran out of nodes - abort
      return;
   }
   m_children[0].m_rectbounds = m_rectbounds;
   m_children[1].m_rectbounds = m_rectbounds;

#ifdef _DEBUGPHYSICS
   g_pplayer->c_kDNextlevels++;
#endif

   const Vertex3Ds vcenter((m_rectbounds.left + m_rectbounds.right)*0.5f, (m_rectbounds.top + m_rectbounds.bottom)*0.5f, (m_rectbounds.zlow + m_rectbounds.zhigh)*0.5f);
   if (axis == 0)
   {
      m_children[0].m_rectbounds.right = vcenter.x;
      m_children[1].m_rectbounds.left = vcenter.x;
   }
   else if (axis == 1)
   {
      m_children[0].m_rectbounds.bottom = vcenter.y;
      m_children[1].m_rectbounds.top = vcenter.y;
   }
   else
   {
      m_children[0].m_rectbounds.zhigh = vcenter.z;
      m_children[1].m_rectbounds.zlow = vcenter.z;
   }

   m_children[0].m_hitoct = m_hitoct; //!! meh
   m_children[0].m_items = 0;
   m_children[0].m_children = NULL;
   m_children[1].m_hitoct = m_hitoct; //!! meh
   m_children[1].m_items = 0;
   m_children[1].m_children = NULL;

   // determine amount of items that cross splitplane, or are passed on to the children
   if (axis == 0)
   {
      for (unsigned int i = m_start; i < m_start + org_items; ++i)
      {
         const HitObject * const pho = m_hitoct->GetItemAt(i);

         if (pho->m_hitBBox.right < vcenter.x)    m_children[0].m_items++;
         else if (pho->m_hitBBox.left > vcenter.x)     m_children[1].m_items++;
      }
   }
   else if (axis == 1)
   {
      for (unsigned int i = m_start; i < m_start + org_items; ++i)
      {
         const HitObject * const pho = m_hitoct->GetItemAt(i);

         if (pho->m_hitBBox.bottom < vcenter.y)   m_children[0].m_items++;
         else if (pho->m_hitBBox.top > vcenter.y)      m_children[1].m_items++;
      }
   }
   else // axis == 2
   {
      for (unsigned int i = m_start; i < m_start + org_items; ++i)
      {
         const HitObject * const pho = m_hitoct->GetItemAt(i);

         if (pho->m_hitBBox.zhigh < vcenter.z)    m_children[0].m_items++;
         else if (pho->m_hitBBox.zlow > vcenter.z)     m_children[1].m_items++;
      }
   }

   // check if at least two nodes feature objects, otherwise don't bother subdividing further
   unsigned int count_empty = 0;
   if (m_children[0].m_items == 0)
      count_empty = 1;
   if (m_children[1].m_items == 0)
      ++count_empty;
   if (org_items - m_children[0].m_items - m_children[1].m_items == 0)
      ++count_empty;

   if (count_empty >= 2)
      ++level_empty;
   else
      level_empty = 0;

   if (level_empty > 8) // If 8 levels were all just subdividing the same objects without luck, exit & Free the nodes again (but at least empty space was cut off)
   {
      m_hitoct->m_num_nodes -= 2;
      m_children = NULL;
      return;
   }

   m_children[0].m_start = m_start + org_items - m_children[0].m_items - m_children[1].m_items;
   m_children[1].m_start = m_children[0].m_start + m_children[0].m_items;

   unsigned int items = 0;
   m_children[0].m_items = 0;
   m_children[1].m_items = 0;

   // sort items that cross splitplane in-place, the others are sorted into a temporary
   if (axis == 0)
   {
      for (unsigned int i = m_start; i < m_start + org_items; ++i)
      {
         const HitObject * const pho = m_hitoct->GetItemAt(i);

         if (pho->m_hitBBox.right < vcenter.x)
            m_hitoct->tmp[m_children[0].m_start + (m_children[0].m_items++)] = m_hitoct->m_org_idx[i];
         else if (pho->m_hitBBox.left > vcenter.x)
            m_hitoct->tmp[m_children[1].m_start + (m_children[1].m_items++)] = m_hitoct->m_org_idx[i];
         else
            m_hitoct->m_org_idx[m_start + (items++)] = m_hitoct->m_org_idx[i];
      }
   }
   else if (axis == 1)
   {
      for (unsigned int i = m_start; i < m_start + org_items; ++i)
      {
         const HitObject * const pho = m_hitoct->GetItemAt(i);

         if (pho->m_hitBBox.bottom < vcenter.y)
            m_hitoct->tmp[m_children[0].m_start + (m_children[0].m_items++)] = m_hitoct->m_org_idx[i];
         else if (pho->m_hitBBox.top > vcenter.y)
            m_hitoct->tmp[m_children[1].m_start + (m_children[1].m_items++)] = m_hitoct->m_org_idx[i];
         else
            m_hitoct->m_org_idx[m_start + (items++)] = m_hitoct->m_org_idx[i];
      }
   }
   else // axis == 2
   {
      for (unsigned int i = m_start; i < m_start + org_items; ++i)
      {
         const HitObject * const pho = m_hitoct->GetItemAt(i);

         if (pho->m_hitBBox.zhigh < vcenter.z)
            m_hitoct->tmp[m_children[0].m_start + (m_children[0].m_items++)] = m_hitoct->m_org_idx[i];
         else if (pho->m_hitBBox.zlow > vcenter.z)
            m_hitoct->tmp[m_children[1].m_start + (m_children[1].m_items++)] = m_hitoct->m_org_idx[i];
         else
            m_hitoct->m_org_idx[m_start + (items++)] = m_hitoct->m_org_idx[i];
      }
   }
   // The following assertions hold after this step:
   //assert( m_start + items == m_children[0].m_start );
   //assert( m_children[0].m_start + m_children[0].m_items == m_children[1].m_start );
   //assert( m_children[1].m_start + m_children[1].m_items == m_start + org_items );
   //assert( m_start + org_items <= m_hitoct->tmp.size() );

   m_items = items | (axis << 30);

   // copy temporary back //!! could omit this by doing everything inplace
   memcpy(&m_hitoct->m_org_idx[m_children[0].m_start], &m_hitoct->tmp[m_children[0].m_start], m_children[0].m_items*sizeof(unsigned int));
   memcpy(&m_hitoct->m_org_idx[m_children[1].m_start], &m_hitoct->tmp[m_children[1].m_start], m_children[1].m_items*sizeof(unsigned int));

   m_children[0].CreateNextLevel(level + 1, level_empty);
   m_children[1].CreateNextLevel(level + 1, level_empty);
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

void HitKDNode::HitTestBall(Ball * const pball, CollisionEvent& coll) const
{
#ifdef KDTREE_SSE_LEAFTEST
   /// with SSE optimizations ///////////////////////

   HitTestBallSse(pball, coll);

#else
   /// without SSE optimization /////////////////////

   const unsigned int org_items = (m_items&0x3FFFFFFF);
   const unsigned int axis = (m_items>>30);

   for (unsigned i=m_start; i<m_start+org_items; i++)
   {
#ifdef _DEBUGPHYSICS
      g_pplayer->c_tested++;
#endif
      HitObject * const pho = m_hitoct->GetItemAt( i );
      if ((pball != pho) // ball can not hit itself
		  /*&& fRectIntersect3D(pball->m_hitBBox, pho->m_hitBBox)*/ //!! do bbox test before to save alu-instructions? or not to save registers? -> currently not, as just sphere vs sphere
		 && fRectIntersect3D(pball->m_pos, pball->m_rcHitRadiusSqr, pho->m_hitBBox))
      {
         DoHitTest(pball, pho, coll);
      }
   }

   if (m_children) // not a leaf
   {
#ifdef _DEBUGPHYSICS
      g_pplayer->c_traversed++;
#endif
      if(axis == 0)
      {
         const float vcenter = (m_rectbounds.left+m_rectbounds.right)*0.5f;
         if(pball->m_hitBBox.left <= vcenter)
            m_children[0].HitTestBall(pball, coll);
         if(pball->m_hitBBox.right >= vcenter)
            m_children[1].HitTestBall(pball, coll);
      }
      else
         if(axis == 1)
         {
            const float vcenter = (m_rectbounds.top+m_rectbounds.bottom)*0.5f;
            if (pball->m_hitBBox.top <= vcenter)
               m_children[0].HitTestBall(pball, coll);
            if (pball->m_hitBBox.bottom >= vcenter)
               m_children[1].HitTestBall(pball, coll);
         }
         else
         {
            const float vcenter = (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f;

            if(pball->m_hitBBox.zlow <= vcenter)
               m_children[0].HitTestBall(pball, coll);
            if(pball->m_hitBBox.zhigh >= vcenter)
               m_children[1].HitTestBall(pball, coll);
         }
   }
#endif
}

#ifdef KDTREE_SSE_LEAFTEST
void HitKDNode::HitTestBallSse(Ball * const pball, CollisionEvent& coll) const
{
   const HitKDNode* stack[128]; //!! should be enough, but better implement test in construction to not exceed this
   unsigned int stackpos = 0;
   stack[0] = NULL; // sentinel

   const HitKDNode* __restrict current = this;

   const unsigned int padded = (m_hitoct->m_num_items + 3) & 0xFFFFFFFC;

   const __m128* __restrict const pL = (__m128*)m_hitoct->l_r_t_b_zl_zh;
   const __m128* __restrict const pR = (__m128*)(m_hitoct->l_r_t_b_zl_zh + padded);
   const __m128* __restrict const pT = (__m128*)(m_hitoct->l_r_t_b_zl_zh + padded * 2);
   const __m128* __restrict const pB = (__m128*)(m_hitoct->l_r_t_b_zl_zh + padded * 3);
   const __m128* __restrict const pZl = (__m128*)(m_hitoct->l_r_t_b_zl_zh + padded * 4);
   const __m128* __restrict const pZh = (__m128*)(m_hitoct->l_r_t_b_zl_zh + padded * 5);

   // init SSE registers with ball bbox
   /*const __m128 bleft = _mm_set1_ps(pball->m_hitBBox.left);
   const __m128 bright = _mm_set1_ps(pball->m_hitBBox.right);
   const __m128 btop = _mm_set1_ps(pball->m_hitBBox.top);
   const __m128 bbottom = _mm_set1_ps(pball->m_hitBBox.bottom);
   const __m128 bzlow = _mm_set1_ps(pball->m_hitBBox.zlow);
   const __m128 bzhigh = _mm_set1_ps(pball->m_hitBBox.zhigh);*/

   const __m128 posx = _mm_set1_ps(pball->m_pos.x);
   const __m128 posy = _mm_set1_ps(pball->m_pos.y);
   const __m128 posz = _mm_set1_ps(pball->m_pos.z);
   const __m128 rsqr = _mm_set1_ps(pball->m_rcHitRadiusSqr);

   const bool traversal_order = (rand_mt_01() < 0.5f); // swaps test order in leafs randomly
   const unsigned int dt = traversal_order ? 1 : -1;

   do
   {
      const unsigned int org_items = (current->m_items & 0x3FFFFFFF);
      const unsigned int axis = (current->m_items >> 30);

      // loop implements 4 collision checks at once
      // (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow)
      const unsigned int size = (current->m_start + org_items + 3) / 4;
      const unsigned int start = traversal_order ? current->m_start / 4 : (size - 1);
      const unsigned int end = traversal_order ? size : (current->m_start / 4 - 1);
      for (unsigned int i = start; i != end; i += dt)
      {
#ifdef _DEBUGPHYSICS
         g_pplayer->c_tested++; //!! +=4? or is this more fair?
#endif
         // comparisons set bits if bounds miss. if all bits are set, there is no collision. otherwise continue comparisons
         // bits set, there is a bounding box collision
         /*__m128 cmp = _mm_cmpge_ps(bright, pL[i]);
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
         if (mask == 0) continue;*/ //!! do bbox test before to save alu-instructions? or not to save registers? -> currently not, as just sphere vs sphere

		 // test actual sphere against box(es)
		 const __m128 zero = _mm_setzero_ps();
		 __m128 ex = _mm_add_ps(_mm_max_ps(_mm_sub_ps(pL[i],  posx), zero), _mm_max_ps(_mm_sub_ps(posx, pR[i] ), zero));
		 __m128 ey = _mm_add_ps(_mm_max_ps(_mm_sub_ps(pT[i],  posy), zero), _mm_max_ps(_mm_sub_ps(posy, pB[i] ), zero));
		 __m128 ez = _mm_add_ps(_mm_max_ps(_mm_sub_ps(pZl[i], posz), zero), _mm_max_ps(_mm_sub_ps(posz, pZh[i]), zero));
		 ex = _mm_mul_ps(ex, ex);
		 ey = _mm_mul_ps(ey, ey);
		 ez = _mm_mul_ps(ez, ez);
		 const __m128 d = _mm_add_ps(_mm_add_ps(ex, ey), ez);
		 const __m128 cmp2 = _mm_cmple_ps(d, rsqr);
		 const int mask2 = _mm_movemask_ps(cmp2);
		 if (mask2 == 0) continue;
		 
		 // now there is at least one bbox collision
         if ((mask2 & 1) != 0)
         {
            HitObject * const pho = m_hitoct->GetItemAt(i * 4);
            if (pball != pho) // ball can not hit itself
               DoHitTest(pball, pho, coll);
         }
         // array boundary checks for the rest not necessary as non-valid entries were initialized to keep these maskbits 0
         if ((mask2 & 2) != 0 /*&& (i*4+1)<m_hitoct->m_num_items*/)
         {
            HitObject * const pho = m_hitoct->GetItemAt(i * 4 + 1);
            if (pball != pho) // ball can not hit itself
               DoHitTest(pball, pho, coll);
         }
         if ((mask2 & 4) != 0 /*&& (i*4+2)<m_hitoct->m_num_items*/)
         {
            HitObject * const pho = m_hitoct->GetItemAt(i * 4 + 2);
            if (pball != pho) // ball can not hit itself
               DoHitTest(pball, pho, coll);
         }
         if ((mask2 & 8) != 0 /*&& (i*4+3)<m_hitoct->m_num_items*/)
         {
            HitObject * const pho = m_hitoct->GetItemAt(i * 4 + 3);
            if (pball != pho) // ball can not hit itself
               DoHitTest(pball, pho, coll);
         }
      }

      //if (stackpos >= 127)
      //	ShowError("kdtree stack size to be exceeded");

      if (current->m_children) // not a leaf
      {
#ifdef _DEBUGPHYSICS
         g_pplayer->c_traversed++;
#endif
         if (axis == 0)
         {
            const float vcenter = (current->m_rectbounds.left + current->m_rectbounds.right)*0.5f;
            if (pball->m_hitBBox.left <= vcenter) stack[++stackpos] = current->m_children;
            if (pball->m_hitBBox.right >= vcenter) stack[++stackpos] = current->m_children + 1;
         }
         else
            if (axis == 1)
            {
               const float vcenter = (current->m_rectbounds.top + current->m_rectbounds.bottom)*0.5f;
               if (pball->m_hitBBox.top <= vcenter) stack[++stackpos] = current->m_children;
               if (pball->m_hitBBox.bottom >= vcenter) stack[++stackpos] = current->m_children + 1;
            }
            else
            {
               const float vcenter = (current->m_rectbounds.zlow + current->m_rectbounds.zhigh)*0.5f;
               if (pball->m_hitBBox.zlow <= vcenter) stack[++stackpos] = current->m_children;
               if (pball->m_hitBBox.zhigh >= vcenter) stack[++stackpos] = current->m_children + 1;
            }
      }

      //current = stack[stackpos];
      //if (stackpos > 0)
      //    stackpos--;
      current = stack[stackpos--]; // above test not needed due to sentinel in stack[0]=NULL

   } while (current);
}
#endif

void HitKDNode::HitTestXRay(const Ball * const pball, Vector<HitObject> * const pvhoHit, CollisionEvent& coll) const
{
   const unsigned int org_items = (m_items & 0x3FFFFFFF);
   const unsigned int axis = (m_items >> 30);

   for (unsigned i = m_start; i < m_start + org_items; i++)
   {
#ifdef _DEBUGPHYSICS
      g_pplayer->c_tested++;
#endif
      HitObject * const pho = m_hitoct->GetItemAt(i);
      if ((pball != pho) && // ball cannot hit itself
         /*fRectIntersect3D(pball->m_hitBBox, pho->m_hitBBox) &&*/ //!! do bbox test before to save alu-instructions? or not to save registers? -> currently not, as just sphere vs sphere
		 fRectIntersect3D(pball->m_pos, pball->m_rcHitRadiusSqr, pho->m_hitBBox))
      {
#ifdef _DEBUGPHYSICS
         g_pplayer->c_deepTested++;
#endif
         const float newtime = pho->HitTest(pball, coll.m_hittime, coll);
         if (newtime >= 0)
            pvhoHit->AddElement(pho);
      }
   }

   if (m_children) // not a leaf
   {
#ifdef _DEBUGPHYSICS
      g_pplayer->c_traversed++;
#endif
      if (axis == 0)
      {
         const float vcenter = (m_rectbounds.left + m_rectbounds.right)*0.5f;
         if (pball->m_hitBBox.left <= vcenter)
            m_children[0].HitTestXRay(pball, pvhoHit, coll);
         if (pball->m_hitBBox.right >= vcenter)
            m_children[1].HitTestXRay(pball, pvhoHit, coll);
      }
      else
         if (axis == 1)
         {
            const float vcenter = (m_rectbounds.top + m_rectbounds.bottom)*0.5f;
            if (pball->m_hitBBox.top <= vcenter)
               m_children[0].HitTestXRay(pball, pvhoHit, coll);
            if (pball->m_hitBBox.bottom >= vcenter)
               m_children[1].HitTestXRay(pball, pvhoHit, coll);
         }
         else
         {
            const float vcenter = (m_rectbounds.zlow + m_rectbounds.zhigh)*0.5f;

            if (pball->m_hitBBox.zlow <= vcenter)
               m_children[0].HitTestXRay(pball, pvhoHit, coll);
            if (pball->m_hitBBox.zhigh >= vcenter)
               m_children[1].HitTestXRay(pball, pvhoHit, coll);
         }
   }
}
