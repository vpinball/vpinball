// license:GPLv3+

#pragma once

#include "collide.h"

//#define DISABLE_ZTEST // z values of the BBox of (objects within) a node can be constant over some traversal levels (as its a quadtree and not an octree!), so we could also just ignore z tests overall. This can lead to performance benefits on some tables ("flat" ones) and performance penalties on others (e.g. when a ball moves under detailed meshes)

//#define USE_EMBREE //!! experimental, but working, collision detection replacement for our quad and kd-tree //!! picking in debug mode so far not implemented though
#ifdef USE_EMBREE
 #include "embree3/rtcore.h"
#endif

class HitQuadtree final
{
public:
   HitQuadtree()
   {
#ifndef USE_EMBREE
      m_unique = nullptr;
      m_leaf = true;
      lefts_rights_tops_bottoms_zlows_zhighs = 0;
#else
      m_embree_device = rtcNewDevice(nullptr);
      m_scene = nullptr;
#endif
   }

   ~HitQuadtree();

   void Reset(const vector<HitObject*>& vho);
   void AddElement(HitObject *pho) { m_vho.push_back(pho); }
   void Initialize(const FRect& bounds); // FRect3D for an octree

   unsigned int GetObjectCount() const { return (unsigned int) m_vho.size(); }
   unsigned int GetNLevels() const { return m_nLevels; }

#ifdef USE_EMBREE
   void FillFromVector(vector<HitObject*>& vho);
   void Update();
#endif

#ifndef USE_EMBREE
   void HitTestBall(const HitBall* const pball, CollisionEvent& coll) const;
#else
   void HitTestBall(vector<HitBall*> ball) const;
#endif
   void HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const;


private:
   void Initialize();

#ifndef USE_EMBREE
   void CreateNextLevel(const FRect& bounds, const unsigned int level, unsigned int level_empty); // FRect3D for an octree
   void HitTestBallSse(const HitBall* const pball, CollisionEvent& coll) const;

   IFireEvents* __restrict m_unique; // everything below/including this node shares the same original primitive/hittarget object (just for early outs if not collidable),
                                     // so this is actually cast then to a Primitive* or HitTarget*
   HitQuadtree * __restrict m_children; // always 4 entries
   Vertex2D m_vcenter; // should be Vertex3Ds for a real octree

   // helper arrays for SSE boundary checks
   void InitSseArrays();
   float* __restrict lefts_rights_tops_bottoms_zlows_zhighs; // 4xSIMD rearranged BBox data, layout: 4xleft,4xright,4xtop,4xbottom,4xzlow,4xzhigh, 4xleft... ... ... the last entries are potentially filled with 'invalid' boxes for alignment/padding

   bool m_leaf;
   eObjType m_ObjType; // only used if m_unique != nullptr, to identify which object type this is
#else
   vector<HitObject*> *m_pvho;

   RTCDevice m_embree_device; //!! have only one instead of the two (dynamic and non-dynamic)?
   RTCScene m_scene;
#endif

   vector<HitObject*> m_vho;

#if !defined(NDEBUG) && defined(PRINT_DEBUG_COLLISION_TREE)
public:
   void DumpTree(const int indentLevel)
   {
#ifndef USE_EMBREE
      char indent[256];
      for (int i = 0; i <= indentLevel; ++i)
         indent[i] = (i == indentLevel) ? '\0' : ' ';
      char msg[256];
      sprintf_s(msg, sizeof(msg), "[%f %f], items=%u", m_vcenter.x, m_vcenter.y, m_vho.size());
      strncat_s(indent, msg, sizeof(indent)-strnlen_s(indent, sizeof(indent))-1);
      OutputDebugString(indent);
      if (!m_leaf)
      {
         m_children[0].DumpTree(indentLevel + 1);
         m_children[1].DumpTree(indentLevel + 1);
         m_children[2].DumpTree(indentLevel + 1);
         m_children[3].DumpTree(indentLevel + 1);
      }
#endif
   }
#endif

   unsigned int m_nLevels = 0;
};
