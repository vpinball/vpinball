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
   for (const auto& dynEd : m_dynamicEditables)
   {
      dynEd->editable->GetIHitable()->PhysicRelease(m_physics, m_isUI);
      std::ranges::for_each(dynEd->hitObjects.begin(), dynEd->hitObjects.end(), [](HitObject* ho) { delete ho; });
   }

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

   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::shared_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   if (dynEdIt != m_dynamicEditables.end())
      return (*dynEdIt)->hitObjects;

   const auto updEdIt = std::ranges::find_if(m_updatedEditables, [editable](const std::shared_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
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
   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::shared_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   return (dynEdIt == m_dynamicEditables.end()) || (*dynEdIt)->pendingStaticInclusion;
}

void AsyncDynamicQuadTree::SetDynamic(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);
   assert(editable->GetItemType() != eItemBall); // Balls are not supported as they manage the hit object lifecycle
   assert(IsStatic(editable));
   //PLOGD << "Setting item " << editable->GetName() << " as dynamic.";

   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::shared_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   if (dynEdIt != m_dynamicEditables.end())
   {
      (*dynEdIt)->pendingStaticInclusion = false;
   }
   else
   {
      // Move part from static quadtree to dynamic
      // Instead of PhysicRelease/PhysicSetup, we could move the hitobjects from the place we remove it (quadtree or update thread) but the benefit seems slight as the aim of making the part dynamic is to update it
      // 'Release' it (this does not delete the editable's hit objects but allow the editable to adjust its internal state)
      editable->GetIHitable()->PhysicRelease(m_physics, m_isUI);
      if (std::ranges::find_if(m_updatedEditables, [editable](std::shared_ptr<DynamicEditable> dynEd) { return dynEd->editable == editable; }) != m_updatedEditables.end())
      {
         // The updated editable is being updated by the update thread, and not yet part of the dynamic data
         // Don't delete its hit objects as they are in use by the update thread (they will be deleted after the update process)
         // Don't remove it from active quadtree as it has already been removed
         assert(m_quadTreeUpdateInProgress);
         auto dynEd = std::make_shared<DynamicEditable>();
         dynEd->editable = editable;
         dynEd->pendingStaticInclusion = false;
         m_physics->CollectColliders(editable, &(dynEd)->hitObjects, m_isUI);
         m_dynamicEditables.push_back(dynEd);
      }
      else
      {
         // The updated editable is part of the active quadtree and is not being updated, remove from there and add it to the dynamic objects
         auto dynEd = std::make_shared<DynamicEditable>();
         dynEd->editable = editable;
         dynEd->pendingStaticInclusion = false;
         m_physics->CollectColliders(editable, &(dynEd)->hitObjects, m_isUI);
         m_dynamicEditables.push_back(dynEd);
         vector<HitObject*>& vho = m_quadTree->BeginReset(); // FIXME somewhat hacky way to get a non const access to the ho vector
         for (size_t i = 0, n = vho.size(); i < n; i++)
         {
            if (vho[i] != nullptr && vho[i]->m_editable == editable)
            {
               if (m_quadTreeUpdateInProgress) // We are not allowed to delete any hit object or modify nullSlots as they are shared with the async updater, so defer to end of update
                  m_deferredDeleteHitObjects.push_back(vho[i]);
               else
               {
                  delete vho[i];
                  m_nullSlots.push_back(i);
               }
               vho[i] = nullptr;
            }
         }
      }
   }
}

void AsyncDynamicQuadTree::SetStatic(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);
   assert(editable->GetItemType() != eItemBall); // Balls are not supported as they manage the hit object lifecycle
   assert(!IsStatic(editable));
   //PLOGD << "Setting item " << editable->GetName() << " as static.";

   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::shared_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   (*dynEdIt)->pendingStaticInclusion = true;
   UpdateAsync();
}

void AsyncDynamicQuadTree::Update(IEditable* editable)
{
   assert(editable->GetIHitable() != nullptr);
   assert(editable->GetItemType() != eItemBall); // Balls are not supported as they manage the hit object lifecycle
   //PLOGD << "Updating item " << editable->GetName();

   const auto dynEdIt = std::ranges::find_if(m_dynamicEditables, [editable](const std::shared_ptr<DynamicEditable>& dynEd) { return dynEd->editable == editable; });
   if (dynEdIt != m_dynamicEditables.end())
   {
      // The updated editable is part of the dynamic data (eventually waiting for inclusion in the static quadtree), update it
      // 'Release' it (this does not delete the editable's hit objects but allow the editable to adjust its internal state)
      editable->GetIHitable()->PhysicRelease(m_physics, m_isUI);
      std::ranges::for_each((*dynEdIt)->hitObjects.begin(), (*dynEdIt)->hitObjects.end(), [](HitObject* ho) { delete ho; });
      (*dynEdIt)->hitObjects.clear();
      m_physics->CollectColliders(editable, &(*dynEdIt)->hitObjects, m_isUI);
      if ((*dynEdIt)->pendingStaticInclusion)
         UpdateAsync();
   }
   else
   {
      // This is a static object, toggle its state to force an update
      SetDynamic(editable);
      SetStatic(editable);
   }
}

void AsyncDynamicQuadTree::UpdateAsync()
{
   if (m_quadtreeUpdateReady.try_acquire())
   {
      m_quadTreeUpdateInProgress = false;
      // Swap quad trees (to limit memory reallocations)
      HitQuadtree* tmp = m_quadTree;
      m_quadTree = m_pendingQuadTree;
      m_pendingQuadTree = tmp;
      // We need to nullify hit objects that have been updated since we started the async update and are now part of the dynamic data
      // as their hit objects are no more valid and part of the quadtree. 
      // We also must delete the hit objects if they were part of the async update.
      m_nullSlots.clear();
      vector<HitObject*>& vho = m_quadTree->BeginReset(); // FIXME somewhat hacky way to get a non const access to the ho vector
      for (const auto& dynEd : m_dynamicEditables)
      {
         for (size_t i = 0, n = vho.size(); i < n; i++)
         {
            if ((vho[i] != nullptr) && (vho[i]->m_editable == dynEd->editable))
            {
               vho[i] = nullptr;
               m_nullSlots.push_back(i);
            }
         }
         const auto updEdIt = std::ranges::find_if(m_updatedEditables, [dynEd](const std::shared_ptr<DynamicEditable>& updEd) { return dynEd->editable == updEd->editable; });
         if (updEdIt != m_updatedEditables.end())
            std::ranges::for_each((*updEdIt)->hitObjects.begin(), (*updEdIt)->hitObjects.end(), [](HitObject* ho) { delete ho; });
      }
      std::ranges::for_each(m_deferredDeleteHitObjects.begin(), m_deferredDeleteHitObjects.end(), [](HitObject* ho) { delete ho; });
      m_deferredDeleteHitObjects.clear();
      m_updatedEditables.clear();
   }

   if (!m_quadTreeUpdateInProgress && !m_dynamicEditables.empty())
   {
      auto it = std::partition(m_dynamicEditables.begin(), m_dynamicEditables.end(), [](const std::shared_ptr<DynamicEditable>& dynEd) { return !dynEd->pendingStaticInclusion; });
      if (it != m_dynamicEditables.end())
      {
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
            m_quadtreeUpdateThread = std::thread([&] { UpdateQuadtreeThread(); });
      }
   }
}

void AsyncDynamicQuadTree::UpdateQuadtreeThread()
{
   while (true)
   {
      m_quadtreeUpdateWaiting.acquire();
      if (!m_quadTreeUpdateInProgress)
         return;

      /* static int nCall = 0;
      static std::chrono::duration<double> total;
      nCall++;
      auto start = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed;*/

      // Fill in with pending hit objects, reusing free slots if possible
      vector<size_t>::iterator nullSlotsIt = m_nullSlots.begin(), nullSlotsEnd = m_nullSlots.end();
      for (const auto& updEd : m_updatedEditables)
      {
         vector<HitObject*>::iterator hitObjectIt = updEd->hitObjects.begin();
         for (; (hitObjectIt < updEd->hitObjects.end()) && (nullSlotsIt < nullSlotsEnd); ++hitObjectIt, ++nullSlotsIt)
            (*m_quadTreeHitobjects)[*nullSlotsIt] = *hitObjectIt;
         if (hitObjectIt < updEd->hitObjects.end())
            m_quadTreeHitobjects->insert(m_quadTreeHitobjects->end(), hitObjectIt, updEd->hitObjects.end());
      }

      // Remove any remaining null slot if any
      if (nullSlotsIt < nullSlotsEnd)
      {
         // TODO this is very inefficient, we could do a bunch of memcpy since we have the index of the null slots
         // On the other hand, I never encountered a situation where this path would be triggered as the number of hit objects remains constant (we don't add/remove editables for the time being)
         m_quadTreeHitobjects->erase(std::remove(m_quadTreeHitobjects->begin(), m_quadTreeHitobjects->end(), nullptr), m_quadTreeHitobjects->end());
      }

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
      if (std::ranges::find_if(m_dynamicEditables, [updEd](const std::shared_ptr<DynamicEditable>& dynEd) { return dynEd->editable == updEd->editable; }) == m_dynamicEditables.end())
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
      if (std::ranges::find_if(m_dynamicEditables, [updEd](const std::shared_ptr<DynamicEditable>& dynEd) { return dynEd->editable == updEd->editable; }) == m_dynamicEditables.end())
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
            pvhoHit.push_back({ pho, newtime });
      }
   }
}
