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
   void Reset() { m_children = nullptr; m_start = 0; m_items = 0; }

   void HitTestBall(const HitKD* hitoct, const HitBall* const pball, CollisionEvent& coll) const;
   void HitTestXRay(const HitKD* hitoct, const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const;

   void CreateNextLevel(HitKD* hitoct, const unsigned int level, unsigned int level_empty);

   FRect3D m_rectbounds;
   unsigned int m_start; // index of first item in HitKD.m_vho
   unsigned int m_items; // number of items (bit 0..29) and axis (bits 30..31)

   HitKDNode * m_children; // if nullptr, is a leaf; otherwise keeps the 2 children

   friend class HitKD;
};

class HitKD final
{
public:
   HitKD();
   ~HitKD();

   void Reset(const vector<HitObject*> &vho);
   void Insert(HitObject* ho);
   void Remove(HitObject* ho);
   void Update(); // call when the bounding boxes of the HitObjects have changed to update the tree
   void Finalize(); // call when finalizing a tree (no dynamic changes planned on it)
   const vector<HitObject*>& GetHitObjects() const { return m_vho; }

   // FIXME dead code. Remove ?
   void AddElementByIndex(unsigned i) { m_org_idx.push_back(i); }
   void FillFromIndices();
   void FillFromIndices(const FRect3D& initialBounds);

   unsigned int GetObjectCount() const { return m_num_items; }
   unsigned int GetNLevels() const { return m_nLevels; }

   void HitTestBall(const HitBall* const pball, CollisionEvent& coll) const { m_rootNode.HitTestBall(this, pball, coll); }
   void HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const { m_rootNode.HitTestXRay(this, pball, pvhoHit, coll); }

private:
   void Initialize();
   void InitSseArrays();

   vector<HitObject*> m_vho; // all items
   unsigned int m_num_items = 0; // alias of m_vho.size()
   unsigned int m_max_items = 0;

   vector<unsigned int> m_org_idx;
   HitObject* GetItemAt(const unsigned i) const { return m_vho[m_org_idx[i]]; }

   HitKDNode m_rootNode;
   
   // Node pool
   vector<HitKDNode> m_nodes;
   unsigned m_num_nodes = 0;
   HitKDNode* AllocTwoNodes();

   vector<unsigned int> tmp;

   #ifdef KDTREE_SSE_LEAFTEST
      float * __restrict l_r_t_b_zl_zh = nullptr;
   #endif

   unsigned int m_nLevels = 0;

   friend class HitKDNode;
};
