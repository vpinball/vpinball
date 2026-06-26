// license:GPLv3+

#include "core/stdafx.h"
#include "KeyboardNudge.h"

#include "core/player.h"
#include "parts/pintable.h"


namespace VPX::Physics
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Legacy Keyboard Nudge, as used in VP9
//

PushRetractKeyboardNudge::PushRetractKeyboardNudge(float nudgeStrength)
   : m_strength(nudgeStrength)
{
   m_cabinetAcceleration.SetZero();
   m_cabinetPosition.SetZero();
}

PushRetractKeyboardNudge::~PushRetractKeyboardNudge() { }

bool PushRetractKeyboardNudge::IsActive() const { return m_deactivationDelay > 0; }

void PushRetractKeyboardNudge::Nudge(float angle, float force)
{
   m_deactivationDelay = 10000;
   if (m_nudgeTime != 0)
      return;

   const float a = ANGTORAD(angle);
   m_impulse.x =  sinf(a) * (m_strength * force);
   m_impulse.y = -cosf(a) * (m_strength * force);
   m_nudgeTime = 100;
}

void PushRetractKeyboardNudge::StepOneMillisecond()
{
   if (m_deactivationDelay)
      m_deactivationDelay--;

   if (m_nudgeTime != 0)
   {
      m_nudgeTime--;
      if (m_nudgeTime == 95) // 5ms front
      {
         m_cabinetAcceleration.x = -m_impulse.x * 2.0f;
         m_cabinetAcceleration.y = m_impulse.y * 2.0f;
         m_cabinetAcceleration *= static_cast<float>(1.0 / PHYS_FACTOR); // Convert to force
         m_cabinetAcceleration = VPUVPT2TOMS2(m_cabinetAcceleration);
      }
      else if (m_nudgeTime == 90) // 5ms back
      {
         m_cabinetAcceleration.x = m_impulse.x;
         m_cabinetAcceleration.y = -m_impulse.y;
         m_cabinetAcceleration *= static_cast<float>(1.0 / PHYS_FACTOR); // Convert to force
         m_cabinetAcceleration = VPUVPT2TOMS2(m_cabinetAcceleration);
      }
      else // Prevent new nudge during the remaining 90ms
      {
         m_cabinetAcceleration.SetZero();
      }
   }

   const float attenuation = sqrf(static_cast<float>(m_nudgeTime) * 0.01f);
   m_cabinetPosition.x = m_impulse.x * attenuation;
   m_cabinetPosition.y = -m_impulse.y * attenuation;
   m_cabinetPosition = VPUTOM(m_cabinetPosition);
}

const Vertex2D& PushRetractKeyboardNudge::GetCabinetAcceleration() const { return m_cabinetAcceleration; }

const Vertex2D& PushRetractKeyboardNudge::GetCabinetOffset() const { return m_cabinetPosition; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// Keyboard Nudge used since VP10
// Box model damped oscillator
//

BoxModelKeyboardNudge::BoxModelKeyboardNudge(float nudgeStrength)
   : m_strength(nudgeStrength)
{
   m_cabinetAcceleration.SetZero();
   m_cabinetPositionVPU.SetZero();
   m_cabinetPosition.SetZero();

   // Table movement (displacement u) is modeled as a mass-spring-damper system
   //   u'' = -k u - c u'
   // with a spring constant k and a damping coefficient c.
   // See http://en.wikipedia.org/wiki/Damping#Linear_damping
   const float nudgeTime = g_pplayer ? g_pplayer->m_ptable->m_nudgeTime : 5.f; // T
   constexpr float dampingRatio = 0.5f; // zeta

   // time for one half period (one swing and swing back):
   //   T = pi / omega_d,
   // where
   //   omega_d = omega_0 * sqrt(1 - zeta^2)       (damped frequency)
   //   omega_0 = sqrt(k)                          (undamped frequency)
   // Solving for the spring constant k, we get
   m_nudgeSpring = (float)(M_PI * M_PI) / (nudgeTime * nudgeTime * (1.0f - dampingRatio * dampingRatio));

   // The formula for the damping ratio is
   //   zeta = c / (2 sqrt(k)).
   // Solving for the damping coefficient c, we get
   m_nudgeDamping = dampingRatio * 2.0f * sqrtf(m_nudgeSpring);
}

BoxModelKeyboardNudge::~BoxModelKeyboardNudge() { }

bool BoxModelKeyboardNudge::IsActive() const { return m_deactivationDelay > 0; }

void BoxModelKeyboardNudge::Nudge(float angle, float force)
{
   m_deactivationDelay = 10000;
   const float a = ANGTORAD(angle);
   m_cabinetVelocity.x +=  sinf(a) * (m_strength * force);
   m_cabinetVelocity.y += -cosf(a) * (m_strength * force);
}

// Note that this used to be mixed up with accelerator based nudging by replacing acceleration by the acquired one
void BoxModelKeyboardNudge::StepOneMillisecond()
{
   if (m_deactivationDelay)
      m_deactivationDelay--;

   // Perform keyboard nudge by simulating table movement modeled as a mass-spring-damper system
   //   u'' = -k u - c u'
   // with a spring constant k and a damping coefficient c
   const Vertex2D force = -m_nudgeSpring * m_cabinetPositionVPU - m_nudgeDamping * m_cabinetVelocity;
   m_cabinetVelocity += static_cast<float>(PHYS_FACTOR) * force;
   m_cabinetPositionVPU += static_cast<float>(PHYS_FACTOR) * m_cabinetVelocity;
   m_cabinetPosition = VPUTOM(m_cabinetPositionVPU);
   m_cabinetAcceleration = VPUVPT2TOMS2((m_cabinetVelocity - m_cabinetPrevVelocity) * static_cast<float>(1.0 / PHYS_FACTOR));
   m_cabinetPrevVelocity = m_cabinetVelocity;
}

const Vertex2D& BoxModelKeyboardNudge::GetCabinetAcceleration() const { return m_cabinetAcceleration; }

const Vertex2D& BoxModelKeyboardNudge::GetCabinetOffset() const { return m_cabinetPosition; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// Keyboard Nudge, used since VP10.9
// Simulated cabinet model, used by the intent nudge system
//

CabModelKeyboardNudge::CabModelKeyboardNudge(float nudgeStrength)
   : m_strength(nudgeStrength)
{
}

CabModelKeyboardNudge::~CabModelKeyboardNudge() { }

bool CabModelKeyboardNudge::IsActive() const { return m_deactivationDelay > 0; }

void CabModelKeyboardNudge::Nudge(float angle, float force)
{
   m_deactivationDelay = 10000;
   const float a = ANGTORAD(angle);
   // 6 is a magic number to match the legacy force value, hardcoded to 2 (no unit), to the 12 m/s^2 observed on strong nudges
   const float xForce =  sinf(a) * (force * m_strength * 6.f);
   const float yForce = -cosf(a) * (force * m_strength * 6.f);
   m_impulses.emplace_back(25, Vertex2D { xForce, yForce });
}

CabModelKeyboardNudge::Impulse::Impulse(const int length, Vertex2D impulse)
   : m_impulseLength(length)
   , m_impulse(impulse)
{
}

Vertex2D CabModelKeyboardNudge::Impulse::GetImpulseAceleration() const
{
   if (!IsInProgress())
      return { 0.f, 0.f };
   const float t = static_cast<float>(m_impulseElapsed) / static_cast<float>(m_impulseLength);
   return m_impulse * 0.5f * (1.0f - cosf((float)(2. * M_PI) * t));
}

void CabModelKeyboardNudge::StepOneMillisecond()
{
   if (m_deactivationDelay)
      m_deactivationDelay--;

   Vertex2D impulse { 0.f, 0.f };
   for (std::vector<Impulse>::iterator it = m_impulses.begin(); it != m_impulses.end();)
   {
      it->StepOneMillisecond();
      if (it->IsInProgress())
      {
         impulse += it->GetImpulseAceleration();
         it++;
      }
      else
      {
         it = m_impulses.erase(it);
      }
   }

   m_cabinet.StepOneMillisecond(m_cabinet.GetMass() * impulse);

   // Log for debugging purposes as CSV: Intent acceleration (m/s^2);Cab acceleration (m/s^2);Cab position (mm)
   PLOGD_IF(false) << std::format(";{:8.5f};{:8.5f};{:8.5f}", impulse.y, m_cabinet.GetCabinetAcceleration().y, m_cabinet.GetCabinetOffset().y * 1000.f);
}

const Vertex2D& CabModelKeyboardNudge::GetCabinetAcceleration() const { return m_cabinet.GetCabinetAcceleration(); }

const Vertex2D& CabModelKeyboardNudge::GetCabinetOffset() const { return m_cabinet.GetCabinetOffset(); }

}
