// license:GPLv3+

#include "core/stdafx.h"
#include "PlumbHandler.h"

#include "core/player.h"
#include "core/VPApp.h"
#include "parts/pintable.h"
#include "physics/cabinet/NudgeHandler.h"


namespace VPX::Physics
{

PlumbHandler::PlumbHandler(const Settings& settings)
{
   m_enablePlumbTilt = settings.GetPlayer_SimulatedPlumb();
   m_plumbAngularDamping0 = m_dampingCoef0 * settings.GetPlayer_PlumbDamping();
   m_plumbAngularDamping1 = m_dampingCoef1 * settings.GetPlayer_PlumbDamping();
   m_plumbTiltThreshold = ANGTORAD(settings.GetPlayer_PlumbThresholdAngle());
   m_plumbPos.Set(0.f, 0.f, -m_plumbPoleLength);
   m_plumbOmega.SetZero();
}

PlumbHandler::~PlumbHandler()
{
}

float PlumbHandler::GetPlumbDamping() const
{
   return m_plumbAngularDamping0 / 2.5f;
   //return m_plumbAngularDamping1 / 1.5f;
}

void PlumbHandler::SetPlumbDamping(float v)
{
   m_plumbAngularDamping0 = m_dampingCoef0 * v;
   m_plumbAngularDamping1 = m_dampingCoef0 * v;
}

void PlumbHandler::StepOneMillisecond(const Vertex2D& cabAcceleration)
{
   // Up to VPX 10.8, mechanical tilt was handled in cvpmNudge defined either in core.vbs or in a NudgePlugIn_xxx defined by the user (so in VBS, with a low and unstable integration period, using very crude model)
   // This new implementation replaces it by a physics based model, with the following differences:
   // - use full VPX 10 nudge (include table velocity) instead of partial one (would say that this is a bug of previous implementation)
   // - simulate a simplified pendulum with 3 (simplified) forces: gravity, nudge and pole, and some velocity dampening
   // - send mechanical tilt like the plumb on real machine, triggering rom tile (instead of a fake keyboard central nudge like in previous implementation. not sure why previous implementation did that: isn't the point of a plumb to actually tilt ?)
   // Consequently NudgePlugIn_xxx scripts were also removed, support VBS script were also adapted to handle nudging and tilting separately.
   //
   // In 10.8.1 the cabinet physics was rewritten, using an angular pendulum model.
   
   if (!m_enablePlumbTilt || m_plumbTiltThreshold <= 0.0f)
      return;

   constexpr float dt = 0.001f;
   Vertex3Ds poleAxis = m_plumbPos / m_plumbPoleLength;

   // Adjust m_plumbCabAccelScale to adjust the coupling between the pendulum and the cabinet
   Vertex3Ds plumbAcc = {
      -cabAcceleration.x * m_plumbCabAccelScale, // Cabinet nudge (m/s^2, change of reference frame)
      -cabAcceleration.y * m_plumbCabAccelScale, // Cabinet nudge (m/s^2, change of reference frame)
      -9.80665f // Gravity (m/s^2)
   };

   // Torque per unit mass about pivot: tau = r x a
   const Vertex3Ds torque = CrossProduct(m_plumbPos, plumbAcc);

   // Angular acceleration for a point mass at distance L:
   // I = m L^2  => alpha = tau / L^2   (mass cancels out)
   Vertex3Ds alpha = torque / (m_plumbPoleLength * m_plumbPoleLength);

   // Nonlinear angular damping: alpha_damp = -omega * (c0 + c1 * |omega|)
   // c0 controls small oscillation decay
   // c1 prevents unrealistic large swings at high speed
   const float damping = m_plumbAngularDamping0 + m_plumbAngularDamping1 * m_plumbOmega.Length();
   alpha -= m_plumbOmega * damping;

   // Integrate angular velocity, keeping it orthogonal to rod axis (component along the rod is physically irrelevant spin and only causes drift)
   m_plumbOmega += alpha * dt;
   m_plumbOmega -= poleAxis * m_plumbOmega.Dot(poleAxis);

   // Advance bob position using rigid-body kinematics: r_dot = omega x r, ensuring the rod length
   m_plumbPos += CrossProduct(m_plumbOmega, m_plumbPos) * dt;
   const float posLen = m_plumbPos.Length();
   if (posLen > 1.0e-8f)
      m_plumbPos *= (m_plumbPoleLength / posLen);
   else
      m_plumbPos = { 0.0f, 0.0f, -m_plumbPoleLength };

   // Recompute axis after normalization and reproject omega after renormalization to remove numerical drift
   poleAxis = m_plumbPos / m_plumbPoleLength;
   m_plumbOmega -= poleAxis * m_plumbOmega.Dot(poleAxis);

   // Check if we hit the edge, using the pole angle (tilt threshold is 0..1000/1000 corresponding to 0..PI/4)
   const float psi = atan2f(sqrtf(m_plumbPos.x * m_plumbPos.x + m_plumbPos.y * m_plumbPos.y), -m_plumbPos.z);
   const float tiltAngle = m_plumbTiltThreshold;
   const float tiltPerc = 100.0f * psi / tiltAngle;
   bool tilted = false;
   if (tiltPerc > 100.0f)
   {
      tilted = true;
      // Keep plumb inside tile limits
      const float limitAngle = tiltAngle - 1e-3f;
      m_plumbPos.z = -m_plumbPoleLength * cosf(limitAngle);
      const float xy = m_plumbPoleLength * sinf(limitAngle);
      const float theta = atan2f(m_plumbPos.x, m_plumbPos.y);
      const Vertex3Ds axis(sinf(theta), cosf(theta), 0.f);
      m_plumbPos.x = xy * axis.x;
      m_plumbPos.y = xy * axis.y;
      // Bounce the plumb (reflect velocity against circle normal, dampen it)
      {
         // Linear velocity of bob
         const Vertex3Ds v = CrossProduct(m_plumbOmega, m_plumbPos);
         // Reflect against normal (poleAxis)
         const float dot = v.Dot(poleAxis);
         const Vertex3Ds vRef = v - 2.0f * dot * poleAxis;
         // Compute new angular velocity from reflected linear velocity and apply damping
         m_plumbOmega = CrossProduct(m_plumbPos, vRef) / (m_plumbPoleLength * m_plumbPoleLength);
         m_plumbOmega *= 0.8f; // magic damping factor
      }
   }

   PLOGD_IF(false) << std::format(
      ";{:8.5f};{:8.5f};{:8.5f};{:8.5f}", g_pplayer->m_pininput.m_nudgeHandler->GetCabinetAcceleration().y, m_plumbPos.x, m_plumbPos.y, psi * (float)(180. / M_PI));

   // Fire event (same as keyboard tilt)
   if (m_plumbTiltHigh != tilted)
   {
      m_plumbTiltHigh = tilted;
      if (tilted)
         m_plumbTiltIndex++;
      if (m_plumbTiltInputSlot == -1)
         m_plumbTiltInputSlot = g_pplayer->m_pininput.GetInputActions()[g_pplayer->m_pininput.GetTiltActionId()]->NewDirectStateSlot();
      g_pplayer->m_pininput.GetInputActions()[g_pplayer->m_pininput.GetTiltActionId()]->SetDirectState(m_plumbTiltInputSlot, m_plumbTiltHigh);
   }

   // Update player for diagnostic/table script visibility. Only update if input value is larger than what's there.
   // When the table script reads the values, they will reset to 0.
   if (tiltPerc > g_pplayer->m_ptable->m_tblNudgeReadTilt)
      g_pplayer->m_ptable->m_tblNudgeReadTilt = tiltPerc;
   if (fabsf(g_pplayer->m_pininput.m_nudgeHandler->GetCabinetAcceleration().x) > fabsf(g_pplayer->m_ptable->m_tblNudgeRead.x))
      g_pplayer->m_ptable->m_tblNudgeRead.x = g_pplayer->m_pininput.m_nudgeHandler->GetCabinetAcceleration().x;
   if (fabsf(g_pplayer->m_pininput.m_nudgeHandler->GetCabinetAcceleration().y) > fabsf(g_pplayer->m_ptable->m_tblNudgeRead.y))
      g_pplayer->m_ptable->m_tblNudgeRead.y = g_pplayer->m_pininput.m_nudgeHandler->GetCabinetAcceleration().y;
   if (fabsf(m_plumbPos.x) > fabsf(g_pplayer->m_ptable->m_tblNudgePlumb.x))
      g_pplayer->m_ptable->m_tblNudgePlumb.x = m_plumbPos.x;
   if (fabsf(m_plumbPos.y) > fabsf(g_pplayer->m_ptable->m_tblNudgePlumb.y))
      g_pplayer->m_ptable->m_tblNudgePlumb.y = m_plumbPos.y;
}

}
