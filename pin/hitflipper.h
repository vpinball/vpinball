#pragma once

//#define DEBUG_FLIPPERS

class FlipperMoverObject : public MoverObject
{
public:
   FlipperMoverObject(const Vertex2D& center, const float baser, const float endr, const float flipr, const float angleStart, float angleEnd,
      const float zlow, const float zhigh, Flipper* const pflipper);

   virtual void UpdateDisplacements(const float dtime);
   virtual void UpdateVelocities();

   virtual bool AddToList() const { return true; }

   void SetSolenoidState(const bool s);
   float GetStrokeRatio() const;

   void SetStartAngle(const float r);
   void SetEndAngle(const float r);
   float GetReturnRatio() const;
   float GetMass() const;
   void SetMass(const float m);
   float GetStrength() const;

   // rigid body functions
   Vertex3Ds SurfaceVelocity(const Vertex3Ds& surfP) const;
   Vertex3Ds SurfaceAcceleration(const Vertex3Ds& surfP) const;

   float GetHitTime() const;

   void ApplyImpulse(const Vertex3Ds& rotI);

   Flipper *m_pflipper;

   //float m_faceLength;
   HitCircle m_hitcircleBase;
   float m_endradius;
   float m_flipperradius;

   // kinematic state
   float m_angularMomentum;
   float m_angularAcceleration;
   float m_angleSpeed;
   float m_angleCur;

   float m_curTorque;
   float m_contactTorque;

   float m_angleStart, m_angleEnd;

   float m_inertia;	        // moment of inertia

   Vertex2D m_zeroAngNorm;  // base norms at zero degrees

   short m_EnableRotateEvent; // -1,0,1

   bool m_direction;

   bool m_solState;         // is solenoid enabled?
   bool m_isInContact;

   bool m_fEnabled;
   bool m_fVisible;
   bool m_lastHitFace;

#ifdef DEBUG_FLIPPERS
   U32 m_startTime;
#endif
};

class HitFlipper : public HitObject
{
public:
   HitFlipper(const Vertex2D& center, const float baser, const float endr, const float flipr, const float angleStart, const float angleEnd,
      const float zlow, const float zhigh, Flipper* const pflipper);
   ~HitFlipper() { /*m_pflipper->m_phitflipper = NULL;*/ }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) const;
   virtual int GetType() const { return eFlipper; }
   virtual void Collide(const CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime);
   virtual void CalcHitBBox();
   virtual MoverObject *GetMoverObject() { return &m_flipperMover; }

   float HitTestFlipperFace(const Ball * const pball, const float dtime, CollisionEvent& coll, const bool face1) const;
   float HitTestFlipperEnd(const Ball * const pball, const float dtime, CollisionEvent& coll) const;

   float GetHitTime() const { return m_flipperMover.GetHitTime(); }

   FlipperMoverObject m_flipperMover;
   U32 m_last_hittime;
};
