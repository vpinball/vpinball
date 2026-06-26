// license:GPLv3+

#include "core/stdafx.h"
#include "CabinetNudgeSensor.h"

#include "input/PhysicsSensor.h"

namespace VPX::Physics
{

CabinetNudgeSensor::CabinetNudgeSensor(InputManager* inputManager)
   : m_xGainCalibrator(MotionGainCalibratorAxis::Config())
   , m_yGainCalibrator(MotionGainCalibratorAxis::Config())
   , m_xVelSensor(inputManager, "Side nudge velocity sensor", SensorMapping::Type::Velocity)
   , m_yVelSensor(inputManager, "Front nudge velocity sensor", SensorMapping::Type::Velocity)
   , m_xAccSensor(inputManager, "Side nudge acceleration sensor", SensorMapping::Type::Acceleration)
   , m_yAccSensor(inputManager, "Front nudge acceleration sensor", SensorMapping::Type::Acceleration)
   , m_kalmanX(MotionKalmanAxis::Config())
   , m_kalmanY(MotionKalmanAxis::Config())
   , m_emaX(0.004f) // Time constant adjusted for default USB acquisition period at 8.125ms and limited latency
   , m_emaY(0.004f)
{
   m_cabinetAcceleration.SetZero();
}

CabinetNudgeSensor::~CabinetNudgeSensor() { }

void CabinetNudgeSensor::Load(const Settings& settings, int sensorIndex)
{
   auto typePropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Type", sensorIndex));
   assert(typePropId.has_value());
   assert(settings.GetInt(typePropId.value()) == 1 || settings.GetInt(typePropId.value()) == 2);
   SetIntentSensor(settings.GetInt(typePropId.value()) == 1);

   auto strengthPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Strength", sensorIndex));
   assert(strengthPropId.has_value());
   SetStrengthScale(settings.GetFloat(strengthPropId.value()));

   auto cabWeightPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.CabWeight", sensorIndex));
   assert(cabWeightPropId.has_value());
   SetCabinetMass(settings.GetFloat(cabWeightPropId.value()));

   m_xVelSensor.m_sensor.LoadMapping(settings, std::format("Nudge{}.VelX", sensorIndex));
   m_yVelSensor.m_sensor.LoadMapping(settings, std::format("Nudge{}.VelY", sensorIndex));

   m_xAccSensor.m_sensor.LoadMapping(settings, std::format("Nudge{}.AccX", sensorIndex));
   m_yAccSensor.m_sensor.LoadMapping(settings, std::format("Nudge{}.AccY", sensorIndex));
}

void CabinetNudgeSensor::Save(Settings& settings, int sensorIndex) const
{
   auto typePropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Type", sensorIndex));
   assert(typePropId.has_value());
   settings.Set(typePropId.value(), IsIntentSensor() ? 1 : 2, false);

   auto strengthPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Strength", sensorIndex));
   assert(strengthPropId.has_value());
   settings.Set(strengthPropId.value(), GetStrengthScale(), false);

   auto cabWeightPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.CabWeight", sensorIndex));
   assert(cabWeightPropId.has_value());
   settings.Set(cabWeightPropId.value(), GetCabinetMass(), false);

   m_xVelSensor.m_sensor.SaveMapping(settings, std::format("Nudge{}.VelX", sensorIndex));
   m_yVelSensor.m_sensor.SaveMapping(settings, std::format("Nudge{}.VelY", sensorIndex));

   m_xAccSensor.m_sensor.SaveMapping(settings, std::format("Nudge{}.AccX", sensorIndex));
   m_yAccSensor.m_sensor.SaveMapping(settings, std::format("Nudge{}.AccY", sensorIndex));
}

bool CabinetNudgeSensor::IsSameSensor(const std::unique_ptr<NudgeSensor>& sensor) const
{
   if (const CabinetNudgeSensor* cabSensor = dynamic_cast<const CabinetNudgeSensor*>(sensor.get()); cabSensor)
   {
      for (int i = 0; i < 4; i++)
      {
         const PhysicsSensor& sensorAxis = (i == 0) ? cabSensor->m_xVelSensor.m_sensor
            : (i == 1)                              ? cabSensor->m_yVelSensor.m_sensor
            : (i == 2)                              ? cabSensor->m_xAccSensor.m_sensor
                                                    : cabSensor->m_yAccSensor.m_sensor;
         const PhysicsSensor& thisAxis = (i == 0) ? m_xVelSensor.m_sensor : (i == 1) ? m_yVelSensor.m_sensor : (i == 2) ? m_xAccSensor.m_sensor : m_yAccSensor.m_sensor;
         if (sensorAxis.IsMapped() != thisAxis.IsMapped())
            return false;
         if (sensorAxis.IsMapped() && !sensorAxis.GetMapping().IsSame(thisAxis.GetMapping()))
            return false;
      }
      return true;
   }
   return false;
}

bool CabinetNudgeSensor::IsMappedToDevice(uint16_t deviceId) const
{
   for (const PhysicsSensor* thisAxis : { &m_xVelSensor.m_sensor, &m_yVelSensor.m_sensor, &m_xAccSensor.m_sensor, &m_yAccSensor.m_sensor })
      if (thisAxis->IsMapped() && thisAxis->GetMapping().GetDeviceId() == deviceId)
         return true;
   return false;
}

string CabinetNudgeSensor::GetDisplayName() const
{
   if (g_pplayer == nullptr)
      return "Cabinet nudge sensor";
   else if (m_xAccSensor.m_sensor.IsMapped())
      return std::format("{} (Cabinet {} nudge){}", g_pplayer->m_pininput.GetDeviceName(m_xAccSensor.m_sensor.GetMapping().GetDeviceId()), IsIntentSensor() ? "intent" : "direct",
         g_pplayer->m_pininput.IsDeviceConnected(m_xAccSensor.m_sensor.GetMapping().GetDeviceId()) ? "" : " [disconnected]");
   else if (m_yAccSensor.m_sensor.IsMapped())
      return std::format("{} (Cabinet {} nudge){}", g_pplayer->m_pininput.GetDeviceName(m_yAccSensor.m_sensor.GetMapping().GetDeviceId()), IsIntentSensor() ? "intent" : "direct",
         g_pplayer->m_pininput.IsDeviceConnected(m_yAccSensor.m_sensor.GetMapping().GetDeviceId()) ? "" : " [disconnected]");
   else if (m_xVelSensor.m_sensor.IsMapped())
      return std::format("{} (Cabinet {} nudge){}", g_pplayer->m_pininput.GetDeviceName(m_xVelSensor.m_sensor.GetMapping().GetDeviceId()), IsIntentSensor() ? "intent" : "direct",
         g_pplayer->m_pininput.IsDeviceConnected(m_xVelSensor.m_sensor.GetMapping().GetDeviceId()) ? "" : " [disconnected]");
   else if (m_yVelSensor.m_sensor.IsMapped())
      return std::format("{} (Cabinet {} nudge){}", g_pplayer->m_pininput.GetDeviceName(m_yVelSensor.m_sensor.GetMapping().GetDeviceId()), IsIntentSensor() ? "intent" : "direct",
         g_pplayer->m_pininput.IsDeviceConnected(m_yVelSensor.m_sensor.GetMapping().GetDeviceId()) ? "" : " [disconnected]");
   else
      return "Unmapped cabinet nudge sensor";
}

bool CabinetNudgeSensor::IsIntentSensor() const { return m_nudgeIntentHandler != nullptr; }

void CabinetNudgeSensor::SetIntentSensor(bool isIntent)
{
   if (isIntent == IsIntentSensor())
      return;
   m_nudgeIntentHandler = isIntent ? std::make_unique<NudgeIntentHandler>(false) : nullptr;
}

bool CabinetNudgeSensor::IsActive() const { return m_deactivationDelay > 0; }

void CabinetNudgeSensor::UpdateAxisSensor(SyncedSensor& sensor, MotionKalmanAxis& axis, float axisGain)
{
   if (!sensor.m_sensor.IsMapped())
   {
      sensor.m_restCount = 10000;
      return;
   }

   const float restThresold = (sensor.m_sensor.GetType() == SensorMapping::Type::Acceleration) ? 0.020f : 0.002f;
   if (abs(sensor.m_sensor.GetValue()) < restThresold)
   {
      sensor.m_restCount++;
      if (sensor.m_forceRest)
         return;
   }
   else
   {
      sensor.m_restCount = 0;
      sensor.m_forceRest = false;
      m_deactivationDelay = 10000;
   }

   const uint64_t timestampNS = sensor.m_sensor.GetMapping().GetRawValueTimestampNs();
   if (timestampNS == 0 || timestampNS == sensor.m_lastTimestampNs)
      return;
   sensor.m_lastTimestampNs = timestampNS;
   const uint64_t timestampUS = timestampNS / 1000;
   uint64_t alignedTimestampUS = timestampUS + sensor.m_clockDeltaUs;
   if (alignedTimestampUS > m_timeUs)
   {
      // Acquisition is evaluated to be in the future of the Kalman filter master clock, so we realign the sensor clock to the Kalman filter clock
      sensor.m_clockDeltaUs = static_cast<int64_t>(m_timeUs) - static_cast<int64_t>(timestampUS);
      alignedTimestampUS = m_timeUs;
   }

   if (sensor.m_sensor.GetType() == SensorMapping::Type::Velocity)
   {
      axis.UpdateVelocity(alignedTimestampUS, axisGain * sensor.m_sensor.GetValue());
   }
   else if (sensor.m_sensor.GetType() == SensorMapping::Type::Acceleration)
   {
      axis.UpdateAcceleration(alignedTimestampUS, axisGain * sensor.m_sensor.GetValue());
   }
}

void CabinetNudgeSensor::UpdateAxis(SyncedSensor& velSensor, SyncedSensor& accSensor, MotionKalmanAxis& kalmanFilter, MotionGainCalibratorAxis& gainCalibrator)
{
   constexpr float gainConfidenceThreshold = 0.5f;

   // Push samples in chronological order if both velocity and acceleration sensors are mapped, to ensure proper synchronization for gain calibration. If only one of them is mapped, just update it.
   const bool isAccAndVelMapped = accSensor.m_sensor.IsMapped() && velSensor.m_sensor.IsMapped();
   if (isAccAndVelMapped)
   {
      if ((gainCalibrator.GetGlobalConfidence() < gainConfidenceThreshold) || (gainCalibrator.GetGain() < 0.01f))
      {
         UpdateAxisSensor(accSensor, kalmanFilter, 1.f);
         velSensor.m_restCount = accSensor.m_restCount;
      }
      else if (accSensor.m_sensor.GetMapping().GetRawValueTimestampNs() < velSensor.m_sensor.GetMapping().GetRawValueTimestampNs())
      {
         UpdateAxisSensor(accSensor, kalmanFilter, 1.f);
         UpdateAxisSensor(velSensor, kalmanFilter, 1.f / gainCalibrator.GetGain());
      }
      else
      {
         UpdateAxisSensor(velSensor, kalmanFilter, 1.f / gainCalibrator.GetGain());
         UpdateAxisSensor(accSensor, kalmanFilter, 1.f);
      }
   }
   else
   {
      UpdateAxisSensor(accSensor, kalmanFilter, 1.f);
      UpdateAxisSensor(velSensor, kalmanFilter, 1.f);
   }

   // Detect rest to calibrate bias (enter after a delay, exit immediately upon motion detection).
   constexpr int crossRestCountThreshold = 75; // We are entering a rest state, try to do it when crossing for a cleaner output
   accSensor.m_forceRest |= (accSensor.m_restCount > crossRestCountThreshold) && (accSensor.m_lastValue * accSensor.m_sensor.GetValue() < 0.f);
   velSensor.m_forceRest |= (velSensor.m_restCount > crossRestCountThreshold) && (velSensor.m_lastValue * velSensor.m_sensor.GetValue() < 0.f);
   constexpr int forceRestCountThreshold
      = crossRestCountThreshold + 200; // We waited long enough in a rest state without crossing, directly enforce rest calibration (causing a little step beside derivatives)
   const bool isRest = (accSensor.m_forceRest || accSensor.m_restCount > forceRestCountThreshold) && (velSensor.m_forceRest || velSensor.m_restCount > forceRestCountThreshold);
   if (isRest)
   {
      velSensor.m_forceRest = true;
      accSensor.m_forceRest = true;
      kalmanFilter.UpdateRestConstraints(m_timeUs);
   }
   accSensor.m_lastValue = accSensor.m_sensor.GetValue();
   velSensor.m_lastValue = velSensor.m_sensor.GetValue();

   // Also use rest condition to identify nudge 'segments' that are used for gain calibration between acceleration and velocity channels.
   if (isAccAndVelMapped)
   {
      if (!isRest)
      {
         if (!gainCalibrator.IsSegmentActive())
            gainCalibrator.StartSegment(m_timeUs);
         gainCalibrator.AddSample(m_timeUs, velSensor.m_sensor.GetValue(), accSensor.m_sensor.GetValue());
      }
      else if (gainCalibrator.IsSegmentActive())
      {
         gainCalibrator.EndSegment();
         if (!m_accVelGainLog && gainCalibrator.GetGlobalConfidence() >= gainConfidenceThreshold)
         {
            m_accVelGainLog = true;
            PLOGI << std::format(
               "Velocity/Acceleration initial gain calibrated to {:5.3f} (confidence: {:5.3f}), dual sensor activated", gainCalibrator.GetGain(), gainCalibrator.GetGlobalConfidence());
         }
      }
   }

   kalmanFilter.PredictTo(m_timeUs);
}

void CabinetNudgeSensor::StepOneMillisecond()
{
   if (m_deactivationDelay > 0)
      m_deactivationDelay--;

   m_timeUs += 1000;

   // Apply Kalman filter and rest detection for bias & gain calibration
   UpdateAxis(m_xVelSensor, m_xAccSensor, m_kalmanX, m_xGainCalibrator);
   UpdateAxis(m_yVelSensor, m_yAccSensor, m_kalmanY, m_yGainCalibrator);

   // Note that sensors are expected to return value in m/s^2 or m/s (user may adjust scale in the sensor mapping to guarantee that)

   if (m_nudgeIntentHandler)
   {
      // Hacky empirical balancing of front vs side energy, needs some more physics study to validate this
      m_nudgeIntentHandler->StepOneMillisecond(Vertex2D(m_kalmanX.GetAcceleration() * (float)(4. / 3.), m_kalmanY.GetAcceleration()));

      if (m_nudgeIntentHandler->IsImpulseInProgress())
         m_cabinetModel.StepOneMillisecond(m_cabinetModel.GetMass() * m_nudgeIntentHandler->GetImpulseAceleration());
      else
         m_cabinetModel.StepOneMillisecond({ 0.f, 0.f });

      m_cabinetAcceleration = m_cabinetModel.GetCabinetAcceleration();

      // Log for debugging purposes as CSV: Time;Sensor acceleration;Kalman acceleration;Intent nudge;Cab acceleration;Cab position
      PLOGD_IF(false) << std::format(";{:6d};{:8.5f};{:8.5f};{:8.5f};{:8.5f};{:8.5f}", m_timeUs / 1000, m_yAccSensor.m_sensor.GetValue(), m_kalmanY.GetAcceleration(),
         m_nudgeIntentHandler->GetImpulseAceleration().y, m_cabinetAcceleration.y, m_cabinetModel.GetCabinetOffset().y);
   }
   else
   {
      // Apply an exponential moving average to the Kalman acceleration output to smooth out the cabinet force applied to the ball (as the Kalman acceleration state does not smooth between aquired values)
      m_emaX.Update(m_kalmanX.GetAcceleration(), 0.001f);
      m_emaY.Update(m_kalmanY.GetAcceleration(), 0.001f);
      m_cabinetAcceleration.x = m_emaX.Get() * m_nudgeStrengthScale;
      m_cabinetAcceleration.y = m_emaY.Get() * m_nudgeStrengthScale;

      // The acquired acceleration correspond to a force F = a.m where m is the real world cabinet mass. The acceleration computed here is the
      // one of the **virtual** cabinet which has a different mass corresponding to the simulated table.
      m_cabinetAcceleration *= m_nudgeStrengthScale * m_cabinetMass / m_cabinetModel.GetMass();

      // Since the position state in the Kalman filter is not good enough (double integration resulting in drift + snap when enforcing rest conditions),
      // we use the cabinet model directly driven by Kalman acceleration for visual feedback
      m_cabinetModel.StepOneMillisecond(m_cabinetModel.GetMass() * m_cabinetAcceleration);

      // Log for debugging purposes as CSV: Time;Sensor acceleration;Kalman acceleration;Cab acceleration;Cab position
      PLOGD_IF(false) << std::format(";{:6d};{:8.5f};{:8.5f};{:8.5f};{:8.5f}", m_timeUs / 1000, m_yAccSensor.m_sensor.GetValue(), m_kalmanY.GetAcceleration(), m_cabinetAcceleration.y,
         m_cabinetModel.GetCabinetOffset().y);
   }
}

const Vertex2D& CabinetNudgeSensor::GetCabinetAcceleration() const { return m_cabinetAcceleration; }

const Vertex2D& CabinetNudgeSensor::GetCabinetOffset() const { return m_cabinetModel.GetCabinetOffset(); }

}
