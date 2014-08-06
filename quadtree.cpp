#include "stdafx.h"
#include "quadtree.h"

HitQuadtree::~HitQuadtree()
{
    if(lefts != 0)
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
        for (int i=0; i<4; i++)
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
        bounds.Extend(m_vho[i]->m_rcHitRect);

    CreateNextLevel(bounds);
}

void HitQuadtree::CreateNextLevel(const FRect3D& bounds)
{
    m_fLeaf = false;

    m_vcenter.x = (bounds.left + bounds.right)*0.5f;
    m_vcenter.y = (bounds.top + bounds.bottom)*0.5f;
    m_vcenter.z = (bounds.zlow + bounds.zhigh)*0.5f;

    for (int i=0; i<4; i++)
    {
        m_children[i] = new HitQuadtree();
        m_children[i]->m_fLeaf = true;
    }

    std::vector<HitObject*> vRemain; // hit objects which did not go to a quadrant

    // sort items into appropriate child nodes
    for (unsigned i=0; i<m_vho.size(); i++)
    {
        int oct;
        HitObject * const pho = m_vho[i];

        if (pho->m_rcHitRect.right < m_vcenter.x)
            oct = 0;
        else if (pho->m_rcHitRect.left > m_vcenter.x)
            oct = 1;
        else
            oct = 128;

        if (pho->m_rcHitRect.bottom < m_vcenter.y)
            oct |= 0;
        else if (pho->m_rcHitRect.top > m_vcenter.y)
            oct |= 2;
        else
            oct |= 128;

        if ((oct & 128) == 0)
            m_children[oct]->m_vho.push_back(pho);
        else
            vRemain.push_back(pho);
    }

    m_vho.swap(vRemain);

    if (m_vcenter.x - bounds.left > 125.0f)       // TODO: bad heuristic, improve this
    {
        for (int i=0; i<4; ++i)
        {
            FRect3D childBounds;

            childBounds.left = (i&1) ? m_vcenter.x : bounds.left;
            childBounds.top  = (i&2) ? m_vcenter.y : bounds.top;
            childBounds.zlow = bounds.zlow;

            childBounds.right  = (i&1) ? bounds.right  : m_vcenter.x;
            childBounds.bottom = (i&2) ? bounds.bottom : m_vcenter.y;
            childBounds.zhigh  = bounds.zhigh;

            m_children[i]->CreateNextLevel(childBounds);
        }
    }

    InitSseArrays();
    for (int i=0; i<4; ++i)
        m_children[i]->InitSseArrays();
}

void HitQuadtree::InitSseArrays()
{
    // build SSE boundary arrays of the local hit-object list
    // (don't init twice)
    const size_t padded = ((m_vho.size()+3)/4)*4;
    const size_t ssebytes = sizeof(float) * padded;
    if (ssebytes > 0 && lefts == 0)
    {
        lefts   = (float*)_aligned_malloc(ssebytes, 16);
        rights  = (float*)_aligned_malloc(ssebytes, 16);
        tops    = (float*)_aligned_malloc(ssebytes, 16);
        bottoms = (float*)_aligned_malloc(ssebytes, 16);
        zlows   = (float*)_aligned_malloc(ssebytes, 16);
        zhighs  = (float*)_aligned_malloc(ssebytes, 16);

        for (size_t j=0; j<m_vho.size(); j++)
        {
            const FRect3D r = m_vho[j]->m_rcHitRect;
            lefts[j] = r.left;
            rights[j] = r.right;
            tops[j] = r.top;
            bottoms[j] = r.bottom;
            zlows[j] = r.zlow;
            zhighs[j] = r.zhigh;
        }

        for (size_t j=m_vho.size(); j<padded; j++)
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

void HitQuadtree::HitTestBall(Ball * const pball, CollisionEvent& coll) const
{
#if 1   /// with SSE optimizations //////////////////////////

    HitTestBallSse(pball, coll);

#else   /// without SSE optimization ////////////////////////

    for (unsigned i=0; i<m_vho.size(); i++)
    {
#ifdef LOG
        g_pplayer->c_tested++;
#endif
        if ((pball != m_vho[i]) // ball can not hit itself
                && fRectIntersect3D(pball->m_rcHitRect, m_vho[i]->m_rcHitRect))
        {
            DoHitTest(pball, m_vho[i], coll);
        }
    }//end for loop

    if (!m_fLeaf)
    {
        const bool fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
        const bool fRight = (pball->m_rcHitRect.right >= m_vcenter.x);

#ifdef LOG
        g_pplayer->c_tested++;
#endif
        if (pball->m_rcHitRect.top <= m_vcenter.y) // Top
        {
            if (fLeft)  m_children[0]->HitTestBall(pball, coll);
            if (fRight) m_children[1]->HitTestBall(pball, coll);
        }
        if (pball->m_rcHitRect.bottom >= m_vcenter.y) // Bottom
        {
            if (fLeft)  m_children[2]->HitTestBall(pball, coll);
            if (fRight) m_children[3]->HitTestBall(pball, coll);
        }
    }
#endif
}


void HitQuadtree::HitTestBallSseInner(Ball * const pball, const size_t i, CollisionEvent& coll) const
{
    // ball can not hit itself
    if (pball == m_vho[i])
        return;

    DoHitTest(pball, m_vho[i], coll);
}

void HitQuadtree::HitTestBallSse(Ball * const pball, CollisionEvent& coll) const
{
    if (lefts != 0)
    {
        // init SSE registers with ball bbox
        const __m128 bleft   = _mm_set1_ps(pball->m_rcHitRect.left);
        const __m128 bright  = _mm_set1_ps(pball->m_rcHitRect.right);
        const __m128 btop    = _mm_set1_ps(pball->m_rcHitRect.top);
        const __m128 bbottom = _mm_set1_ps(pball->m_rcHitRect.bottom);
        const __m128 bzlow   = _mm_set1_ps(pball->m_rcHitRect.zlow);
        const __m128 bzhigh  = _mm_set1_ps(pball->m_rcHitRect.zhigh);

        const __m128* const pL = (__m128*)lefts;
        const __m128* const pR = (__m128*)rights;
        const __m128* const pT = (__m128*)tops;
        const __m128* const pB = (__m128*)bottoms;
        const __m128* const pZl = (__m128*)zlows;
        const __m128* const pZh = (__m128*)zhighs;

        // loop implements 4 collision checks at once
        // (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow)
        const size_t size = (m_vho.size()+3)/4;
        for (size_t i=0; i<size; ++i)
        {
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
            if ((mask & 2) != 0 /*&& (i*4+1)<m_vho.size()*/) HitTestBallSseInner(pball, i*4+1, coll); // boundary checks not necessary
            if ((mask & 4) != 0 /*&& (i*4+2)<m_vho.size()*/) HitTestBallSseInner(pball, i*4+2, coll); //  anymore as non-valid entries were
            if ((mask & 8) != 0 /*&& (i*4+3)<m_vho.size()*/) HitTestBallSseInner(pball, i*4+3, coll); //  initialized to keep these maskbits 0
        }
    }

    if (!m_fLeaf)
    {
        const bool fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
        const bool fRight = (pball->m_rcHitRect.right >= m_vcenter.x);

        if (pball->m_rcHitRect.top <= m_vcenter.y) // Top
        {
            if (fLeft)  m_children[0]->HitTestBallSse(pball, coll);
            if (fRight) m_children[1]->HitTestBallSse(pball, coll);
        }
        if (pball->m_rcHitRect.bottom >= m_vcenter.y) // Bottom
        {
            if (fLeft)  m_children[2]->HitTestBallSse(pball, coll);
            if (fRight) m_children[3]->HitTestBallSse(pball, coll);
        }
    }
}

void HitQuadtree::HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit, CollisionEvent& coll) const
{
    for (unsigned i=0; i<m_vho.size(); i++)
    {
#ifdef LOG
        g_pplayer->c_tested++;
#endif
        if ((pball != m_vho[i]) && fRectIntersect3D(pball->m_rcHitRect, m_vho[i]->m_rcHitRect))
        {
#ifdef LOG
            g_pplayer->c_deepTested++;
#endif
            const float newtime = m_vho[i]->HitTest(pball, coll.hittime, coll);
            if (newtime >= 0)
            {
                pvhoHit->AddElement(m_vho[i]);
            }
        }
    }

    if (!m_fLeaf)
    {
        const bool fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
        const bool fRight = (pball->m_rcHitRect.right >= m_vcenter.x);

#ifdef LOG
        g_pplayer->c_tested++;
#endif

        if (pball->m_rcHitRect.top <= m_vcenter.y) // Top
        {
            if (fLeft)  m_children[0]->HitTestXRay(pball, pvhoHit, coll);
            if (fRight) m_children[1]->HitTestXRay(pball, pvhoHit, coll);
        }
        if (pball->m_rcHitRect.bottom >= m_vcenter.y) // Bottom
        {
            if (fLeft)  m_children[2]->HitTestXRay(pball, pvhoHit, coll);
            if (fRight) m_children[3]->HitTestXRay(pball, pvhoHit, coll);
        }
    }
}
