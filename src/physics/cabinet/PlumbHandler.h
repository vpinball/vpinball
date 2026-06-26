// license:GPLv3+
#pragma once

namespace VPX::Physics
{

class PlumbHandler final
{
public:
   PlumbHandler(const Settings& settings);
   ~PlumbHandler();

   void StepOneMillisecond(const Vertex2D& cabAcceleration); // m/s^2

   bool IsPlumbSimulated() const { return m_enablePlumbTilt; }
   void EnablePlumbSimulation(bool enable) { m_enablePlumbTilt = enable; }
   float GetPlumbPoleLength() const { return m_plumbPoleLength; } // m
   float GetPlumbTiltThreshold() const { return m_plumbTiltThreshold; } // rad
   void SetPlumbTiltThreshold(float v) { m_plumbTiltThreshold = v; } // rad
   float GetPlumbDamping() const;
   void SetPlumbDamping(float v);

   const Vertex3Ds& GetPlumbPos() const { return m_plumbPos; } // m
   const Vertex3Ds& GetPlumbVel() const { return m_plumbOmega; } // rad/s
   int GetPlumbTiltIndex() const { return m_plumbTiltIndex; }

private:
   // Config
   bool  m_enablePlumbTilt;
   float m_plumbPoleLength = 0.10f;    // meters
   float m_plumbAngularDamping0;       // linear angular damping, s^-1
   float m_plumbAngularDamping1;       // nonlinear angular damping, s^-1 per rad/s
   float m_plumbCabAccelScale = 1.0f;  // adjust coupling with cabinet acceleration
   float m_plumbTiltThreshold;         // rad (angle where the tilt circle sits)

   // State
   Vertex3Ds m_plumbPos; // always length = m_plumbPoleLength
   Vertex3Ds m_plumbOmega; // angular velocity in rad/s
   bool m_plumbTiltHigh = false;
   int m_plumbTiltIndex = 0;

   // Input system integration
   int m_plumbTiltInputSlot = -1;

   const float m_dampingCoef0 = 1.25f;
   const float m_dampingCoef1 = 0.75f;
};

}
