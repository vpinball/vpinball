// license:GPLv3+

#include "core/stdafx.h"
#include "PlungerHandler.h"

#include "core/VPApp.h"

#include <algorithm>


///////////////////////////////////////////////////////////////////////////////////////////////////
// Plunger Handler
//
PlungerHandler::PlungerHandler(InputManager* inputManager)
{
   const Settings& settings = g_app->m_settings;

   m_isPullBackAndRetract = settings.GetPlayer_PlungerRetract();

   const int nSensors = settings.GetInput_PlungerSensorCount();
   for (int i = 0; i < nSensors; i++)
   {
      std::unique_ptr<PlungerSensor> sensor;
      AddSensor(sensor);
      sensor = std::make_unique<PlungerSensor>(inputManager);
      sensor->Load(settings, i);
      ReplaceSensor(i, std::move(sensor));
   }
}

void PlungerHandler::StepOneMillisecond()
{
   for (const auto& sensor : m_sensors)
      sensor->StepOneMillisecond();

   m_position = 0.f;
   m_rawVelocity = 0.f;

   for (const auto& sensor : m_sensors)
   {
      if (sensor->IsActive())
      {
         m_position = sensor->GetPosition();
         m_rawVelocity = sensor->GetRawVelocity();
         break;
      }
   }
}

bool PlungerHandler::HasPlungerSensor() const
{
   for (const auto& sensor : m_sensors)
      if (sensor->IsActive())
         return true;
   return false;
}

float PlungerHandler::GetRawVelocity() const
{
   return m_rawVelocity;
}

float PlungerHandler::GetHitVelocity(float restPos) const
{
   for (const auto& sensor : m_sensors)
      if (sensor->IsActive())
         return sensor->GetHitVelocity(restPos);
   return 0.f;
}

float PlungerHandler::GetPosition(float restPos) const
{
   // Symmetric calibration: 0 is rest, 1 is fully retracted, scale is symmetric along rest position
   // (The maximum forward position is not calibrated, in the negative side)
   // -1 maps to restPos - (1 - restPos)
   //  0 maps to restPos
   //  1 maps to 1
   // This lead to a range of [2 * restPos - 1, 1] for the plunger position, with 0 being the rest position.
   assert(restPos >= 0.f && restPos <= 1.f);
   return lerp(restPos, 1.f, m_position);
}

bool PlungerHandler::IsPullBackandRetract() const { return m_isPullBackAndRetract; }

void PlungerHandler::SetPullBackandRetract(bool isPullBackAndRetract) { m_isPullBackAndRetract = isPullBackAndRetract; }

int PlungerHandler::GetSensorCount() const { return static_cast<int>(m_sensors.size()); }

const std::unique_ptr<PlungerSensor>& PlungerHandler::GetSensor(int index) const { return m_sensors[index]; }

void PlungerHandler::AddSensor(std::unique_ptr<PlungerSensor>& sensor)
{
   const int sensorIndex = static_cast<int>(m_sensors.size());
   if (sensor)
   {
      sensor->Save(g_app->m_settings, sensorIndex);
      g_app->m_settings.SetInput_PlungerSensorCount(sensorIndex + 1, false);
      g_app->m_settings.Save();
   }
   m_sensors.push_back(std::move(sensor));
}

void PlungerHandler::ReplaceSensor(int index, std::unique_ptr<PlungerSensor> sensor) { m_sensors[index] = std::move(sensor); }

void PlungerHandler::RemoveSensor(int index)
{
   // We must shift all saved mappings with index greater than the suppressed one
   if (index < m_sensors.size() - 1)
   {
      for (size_t i = index + 1; i < m_sensors.size(); i++)
      {
         const auto prepend = std::format("Mapping.Plunger{}.", i);
         const auto prependLen = prepend.length();
         for (const auto id : Settings::GetRegistry().GetPropertyIds())
         {
            const VPX::Properties::PropertyDef* propDef = Settings::GetRegistry().GetProperty(id);
            if (propDef->m_groupId == "Input"s && propDef->m_propId.starts_with(prepend))
            {
               const string newId = std::format("Mapping.Plunger{}.{}", i - 1, propDef->m_propId.substr(prependLen));
               for (const auto idNew : Settings::GetRegistry().GetPropertyIds())
               {
                  const VPX::Properties::PropertyDef* propDef = Settings::GetRegistry().GetProperty(idNew);
                  if (propDef->m_groupId == "Input"s && propDef->m_propId == newId)
                  {
                     switch (Settings::GetRegistry().GetStoreType(propDef->m_type))
                     {
                     case VPX::Properties::PropertyRegistry::StoreType::Float: g_app->m_settings.Set(idNew, g_app->m_settings.GetFloat(id), false); break;
                     case VPX::Properties::PropertyRegistry::StoreType::Int: g_app->m_settings.Set(idNew, g_app->m_settings.GetInt(id), false); break;
                     case VPX::Properties::PropertyRegistry::StoreType::String: g_app->m_settings.Set(idNew, g_app->m_settings.GetString(id), false); break;
                     }
                     break;
                  }
               }
            }
         }
      }
   }
   m_sensors.erase(m_sensors.begin() + index);
   g_app->m_settings.SetInput_PlungerSensorCount(static_cast<int>(m_sensors.size()), false);
   g_app->m_settings.Save();
}

bool PlungerHandler::HasSensor(const std::unique_ptr<PlungerSensor>& sensor) const
{
   const bool sensorPosMapped = sensor->GetPositionSensor()->IsMapped();
   const bool sensorVelMapped = sensor->GetVelocitySensor()->IsMapped();
   for (const auto& existing : m_sensors)
   {
      const bool existingPosMapped = existing->GetPositionSensor()->IsMapped();
      const bool existingVelMapped = existing->GetVelocitySensor()->IsMapped();
      if (sensorPosMapped != existingPosMapped || sensorVelMapped != existingVelMapped)
         continue;
      if (sensorPosMapped && !sensor->GetPositionSensor()->GetMapping().IsSame(existing->GetPositionSensor()->GetMapping()))
         continue;
      if (sensorVelMapped && !sensor->GetVelocitySensor()->GetMapping().IsSame(existing->GetVelocitySensor()->GetMapping()))
         continue;
      return true;
   }
   return false;
}

bool PlungerHandler::IsMappedToDevice(uint16_t deviceId) const
{
   for (const auto& sensor : m_sensors)
      if ((sensor->GetPositionSensor()->IsMapped() && sensor->GetPositionSensor()->GetMapping().GetDeviceId() == deviceId)
         || (sensor->GetVelocitySensor()->IsMapped() && sensor->GetVelocitySensor()->GetMapping().GetDeviceId() == deviceId))
         return true;
   return false;
}

void PlungerHandler::UnmapDevice(uint16_t deviceId)
{
   bool removed = true;
   while (removed)
   {
      removed = false;
      for (size_t i = 0; i < m_sensors.size(); i++)
      {
         auto& sensor = m_sensors[i];
         if ((sensor->GetPositionSensor()->IsMapped() && sensor->GetPositionSensor()->GetMapping().GetDeviceId() == deviceId)
            || (sensor->GetVelocitySensor()->IsMapped() && sensor->GetVelocitySensor()->GetMapping().GetDeviceId() == deviceId))
         {
            RemoveSensor(static_cast<int>(i));
            removed = true;
            break;
         }
      }
   }
}

void PlungerHandler::SetExternalPlunger(bool enableOverride, const float velocity, const float displacement)
{
   // FIXME implement
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Plunger sensor
//
PlungerSensor::PlungerSensor(InputManager* inputManager)
   : m_positionSensor(std::make_unique<PhysicsSensor>(inputManager, "Plunger position sensor mapping", SensorMapping::Type::Position))
   , m_velocitySensor(std::make_unique<PhysicsSensor>(inputManager, "Plunger velocity sensor mapping", SensorMapping::Type::Velocity))
   , m_emaPosition(0.004f) // Time constant adjusted for default USB acquisition period of 125Hz and limited latency
   , m_emaVelocity(0.004f)
{
   PlungerKalmanFilter::Config config;

   config.positionMeasurementVariance = 1.0e-5f;
   config.accelerationProcessNoise = 0.5f;

   config.initialPositionVariance = 1.0e-4f;
   config.initialVelocityVariance = 1.0e-2f;

   config.enablePositionLimits = true;
   config.minPosition = -m_kalmanUnitScale / 3.f; // Fully extended
   config.maxPosition = m_kalmanUnitScale; // Fully retracted

   m_pvKalmanFilter.SetConfig(config);
   m_pvKalmanFilter.Reset(0.0f, 0.0f);
}

PlungerSensor::~PlungerSensor() { }

void PlungerSensor::Load(const Settings& settings, int sensorIndex)
{
   m_positionSensor->LoadMapping(settings, std::format("Plunger{}.Position", sensorIndex));
   m_velocitySensor->LoadMapping(settings, std::format("Plunger{}.Velocity", sensorIndex));
}

void PlungerSensor::Save(Settings& settings, int sensorIndex) const
{
   m_positionSensor->SaveMapping(settings, std::format("Plunger{}.Position", sensorIndex));
   m_velocitySensor->SaveMapping(settings, std::format("Plunger{}.Velocity", sensorIndex));
}

bool PlungerSensor::IsActive() const { return m_deactivationDelay > 0; }

void PlungerSensor::StepOneMillisecond()
{
   m_timeNs += 1000'000ull;

   if (m_deactivationDelay)
      m_deactivationDelay--;

   if (!m_positionSensor->IsMapped())
      return;

   // Detect rest state
   if (fabs(m_positionSensor->GetValue()) < 0.01f)
      m_nRestSamples++;
   else
      m_nRestSamples = 0;
   if (m_nRestSamples < 10)
      m_deactivationDelay = 10000; // Activate for the next 10s after a movement
   const bool isPlungerAtRest = m_nRestSamples > 30;

   // Position evaluation
   // 
   // We recreate a continuous 1ms position estimation from irregular sensor sampling, usually done at default USB rate (125Hz) 
   // with repeated report when not moving or not, and eventual custom processing (for example Pinscape will report continuously 
   // the first value it sees when crossing from retracted to extended during 75ms...). To do so, we use a simple Kalman state 
   // estimator that continuously predicts the position and velocity, and is updated when a new sample is acquired.
   if (uint64_t timestampNS = m_positionSensor->GetMapping().GetRawValueTimestampNs() + m_clockDeltaNs; timestampNS > m_lastTimestampNs)
   {
      if (timestampNS > m_timeNs)
      {
         // Acquisition is evaluated to be in the future of the Kalman filter master clock, so we realign the sensor clock to the Kalman filter clock
         timestampNS = m_timeNs;
         m_clockDeltaNs = static_cast<int64_t>(timestampNS) - static_cast<int64_t>(m_positionSensor->GetMapping().GetRawValueTimestampNs());
      }
      m_lastTimestampNs = timestampNS;
      m_pvKalmanFilter.UpdatePosition(m_lastTimestampNs, m_positionSensor->GetValue() * m_kalmanUnitScale);
   }
   else if (m_timeNs > m_lastTimestampNs + 10'000'000ull)
   {
      // Don't let the estimator runs freely too long by periodically repeating last sample
      m_lastTimestampNs = m_timeNs - 5'000'000ull;
      m_pvKalmanFilter.UpdatePosition(m_lastTimestampNs, m_positionSensor->GetValue() * m_kalmanUnitScale);
   }
   m_pvKalmanFilter.PredictTo(m_timeNs);
   if (isPlungerAtRest)
      m_pvKalmanFilter.UpdateVelocityZero(1.0e-4f);

   m_emaVelocity.Update(m_pvKalmanFilter.GetVelocity() / m_kalmanUnitScale, 0.001f);
   m_position = m_emaPosition.Update(m_pvKalmanFilter.GetPosition() / m_kalmanUnitScale, 0.001f);
   m_prevPosition[m_PrevPositionPos] = m_position;
   m_PrevPositionPos = (m_PrevPositionPos + 1) % m_prevPosition.size();

   // Log for debugging purposes as CSV
   if (false && IsActive())
   {
      const float restPos = 0.f;
      const float releaseApex = *std::max_element(m_prevPosition.begin(), m_prevPosition.end());
      const float hitSpeed = (m_position >= (0.5f + 0.5f * restPos) || m_emaVelocity.Get() >= 0.f) ? 0.f : -max(0.f, releaseApex - restPos) * 100.f / 13.0f;
      PLOGD << std::format(";{:8.5f};{:8.5f};{:8.5f};{:8.5f};{:8.5f};{};{}", //
         m_positionSensor->GetValue(), m_velocitySensor->GetValue(), // Sensors
         m_position, // Estimated position
         m_emaVelocity.Get(), hitSpeed, // Estimated speeds (raw and hit speed)
         (int)(m_timeNs / 1000'000ull), (int)(m_lastTimestampNs / 1000'000ull));
   }
}

// Hit velocity evaluation
//
// The position state estimator allows to derive a continuous velocity, but we are only interested by the velocity when the plunger
// hit the ball which may not happen most of the time.
// 
// When we only have a position sensor, the acquired release motion of a physical plunger is much faster than our sampling rate can
// keep up with, so we can't just use the joystick readings directly.  The problem is that a real plunger can shoot all the way forward,
// bounce all the way back, and shoot forward again in the time between two consecutive samples.  A real plunger moves at around 3-5m/s,
// which translates to 3-5mm/ms, or 30-50mm per 10ms sampling period.  The whole plunger travel distance is ~65mm. So in one reading,
// we can travel almost the whole range!  This means that samples are effectively random during a release motion.  We might happen to get
// lucky and have our sample timing align perfectly with a release, so that we get one reading at the retracted position just before a
// release and the very next reading at the full forward position.  Or we might get unlikely and catch one reading halfway down the initial
// lunge and the next reading at the very apex of the bounce back - and if we took those two readings at face value, we'd be fooled into
// thinking the plunger was stationary at the halfway point!
// 
// Luckily, we only need to evaluate speed when plunger actually hit the ball. This happens in 2 situations:
// . plunger is at rest and ball either is stationary (ball rest on plunger) or hits the plunger (ball fall from the plunger lane on the plunger)
// . plunger moves and hits the ball
// 
// We can use the same strategy for both these situations:
// . either a custom hardware is measuring velocity at a high rate and providing us with the velocity and we can just use it (for example
//   Pinscape that provides either the speed of the plunger after a release when it crosses the park zone, or a stable rest state speed)
// . either we compute a velocity from the position where the plunger was released if any, leading to either no velocity or a peak standard
//   speed, matching the one we would have obtained from a keyboard plunger
float PlungerSensor::GetHitVelocity(float restPos) const
{
   if (m_velocitySensor->IsMapped())
   {
      return m_velocitySensor->GetValue();
   }
   else
   {
      // Use the position & speed derived from the position sensor to filter out rest state from plunges
      if (m_position >= (0.5f + 0.5f * restPos) || m_emaVelocity.Get() >= 0.f)
         return 0.f;

      // Evaluate the velocity by supposing the player did a 'free' release, that is to say the plunger was retracted to a given position then entirely free to move.
      // 
      // In this scheme the speed is a consequence of 2 parameters (beside physical constants like spring stiffness, damping,...):
      // . the retracted position when the plunger was released (apex). Measures on real hardware show that the the retracted to extended move last around 50ms, and that the next
      //   oscillation to extended position happens roughly 100ms after the first. This lead to search the apex within the last 100ms
      // . the actual hit position that we guess to be at the park position
      // So we figure the release speed as a fraction of the fire speed property, linearly proportional to the starting distance.
      // The 100/13 factor is a magic conversion factor that matches what is used for keyboard driven plunger.
      const float releaseApex = *std::max_element(m_prevPosition.begin(), m_prevPosition.end());
      const float hitSpeed = - max(0.f, releaseApex - restPos) * 100.f / 13.0f;
      return hitSpeed;
   }
}
