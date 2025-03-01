// license:GPLv3+

#pragma once

#include "collide.h"
#include <semaphore>

// QuadTree for mostly static hit objects
//
// This is a QuadTree which can hit test large number of static hit objects efficiently. It also supports
// dynamic parts by moving them out of the quadtree when they are switched to dynamic. When they are 
// switched back to static, they are re-integrated into the QuadTree but, since this is a lengthy operation,
// the QuadTree is recomputed asynchronously, keeping the part dynamic until the update is processed.
//
// The dynamic parts have a big impact on hit testing performace since they are brute force hit tested
// without any advanced space partioning trick. Therefore, the number of dynamic parts should be kept low
// and/or restricted to parts with a low count of hit objects.
//
// Note that updating the quadtree is a fairly heavy multithreaded operation, so it should be only performed
// when the editable won't be moved again.

class PhysicsEngine;

class AsyncDynamicQuadTree final
{
public:
   AsyncDynamicQuadTree(PhysicsEngine* physics, PinTable* table, bool isUI);
   ~AsyncDynamicQuadTree();

   bool IsStatic(IEditable* editable) const;
   void SetDynamic(IEditable* editable);
   void Update(IEditable* editable);
   void SetStatic(IEditable* editable);

   void HitTestBall(const HitBall* const pball, CollisionEvent& coll);
   void HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll);

   HitQuadtree* GetQuadTree() const { return m_quadTree; }
   const vector<HitObject*> GetHitObjects(IEditable* editable);

private:
   void UpdateAsync();

   // Main optimized quadtree, updated asynchronously (note that it may contain null slots for dynamic hit objects)
   PhysicsEngine* const m_physics;
   const bool m_isUI;
   HitQuadtree* m_quadTree;

   // Dynamic parts datas
   struct DynamicEditable
   {
      bool pendingStaticInclusion = false;
      IEditable* editable = nullptr;
      vector<HitObject*> hitObjects;
      void HitTestBall(const HitBall* const pball, CollisionEvent& coll);
      void HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll);
   };
   vector<std::shared_ptr<DynamicEditable>> m_dynamicEditables;
   vector<size_t> m_nullSlots; // null slots in static QuadTree (created when removing the parts that have been switched to dynamic)

   // Asynchronous thread that update the static quadtree with parts switched back to static
   bool m_quadTreeUpdateInProgress = false;
   HitQuadtree* m_pendingQuadTree = nullptr; // Result of async update
   vector<std::shared_ptr<DynamicEditable>> m_updatedEditables; // Objects that are being processed or have been updated by the update thread
   vector<HitObject*>* m_quadTreeHitobjects = nullptr; // List of objects of the updated quadtree
   vector<HitObject*> m_deferredDeleteHitObjects; // HitObjects that have been recreated (their editable was updated) but not deleted as they were part of an in progress update
   std::binary_semaphore m_quadtreeUpdateWaiting { 0 };
   std::binary_semaphore m_quadtreeUpdateReady { 0 };
   std::thread m_quadtreeUpdateThread;
   void UpdateQuadtreeThread();
};
