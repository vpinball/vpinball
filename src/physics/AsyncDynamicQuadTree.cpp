// license:GPLv3+

#include "core/stdafx.h"
#include "quadtree.h"
#include "AsyncDynamicQuadTree.h"
#include <algorithm>

AsyncDynamicQuadTree::AsyncDynamicQuadTree(PhysicsEngine* const physics, PinTable* const table, bool isUI)
   : m_physics(physics)
   , m_isUI(isUI)
   , m_quadTree(new HitQuadtree())
{
   vector<HitObject*>* hitObjects = &m_quadTree->BeginReset();
   for (IEditable* const pe : table->m_vedit)
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

   // Cleanup dynamic data
   m_dynamicEditables.clear();

   // Cleanup main quadtree
   vector<IEditable*> editables;
   for (HitObject* ho : m_quadTree->GetHitObjects())
   {
      if (ho != nullptr)
      {
         if (ho->m_editable->GetIHitable() && (std::ranges::find(editables, ho->m_editable) == editables.end()))
         {
            editables.push_back(ho->m_editable);
            ho->m_editable->GetIHitable()->PhysicRelease(m_physics, m_isUI);
         }
         if (ho->GetType() != eBall) // As balls own their HitBall hit object
            delete ho;
      }
   }
   delete m_quadTree;
}

vector<HitObject*> AsyncDynamicQuadTree::GetHitObjects(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);

   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   if (dynEdIt != m_dynamicEditables.end())
      return (*dynEdIt)->hitObjects;

   const auto updEdIt = std::ranges::find_if(m_updatedEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   if (updEdIt != m_updatedEditables.end())
      return (*updEdIt)->hitObjects;

   vector<HitObject*> result;
   const vector<HitObject*> &  vho = m_quadTree->GetHitObjects();
   std::ranges::copy_if(vho.begin(), vho.end(), std::back_inserter(result), [editable](HitObject* ho) { return (ho != nullptr) && (ho->m_editable == editable); });
   return result;
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
      // Remove from static quadtree
      editable->GetIHitable()->PhysicRelease(m_physics, m_isUI); // 'Release' it (this does not delete the editable's hit objects but allow the editable to adjust its internal state)
      if (std::ranges::find_if(m_updatedEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; }) != m_updatedEditables.end())
      {
         // The editable is being updated by the update thread for inclusion in the static data
         // Defer removing its hit objects from the static quadtree at the end of the async process
      }
      else if (m_quadTreeUpdateInProgress)
      {
         // We are not allowed to delete any hit object or modify nullSlots as they are shared with the async updater, so defer to end of update
         // Until then, objects will be hit tested against both its dynamic and static version
      }
      else
      {
         // The updated editable is part of the active quadtree and is not being updated, remove its hit objects from there
         vector<HitObject*>& vho = m_quadTree->BeginReset(); // FIXME somewhat hacky way to get a non const access to the ho vector
         for (size_t i = 0, n = vho.size(); i < n; i++)
         {
            if (vho[i] != nullptr && vho[i]->m_editable == editable)
            {
               delete vho[i];
               m_nullSlots.push_back(i);
               vho[i] = nullptr;
            }
         }
      }

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

void AsyncDynamicQuadTree::Remove(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);
   assert(editable->GetItemType() != eItemBall); // Balls are not supported as they manage the hit object lifecycle

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
   assert(editable->GetItemType() != eItemBall); // Balls are not supported as they manage the hit object lifecycle
   //PLOGD << "Updating item " << editable->GetName();

   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::unique_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   assert(dynEdIt != m_dynamicEditables.end() && !(*dynEdIt)->pendingStaticInclusion); // We do not support updating static parts
   if (!editable->GetIHitable()->PhysicUpdate(m_physics, m_isUI))
      // update was not performed: release and reallocate colliders
      *dynEdIt = std::make_unique<DynamicEditable>(editable, m_physics, m_isUI);
}

void AsyncDynamicQuadTree::UpdateAsync()
{
   if (m_quadtreeUpdateReady.try_acquire()) // Update ready ?
   {
      m_quadTreeUpdateInProgress = false;
      // Swap quad trees (to limit memory reallocations)
      HitQuadtree* tmp = m_quadTree;
      m_quadTree = m_pendingQuadTree;
      m_pendingQuadTree = tmp;

      // Remove from static quadtree editables that have been made dynamic during the update
      vector<HitObject*>& vho = m_quadTree->BeginReset(); // FIXME somewhat hacky way to get a non const access to the ho vector
      for (const auto& dynEd : m_dynamicEditables)
      {
         for (size_t i = 0, n = vho.size(); i < n; i++)
         {
            if ((vho[i] != nullptr) && (vho[i]->m_editable == dynEd->editable))
            {
               delete vho[i];
               m_nullSlots.push_back(i);
               vho[i] = nullptr;
            }
         }
      }
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
            m_pendingQuadTree = new HitQuadtree();
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

   while (true)
   {
      m_quadtreeUpdateWaiting.acquire();
      if (!m_quadTreeUpdateInProgress)
         return;

      /* static int nCall = 0;
      static std::chrono::duration<double> total;
      nCall++;
      const auto start = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed;*/

      // Move pending hit objects into the static quadtree, reusing free slots if possible
      vector<size_t>::iterator nullSlotsIt = m_nullSlots.begin(), nullSlotsEnd = m_nullSlots.end();
      for (const auto& updEd : m_updatedEditables)
      {
         vector<HitObject*>::iterator hitObjectIt = updEd->hitObjects.begin();
         for (; (hitObjectIt < updEd->hitObjects.end()) && (nullSlotsIt < nullSlotsEnd); ++hitObjectIt, ++nullSlotsIt)
            (*m_quadTreeHitobjects)[*nullSlotsIt] = *hitObjectIt;
         if (hitObjectIt < updEd->hitObjects.end())
            m_quadTreeHitobjects->insert(m_quadTreeHitobjects->end(), hitObjectIt, updEd->hitObjects.end());
         updEd->hitObjects.clear();
         updEd->editable = nullptr;
      }
      m_updatedEditables.clear();

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

      /* elapsed = std::chrono::high_resolution_clock::now() - start; total += elapsed;
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
   for (const HitObject* const pho : hitObjects)
   {
      #ifdef DEBUGPHYSICS
         g_pplayer->m_physics->c_tested++;
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
   for (HitObject* const pho : hitObjects)
   {
      #ifdef DEBUGPHYSICS
         g_pplayer->m_physics->c_tested++;
      #endif
      if ((pball != pho) // ball can not hit itself
         && fRectIntersect3D(pball->m_hitBBox, pho->m_hitBBox)
         && fRectIntersect3D(pball->m_d.m_pos, rcHitRadiusSqr, pho->m_hitBBox))
      {
         #ifdef DEBUGPHYSICS
            g_pplayer->m_physics->c_deepTested++;
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
   physics->CollectColliders(editable, &hitObjects, isUI);
}

AsyncDynamicQuadTree::DynamicEditable::~DynamicEditable()
{
   if (editable)
      editable->GetIHitable()->PhysicRelease(m_physics, m_isUI);
   for (HitObject* ho : hitObjects)
      delete ho;
   hitObjects.clear();
}
