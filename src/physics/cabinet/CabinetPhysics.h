// license:GPLv3+

#pragma once

#include "math/vector.h"
#include "physics/cabinet/DampedHarmonicOscillator.h"


namespace VPX::Physics
{

// Cabinet physics model
//
// For the time being, we use a very basic 2D damped harmonic oscillator model (missing torque
// and therefore resulting in equivalent side nudge at top and bottom).
//
// Cabinet either moves freely (player resting is neglected) or forced when nudging.
// 
// For reference, a firm nudge should result in around 3 to 5mm cabinet displacement, resulting
// in a (initially static against metal) ball move more or less the double, so 6 to 10mm.
class CabinetPhysics
{
public:
   CabinetPhysics(float mass = 113.f);

   void StepOneMillisecond(const Vertex2D& force); // Free move under a given force (N)

   const Vertex2D& GetCabinetAcceleration() const { return m_cabinetAcceleration; } // m/s^2

   const Vertex2D& GetCabinetOffset() const { return m_cabinetPosition; } // m

   float GetMass() const { return m_mass; }

private:
   const float m_mass;

   DampedHarmonicOscillator m_cabinetOscillatorX;
   DampedHarmonicOscillator m_cabinetOscillatorY;

   Vertex2D m_cabinetAcceleration;
   Vertex2D m_cabinetPosition;
};

}
