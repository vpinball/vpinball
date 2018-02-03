#pragma once


#define PHYSICS_STEPTIME    1000         // usecs to go between each physics update

#define PHYSICS_STEPTIME_S  (PHYSICS_STEPTIME * 1e-6)     // step time in seconds

#define DEFAULT_STEPTIME      10000      // default physics rate: 1000Hz
#define DEFAULT_STEPTIME_S    0.01       // default physics rate: 1000Hz

#define PHYS_FACTOR         (PHYSICS_STEPTIME_S / DEFAULT_STEPTIME_S)

#define DEFAULT_TABLE_GRAVITY           0.97f
#define DEFAULT_TABLE_CONTACTFRICTION   0.075f
#define DEFAULT_TABLE_SCATTERANGLE      0.5f
#define DEFAULT_TABLE_ELASTICITY        0.25f
#define DEFAULT_TABLE_ELASTICITY_FALLOFF 0.f
#define DEFAULT_TABLE_PFSCATTERANGLE    0.f

//#define PRINT_DEBUG_COLLISION_TREE     // print collision acceleration structure info (will slow down debugging startup time if enabled)

/*
 * NOTE ABOUT VP PHYSICAL UNITS:
 *
 * By convention, one VP length unit (U) corresponds to
 *   1 U = .53975 mm = 5.3975E-4 m,   or   1 m = 1852.71 U
 *
 * For historical reasons, one VP time unit (T) corresponds to
 *   1 T = 10 ms = 0.01 s,            or   1 s = 100 T
 *
 * Therefore, Earth gravity in VP units can be computed as
 *   g  =  9.81 m/s^2  =  1.81751 U/T^2
 */
/*
 * nFozzy explanation:
 * The inch conversion is nice because VP units are somewhat based on inches so you can use fractions:
 * VP units = (inches * 17) / 800
 * Inches = (VP units * 800) / 17
 * 
 * I have to switch between metric and VP units when modeling / rendering and I do that with these scales:
 * CM to VP units: (50 / 2.69875)
 * VP units to CM: (2.69875 / 50)
 * 
 * I’m trying to move to this sort of thing in the future, modeling in pseudo-inches because Maya’s native inch support is for shits
 * Working inches Scale: 1
 * Scale up to CM for rendering: 2.54
 * Scale way up to VPu for export: (1*800)/17
 */

#define GRAVITYCONST    1.81751f

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
// Layer outside object which increases it's size for contact measurements. Used to determine clearances.
// Setting this value during testing to 0.1 will insure clearance. After testing set the value to 0.005
// Default 0.01
#define PHYS_TOUCH 0.05
// Low Normal speed collison is handled as contact process rather than impulse collision
#define C_LOWNORMVEL 0.0001f
#define C_CONTACTVEL 0.099f

//#define NEW_PHYSICS

// low velocity stabilization ... if embedding occurs add some velocity
#ifdef NEW_PHYSICS
 #define C_EMBEDVELLIMIT 5.f
#endif

// old workarounds, not needed anymore?!
#ifndef NEW_PHYSICS
 #define C_EMBEDSHOT_PLANE // push pos up if ball embedded in plane
 #define C_EMBEDDED 0.0f
 #define C_EMBEDSHOT 0.05f
 // Contact displacement corrections, hard ridgid contacts i.e. steel on hard plastic or hard wood
 #define C_DISP_GAIN 0.9875f
 #define C_DISP_LIMIT 5.0f
 // Have special cases for balls that are determined static? (C_DYNAMIC is kind of a counter for detection) -> does not work stable enough anymore nowadays
 //#define C_DYNAMIC 2
 // choose only one of these two heuristics:
 #define C_BALL_SPIN_HACK // original ball spin reduction code, based on automatic detection/heuristic of resting balls
 //#define C_BALL_SPIN_HACK2 0.1 // dampens ball spin on collision contacts and at the same time very slow moving balls (smaller = less damp)
#endif

//trigger/kicker boundary crossing hysterisis
#define STATICTIME 0.005f
#define STATICCNTS 10

//Flippers:
#define C_INTERATIONS 20 // Precision level and cycles for interative calculations // acceptable contact time ... near zero time

//Plumb:
#define	VELOCITY_EPSILON 0.05f	// The threshold for zero velocity.
