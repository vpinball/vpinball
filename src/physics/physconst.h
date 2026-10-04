// license:GPLv3+

// Ported at: VisualPinball.Engine/Common/Constants.cs

#pragma once


#define PHYSICS_STEPTIME    1000         // usecs to go between each physics update (1000Hz physics)
#define PHYSICS_STEPTIME_S  (PHYSICS_STEPTIME * 1e-6) // step time in seconds

#define DEFAULT_STEPTIME      10000      // VPT in microseconds, custom time unit used for historical reason (1VPT = 10ms)
#define DEFAULT_STEPTIME_S    0.01       // VPT in seconds, custom time unit used for historical reason (1VPT = 10ms)

#define PHYS_FACTOR         (PHYSICS_STEPTIME_S / DEFAULT_STEPTIME_S) // Physics step time expressed in Visual Pinball Time (VPT)

#define DEFAULT_TABLE_GRAVITY            0.97f
#define DEFAULT_TABLE_CONTACTFRICTION    0.075f
#define DEFAULT_TABLE_SCATTERANGLE       0.5f
#define DEFAULT_TABLE_ELASTICITY         0.25f
#define DEFAULT_TABLE_ELASTICITY_FALLOFF 0.f
#define DEFAULT_TABLE_PFSCATTERANGLE     0.f
#define DEFAULT_TABLE_MIN_SLOPE          6.0f
#define DEFAULT_TABLE_MAX_SLOPE          6.0f

#define HIT_SHAPE_DETAIL_LEVEL           7.0f // static detail level to approximate ramps and rubbers for the physics/collision code

//#define PRINT_DEBUG_COLLISION_TREE // print collision acceleration structure info (will slow down debugging startup time if enabled)

/*
 * NOTE ABOUT VP PHYSICAL UNITS:
 *
 * By convention, 50 VP length unit (U) corresponds to 1"1/16 (standard ball size)
 *   1 U = 1.0625 / 50 inches = .53975 mm = 5.3975E-4 m,   or approximately  1 m = 1852.71 U
 *
 * For historical reasons, one VP time unit (T) corresponds to 0.01s
 *   1 T = 10 ms = 0.01 s,            or   1 s = 100 T
 *
 * Therefore, Earth gravity in VP units can be computed as
 *   g  =  9.81 m/s^2  =  1.81751 U/T^2
 *
 * For historical reason, 1 VP mass unit is defined as the mass of a standard ball, that is to say 80g.
 *
 * see def.h for exact helper conversion macros
 */
#define GRAVITYCONST 1.81751f

// Collisions:
//
// test near zero conditions in linear, well behaved, conditions
#define C_PRECISION 0.01f
// tolerance for line segment endpoint and point radii collisions
#define C_TOL_ENDPNTS 0.0f
#define C_TOL_RADIUS 0.005f
// Physical Skin ... postive contact layer. Any contact (collision) in this layer reports zero time.
// layer is used to calculate contact effects ... beyond this and objects pass through each other
// Default 25.0
#define PHYS_SKIN 25.0 //!! seems like this mimics the radius of the ball -> replace with radius where possible?
#define DEFAULT_BALL_SIZE 25.f
// Layer outside object which increases it's size for contact measurements. Used to determine clearances.
// Setting this value during testing to 0.1 will insure clearance. After testing set the value to 0.005
// Default 0.01
#define PHYS_TOUCH 0.05
// Low Normal speed collison is handled as contact process rather than impulse collision
#define C_LOWNORMVEL 0.0001f
#define C_CONTACTVEL 0.099f

// Experimental, on top of FIX_PHYSICS (constants not calibrated yet!):
// - drops the legacy embedding/spin workarounds (C_EMBEDDED/C_EMBEDSHOT, C_EMBEDSHOT_PLANE, C_DISP_GAIN, C_BALL_SPIN_HACK)
// - replaces them: embedded balls in contact are pushed out by velocity (C_EMBEDVELLIMIT), overlapping balls are
//   separated positionally (C_EMBEDDISPLIMIT), slow balls embedded deeper than PHYS_TOUCH become contacts
//   (walls, Hit3DPolys, playfield), and contacts apply rolling resistance (C_ROLLING_RESISTANCE)
// - contact friction (constants are initial guesses, to be calibrated with help of real tables):
//   - spin friction about the contact normal (mu * C_CONTACT_PATCH_RADIUS): point contacts could not stop a ball spinning in place
//   - ball/ball collisions apply Coulomb friction with C_BALL_BALL_FRICTION (as ball/ball contacts do, see FIX_PHYSICS)
//   - spin friction between balls uses their relative spin
//#define NEW_PHYSICS

// Active behavioral fixes of the physics engine. Keep this list in
// sync with the FIX_PHYSICS-guarded code: document each fix the flag enables.
// - Slow shallow touches (bnd <= PHYS_TOUCH, |bnv| <= C_CONTACTVEL) reported a fake
//   hittime of bnd*10+0.5 (a distance remapped into (0,1] T), systematically > dtime and therefore
//   dropped before any contact was recorded: the wall/poly touch band was dead until actual
//   penetration, and surviving fake times corrupted the min-time event ordering. With the fix,
//   LineSeg/Hit3DPoly report hittime 0 so the contact flag engages (HitBall already reports
//   hittime 0 for the touch layer since ball-ball contacts were enabled).
// - The static-friction gate "normVel <= 0.025" in ApplyFriction measured the post-contact
//   residual ~ -m(g.n)dtime, an accidental test of the normal orientation that always selected the
//   static (acceleration-directed) branch on walls even with real slip; the clause is now restricted
//   to support-like contacts (hitnormal.z > 0.5) so walls use slip-directed dynamic friction.
// - The friction cone used -m(g.n) as its sole normal-force estimate on contacts (~0 on
//   walls, dead on the top glass) while Collide3DWall budgeted the pre-restitution impulse
//   (under-counting by 1+elasticity). Contacts now clamp the friction impulse by
//   mu*m*(the normal delta-v the contact applied this step), and collisions by the post-restitution
//   impulse, matching HitFlipper's convention.
// - The flipper moment of inertia uses the real flipper shape instead of a rod; coil torque is
//   rescaled (m_backwards_compatibility) to keep the free swing, ball impacts do change.
// - A slow ball embedded deeper than PHYS_TOUCH in the playfield was ignored (and could sink through), now it collides.
// - Sliding contact friction requests the full slip-removing impulse (Coulomb bound clamped)
//   instead of dtime times it, which made it depend on the step length.
// - Ball/ball contacts (HitBall::Contact): the contact friction uses the motion relative to the other ball, which was
//   treated as a static surface, and C_BALL_BALL_FRICTION (was 0.3).
// - Ball/ball contacts are resolved before the other contacts, and static contacts also cancel the velocity change the
//   contacts resolved before them in the same pass applied to the ball (all of it for ball/ball contacts, the friction
//   part for static ones), on top of the hit test velocity they otherwise work from: the friction stopping the spin of a
//   ball pressed against another ball or a wall (spin its playfield friction gives it every step) pushes it up, and the
//   ball climbed off the playfield until it dropped back (jitter of balls resting in a row). The normal impulses of the
//   other static contacts stay out (two wires of a wire ramp).
// - A ball/ball contact cancels at most a contact-scale approach (C_CONTACTVEL plus a step of gravity): it cancelled all of
//   it, so an impact a ball received in the same cycle (a ball hit into a resting pair) was absorbed instead of reaching
//   the other ball (Newton's cradle).
// - HitFlipper::Contact tests the gap acceleration with the ball's center acceleration instead of the one of its material
//   contact point, whose centripetal term (>= 0 along the normal for any spin) made a spinning ball lose its support.
#define FIX_PHYSICS

#if defined(NEW_PHYSICS) && !defined(FIX_PHYSICS)
 #error NEW_PHYSICS builds on FIX_PHYSICS
#endif

// contact dissipation (initial guesses, to be calibrated with help of real tables)
#ifdef FIX_PHYSICS
 #define C_BALL_BALL_FRICTION 0.1f   // Coulomb friction coefficient between two balls (steel on steel): contacts, with NEW_PHYSICS also collisions
#endif
#ifdef NEW_PHYSICS
 #define C_CONTACT_PATCH_RADIUS 1.0f // ball contact patch radius (VPU): spin friction about the normal = mu * radius * normal force
#endif

// low velocity stabilization ... if embedding occurs add some velocity
#ifdef NEW_PHYSICS
 #define C_EMBEDVELLIMIT 5.f         // max push-out velocity for a ball embedded in a contact surface, can be undefd
 #define C_EMBEDDISPLIMIT 5.0f       // max separation (per collision) of two embedded balls
 #define C_ROLLING_RESISTANCE 0.002f // rolling resistance coefficient C_rr (torque = C_rr * radius * normal force), initial guess
#endif

// old workarounds, not needed anymore?!
#ifndef NEW_PHYSICS
 #define C_EMBEDSHOT_PLANE // push pos up if ball embedded in plane
 #define C_EMBEDDED 0.0f   // can be undefd
 #define C_EMBEDSHOT 0.05f
 // Contact displacement corrections, hard ridgid contacts i.e. steel on hard plastic or hard wood
 #define C_DISP_GAIN 0.9875f // can be undefd
 #ifdef C_DISP_GAIN
  #define C_DISP_LIMIT 5.0f
 #endif
 // Have special cases for balls that are determined static? (C_DYNAMIC is kind of a counter for detection) -> does not work stable enough anymore nowadays
 //#define C_DYNAMIC 2
 // choose only one of these two heuristics:
 #define C_BALL_SPIN_HACK // original ball spin reduction code, based on automatic detection/heuristic of resting balls
 //#define C_BALL_SPIN_HACK2 0.1 // dampens ball spin on collision contacts and at the same time very slow moving balls (smaller = less damp)
#endif

// trigger/kicker boundary crossing hysterisis, also slow/static ball<->ball and to some extent general ball<->object interactions
#define STATICTIME 0.02f // smallest time/intersection difference allowed in the simulation, if amount of all intersections found within that smaller timeframe is > STATICCNTS
#define STATICCNTS 10    // 0=always clamp to the minimum STATICTIME difference, no exceptions, will/should lead to more penetration!

// Flippers:
#define C_INTERATIONS 20 // Precision level and cycles for interative calculations // acceptable contact time ... near zero time

// Plumb:
#define	VELOCITY_EPSILON 0.05f // The threshold for zero velocity.
