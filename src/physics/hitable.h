// license:GPLv3+

#pragma once

#include "collide.h"

// Part contributing to collision detection for physics simulation and/or UI picking.
//
// Each part owns the HitObject instances it contributes: they are created by PhysicSetup
// (through AddHitObject which fills the part's list), may be updated in place by PhysicUpdate,
// and are deleted by PhysicRelease (through ReleaseHitObjects). The contributed objects are
// exposed through GetHitObjects so that physics engines can build their spatial indexes on top
// of them (indexes only reference the hit objects, they do not own them). Parts contribute
// two independent sets: gameplay colliders (isUI = false) and UI picking ones (isUI = true).
class IHitable
{
public:
   virtual ~IHitable()
   {
      assert(m_hitObjects[0].empty() && m_hitObjects[1].empty()); // contributed hit objects must be released before destruction
   }

   virtual void PhysicSetup(class PhysicsEngine* physics, const bool isUI) = 0;
   virtual bool PhysicUpdate(class PhysicsEngine* physics, const bool isUI) { return false; } // Return false if update was not performed (object will be released and setup from scratch)
   virtual void PhysicRelease(class PhysicsEngine* physics, const bool isUI) = 0;

   // Hit objects contributed by this part for physics simulation (isUI = false) or UI picking (isUI = true)
   const vector<HitObject*>& GetHitObjects(const bool isUI) const { return m_hitObjects[isUI]; }

   virtual bool IsConstCollidable() const { return true; } // Flag if the collidable state reported by IsCollidable won't change during simulation
   virtual bool IsCollidable() const { return true; } // Allow to early out when a quadtree section only contains a unique Hitable

protected:
   // Registers a hit object contributed by this part, which takes ownership of it (deleted by ReleaseHitObjects)
   void AddHitObject(class PhysicsEngine* physics, HitObject* const hitObject, const bool isUI)
   {
      hitObject->SetPhysics(physics);
      hitObject->CalcHitBBox();
      m_hitObjects[isUI].push_back(hitObject);
   }

   // Registers a hit object contributed by this part but owned elsewhere (e.g. a member like Ball's HitBall)
   void TrackHitObject(HitObject* const hitObject, const bool isUI) { m_hitObjects[isUI].push_back(hitObject); }

   // Deletes all the hit objects contributed by this part through AddHitObject
   void ReleaseHitObjects(const bool isUI)
   {
      for (HitObject* const hitObject : m_hitObjects[isUI])
         delete hitObject;
      m_hitObjects[isUI].clear();
   }

   // Forgets the hit objects contributed by this part without deleting them (see TrackHitObject)
   void UntrackHitObjects(const bool isUI) { m_hitObjects[isUI].clear(); }

private:
   vector<HitObject*> m_hitObjects[2];
};
