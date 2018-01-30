#pragma once

//#define DEBUG_FLIPPERS

class FlipperMoverObject : public MoverObject
{
public:
   FlipperMoverObject(const Vertex2D& center, float baser, float endr, float flipr, float angleStart, float angleEnd,
      float zlow, float zhigh, float strength, float mass, float returnRatio);

   virtual void UpdateDisplacements(const float dtime);
   virtual void UpdateVelocities();

   virtual bool AddToList() const { return true; }

   void SetSolenoidState(const bool s);
   float GetStrokeRatio() const;

   void SetStartAngle(const float r);
   void SetEndAngle(const float r);
   float GetReturnRatio();
   void SetReturnRatio(const float r);
   float GetMass();
   void SetMass(const float m);
   float GetStrength();
   void SetStrength(const float s);

   // rigid body functions
   Vertex3Ds SurfaceVelocity(const Vertex3Ds& surfP) const;
   Vertex3Ds SurfaceAcceleration(const Vertex3Ds& surfP) const;

   float GetHitTime() const;

   void ApplyImpulse(const Vertex3Ds& rotI);

   Flipper *m_pflipper;

   float m_faceLength;
   HitCircle m_hitcircleBase;
   float m_endradius;

   // kinematic state
   float m_angularMomentum;
   float m_angularAcceleration;
   float m_anglespeed;
   float m_angleCur;

   float m_flipperradius;
   float m_force;
   float m_returnRatio;

   float m_height;

   int m_dir;
   float m_curTorque;
   float m_contactTorque;
   float m_torqueRampupSpeed;

   float m_angleStart, m_angleEnd;
   float m_angleMin, m_angleMax;

   float m_inertia;	//moment of inertia

   float m_torqueDamping;
   float m_torqueDampingAngle;

   int m_EnableRotateEvent;

   Vertex2D zeroAngNorm; // base norms at zero degrees	

   bool m_solState;        // is solenoid enabled?
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
   HitFlipper(const Vertex2D& center, float baser, float endr, float flipr, float angleStart, float angleEnd,
      float zlow, float zhigh, float strength, float mass, float returnRatio);
   ~HitFlipper();

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) const;
   virtual int GetType() const { return eFlipper; }
   virtual void Collide(CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime);
   virtual void CalcHitBBox();
   virtual MoverObject *GetMoverObject() { return &m_flipperMover; }

   float HitTestFlipperFace(const Ball * const pball, const float dtime, CollisionEvent& coll, const bool face1) const;
   float HitTestFlipperEnd(const Ball * const pball, const float dtime, CollisionEvent& coll) const;

   float GetHitTime() const { return m_flipperMover.GetHitTime(); }

   Vertex2D v;

   Flipper *m_pflipper;

   FlipperMoverObject m_flipperMover;
   U32 m_last_hittime;
};
