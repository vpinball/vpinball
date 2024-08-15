// license:GPLv3+

#pragma once

#include "physics/hitball.h"
#include "physics/collide.h"

#ifdef ENABLE_SSE_OPTIMIZATIONS
#define KDTREE_SSE_LEAFTEST
#else
#pragma message ("Warning: No SSE kdtree tests")
#endif

class HitKD;

class HitKDNode final
{
private:
   void Reset() { m_children = nullptr; m_hitoct = nullptr; m_start = 0; m_items = 0; }

   void HitTestBall(const HitBall* const pball, CollisionEvent& coll) const;
   void HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const;

   void CreateNextLevel(const unsigned int level, unsigned int level_empty);

#ifdef KDTREE_SSE_LEAFTEST
   void HitTestBallSse(const HitBall* const pball, CollisionEvent& coll) const;
#endif

   FRect3D m_rectbounds;
   unsigned int m_start;
   unsigned int m_items; // contains the 2 bits for axis (bits 30/31)

   HitKDNode * m_children; // if nullptr, is a leaf; otherwise keeps the 2 children

   HitKD * m_hitoct; //!! meh, stupid

   friend class HitKD;
};

class HitKD final
{
public:
   HitKD();
   ~HitKD();

   void Init(vector<HitObject*> &vho);

   void AddElementByIndex(unsigned i)
   {
      m_org_idx.push_back(i);
   }

   void FillFromVector(vector<HitObject*> &vho);
   void FillFromIndices();
   void FillFromIndices(const FRect3D& initialBounds);

   // call when the bounding boxes of the HitObjects have changed to update the tree
   void Update();

   // call when finalizing a tree (no dynamic changes planned on it)
   void Finalize();

   unsigned int GetObjectCount() const { return m_num_items; }
   unsigned int GetNLevels() const { return m_nLevels; }

   void HitTestBall(const HitBall* const pball, CollisionEvent& coll) const
   {
#ifdef KDTREE_SSE_LEAFTEST
      m_rootNode.HitTestBallSse(pball, coll);
#else
      m_rootNode.HitTestBall(pball, coll);
#endif
   }

   void HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const
   {
      m_rootNode.HitTestXRay(pball, pvhoHit, coll);
   }

private:

   void InitSseArrays();

   vector<unsigned int> m_org_idx;

   HitKDNode m_rootNode;

   unsigned int m_num_items;
   unsigned int m_max_items;

   HitObject* GetItemAt(const unsigned i) const
   {
      return (*m_org_vho)[m_org_idx[i]];
   }

   HitKDNode* AllocTwoNodes();

   vector<HitObject*> *m_org_vho;
   vector<unsigned int> tmp;
#ifdef KDTREE_SSE_LEAFTEST
   float * __restrict l_r_t_b_zl_zh;
#endif

   vector< HitKDNode > m_nodes;
   unsigned m_num_nodes;

   friend class HitKDNode;

   unsigned int m_nLevels = 0;
};
