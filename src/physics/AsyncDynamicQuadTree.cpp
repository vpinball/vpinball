// license:GPLv3+

#include "core/stdafx.h"
#include "AsyncDynamicQuadTree.h"

#include "parts/pintable.h"
#include "physics/quadtree.h"
#include "physics/hitable.h"
#include "physics/PhysicsEngine.h"
#include "utils/denormals.h"

#include <algorithm>


AsyncDynamicQuadTree::AsyncDynamicQuadTree(PhysicsEngine* const physics, PinTable* const table, bool isUI)
   : m_physics(physics)
   , m_isUI(isUI)
   , m_quadTree(new HitQuadtree(physics))
{
   vector<HitObject*>* hitObjects = &m_quadTree->BeginReset();
   for (IEditable* const pe : table->GetParts())
      physics->CollectColliders(pe, hitObjects, m_isUI);
   m_quadTree->EndReset();
}

AsyncDynamicQuadTree::~AsyncDynamicQuadTree()
{
   // Purge any pending update
   while (m_quadTreeUpdateInProgress)
   {
      m_quadtreeUpdateReady.acquire();
      m_quadtreeUpdateReady.release();
      UpdateAsync();
   }

   // Stop update thread & clean update quadtree
   m_quadTreeUpdateInProgress = false;
   m_quadtreeUpdateWaiting.release();
   if (m_quadtreeUpdateThread.joinable())
      m_quadtreeUpdateThread.join();
   delete m_pendingQuadTree;

   // Cleanup dynamic data (releases the dynamic hit objects contributed by each part)
   m_dynamicEditables.clear();

   // Release the hit objects contributed by each part referenced by the main quadtree
   // (collect them first as PhysicRelease deletes the hit objects)
   vector<IEditable*> editables;
   for (HitObject* ho : m_quadTree->GetHitObjects())
      if (ho != nullptr && ho->m_editable->GetIHitable() && (std::ranges::find(editables, ho->m_editable) == editables.end()))
         editables.push_back(ho->m_editable);
   for (IEditable* const editable : editables)
      editable->GetIHitable()->PhysicRelease(m_physics, m_isUI);
   delete m_quadTree;
}

const vector<HitObject*>& AsyncDynamicQuadTree::GetHitObjects(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);
   // The part owns the hit objects it contributes, whichever quadtree currently references them
   return editable->GetIHitable()->GetHitObjects(m_isUI);
}

bool AsyncDynamicQuadTree::IsStatic(IEditable* editable) const
{
   assert(editable->GetIHitable() != nullptr);
   assert(editable->GetItemType() != eItemBall); // Balls are not supported as they manage the hit object lifecycle
   // Either part is not in the list of dynamic parts, or it is part but pending for inclusion in the static quadtree
   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   return (dynEdIt == m_dynamicEditables.end()) || (*dynEdIt)->pendingStaticInclusion;
}

void AsyncDynamicQuadTree::SetDynamic(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);
   assert(editable->GetItemType() != eItemBall); // Balls are not supported as they manage the hit object lifecycle
   assert(IsStatic(editable));
   //PLOGD << "Setting item " << editable->GetName() << " as dynamic.";

   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   if (dynEdIt != m_dynamicEditables.end())
   {
      (*dynEdIt)->pendingStaticInclusion = false;
   }
   else
   {
      // Purge any pending update first as it may be processing this part's hit objects (PhysicRelease
      // below deletes them, so they must not be referenced by the quadtree being asynchronously rebuilt)
      while (m_quadTreeUpdateInProgress)
      {
         m_quadtreeUpdateReady.acquire();
         m_quadtreeUpdateReady.release();
         UpdateAsync();
      }

      // Remove the part's hit objects from the static quadtree (they stay valid until PhysicRelease deletes them)
      vector<HitObject*>& vho = m_quadTree->BeginReset(); // FIXME somewhat hacky way to get a non const access to the ho vector
      for (size_t i = 0, n = vho.size(); i < n; i++)
      {
         if (vho[i] != nullptr && vho[i]->m_editable == editable)
         {
            m_nullSlots.push_back(i);
            vho[i] = nullptr;
         }
      }
      editable->GetIHitable()->PhysicRelease(m_physics, m_isUI); // Deletes the editable's static hit objects

      // Add to dynamic part list
      m_dynamicEditables.emplace_back(std::make_unique<DynamicEditable>(editable, m_physics, m_isUI));
   }
}

void AsyncDynamicQuadTree::SetStatic(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);
   assert(editable->GetItemType() != eItemBall); // Balls are not supported as they manage the hit object lifecycle
   assert(!IsStatic(editable));
   //PLOGD << "Setting item " << editable->GetName() << " as static.";

   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   (*dynEdIt)->pendingStaticInclusion = true;
   UpdateAsync();
}

void AsyncDynamicQuadTree::AddEditable(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);

   // Purge any pending update that may modify the static quadtree we are going to update
   while (m_quadTreeUpdateInProgress)
   {
      m_quadtreeUpdateReady.acquire();
      m_quadtreeUpdateReady.release();
      UpdateAsync();
   }

   // Collect the hit objects of the new editable and add them to the static quadtree
   vector<HitObject*> hitObjects;
   m_physics->CollectColliders(editable, &hitObjects, m_isUI);
   if (hitObjects.empty())
      return;
   vector<HitObject*>& vho = m_quadTree->BeginReset();
   std::erase(vho, nullptr); // Compact away the slots nulled for dynamic parts (their recorded indices would be stale after insertion anyway)
   m_nullSlots.clear();
   vho.insert(vho.end(), hitObjects.begin(), hitObjects.end());
   m_quadTree->EndReset();
}

void AsyncDynamicQuadTree::Remove(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);

   if (editable->GetItemType() == eItemBall)
   {
      // Balls are not supported as dynamic editables as they manage their hit object lifecycle.
      // Their HitBall is always part of the static quadtree, remove it without deleting it (owned by the ball)
      while (m_quadTreeUpdateInProgress)
      {
         m_quadtreeUpdateReady.acquire();
         m_quadtreeUpdateReady.release();
         UpdateAsync();
      }
      vector<HitObject*>& vho = m_quadTree->BeginReset();
      std::erase_if(vho, [editable](HitObject* ho) { return (ho != nullptr) && (ho->m_editable == editable); });
      std::erase(vho, nullptr); // Compact away the slots nulled for dynamic parts (their recorded indices would be stale after removal anyway)
      m_nullSlots.clear();
      m_quadTree->EndReset();
      editable->GetIHitable()->PhysicRelease(m_physics, m_isUI); // Releases the contribution (the HitBall is member owned, not deleted)
      return;
   }

   // Remove from static quadtree
   if (IsStatic(editable))
      SetDynamic(editable);

   // Purge any pending update that may use the data
   while (m_quadTreeUpdateInProgress)
   {
      m_quadtreeUpdateReady.acquire();
      m_quadtreeUpdateReady.release();
      UpdateAsync();
   }
   assert(std::ranges::find_if(m_updatedEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; }) == m_updatedEditables.end());

   // Remove
   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   assert(dynEdIt != m_dynamicEditables.end());
   m_dynamicEditables.erase(dynEdIt);
}

void AsyncDynamicQuadTree::Update(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);
   //PLOGD << "Updating item " << editable->GetName();

   if (editable->GetItemType() == eItemBall)
   {
      // Balls are always part of the quadtree as they own their (shared) HitBall: simply update its bounds
      for (HitObject* const ho : GetHitObjects(editable))
         ho->CalcHitBBox();
      return;
   }

   if (IsStatic(editable))
   {
      // Purge any pending update first as it may be processing this part's hit objects.
      while (m_quadTreeUpdateInProgress)
      {
         m_quadtreeUpdateReady.acquire();
         m_quadtreeUpdateReady.release();
         UpdateAsync();
      }
      SetDynamic(editable);
      SetStatic(editable);
      return;
   }

   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   assert(dynEdIt != m_dynamicEditables.end() && !(*dynEdIt)->pendingStaticInclusion);
   if (!editable->GetIHitable()->PhysicUpdate(m_physics, m_isUI))
   {
      // update was not performed: release and reallocate colliders
      *dynEdIt = nullptr; 
      *dynEdIt = std::make_unique<DynamicEditable>(editable, m_physics, m_isUI);
   }
}

void AsyncDynamicQuadTree::UpdateAsync()
{
   if (m_quadtreeUpdateReady.try_acquire()) // Update ready ?
   {
      // The hit objects merged into the new quadtree stay owned by their parts: detach the editables so
      // ~DynamicEditable does not release them (they will be released when the part is made dynamic again or removed)
      for (const auto& updEd : m_updatedEditables)
         updEd->editable = nullptr;
      m_updatedEditables.clear();

      m_quadTreeUpdateInProgress = false;
      // Swap quad trees (to limit memory reallocations)
      HitQuadtree* tmp = m_quadTree;
      m_quadTree = m_pendingQuadTree;
      m_pendingQuadTree = tmp;
   }

   if (!m_quadTreeUpdateInProgress && !m_dynamicEditables.empty())
   {
      auto it = std::partition(m_dynamicEditables.begin(), m_dynamicEditables.end(), [](const std::unique_ptr<DynamicEditable>& dynEd) { return !dynEd->pendingStaticInclusion; });
      if (it != m_dynamicEditables.end())
      {
         assert(m_updatedEditables.empty());
         m_updatedEditables.insert(m_updatedEditables.end(), std::make_move_iterator(it), std::make_move_iterator(m_dynamicEditables.end()));
         m_dynamicEditables.erase(it, m_dynamicEditables.end());
         m_quadTreeUpdateInProgress = true;
         // Initialize the list of hit objects to update as this may be modified later on
         // > but only to nullify a cell, so maybe we could move this (lengthy) copy to the update thread
         // > we could also avoid the copy by keeping track of the update before this one and reusing the array from m_pendingQuadTree
         if (m_pendingQuadTree == nullptr)
            m_pendingQuadTree = new HitQuadtree(m_physics);
         m_quadTreeHitobjects = &m_pendingQuadTree->BeginReset();
         *m_quadTreeHitobjects = m_quadTree->GetHitObjects();

         m_quadtreeUpdateWaiting.release();
         if (!m_quadtreeUpdateThread.joinable())
            m_quadtreeUpdateThread = std::thread([this] { UpdateQuadtreeThread(); });
      }
   }
}

void AsyncDynamicQuadTree::UpdateQuadtreeThread()
{
   SetThreadName("VPX.QuadTree.UpdateThread"s);
   set_denormals_flush_to_zero(); // FPU mode is per thread

   while (true)
   {
      m_quadtreeUpdateWaiting.acquire();
      if (!m_quadTreeUpdateInProgress)
         return;

      /* static int nCall = 0;
      static std::chrono::duration<double> total;
      nCall++;
      const auto start = std::chrono::steady_clock::now();
      std::chrono::duration<double> elapsed;*/

      // Move pending hit objects into the static quadtree, reusing free slots if possible.
      // m_updatedEditables is read-only here: the main thread still hit tests against it and releases it in UpdateAsync
      vector<size_t>::iterator nullSlotsIt = m_nullSlots.begin(), nullSlotsEnd = m_nullSlots.end();
      for (const auto& updEd : m_updatedEditables)
      {
         // The part's hit objects may not be modified while being merged: all the paths mutating them
         // (SetDynamic, Update, Remove, PhysicUpdate) first wait for the update in progress to complete
         const vector<HitObject*>& hitObjects = updEd->editable->GetIHitable()->GetHitObjects(m_isUI);
         vector<HitObject*>::const_iterator hitObjectIt = hitObjects.cbegin();
         for (; (hitObjectIt < hitObjects.cend()) && (nullSlotsIt < nullSlotsEnd); ++hitObjectIt, ++nullSlotsIt)
            (*m_quadTreeHitobjects)[*nullSlotsIt] = *hitObjectIt;
         if (hitObjectIt < hitObjects.cend())
            m_quadTreeHitobjects->insert(m_quadTreeHitobjects->end(), hitObjectIt, hitObjects.cend());
      }

      // Remove any remaining null slot if any
      if (nullSlotsIt < nullSlotsEnd)
      {
         // TODO this is very inefficient, we could do a bunch of memcpy since we have the index of the null slots
         // On the other hand, I never encountered a situation where this path would be triggered as the number of hit objects remains constant (we don't add/remove editables for the time being)
         m_quadTreeHitobjects->erase(std::remove(m_quadTreeHitobjects->begin(), m_quadTreeHitobjects->end(), nullptr), m_quadTreeHitobjects->end());
      }
      m_nullSlots.clear();

      // Reset quadtree with the updated hit object list
      m_pendingQuadTree->EndReset();

      /* elapsed = std::chrono::steady_clock::now() - start; total += elapsed;
      PLOGD << "UI quadtree update: " << (total / nCall) << " (" << m_pendingUIOctree->GetHitObjects().size() << " objects)";*/

      m_quadtreeUpdateReady.release();
   }
}

void AsyncDynamicQuadTree::HitTestBall(const HitBall* const pball, CollisionEvent& coll)
{
   UpdateAsync();

   // Hit test against static object
   m_quadTree->HitTestBall(pball, coll);

   // Hit test against dynamic object
   for (const auto& dynEd : m_dynamicEditables)
      dynEd->HitTestBall(pball, coll);

   // Hit test against dynamic object that are being pushed to the quad tree, but only if they have not been replaced by dynamic data
   for (const auto& updEd : m_updatedEditables)
      if (std::ranges::find_if(m_dynamicEditables, [&updEd](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == updEd->editable; }) == m_dynamicEditables.end())
         updEd->HitTestBall(pball, coll);
}

void AsyncDynamicQuadTree::HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll)
{
   UpdateAsync();

   // Hit test against static object
   m_quadTree->HitTestXRay(pball, pvhoHit, coll);

   // Hit test against dynamic object
   for (const auto& dynEd : m_dynamicEditables)
      dynEd->HitTestXRay(pball, pvhoHit, coll);

   // Hit test against dynamic object that are being pushed to the quad tree, but only if they have not been replaced by dynamic data
   for (const auto& updEd : m_updatedEditables)
      if (std::ranges::find_if(m_dynamicEditables, [&updEd](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == updEd->editable; }) == m_dynamicEditables.end())
         updEd->HitTestXRay(pball, pvhoHit, coll);
}

void AsyncDynamicQuadTree::DynamicEditable::HitTestBall(const HitBall* const pball, CollisionEvent& coll) const
{
   // TODO add some basic performance improvements here (at least just a hit against bounding box...)
   const float rcHitRadiusSqr = pball->HitRadiusSqr();
   for (const HitObject* const pho : editable->GetIHitable()->GetHitObjects(m_isUI))
   {
#ifdef DEBUGPHYSICS
      m_physics->c_tested++;
#endif
      if ((pball != pho) // ball can not hit itself
         && fRectIntersect3D(pball->m_hitBBox, pho->m_hitBBox)
         && fRectIntersect3D(pball->m_d.m_pos, rcHitRadiusSqr, pho->m_hitBBox))
      {
         DoHitTest(pball, pho, coll);
      }
   }
}

void AsyncDynamicQuadTree::DynamicEditable::HitTestXRay(const HitBall* const pball, vector<HitTestResult>& pvhoHit, CollisionEvent& coll) const
{
   // TODO add some basic performance improvements here (at least just a hit against bounding box...)
   const float rcHitRadiusSqr = pball->HitRadiusSqr();
   for (HitObject* const pho : editable->GetIHitable()->GetHitObjects(m_isUI))
   {
#ifdef DEBUGPHYSICS
      m_physics->c_tested++;
#endif
      if ((pball != pho) // ball can not hit itself
         && fRectIntersect3D(pball->m_hitBBox, pho->m_hitBBox)
         && fRectIntersect3D(pball->m_d.m_pos, rcHitRadiusSqr, pho->m_hitBBox))
      {
#ifdef DEBUGPHYSICS
         m_physics->c_deepTested++;
#endif
         const float newtime = pho->HitTest(pball->m_d, coll.m_hittime, coll);
         if (newtime >= 0.f)
            pvhoHit.push_back({pho, newtime});
      }
   }
}


AsyncDynamicQuadTree::DynamicEditable::DynamicEditable(IEditable* edit, PhysicsEngine* physics, bool isUI)
   : editable(edit) 
   , m_physics(physics)
   , m_isUI(isUI)
{
   editable->GetIHitable()->PhysicSetup(physics, isUI);
}

AsyncDynamicQuadTree::DynamicEditable::~DynamicEditable()
{
   if (editable)
      editable->GetIHitable()->PhysicRelease(m_physics, m_isUI);
}
