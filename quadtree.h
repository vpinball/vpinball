#pragma once

#include "collide.h"

class HitQuadtree
{
public:
   HitQuadtree()
   {
      m_unique = NULL;
      m_fLeaf = true;
      lefts = 0;
      rights = 0;
      tops = 0;
      bottoms = 0;
      zlows = 0;
      zhighs = 0;
   }

   ~HitQuadtree();

   void AddElement(HitObject *pho)         { m_vho.push_back(pho); }
   void Initialize();
   void Initialize(const FRect3D& bounds);

   void HitTestBall(Ball * const pball, CollisionEvent& coll) const;
   void HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit, CollisionEvent& coll) const;

private:

   void CreateNextLevel(const FRect3D& bounds, const unsigned int level, unsigned int level_empty);
   void HitTestBallSse(Ball * const pball, CollisionEvent& coll) const;

   Primitive* m_unique; // everything below/including this node shares the same original primitive object (just for early outs if not collidable)

   std::vector<HitObject*> m_vho;
   HitQuadtree * __restrict m_children[4];
   Vertex3Ds m_vcenter;

   // helper arrays for SSE boundary checks
   void InitSseArrays();
   float* __restrict lefts;
   float* __restrict rights;
   float* __restrict tops;
   float* __restrict bottoms;
   float* __restrict zlows;
   float* __restrict zhighs;

   bool m_fLeaf;

#if !defined(NDEBUG) && defined(PRINT_DEBUG_COLLISION_TREE)
public:
   void DumpTree(const int indentLevel)
   {
      char indent[256];
      for (int i = 0; i <= indentLevel; ++i)
         indent[i] = (i == indentLevel) ? 0 : ' ';
      char msg[256];
      sprintf_s(msg, "[%f %f], items=%u", m_vcenter.x, m_vcenter.y, m_vho.size());
      strcat_s(indent, msg);
      OutputDebugString(indent);
      if (!m_fLeaf)
      {
         m_children[0]->DumpTree(indentLevel + 1);
         m_children[1]->DumpTree(indentLevel + 1);
         m_children[2]->DumpTree(indentLevel + 1);
         m_children[3]->DumpTree(indentLevel + 1);
      }
   }
#endif
};
