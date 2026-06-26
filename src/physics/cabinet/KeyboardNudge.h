// license:GPLv3+

#pragma once

#include "math/vector.h"
#include "physics/cabinet/CabinetPhysics.h"
#include "physics/cabinet/NudgeHandler.h"

namespace VPX::Physics
{

class PushRetractKeyboardNudge final : public KeyboardNudge
{
public:
   PushRetractKeyboardNudge(float nudgeStrength);
   ~PushRetractKeyboardNudge() override;

   float GetStrengthScale() const override { return m_strength; }
   void SetStrengthScale(float strength) override { m_strength = strength; };

   void Nudge(float angle, float force) override;

   void StepOneMillisecond() override;
   const Vertex2D& GetCabinetAcceleration() const override;
   const Vertex2D& GetCabinetOffset() const override;

   bool IsActive() const override;

private:
   Vertex2D m_cabinetAcceleration; // m/s^2
   Vertex2D m_cabinetPosition; // m

   float m_strength = 0.f;
   Vertex2D m_impulse;
   int m_nudgeTime = 0;

   int m_deactivationDelay = 0;
};


class BoxModelKeyboardNudge final : public KeyboardNudge
{
public:
   BoxModelKeyboardNudge(float nudgeStrength);
   ~BoxModelKeyboardNudge() override;

   float GetStrengthScale() const override { return m_strength; }
   void SetStrengthScale(float strength) override { m_strength = strength; };

   void Nudge(float angle, float force) override;

   void StepOneMillisecond() override;
   const Vertex2D& GetCabinetAcceleration() const override;
   const Vertex2D& GetCabinetOffset() const override;

   bool IsActive() const override;

private:
   float m_strength = 0.f;

   Vertex2D m_cabinetAcceleration; // m/s^2
   Vertex2D m_cabinetVelocity;
   Vertex2D m_cabinetPrevVelocity;
   Vertex2D m_cabinetPositionVPU; // VPU
   Vertex2D m_cabinetPosition; // m

   float m_nudgeSpring;
   float m_nudgeDamping;

   int m_deactivationDelay = 0;
};


class CabModelKeyboardNudge final : public KeyboardNudge
{
public:
   CabModelKeyboardNudge(float nudgeStrength);
   ~CabModelKeyboardNudge() override;

   float GetStrengthScale() const override { return m_strength; }
   void SetStrengthScale(float strength) override { m_strength = strength; };

   void Nudge(float angle, float force) override;

   void StepOneMillisecond() override;
   const Vertex2D& GetCabinetAcceleration() const override;
   const Vertex2D& GetCabinetOffset() const override;

   bool IsActive() const override;

private:
   float m_strength = 0.f;

   class Impulse final
   {
   public:
      Impulse(const int length, Vertex2D impulse);

      bool IsInProgress() const { return m_impulseElapsed <= m_impulseLength; }
      void StepOneMillisecond() { m_impulseElapsed++; }
      Vertex2D GetImpulseAceleration() const; // m/s^2

   private:
      int m_impulseLength;
      int m_impulseElapsed = 0;
      Vertex2D m_impulse;
   };
   vector<Impulse> m_impulses;

   CabinetPhysics m_cabinet;

   int m_deactivationDelay = 0;
};

}
