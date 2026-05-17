// license:GPLv3+

#include "core/stdafx.h"
#include "CabinetPhysics.h"

namespace VPX::Physics
{

CabinetPhysics::CabinetPhysics(float mass)
   : m_mass(mass)
   // Oscillation and damping calibrated on real cabinets (from CFTBL to King Kong)
   , m_cabinetOscillatorX(mass, 9.3f, 0.052f)
   , m_cabinetOscillatorY(mass, 5.8f, 0.055f)
{
   m_cabinetAcceleration.SetZero();
   m_cabinetPosition.SetZero();
}

void CabinetPhysics::StepOneMillisecond(const Vertex2D& force)
{
   constexpr float deltaTime = 0.001f;
   m_cabinetOscillatorX.StepOneMillisecond(force.x, deltaTime);
   m_cabinetOscillatorY.StepOneMillisecond(force.y, deltaTime);

   m_cabinetAcceleration.x = m_cabinetOscillatorX.GetAcceleration();
   m_cabinetAcceleration.y = m_cabinetOscillatorY.GetAcceleration();

   m_cabinetPosition.x = m_cabinetOscillatorX.GetDisplacement();
   m_cabinetPosition.y = m_cabinetOscillatorY.GetDisplacement();
}

}
