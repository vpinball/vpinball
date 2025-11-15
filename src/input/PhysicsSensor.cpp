// license:GPLv3+

#include "core/stdafx.h"
#include "PhysicsSensor.h"

#include <sstream>

PhysicsSensor::PhysicsSensor(class InputManager* eventManager, const string& settingId, const string& label, SensorMapping::Type sensorType)
   : m_settingId(settingId)
   , m_label(label)
   , m_sensorType(sensorType)
   , m_eventManager(eventManager)
   , m_integrator(std::make_shared<NoOpStoreSensorFilter>())
   , m_filter(std::make_unique<NoOpSensorFilter>(m_integrator))
{
}

void PhysicsSensor::ClearMapping()
{
   m_inputMapping = nullptr;
}

void PhysicsSensor::LoadMapping(const Settings& settings)
{
   const auto propId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Mapping." + m_settingId, "Mapping." + m_settingId, ""s, ""s));
   SetMapping(settings.GetString(propId));
}

void PhysicsSensor::SaveMapping(Settings& settings) const
{
   const auto propId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Mapping." + m_settingId, "Mapping." + m_settingId, ""s, ""s));
   if (m_inputMapping == nullptr)
      settings.Reset(propId);
   else
      settings.Set(propId, GetMappingString(), false);
}

void PhysicsSensor::SetMapping(const string& mappingString)
{
   ClearMapping();

   string token;
   vector<string> fields;
   std::istringstream mappingStream(mappingString);
   while (std::getline(mappingStream, token, ';'))
      fields.push_back(trim_string(token));

   bool invalid = false;

   uint16_t deviceId;
   if (!fields.empty())
      deviceId = m_eventManager->GetDeviceId(trim_string(fields[0]));
   else
      invalid = true;

   int axisId;
   invalid |= fields.size() < 2 ? true : !try_parse_int(fields[1], axisId);

   SensorMapping::Type type = SensorMapping::Type::Position;
   if (fields.size() < 3 || fields[2].length() != 1)
      invalid = true;
   else
   {
      switch (fields[2][0])
      {
      case 'P': type = SensorMapping::Type::Position; break;
      case 'V': type = SensorMapping::Type::Velocity; break;
      case 'A': type = SensorMapping::Type::Acceleration; break;
      default: invalid = true; break;
      }
   }

   float deadZone;
   invalid |= fields.size() < 4 ? true : !try_parse_float(fields[3], deadZone);

   float scale;
   invalid |= fields.size() < 5 ? true : !try_parse_float(fields[4], scale);

   float limit;
   invalid |= fields.size() < 6 ? true : !try_parse_float(fields[5], limit);

   if (!invalid)
   {
      SetMapping(SensorMapping(nullptr, nullptr, deviceId, axisId, type));
      m_inputMapping->SetDeadZone(deadZone);
      m_inputMapping->SetScale(scale);
      m_inputMapping->SetLimit(limit);
   }
   else if (!trim_string(mappingString).empty())
   {
      PLOGE << "Invalid sensor mapping: '" << mappingString << '\'';
   }
}

void PhysicsSensor::SetMapping(const SensorMapping& mapping)
{
   m_inputMapping = std::make_unique<SensorMapping>(mapping.With(m_eventManager, this));
   
   if (mapping.GetType() == SensorMapping::Type::Position && m_sensorType == SensorMapping::Type::Acceleration)
      m_integrator = std::make_shared<PositionToAccelerationFilter>();
   else if ((mapping.GetType() == SensorMapping::Type::Position && m_sensorType == SensorMapping::Type::Velocity)
      || (mapping.GetType() == SensorMapping::Type::Velocity && m_sensorType == SensorMapping::Type::Acceleration))
      m_integrator = std::make_shared<VelocityToAccelerationFilter>();
   else if (mapping.GetType() == SensorMapping::Type::Acceleration && m_sensorType == SensorMapping::Type::Position)
   {
      // Not supported but is it useful ?
      assert(false);
      m_integrator = std::make_shared<NoOpStoreSensorFilter>();
   }
   else if ((mapping.GetType() == SensorMapping::Type::Velocity && m_sensorType == SensorMapping::Type::Position)
      || (mapping.GetType() == SensorMapping::Type::Acceleration && m_sensorType == SensorMapping::Type::Velocity))
   {
      // Not supported but is it useful ?
      assert(false);
      m_integrator = std::make_shared<NoOpStoreSensorFilter>();
   }
   else
      m_integrator = std::make_shared<NoOpStoreSensorFilter>();

   m_filter->SetSource(m_integrator);

   m_integrator->SetName(m_label);
}

string PhysicsSensor::GetMappingString() const
{
   if (m_inputMapping == nullptr)
      return ""s;

   std::stringstream result;
   result << m_eventManager->GetDeviceSettingId(m_inputMapping->GetDeviceId());
   result << ';';
   result << m_inputMapping->GetAxisId();
   result << ';';
   switch (m_inputMapping->GetType())
   {
   case SensorMapping::Type::Position: result << 'P'; break;
   case SensorMapping::Type::Velocity: result << 'V'; break;
   case SensorMapping::Type::Acceleration: result << 'A'; break;
   default: return ""s;
   }
   result << ';';
   result << std::to_string(m_inputMapping->GetDeadZone());
   result << ';';
   result << std::to_string(m_inputMapping->GetScale());
   result << ';';
   result << std::to_string(m_inputMapping->GetLimit());
   return result.str();
}

string PhysicsSensor::GetMappingLabel() const
{
   if (m_inputMapping == nullptr)
      return "Not Defined"s;
   return m_eventManager->GetDeviceName(m_inputMapping->GetDeviceId()) + " - " + m_eventManager->GetDeviceElementName(m_inputMapping->GetDeviceId(), m_inputMapping->GetAxisId());
}

void PhysicsSensor::SetFilter(std::unique_ptr<SourcedSensorFilter> filter)
{
   m_filter = std::move(filter);
   m_filter->SetSource(m_integrator);
}

void PhysicsSensor::OnInputChanged(SensorMapping* mapping)
{
   if (m_overriden)
      return;
   m_integrator->Push(mapping->GetRawValueTimestampNs(), mapping->GetValue());
}

void PhysicsSensor::Override(float value)
{
   if (!m_overriden)
   {
      m_overriden = true;
      m_integrator = std::make_shared<NoOpStoreSensorFilter>();
      m_filter = std::make_unique<NoOpSensorFilter>(m_integrator);
   }
   m_integrator->Push(0, value);
}
