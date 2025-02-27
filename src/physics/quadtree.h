// license:GPLv3+

#pragma once

#include "collide.h"

//#define DISABLE_ZTEST // z values of the BBox of (objects within) a node can be constant over some traversal levels (as its a quadtree and not an octree!), so we could also just ignore z tests overall. This can lead to performance benefits on some tables ("flat" ones) and performance penalties on others (e.g. when a ball moves under detailed meshes)

//#define USE_EMBREE //!! experimental, but working, collision detection replacement for our quad and kd-tree //!! picking in debug mode so far not implemented though
#ifdef USE_EMBREE
   #include "embree3/rtcore.h"
#endif

class HitQuadtree;
class ThreadPool;

class HitQuadtreeNode final
{
public:
   HitQuadtreeNode();
   ~HitQuadtreeNode();

   void Reset();
   void CreateNextLevel(HitQuadtree* const quadTree, const FRect& bounds, const unsigned int level, unsigned int level_empty); // FRect3D for an octree
   void DumpTree(const int indentLevel);

   void HitTestBall(const HitQuadtree* const quadTree, const HitBall* const pball, CollisionEvent& coll) const;
   void HitTestXRay(const HitQuadtree* const quadTree, const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const;

private:
   // Node content
   unsigned int m_start = 0; // index of first item in HitQuadtree.m_vho
   unsigned int m_items = 0; // number of items

   // everything below/including this node shares the same original primitive/hittarget object (just for early outs if not collidable), so this is actually cast then to a Primitive* or HitTarget*
   Hitable* __restrict m_unique = nullptr; 

   HitQuadtreeNode * __restrict m_children = nullptr; // nullptr for leaf, or the 4 children otherwise
   Vertex2D m_vcenter; // center of node bounds, only defined for non leaf node

   friend class HitQuadtree;
};

class HitQuadtree final
{
public:
   HitQuadtree();
   ~HitQuadtree();

   void SetBounds(const FRect& bounds) { m_bounds = bounds; }

   // Except for Update, bounds of hit objects are not updated and must be updated by the caller
   void Reset(const vector<HitObject*>& vho);
   void Insert(HitObject* ho);
   void Remove(HitObject* ho);
   void Update(); // Update bounds of all hit objects and adjust tree accordingly
   void Finalize(); // call when finalizing a tree (no dynamic changes planned on it)
   const vector<HitObject*>& GetHitObjects() const { return m_vho; }

   // Also allow direct modification of list of hit objects to avoid copy
   vector<HitObject*>& BeginReset() { return m_vho; }
   void EndReset() { Initialize(); }

   unsigned int GetObjectCount() const { return static_cast<unsigned int>(m_vho.size()); }
   unsigned int GetNLevels() const { return m_nLevels; }

#ifndef USE_EMBREE
   void HitTestBall(const HitBall* const pball, CollisionEvent& coll) const;
   void HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const { m_rootNode.HitTestXRay(this, pball, pvhoHit, coll); }
#else
   void HitTestBall(vector<HitBall*> ball) const;
   void HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const;
#endif

private:
   void Initialize();
   vector<HitObject*> m_vho; // all items
   FRect m_bounds;

#ifndef USE_EMBREE
   HitQuadtreeNode m_rootNode;

   // Node pool
   vector<HitQuadtreeNode> m_nodes;
   size_t m_numNodes;
   std::mutex m_nodePoolMutex;
   HitQuadtreeNode* AllocFourNodes();
   
   void InitSseArrays();
   float* __restrict l_r_t_b_zl_zh = nullptr; // 4xSIMD rearranged BBox data, layout: 4xleft,4xright,4xtop,4xbottom,4xzlow,4xzhigh, 4xleft... ... ... the last entries are potentially filled with 'invalid' boxes for alignment/padding

   ThreadPool* m_threadPool = nullptr;
   vector<HitObject*> m_tmp1, m_tmp2;

   size_t m_maxItems = 0;

   unsigned int m_nLevels = 0;

   friend class HitQuadtreeNode;
   
#else
   RTCDevice m_embree_device; //!! have only one instead of the two (dynamic and non-dynamic)?
   RTCScene m_scene;
#endif
};
