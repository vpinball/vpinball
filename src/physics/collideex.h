// license:GPLv3+

#pragma once

#include "math/matrix.h"
#include "physics/collide.h"

class Surface;

class Bumper;
class Spinner;
class Gate;
class Trigger;

class BumperHitCircle final : public HitCircle
{
public:
   BumperHitCircle(Bumper* const pBumper, const Vertex2D& c, const float r, const float zlow, const float zhigh);
   void Collide(const CollisionEvent& coll) override;

   Vertex3Ds m_bumperanim_hitBallPosition;
   float m_bumperanim_ringAnimOffset;
   bool m_bumperanim_hitEvent;
};

class LineSegSlingshot final : public LineSeg
{
public:
   LineSegSlingshot(Surface* const psurface, const Vertex2D& p1, const Vertex2D& p2, const float _zlow, const float _zhigh);
   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eLineSegSlingshot; }
   void Collide(const CollisionEvent& coll) override;

   Surface * const m_psurface;

   float m_force;
   unsigned int m_EventTimeReset;

   bool m_doHitEvent;

   void Animate();

   uint32_t m_TimeReset; // Time at which to pull in slingshot, Zero means the slingshot is currently reset
   bool m_animations;
   bool m_iframe;
   LineSegSlingshot *m_animatedSegment = nullptr; // Segment to animate on hits instead of this one (i.e. reversed outlines, see Surface::AddLine)
};

class Hit3DPoly final : public HitObject
{
public:
   Hit3DPoly(IEditable*const editable, Vertex3Ds * const rgv, const int count); // pointer is copied and content deleted in dtor
   Hit3DPoly(IEditable* const editable, const float x, const float y, const float z, const float r, const int sections); // creates a circular hit poly
   ~Hit3DPoly() override;

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return e3DPoly; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override;

   void Init(Vertex3Ds * const rgv, const int count);

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override;

private:
   Vertex3Ds *m_rgv;
   Vertex3Ds m_normal;
   int m_cvertex;
   mutable vector<WORD> m_triIndices; // Lazy cache for UI rendering
};

// Note that HitTriangle ONLY does include the plane and barycentric test, but NOT the edge and vertex test,
// thus one has to add these separately per mesh
class HitTriangle final : public HitObject
{
public:
   // Vertices in counterclockwise order. profile: of the engine the triangle is built for, it selects the thin triangle handling
   HitTriangle(IEditable* const editable, const Vertex3Ds rgv[3], PhysicProfile profile);
   ~HitTriangle() override {}

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eTriangle; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override;

   bool IsDegenerate() const { return m_normal.IsZero(); }

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override;

   Vertex3Ds m_rgv[3];
   Vertex3Ds m_normal;
};


class HitPlane final : public HitObject
{
public:
   HitPlane(IEditable* const editable) : HitObject(editable) { }
   HitPlane(IEditable* const editable, const Vertex3Ds& normal, const float d)
      : HitObject(editable)
      , m_normal(normal)
      , m_d(d)
   {
   }

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return ePlane; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override {}  //!! TODO: this is needed if we want to put it in the quadtree, but then again impossible as infinite area

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override { } // this is not implemented, but this is not used for UI and how to render an infinite plane ?

private:
   Vertex3Ds m_normal;
   float m_d;
};


class HitSpinner;

// Ball-vs-swinging-lamina contact model: a wire-frame/plate lamina hanging from
// a horizontal axis lying in the XY plane along m_tangent at height m_pivotZ.
// The lamina spans m_alongMin..m_alongMax along the rod direction (0 = axis,
// positive = hanging direction), has half thickness m_halfThick, and tips
// toward m_normal for positive applied angles. Gates model the hanging
// wire/plate with [0, depth]; the spinner plate, centered on its axis, uses
// [-d, +d]. The contact math is profile-agnostic: it feeds the collider model
// under the 10.9 physics profile and the ball-tracking animation under all
// profiles.
struct SwingLamina
{
   // True when the ball may touch the lamina inside its sweep zone.
   bool IsInsideZone(const BallS& ball) const;
   // Solves the lamina applied angle matching the contact at a given swing-plane
   // position (s along m_normal, h below the pivot axis, u along the axis),
   // picking the reachable root closest to curApplied, and the normal from the
   // lamina toward the ball. u is folded in as an offset to the closest point on
   // the lamina (its ends count as contact points), shrinking the effective
   // contact radius to sqrt(r²-ue²). blocked is set when every candidate rest
   // angle would be reached by sweeping the lamina through the ball: the contact
   // then presses the lamina in place instead of being followed.
   bool SolveContact(const float s, const float h, const float u, const float ballRadius, const float curApplied, float& applied, bool& blocked, Vertex3Ds& normal) const;
   // True when the ball overlaps the lamina set at the given applied angle.
   bool Overlaps(const BallS& ball, const float applied, Vertex3Ds& normal, const float slack = 0.f) const;
   // True when moving the lamina from curApplied to applied sweeps a lamina end
   // through the ball's center in the swing plane (s coordinate of the ball).
   bool SweepsBall(const float applied, const float curApplied, const float s, const float h, const float ballRadius) const;
   // True when the ball touches the lamina at curApplied, giving the applied
   // angle matching the contact (unclamped) and the normal from the lamina
   // toward the ball. blocked is set when reaching that angle would sweep the
   // lamina through the ball: the contact then presses it in place instead of
   // being followed.
   bool EvalContact(const BallS& ball, const float curApplied, float& applied, bool& blocked, Vertex3Ds& normal) const;
   // Time at which the ball's straight motion reaches contact of the lamina
   // frozen at curApplied, or -1 (returns the swing-plane position in s, h, u).
   float HitTestContact(const BallS& ball, const float dtime, const float curApplied, float& s, float& h, float& u) const;
   // Recomputes the interaction zone extents along m_normal: the lamina tip
   // sweep over the applied angle range plus a ball radius margin.
   void UpdateZone(const float appliedMin, const float appliedMax);

   Vertex2D m_tangent = Vertex2D(1.f, 0.f); // axis direction
   Vertex2D m_normal = Vertex2D(0.f, 1.f); // direction the lamina tips toward for positive applied angles
   Vertex2D m_center; // pivot position
   float m_pivotZ = 0.f; // pivot axis height
   float m_halfSpan = 0.f; // extent of the lamina along the axis
   float m_alongMin = 0.f, m_alongMax = 0.f; // lamina extent along the rod direction (0 = pivot, + = hanging)
   float m_halfThick = 0.f; // wire section radius / plate half thickness
   float m_zoneSMin = 0.f, m_zoneSMax = 0.f; // interaction zone extents along m_normal
   float m_zoneZLow = 0.f, m_zoneZHigh = 0.f;
};

class SpinnerMoverObject final : public MoverObject
{
public:
   void UpdateDisplacements(const float dtime) override;
   void UpdateVelocities() override;

   bool AddToList() const override { return true; }

   Spinner* m_pspinner;
   HitSpinner* m_pHitSpinner = nullptr; // collider owning this mover (set at construction)

   float m_anglespeed;
   float m_angle;
   float m_angleMax;
   float m_angleMin;
   float m_elasticity;
   float m_damping;
   bool m_visible;

   // The swinging plate as a lamina centered on its axis (see SwingLamina):
   // drives the ball-tracking plate animation under all profiles while the
   // axis segments keep handling the ball physics
   SwingLamina m_lamina;
   bool m_hadContact = false; // a ball touched the plate during the last substep
   float m_releaseSpeed = 0.f; // angular speed the contacting ball could transfer to the plate
};

class HitSpinner final : public HitObject
{
public:
   HitSpinner(Spinner* const pspinner, const float height);

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eSpinner; }
   void Collide(const CollisionEvent& coll) override;
   void Contact(CollisionEvent& coll, const float dtime) override { }
   void CalcHitBBox() override;

   MoverObject* GetMoverObject() override { return &m_spinnerMover; }

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override;

   LineSeg m_lineseg[2];

   SpinnerMoverObject m_spinnerMover;
};

class HitGate;

class GateMoverObject final : public MoverObject
{
public:
   void UpdateDisplacements(const float dtime) override;
   void UpdateVelocities() override;

   bool AddToList() const override { return true; }

   // Applies the gate limits to an applied angle (open range: +-max for two-way,
   // -max..-min for one-way; the applied angle is the rendered one: two-way
   // gates use m_angle directly, one-way gates use -m_angle).
   float ClampAppliedAngle(const float appliedAngle) const;
   // Recomputes the interaction zone extents from the current angle limits
   // (scripts can change them at runtime: the zone must follow).
   void UpdateZoneExtents();
   // s coordinate of the ball along m_normal, and whether it sits inside the
   // legacy axis segment's footprint used for the crossing Hit events (its u
   // extent and z window).
   bool IsInCrossingRegion(const BallS& ball, float& s) const;
   // Tracks the balls inside the crossing region and fires a Hit event on each
   // axis-plane crossing, like the legacy axis segment did (10.9 only).
   void TrackAxisCrossings();

   Gate *m_pgate;
   HitGate* m_pHitGate = nullptr; // Collider owning this mover (set at construction, for hit events)

   float m_anglespeed;
   float m_angle;
   float m_damping;
   float m_gravityfactor;

   float m_angleMin, m_angleMax; // Open & close angle when using physics, animation limits when using forced animation
   bool m_open; // True if the table logic is opening the gate, not just the ball passing through
   bool m_forcedMove;   // True if the table logic is opening/closing the gate
   bool m_hitDirection; // For the direction of the little bounce-back

   // Wire/plate shape & interaction zone (object space is scaled by the gate
   // length): drives the ball-tracking animation under all profiles and the
   // contact collider model under the 10.9 profile
   SwingLamina m_lamina;
   float m_axisHalfSpan = 0.f; // legacy axis segment extent along the axis (crossing Hit events, 10.9)
   vector<HitBall*> m_balls; // balls inside the crossing region (10.9 only)
   vector<float> m_ballS; // their last seen s position along m_normal
   bool m_atLimit = false; // the dragged wire/plate reached a limit (EOS fired once)
};

class HitGate final : public HitObject
{
public:
   // profile selects the collider model: legacy rigid segments, or the 10.9+ interaction zone
   HitGate(Gate* const pgate, const float height, PhysicProfile profile = PhysicProfile::V10_8);

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eGate; }
   void Collide(const CollisionEvent& coll) override;
   void Contact(CollisionEvent& coll, const float dtime) override { }
   void CalcHitBBox() override;

   MoverObject *GetMoverObject() override { return &m_gateMover; }

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override;

   GateMoverObject m_gateMover;
   bool m_twoWay;
   // For one-way gates under the 10.9 model: the rigid one-sided segment standing for
   // the wire/plate resting on its stop (created by Gate::PhysicSetup, enabled by the
   // mover while the wire is at its min angle)
   LineSeg* m_pblockseg = nullptr;

private:
   Gate *m_pgate;
   LineSeg m_lineseg[2];
};

class TriggerLineSeg final : public LineSeg
{
public:
   TriggerLineSeg(Trigger* const trigger, const Vertex2D& p1, const Vertex2D& p2, const float zlow, const float zhigh);
   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eTrigger; }
   void Collide(const CollisionEvent& coll) override;

   Trigger * const m_ptrigger;
};

class TriggerHitCircle final : public HitCircle
{
public:
   TriggerHitCircle(Trigger* const trigger, const Vertex2D& c, const float r, const float zlow, const float zhigh);
   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eTrigger; }
   void Collide(const CollisionEvent& coll) override;
};

// Arbitrary line segment in 3D space.
// Implemented by transforming a HitLineZ to the desired orientation.
class HitLine3D final : public HitLineZ
{
public:
   HitLine3D(IEditable* const editable, const Vertex3Ds& v1, const Vertex3Ds& v2);

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return e3DLine; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override { } // already done in constructor

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override;

private:
   Matrix3 m_matrix;
   Vertex3Ds m_v1, m_v2;
};
