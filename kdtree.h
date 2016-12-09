#pragma once

#include "pin/ball.h"
#include "pin/collide.h"

#define KDTREE_SSE_LEAFTEST

class HitKD;

class HitKDNode
{
private:
   void Reset() { m_children = NULL; m_hitoct = NULL; m_start = 0; m_items = 0; }

   void HitTestBall(Ball * const pball, CollisionEvent& coll) const;
   void HitTestXRay(const Ball * const pball, Vector<HitObject> * const pvhoHit, CollisionEvent& coll) const;

   void CreateNextLevel(const unsigned int level, unsigned int level_empty);

#ifdef KDTREE_SSE_LEAFTEST
   void HitTestBallSse(Ball * const pball, CollisionEvent& coll) const;
#endif

   FRect3D m_rectbounds;
   unsigned int m_start;
   unsigned int m_items; // contains the 2 bits for axis (bits 30/31)

   HitKDNode * m_children; // if NULL, is a leaf; otherwise keeps the 2 children

   HitKD * m_hitoct; //!! meh, stupid

   friend class HitKD;
};

class HitKD
{
public:
   HitKD();
   ~HitKD();

   void Init(Vector<HitObject> *vho, const unsigned int num_items);

   void AddElementByIndex(unsigned i)
   {
      m_org_idx.push_back(i);
   }

   void FillFromVector(Vector<HitObject>& vho);
   void FillFromIndices();
   void FillFromIndices(const FRect3D& initialBounds);

   // call when the bounding boxes of the HitObjects have changed to update the tree
   void Update();

   // call when finalizing a tree (no dynamic changes planned on it)
   void Finalize();

   void HitTestBall(Ball * const pball, CollisionEvent& coll) const
   {
      m_rootNode.HitTestBallSse(pball, coll);
   }

   void HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit, CollisionEvent& coll) const
   {
      m_rootNode.HitTestXRay(pball, pvhoHit, coll);
   }

private:

   void InitSseArrays();

   std::vector<unsigned int> m_org_idx;

   HitKDNode m_rootNode;

   unsigned int m_num_items;
   unsigned int m_max_items;

   HitObject* GetItemAt(const unsigned i) const
   {
      return m_org_vho->ElementAt(m_org_idx[i]);
   }

   HitKDNode* AllocTwoNodes();

   Vector<HitObject> *m_org_vho;
   std::vector<unsigned int> tmp;
#ifdef KDTREE_SSE_LEAFTEST
   float * __restrict l_r_t_b_zl_zh;
#endif

   std::vector< HitKDNode > m_nodes;
   unsigned m_num_nodes;

   friend class HitKDNode;
};
