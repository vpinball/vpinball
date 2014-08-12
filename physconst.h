#pragma once


#define PHYSICS_STEPTIME    10000	// usecs to go between each physics update

//#define PHYSICS_STEPTIME_S  ((float)(PHYSICS_STEPTIME * 1e-6))     // step time in seconds
//
//#define DEFAULT_STEPTIME    0.01f      // default physics rate: 100Hz
//#define PHYS_FACTOR         (PHYSICS_STEPTIME_S / DEFAULT_STEPTIME)

#define DEFAULT_TABLE_GRAVITY           1.6774f
#define DEFAULT_TABLE_CONTACTFRICTION   0.0005f
#define DEFAULT_TABLE_SCATTERANGLE      0.5f
#define DEFAULT_TABLE_DAMPENINGSPEED    65.0f
#define DEFAULT_TABLE_DAMPENINGFRICTION 0.95f

#define GRAVITYCONST   0.86543f

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
#define PHYS_SKIN 25.0
// Layer outside object which increases it's size for contact measurements. Used to determine clearances.
// Setting this value during testing to 0.1 will insure clearance. After testing set the value to 0.005
// Default 0.01
#define PHYS_TOUCH 0.05
// Low Normal speed collison is handled as contact process rather than impulse collision
#define C_LOWNORMVEL 0.0001f
#define C_CONTACTVEL 0.099f
// limit ball speed to C_SPEEDLIMIT
#define C_SPEEDLIMIT 60.0f
#define C_DAMPFRICTION 0.95f
// low velocity stabilization ... if embedding occurs add some velocity 
#define C_EMBEDDED 0.0f
#define C_EMBEDSHOT 0.05f
// Contact displacement corrections, hard ridgid contacts i.e. steel on hard plastic or hard wood
#define C_DISP_GAIN 0.9875f
#define C_DISP_LIMIT 5.0f
#define C_FRICTIONCONST 2.5e-3f
#define C_DYNAMIC 2
//trigger/kicker boundary crossing hysterisis
#define STATICTIME 0.005f

#define NUDGE_MANUAL_FRICTION 0.92f     // TODO: depends on STEPTIME


//Flippers:
#define C_FLIPPERACCEL   1.25f
#define C_FLIPPERIMPULSE 1.0f
#define C_INTERATIONS 20 // Precision level and cycles for interative calculations // acceptable contact time ... near zero time

//Ball:
#define ANGULARFORCE 1   // Number I pulled out of my butt - this number indicates the maximum angular change 
					     // per time unit, or the speed at which angluar momentum changes over time, 
					     // depending on how you interpret it.
//Ramp:
#define WALLTILT 0.5f

//Plumb:
#define	VELOCITY_EPSILON 0.05f	// The threshold for zero velocity.

