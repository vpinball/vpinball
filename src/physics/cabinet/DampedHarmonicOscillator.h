// license:GPLv3+

#pragma once

namespace VPX::Physics
{

class DampedHarmonicOscillator
{
public:
   // mass: Total mass [kg]
   // freq: Natural (undamped) frequency [Hz]
   // zeta: Damping ratio (0 = undamped, 1 = critically damped)
   DampedHarmonicOscillator(float mass, float freq, float zeta)
      : m_mass(mass)
      , m_omega0((float)(2.0 * M_PI) * freq)
      , m_k(mass * m_omega0 * m_omega0)
      , m_damping(2.0f * zeta * (mass * m_omega0))
   {
   }

   // Advance the oscillator by dt seconds under external force F [N].
   void StepOneMillisecond(float F, float dt)
   {
      m_acceleration = (F - m_damping * m_velocity - m_k * m_displacement) / m_mass;
      m_velocity += m_acceleration * dt;
      m_displacement += m_velocity * dt;
   }

   void Reset()
   {
      m_acceleration = 0.f;
      m_velocity = 0.f;
      m_displacement = 0.f;
   }

   float GetMass() const { return m_mass; } // kg

   float GetDisplacement() const { return m_displacement; } // m
   float GetVelocity() const { return m_velocity; } // m/s
   float GetAcceleration() const { return m_acceleration; } // m/s^2

private:
   const float m_mass;
   const float m_omega0; // natural angular frequency [rad/s]
   const float m_k; // spring constant [N/m]
   const float m_damping; // damping coefficient [N.s/m]
   float m_displacement = 0.f;
   float m_velocity = 0.f;
   float m_acceleration = 0.f;
};

}
