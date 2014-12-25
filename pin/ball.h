#pragma once

#include "pin/collide.h"


class BallAnimObject : public AnimObject
{
public:
    virtual bool FMover() const {return false;} // We add ourselves to the mover list.  
    // If we allow the table to do that, we might get added twice, 
    // if we get created in Init code
    virtual void UpdateDisplacements(const float dtime);
    virtual void UpdateVelocities();

    Ball *m_pball;
};

int NumBallsInitted();

class Ball : public HitObject 
{
public:
    Ball();

    void Init(const float mass = 1.0f);
    void RenderSetup();

    virtual void UpdateDisplacements(const float dtime);
    virtual void UpdateVelocities();

    // From HitObject
    virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
    virtual int GetType() const {return eBall;}
    virtual void Collide(CollisionEvent *coll);
    virtual void CalcHitRect();
    virtual AnimObject *GetAnimObject() {return &m_ballanim;}

    //semi-generic collide methods
    void Collide2DWall(const Vertex3Ds& hitNormal, const float elasticity, float friction, float scatter_angle)
    { Collide3DWall( Vertex3Ds(hitNormal.x, hitNormal.y, 0), elasticity, friction, scatter_angle); }

    void Collide3DWall(const Vertex3Ds& hitNormal, const float elasticity, float friction, float scatter_angle);

    void ApplyFriction(const Vertex3Ds& hitnormal, float dtime, float fricCoeff);
    void HandleStaticContact(const Vertex3Ds& normal, float origNormVel, float friction, float dtime);

    Vertex3Ds SurfaceVelocity(const Vertex3Ds& surfP) const;
    Vertex3Ds SurfaceAcceleration(const Vertex3Ds& surfP) const;

    void ApplySurfaceImpulse(const Vertex3Ds& surfP, const Vertex3Ds& impulse);

    void EnsureOMObject();

    COLORREF m_color;

    // Per frame info
    CCO(BallEx) *m_pballex; // Object model version of the ball

    char m_szImage[MAXTOKEN];
    char m_szImageFront[MAXTOKEN];

    Texture *m_pinballEnv;
    Texture *m_pinballDecal;

    VectorVoid* m_vpVolObjs;// vector of triggers we are now inside

    CollisionEvent m_coll;  // collision information, may not be a actual hit if something else happens first

    int m_dynamic;			// used to determine static ball conditions and velocity quenching

    BallAnimObject m_ballanim;

    Vertex3Ds m_pos;
    float m_defaultZ;   //normal height of the ball //!! remove

    Vertex3Ds m_oldpos[MAX_BALL_TRAIL_POS]; // for the optional ball trails
    unsigned int m_ringcounter_oldpos;

    Vertex3Ds m_vel;      // ball velocity

    float m_drsq;	// square of distance moved

    float m_radius;
    float m_mass;
    float m_invMass;

    Vertex3Ds m_Event_Pos; // last hit event position (to filter hit 'equal' hit events)

    Matrix3 m_orientation;
    Vertex3Ds m_angularmomentum;
    Vertex3Ds m_angularvelocity;
    float m_inertia;

    unsigned int m_id; // unique ID for each ball

    bool m_frozen;

    static unsigned int ballID; // increased for each ball created to have an unique ID for scripts for each ball
};
