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

   // The model we use seems to largely under estimate the displacement. We have a very simple model that
   // only consider leg elastic deformation, while there may be other factors that could lead to displacement:
   // - the slight feet displacement on the floor (sliding),
   // - the eventual subtle lift of the feet (cabinet slightly rocking),
   // - the leg bolt clearance (how stiff the feet are joint to the cabinet body),
   // - the cabinet slight deformation,
   // - the natural frequency of the cabinet that may lead to dynamic overshoot,
   // Beside, we are considering the cabinet as a separate 2D rectangular box, ignoring torque (needed for side nudge).
   // 
   // In the end we simply apply 'magic' correction factors for it to match reference videos and 'look good'
   m_cabinetPosition.x = m_cabinetOscillatorX.GetDisplacement() * 3.5f;
   m_cabinetPosition.y = m_cabinetOscillatorY.GetDisplacement() * 2.0f;
}

}
