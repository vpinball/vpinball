// license:GPLv3+

#include "core/stdafx.h"
#include "PlungerHandler.h"

#include "core/VPApp.h"


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

   for (const auto& sensor : m_sensors)
   {
      if (sensor->IsActive())
      {
         m_position = sensor->GetPosition();
         m_velocity = sensor->GetVelocity();
         m_isLinear = sensor->IsLinear();
         m_hasVelocity = sensor->HasVelocity();
         break;
      }
   }
}

float PlungerHandler::GetVelocity() const { return m_velocity; }

float PlungerHandler::GetPosition() const { return m_position; }

bool PlungerHandler::IsLinear() const { return m_isLinear; }

bool PlungerHandler::HasVelocity() const { return m_hasVelocity; }

bool PlungerHandler::IsPullBackandRetract() const { return m_isPullBackAndRetract; }

void PlungerHandler::SetPullBackandRetract(bool isPullBackAndRetract) { m_isPullBackAndRetract = isPullBackAndRetract; }

int PlungerHandler::GetSensorCount() const { return static_cast<int>(m_sensors.size()); }

const std::unique_ptr<PlungerSensor>& PlungerHandler::GetSensor(int index) const { return m_sensors[index]; }

void PlungerHandler::AddSensor(std::unique_ptr<PlungerSensor>& sensor)
{
   const int sensorIndex = static_cast<int>(m_sensors.size());

   const auto linearPropId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>("Input"s, std::format("Mapping.Plunger{}.Linear", sensorIndex),
      "Linear Sensor"s,
      "Select between symmetric (linear) and asymetric (legacy) sensor\nThe plunger used to be calibrated differently for pull and push. This is largely deprecated and only kept for backwards compatibility."s,
      false, true));

   const auto posFilterPropId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>(
      "Input"s, std::format("Mapping.Plunger{}.PosFilter", sensorIndex), "Filter position"s, "Apply a denoising filter on position sensor values."s, false, true));

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
// 4th-order Butterworth low-pass filter, 10 Hz passband, 1 kHz sampling rate
// Implemented as Direct Form II Transposed for numerical stability
//
class PlungerPositionFilter final
{
public:
   float Push(float x)
   {
      const float y = a[0] * x + s[0];
      for (int i = 0; i < Order - 1; ++i)
         s[i] = a[i + 1] * x - b[i + 1] * y + s[i + 1];
      s[Order - 1] = a[Order] * x - b[Order] * y;
      return y;
   }

   void Reset() { std::fill(std::begin(s), std::end(s), 0.f); }

private:
   static constexpr int Order = 4;

   // Butterworth low-pass, Fc=10Hz, Fs=1000Hz
   // Generated with: scipy.signal.butter(4, 10, fs=1000, output='ba')
   static constexpr float a[Order + 1] = { 0.0048243445f, 0.019297378f, 0.028946068f, 0.019297378f, 0.0048243445f };
   static constexpr float b[Order + 1] = { 1.00000000f, -2.369513f, 2.3139884f, -1.0546654f, 0.1873795f };

   float s[Order] = {}; // delay state (N slots)
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Plunger sensor
//
PlungerSensor::PlungerSensor(InputManager* inputManager)
   : m_positionSensor(std::make_unique<PhysicsSensor>(inputManager, "Plunger position sensor mapping", SensorMapping::Type::Position))
   , m_velocitySensor(std::make_unique<PhysicsSensor>(inputManager, "Plunger velocity sensor mapping", SensorMapping::Type::Velocity))
{
}

PlungerSensor::~PlungerSensor() { }

void PlungerSensor::Load(const Settings& settings, int sensorIndex)
{
   m_positionSensor->LoadMapping(settings, std::format("Plunger{}.Position", sensorIndex));
   m_velocitySensor->LoadMapping(settings, std::format("Plunger{}.Velocity", sensorIndex));

   const auto linearPropId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>("Input"s, std::format("Mapping.Plunger{}.Linear", sensorIndex),
      "Linear Sensor"s,
      "Select between symmetric (linear) and asymetric (legacy) sensor\nThe plunger used to be calibrated differently for pull and push. This is largely deprecated and only kept for backwards compatibility."s,
      false, true));
   m_linearPlunger = settings.GetBool(linearPropId);

   const auto posFilterPropId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>(
      "Input"s, std::format("Mapping.Plunger{}.PosFilter", sensorIndex), "Filter position"s, "Apply a denoising filter on position sensor values."s, false, true));
   if (settings.GetBool(posFilterPropId))
      m_positionFilter = std::make_unique<PlungerPositionFilter>();
}

void PlungerSensor::Save(Settings& settings, int sensorIndex) const
{
   m_positionSensor->SaveMapping(settings, std::format("Plunger{}.Position", sensorIndex));
   m_velocitySensor->SaveMapping(settings, std::format("Plunger{}.Velocity", sensorIndex));

   auto linearPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Plunger{}.Linear", sensorIndex));
   assert(linearPropId.has_value());
   settings.Set(linearPropId.value(), IsLinear() ? 1 : 0, false);

   auto posFilterPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Plunger{}.PosFilter", sensorIndex));
   assert(posFilterPropId.has_value());
   settings.Set(posFilterPropId.value(), IsPositionFilterEnabled() ? 1 : 0, false);
}

bool PlungerSensor::IsLinear() const { return m_linearPlunger; }

void PlungerSensor::SetLinear(bool isLinear) { m_linearPlunger = isLinear; }

bool PlungerSensor::IsPositionFilterEnabled() const { return m_positionFilter != nullptr; }

void PlungerSensor::EnablePositionFilter(bool enable)
{
   if (enable != IsPositionFilterEnabled())
      m_positionFilter = enable ? std::make_unique<PlungerPositionFilter>() : nullptr;
}

bool PlungerSensor::HasVelocity() const { return m_velocitySensor->IsMapped(); }

bool PlungerSensor::IsActive() const { return m_deactivationDelay > 0; }

void PlungerSensor::StepOneMillisecond()
{
   m_position = m_positionSensor->GetValue();
   if (m_positionFilter)
      m_position = m_positionFilter->Push(m_position);

   m_velocity = m_velocitySensor->GetValue();

   if (abs(m_position) > 0.01f)
      m_deactivationDelay = 10000;
   if (m_deactivationDelay)
      m_deactivationDelay--;
}
