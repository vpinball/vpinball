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
        l_r_t_b_zl_zh = (float*)_aligned_malloc(sizeof(float) * ((m_num_items+3)&0xFFFFFFFC) * 6, 16);
#endif
        m_max_items = m_num_items;
    }

    m_org_idx.clear();
    m_org_idx.reserve( m_num_items );

    tmp.clear();
    tmp.resize( m_num_items );

    m_nodes.clear();
    /* NB:
     * Unfortunately, there is no a priori bound on the number of nodes in the tree. We just make
     * an educated guess on the maximum and truncate the subdivision if we run out of nodes.
     */
    m_nodes.resize( (m_num_items + 1) & ~1u );      // always allocate an even number of nodes, rounded up
    m_num_nodes = 0;

    m_rootNode.Reset();
    m_rootNode.m_hitoct = this;
}

HitKDNode* HitKD::AllocTwoNodes()
{
    if ((m_num_nodes + 1) >= m_nodes.size())        // space for two more nodes?
        return NULL;
    else
    {
        m_num_nodes += 2;
        return &m_nodes[ m_num_nodes - 2 ];
    }
}


// build SSE boundary arrays of the local hit-object/m_vho HitRect list, generated for -full- list completely in the end!
void HitKD::InitSseArrays()
{
#ifdef KDTREE_SSE_LEAFTEST
	const unsigned int padded = (m_num_items+3)&0xFFFFFFFC;
	if(!l_r_t_b_zl_zh)
		l_r_t_b_zl_zh = (float*)_aligned_malloc(sizeof(float) * padded * 6, 16);

    for(unsigned int j = 0; j < m_num_items; ++j)
    {
		const FRect3D& r = GetItemAt(j)->m_rcHitRect;
		l_r_t_b_zl_zh[j] = r.left;
		l_r_t_b_zl_zh[j+padded] = r.right;
		l_r_t_b_zl_zh[j+padded*2] = r.top;
		l_r_t_b_zl_zh[j+padded*3] = r.bottom;
		l_r_t_b_zl_zh[j+padded*4] = r.zlow;
		l_r_t_b_zl_zh[j+padded*5] = r.zhigh;
    }

	for(unsigned int j = m_num_items; j < padded; ++j)
	{
		l_r_t_b_zl_zh[j] = FLT_MAX;
		l_r_t_b_zl_zh[j+padded] = -FLT_MAX;
		l_r_t_b_zl_zh[j+padded*2] = FLT_MAX;
		l_r_t_b_zl_zh[j+padded*3] = -FLT_MAX;
		l_r_t_b_zl_zh[j+padded*4] = FLT_MAX;
		l_r_t_b_zl_zh[j+padded*5] = -FLT_MAX;
	}
#endif
}

void HitKD::FillFromVector(Vector<HitObject>& vho)
{
    Init(&vho, vho.Size());

    m_rootNode.m_start = 0;
    m_rootNode.m_items = vho.Size();

    m_rootNode.m_rectbounds.Clear();

    for (unsigned i = 0; i < m_rootNode.m_items; ++i)
    {
        HitObject * const pho = vho.ElementAt(i);
        pho->CalcHitRect(); //!! omit, as already calced?!
        m_rootNode.m_rectbounds.Extend( pho->m_rcHitRect );

        m_org_idx.push_back( i );
    }

    m_rootNode.CreateNextLevel();
    InitSseArrays();
}

void HitKD::FillFromIndices()
{
    m_rootNode.m_rectbounds.Clear();

    m_rootNode.m_start = 0;
    m_rootNode.m_items = m_org_idx.size();

    for (unsigned i = 0; i < m_rootNode.m_items; ++i)
    {
        HitObject * const pho = GetItemAt( i );
        pho->CalcHitRect(); //!! omit, as already calced?!
        m_rootNode.m_rectbounds.Extend( pho->m_rcHitRect );
    }

    m_rootNode.CreateNextLevel();
    InitSseArrays();
}

void HitKD::FillFromIndices(const FRect3D& initialBounds)
{
    m_rootNode.m_rectbounds = initialBounds;

    m_rootNode.m_start = 0;
    m_rootNode.m_items = m_org_idx.size();

    // assume that CalcHitRect() was already called on the hit objects

    m_rootNode.CreateNextLevel();
    InitSseArrays();
}

void HitKD::Update()
{
    FillFromVector(*m_org_vho);
}


void HitKDNode::CreateNextLevel()
{
	const unsigned int org_items = (m_items&0x3FFFFFFF);

	if(org_items <= 4) //!! magic (will not favor empty space enough for huge objects)
		return;

	const Vertex3Ds vdiag(m_rectbounds.right-m_rectbounds.left, m_rectbounds.bottom-m_rectbounds.top, m_rectbounds.zhigh-m_rectbounds.zlow);

	unsigned int axis;
	if((vdiag.x > vdiag.y) && (vdiag.x > vdiag.z))
	{
		if(vdiag.x < 6.66f) //!! magic (will not subdivide object soups enough)
			return;
		axis = 0;
	}
	else if(vdiag.y > vdiag.z)
	{
		if(vdiag.y < 6.66f)
			return;
		axis = 1;
	}
	else
	{
		if(vdiag.z < 6.66f)
			return;
		axis = 2;
	}

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
	g_pplayer->c_octNextlevels++;
#endif

	const Vertex3Ds vcenter((m_rectbounds.left+m_rectbounds.right)*0.5f, (m_rectbounds.top+m_rectbounds.bottom)*0.5f, (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f);
	if(axis == 0)
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
        for(unsigned int i = m_start; i < m_start+org_items; ++i)
        {
            HitObject * const pho = m_hitoct->GetItemAt( i );

            if      (pho->m_rcHitRect.right < vcenter.x)    m_children[0].m_items++;
            else if (pho->m_rcHitRect.left > vcenter.x)     m_children[1].m_items++;
        }
    } 
    else if (axis == 1)
    {
        for(unsigned int i = m_start; i < m_start+org_items; ++i)
        {
            HitObject * const pho = m_hitoct->GetItemAt( i );

            if      (pho->m_rcHitRect.bottom < vcenter.y)   m_children[0].m_items++;
            else if (pho->m_rcHitRect.top > vcenter.y)      m_children[1].m_items++;
        }
    }
    else // axis == 2
    {
        for(unsigned int i = m_start; i < m_start+org_items; ++i)
        {
            HitObject * const pho = m_hitoct->GetItemAt( i );

            if      (pho->m_rcHitRect.zhigh < vcenter.z)    m_children[0].m_items++;
            else if (pho->m_rcHitRect.zlow > vcenter.z)     m_children[1].m_items++;
        }
    }

	m_children[0].m_start = m_start + org_items - m_children[0].m_items - m_children[1].m_items;
	m_children[1].m_start = m_children[0].m_start + m_children[0].m_items;

    unsigned int items = 0;
    m_children[0].m_items = 0;
    m_children[1].m_items = 0;

    // sort items that cross splitplane in-place, the others are sorted into a temporary
    if (axis == 0)
    {
        for(unsigned int i = m_start; i < m_start+org_items; ++i)
        {
            HitObject * const pho = m_hitoct->GetItemAt( i );

            if (pho->m_rcHitRect.right < vcenter.x)
                m_hitoct->tmp[m_children[0].m_start + (m_children[0].m_items++)] = m_hitoct->m_org_idx[i];
            else if (pho->m_rcHitRect.left > vcenter.x)
                m_hitoct->tmp[m_children[1].m_start + (m_children[1].m_items++)] = m_hitoct->m_org_idx[i];
            else
                m_hitoct->m_org_idx[m_start + (items++)] = m_hitoct->m_org_idx[i];
        }
    } 
    else if (axis == 1)
    {
        for(unsigned int i = m_start; i < m_start+org_items; ++i)
        {
            HitObject * const pho = m_hitoct->GetItemAt( i );

            if (pho->m_rcHitRect.bottom < vcenter.y)
                m_hitoct->tmp[m_children[0].m_start + (m_children[0].m_items++)] = m_hitoct->m_org_idx[i];
            else if (pho->m_rcHitRect.top > vcenter.y)
                m_hitoct->tmp[m_children[1].m_start + (m_children[1].m_items++)] = m_hitoct->m_org_idx[i];
            else
                m_hitoct->m_org_idx[m_start + (items++)] = m_hitoct->m_org_idx[i];
        }
    }
    else // axis == 2
    {
        for(unsigned int i = m_start; i < m_start+org_items; ++i)
        {
            HitObject * const pho = m_hitoct->GetItemAt( i );

            if (pho->m_rcHitRect.zhigh < vcenter.z)
                m_hitoct->tmp[m_children[0].m_start + (m_children[0].m_items++)] = m_hitoct->m_org_idx[i];
            else if (pho->m_rcHitRect.zlow > vcenter.z)
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

    m_items = items | (axis<<30);

    // copy temporary back //!! could omit this by doing everything inplace
    memcpy(&m_hitoct->m_org_idx[ m_children[0].m_start ], &m_hitoct->tmp[ m_children[0].m_start ], m_children[0].m_items*sizeof(unsigned int));
    memcpy(&m_hitoct->m_org_idx[ m_children[1].m_start ], &m_hitoct->tmp[ m_children[1].m_start ], m_children[1].m_items*sizeof(unsigned int));

	m_children[0].CreateNextLevel();
	m_children[1].CreateNextLevel();
}


/*  RLC

Hit logic needs to be expanded, during static and psudo-static conditions, multiple hits (multi-face contacts)
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
		       && fRectIntersect3D(pball->m_rcHitRect, pho->m_rcHitRect))
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
			if(pball->m_rcHitRect.left <= vcenter)
				m_children[0].HitTestBall(pball, coll);
			if(pball->m_rcHitRect.right >= vcenter)
				m_children[1].HitTestBall(pball, coll);
		}
		else
		if(axis == 1)
		{
			const float vcenter = (m_rectbounds.top+m_rectbounds.bottom)*0.5f;
			if (pball->m_rcHitRect.top <= vcenter)
				m_children[0].HitTestBall(pball, coll);
			if (pball->m_rcHitRect.bottom >= vcenter)
				m_children[1].HitTestBall(pball, coll);
		}
		else
		{
			const float vcenter = (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f;

			if(pball->m_rcHitRect.zlow <= vcenter)
				m_children[0].HitTestBall(pball, coll);
			if(pball->m_rcHitRect.zhigh >= vcenter)
				m_children[1].HitTestBall(pball, coll);
		}
	}
#endif
}

#ifdef KDTREE_SSE_LEAFTEST
void HitKDNode::HitTestBallSseInner(Ball * const pball, const int i, CollisionEvent& coll) const
{
  HitObject * const pho = m_hitoct->GetItemAt( i );

  // ball can not hit itself
  if (pball == pho)
    return;

  DoHitTest(pball, pho, coll);
}

void HitKDNode::HitTestBallSse(Ball * const pball, CollisionEvent& coll) const
{
	const unsigned int org_items = (m_items&0x3FFFFFFF);
	const unsigned int axis = (m_items>>30);

    const unsigned int padded = (m_hitoct->m_num_items+3)&0xFFFFFFFC;

    // init SSE registers with ball bbox
    const __m128 bleft = _mm_set1_ps(pball->m_rcHitRect.left);
    const __m128 bright = _mm_set1_ps(pball->m_rcHitRect.right);
    const __m128 btop = _mm_set1_ps(pball->m_rcHitRect.top);
    const __m128 bbottom = _mm_set1_ps(pball->m_rcHitRect.bottom);
    const __m128 bzlow = _mm_set1_ps(pball->m_rcHitRect.zlow);
    const __m128 bzhigh = _mm_set1_ps(pball->m_rcHitRect.zhigh);

    const __m128* __restrict const pL = (__m128*)m_hitoct->l_r_t_b_zl_zh;
    const __m128* __restrict const pR = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded);
    const __m128* __restrict const pT = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded*2);
    const __m128* __restrict const pB = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded*3);
    const __m128* __restrict const pZl = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded*4);
    const __m128* __restrict const pZh = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded*5);

    // loop implements 4 collision checks at once
    // (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow)
    const unsigned int size = (m_start+org_items+3)/4;
    for (unsigned int i = m_start/4; i < size; ++i)
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

      // now there is at least one bbox collision
      if ((mask & 1) != 0) HitTestBallSseInner(pball, i*4, coll);
      if ((mask & 2) != 0 /*&& (i*4+1)<m_hitoct->m_num_items*/) HitTestBallSseInner(pball, i*4+1, coll); // boundary checks not necessary
      if ((mask & 4) != 0 /*&& (i*4+2)<m_hitoct->m_num_items*/) HitTestBallSseInner(pball, i*4+2, coll); //  anymore as non-valid entries were
      if ((mask & 8) != 0 /*&& (i*4+3)<m_hitoct->m_num_items*/) HitTestBallSseInner(pball, i*4+3, coll); //  initialized to keep these maskbits 0
    }

	if (m_children) // not a leaf
	{
#ifdef _DEBUGPHYSICS
		g_pplayer->c_traversed++;
#endif
		if(axis == 0)
		{
			const float vcenter = (m_rectbounds.left+m_rectbounds.right)*0.5f;
			if(pball->m_rcHitRect.left <= vcenter)
				m_children[0].HitTestBallSse(pball, coll);
			if(pball->m_rcHitRect.right >= vcenter)
				m_children[1].HitTestBallSse(pball, coll);
		}
		else
		if(axis == 1)
		{
			const float vcenter = (m_rectbounds.top+m_rectbounds.bottom)*0.5f;
			if (pball->m_rcHitRect.top <= vcenter)
				m_children[0].HitTestBallSse(pball, coll);
			if (pball->m_rcHitRect.bottom >= vcenter)
				m_children[1].HitTestBallSse(pball, coll);
		}
		else
		{
			const float vcenter = (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f;

			if(pball->m_rcHitRect.zlow <= vcenter)
				m_children[0].HitTestBallSse(pball, coll);
			if(pball->m_rcHitRect.zhigh >= vcenter)
				m_children[1].HitTestBallSse(pball, coll);
		}
	}
}
#endif

void HitKDNode::HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit, CollisionEvent& coll) const
{
	const unsigned int org_items = (m_items&0x3FFFFFFF);
	const unsigned int axis = (m_items>>30);

	for (unsigned i=m_start; i<m_start+org_items; i++)
	{
#ifdef _DEBUGPHYSICS
		g_pplayer->c_tested++;
#endif
		HitObject * const pho = m_hitoct->GetItemAt( i );
		if ((pball != pho) && // ball cannot hit itself
			fRectIntersect3D(pball->m_rcHitRect,pho->m_rcHitRect))
		{
#ifdef _DEBUGPHYSICS
			g_pplayer->c_deepTested++;
#endif
			const float newtime = pho->HitTest(pball, coll.hittime, coll);
			if (newtime >= 0)
				pvhoHit->AddElement(pho);
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
			if(pball->m_rcHitRect.left <= vcenter)
				m_children[0].HitTestXRay(pball, pvhoHit, coll);
			if(pball->m_rcHitRect.right >= vcenter)
				m_children[1].HitTestXRay(pball, pvhoHit, coll);
		}
		else
		if(axis == 1)
		{
			const float vcenter = (m_rectbounds.top+m_rectbounds.bottom)*0.5f;
			if (pball->m_rcHitRect.top <= vcenter)
				m_children[0].HitTestXRay(pball, pvhoHit, coll);
			if (pball->m_rcHitRect.bottom >= vcenter)
				m_children[1].HitTestXRay(pball, pvhoHit, coll);
		}
		else
		{
			const float vcenter = (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f;

			if(pball->m_rcHitRect.zlow <= vcenter)
				m_children[0].HitTestXRay(pball, pvhoHit, coll);
			if(pball->m_rcHitRect.zhigh >= vcenter)
				m_children[1].HitTestXRay(pball, pvhoHit, coll);
		}
	}
}
