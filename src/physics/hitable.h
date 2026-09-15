// license:GPLv3+

#pragma once

// Part conttributing to collision detection for physics simulation and/or UI picking
class IHitable
{
public:
   virtual void PhysicSetup(class PhysicsEngine* physics, const bool isUI) = 0;
   virtual bool PhysicUpdate(class PhysicsEngine* physics, const bool isUI) { return false; } // Return false if update was not performed (object will be released and setup from scratch)
   virtual void PhysicRelease(class PhysicsEngine* physics, const bool isUI) = 0;

   virtual bool IsConstCollidable() const { return true; } // Flag if the collidable state reported by IsCollidable may change during simulation
   virtual bool IsCollidable() const { return true; } // Allow to early out when a quadtree section only contains a unique Hitable
};
