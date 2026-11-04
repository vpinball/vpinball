// license:GPLv3+

#pragma once

#include <random>

#include "common.h"
#include "B2SDataModel.h"

namespace B2S {

// B2S Pro physics ball: a ball artwork simulated in the authored backglass space against the Pro
// boundaries (polylines with per segment restitution), round obstacles, a pivot picture used as a
// flipper, a launcher that holds and shoots it, and switch zones that pulse a controller switch
// when the ball enters them. Port of B2S Pro's PhysicsBallState (Ken Wildman), stepped at a fixed
// 1 ms like the reference, presented once per frame.
class B2SProPhysicsBall final
{
public:
   // ball must be a physics ball bulb with non empty bounds; flipper is the registered pivot bulb named by
   // PhysicsFlipperName, or nullptr
   B2SProPhysicsBall(B2SBulb& ball, B2SBulb* flipper);

   void Start();
   void Launch();
   // One fixed substep (dt in seconds, 0.001 in the reference). The flipper angle is read from the pivot runtime
   void Step(float dt);
   // Publish the simulated centre for drawing and advance the rolling angle (PresentServerFrame)
   void Present();

   bool IsActive() const { return m_active; }
   bool HasLauncher() const { return m_hasLauncher; }
   B2SBulb& Ball() const { return m_ball; }
   B2SBulb* Flipper() const { return m_flipper; }

   // Physics selection mask: an alpha image in backglass coordinates multiplied into the ball artwork
   // wherever it travels (hides it behind the hoop, the net, ...). Returns the masked, rolled frame of the
   // ball as an axis aligned texture with its placement, or nullptr when the ball has no mask (then the
   // artwork is drawn directly). Must be called on the API thread (texture access)
   bool HasMask() const { return m_ball.m_pro.physicsSelectionMask != nullptr; }
   VPXTexture MaskedFrame(float& x, float& y, float& w, float& h);
   ~B2SProPhysicsBall();

   std::function<void(int)> pulseSwitch = [](int) { };

private:
   void LoadMask();
   const uint8_t* m_maskPixels = nullptr; // The decoded mask texture's pixels (owned by the bulb's texture), alpha at byte 3
   int m_maskBpp = 0;
   int m_maskWidth = 0;
   int m_maskHeight = 0;
   bool m_maskLoaded = false;
   VPXTexture m_frameTexture = nullptr;
   vector<uint8_t> m_frameBuffer;
   float m_frameX = 0.f, m_frameY = 0.f;
   int m_frameW = 0, m_frameH = 0;
   float m_framePosX = 0.f, m_framePosY = 0.f, m_frameAngle = 0.f;
   bool m_frameValid = false;
   void StepPhysics(float dt, float flipperAngularVelocity);
   void GetLauncherPose(B2SProPoint& origin, float& angle) const;
   void CheckLauncherCapture(B2SProPoint& center);
   void CheckSwitchZones(const B2SProPoint& previousCenter, const B2SProPoint& center);
   void ResolveFlipperCollision(B2SProPoint& center, float ballRadius, B2SProPoint& velocity, float angularVelocity);
   void ResolveStaticFloorCollision(B2SProPoint& center, const B2SProPoint& previousCenter, float radius, B2SProPoint& velocity, const B2SProPoint& from, const B2SProPoint& to, float bounce);
   void ResolveCircularObstacleCollision(B2SProPoint& center, float ballRadius, B2SProPoint& velocity, const B2SProRect& obstacle, float bounce);
   void ReflectBoundaryVelocity(B2SProPoint& velocity, float normalX, float normalY, float bounce, float incomingNormalSpeed);
   static bool TryGetSweptStaticFloorCollision(const B2SProPoint& previousCenter, const B2SProPoint& currentCenter, float radius, const B2SProPoint& from, const B2SProPoint& to,
      B2SProPoint& impactCenter, B2SProPoint& impactNormal);
   static void ApplyShallowSurfaceRollingBoost(B2SProPoint& velocity, const B2SProPoint& from, const B2SProPoint& to, float dt, float gravity);
   static bool PointInSwitchZone(const B2SProPoint& point, const B2SProBulb::SwitchZone& zone);
   static bool SegmentIntersectsSwitchZone(const B2SProPoint& from, const B2SProPoint& to, const B2SProBulb::SwitchZone& zone);
   static bool ClipSwitchSegment(double direction, double distance, double& first, double& last);
   static B2SProPoint SwitchZoneLocalPoint(const B2SProPoint& point, const B2SProBulb::SwitchZone& zone);
   float Random() { return std::uniform_real_distribution<float>(0.f, 1.f)(m_random); }

   B2SBulb& m_ball;
   B2SBulb* const m_flipper;
   const B2SProRect m_bounds;
   const float m_gravity;
   const float m_flipperStrength;
   const float m_boundaryBounce;
   vector<vector<B2SProPoint>> m_boundaryPaths;
   vector<vector<float>> m_boundarySegmentBounces;
   vector<B2SProRect> m_obstacles;
   vector<float> m_obstacleBounces;
   vector<B2SProBulb::SwitchZone> m_switchZones;
   vector<bool> m_switchZoneInside;
   const bool m_hasLauncher;
   B2SProBulb::Launcher m_launcher;
   bool m_launcherArmed = true;
   bool m_launcherHolding = false;
   bool m_launcherExitedCapture = false;
   const float m_ballWidth;
   const float m_ballHeight;
   const B2SProPoint m_authoredCenter;
   bool m_active = false;
   B2SProPoint m_velocity;
   B2SProPoint m_center; // Simulation position in the authored backglass space
   float m_lastFlipperAngle = 0.f;
   bool m_hasLastFlipperAngle = false;
   B2SProPoint m_presented; // Last centre published for drawing (rolling is measured from it)
   std::mt19937 m_random { std::random_device {}() };
};

}
