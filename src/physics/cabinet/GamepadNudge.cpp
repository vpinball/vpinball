// license:GPLv3+

#include "core/stdafx.h"
#include "GamepadNudge.h"

#include "core/VPApp.h"
#include "input/PhysicsSensor.h"
#include "physics/cabinet/NudgeIntentHandler.h"

namespace VPX::Physics
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Intent sensor
//
// The sensor is used to acquire the player's intent to nudge (strength and direction) instead of
// the actual cabinet movement. This intent is then applied to a cabinet physic model. This allows
// stable results on most hardwares/sensors (gamepad, desktop, cabinets, VR stub,... providing
// position, velocity or acceleration). The drawback is that it only models 'impulse' nudging.
//
// Cabinet physics model is a simplified 2D second order oscillator fitted on real machine behavior.
// Measurements show that modern (1990-2020) real pinball machines, weighting around 113kg,
// oscillate at around 11Hz for a side nudge and 5.5Hz for a front nudge. Damping has also been
// fitted to correspond to the observed decay of oscillation amplitude.
//
// Older machines used to be fairly lighter and the model could be improved for these.
//
// Player intent is evaluated using a high pass filter on the acquired sensor value, and only react
// on the inversion point of the filtered signal, considering it as the impulse that feeds the
// model. As the expected natural oscillation is between 3..15Hz, the filter has a cut frequency of
// 20Hz. The strength is derived from the amplitude at the inversion point (depending on the sensor type).

GamepadNudge::GamepadNudge(InputManager* inputManager)
   : m_xSensor(inputManager, "Side nudge sensor mapping", SensorMapping::Type::Acceleration)
   , m_ySensor(inputManager, "Front nudge sensor mapping", SensorMapping::Type::Acceleration)
   , m_nudgeIntentHandler(true)
{
}

GamepadNudge::~GamepadNudge() { }

void GamepadNudge::Load(const Settings& settings, int sensorIndex)
{
   auto strengthPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Strength", sensorIndex));
   assert(strengthPropId.has_value());
   SetStrengthScale(settings.GetFloat(strengthPropId.value()));

   m_xSensor.LoadMapping(settings, std::format("Nudge{}.X", sensorIndex));
   m_ySensor.LoadMapping(settings, std::format("Nudge{}.Y", sensorIndex));
}

void GamepadNudge::Save(Settings& settings, int sensorIndex) const
{
   auto typePropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Type", sensorIndex));
   assert(typePropId.has_value());
   settings.Set(typePropId.value(), 0, false);

   auto strengthPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Strength", sensorIndex));
   assert(strengthPropId.has_value());
   settings.Set(strengthPropId.value(), m_nudgeStrengthScale, false);

   m_xSensor.SaveMapping(settings, std::format("Nudge{}.X", sensorIndex));
   m_ySensor.SaveMapping(settings, std::format("Nudge{}.Y", sensorIndex));
}

bool GamepadNudge::IsSameSensor(const std::unique_ptr<NudgeSensor>& sensor) const
{
   if (const GamepadNudge* gamepadSensor = dynamic_cast<const GamepadNudge*>(sensor.get()); gamepadSensor)
   {
      for (int i = 0; i < 2; i++)
      {
         const PhysicsSensor& sensorAxis = (i == 0) ? gamepadSensor->m_xSensor : gamepadSensor->m_ySensor;
         const PhysicsSensor& thisAxis = (i == 0) ? m_xSensor : m_ySensor;
         if (sensorAxis.IsMapped() != thisAxis.IsMapped())
            return false;
         if (sensorAxis.IsMapped() && !sensorAxis.GetMapping().IsSame(thisAxis.GetMapping()))
            return false;
      }
      return true;
   }
   return false;
}

bool GamepadNudge::IsMappedToDevice(uint16_t deviceId) const
{
   for (const PhysicsSensor* thisAxis : { &m_xSensor, &m_ySensor })
      if (thisAxis->IsMapped() && thisAxis->GetMapping().GetDeviceId() == deviceId)
         return true;
   return false;
}

string GamepadNudge::GetDisplayName() const
{
   if (g_pplayer == nullptr)
      return "Gamepad nudge sensor";
   else if (GetXSensor().IsMapped())
      return std::format("{} (Gamepad nudge){}", g_pplayer->m_pininput.GetDeviceName(GetXSensor().GetMapping().GetDeviceId()),
         g_pplayer->m_pininput.IsDeviceConnected(GetXSensor().GetMapping().GetDeviceId()) ? "" : " [disconnected]");
   else if (GetYSensor().IsMapped())
      return std::format("{} (Gamepad nudge){}", g_pplayer->m_pininput.GetDeviceName(GetYSensor().GetMapping().GetDeviceId()),
         g_pplayer->m_pininput.IsDeviceConnected(GetYSensor().GetMapping().GetDeviceId()) ? "" : " [disconnected]");
   else
      return "Unmapped gamepad nudge sensor";
}

bool GamepadNudge::IsActive() const { return m_deactivationDelay > 0; }

void GamepadNudge::StepOneMillisecond()
{
   // Convert stick position to acceleration
   // 12m/s^2 for firm front nudge, resulting in a 5mm cabinet move, side nudge needs a somewhat higher energy for the same move
   // (Hacky empirical balancing of front vs side energy, needs some more physics study to validate this)
   const float xSensor = m_xSensor.GetValue() * m_nudgeStrengthScale * 16.f;
   const float ySensor = m_ySensor.GetValue() * m_nudgeStrengthScale * 12.f;

   static bool m_isImpulseInProgress = false;
   static Vertex2D m_initialCabinetPosition;
   m_nudgeIntentHandler.StepOneMillisecond({ xSensor, ySensor });
   if (m_nudgeIntentHandler.IsImpulseInProgress())
   {
      if (!m_isImpulseInProgress)
         m_initialCabinetPosition = m_cabinetModel.GetCabinetOffset();
      m_isImpulseInProgress = true;
      m_cabinetModel.StepOneMillisecond(m_cabinetModel.GetMass() * m_nudgeIntentHandler.GetImpulseAceleration());
      m_deactivationDelay = 10000;
   }
   else
   {
      m_isImpulseInProgress = false;
      m_cabinetModel.StepOneMillisecond({ 0.f, 0.f });
      if (m_deactivationDelay > 0)
         m_deactivationDelay--;
   }

   // Log for debugging purposes as CSV: Sensor;Intent acceleration (m/s^2);Cab acceleration (m/s^2);Cab position (mm)
   PLOGD_IF(false) << std::format(";{:8.5f};{:8.5f};{:8.5f};{:8.5f}", m_ySensor.GetValue(), m_nudgeIntentHandler.GetImpulseAceleration().y, m_cabinetModel.GetCabinetAcceleration().y,
      m_cabinetModel.GetCabinetOffset().y * 1000.f);
   /* static float maxDisp = 0.f;
   if (m_cabinetModel.GetCabinetOffset().y > maxDisp)
   {
      maxDisp = m_cabinetModel.GetCabinetOffset().y;
      PLOGD << "Max cab disp: " << m_cabinetModel.GetCabinetOffset().y * 1000.f << "mm";
   }*/
}

const Vertex2D& GamepadNudge::GetCabinetAcceleration() const { return m_cabinetModel.GetCabinetAcceleration(); }

const Vertex2D& GamepadNudge::GetCabinetOffset() const { return m_cabinetModel.GetCabinetOffset(); }

}
