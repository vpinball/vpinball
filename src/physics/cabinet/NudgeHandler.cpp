// license:GPLv3+

#include "core/stdafx.h"
#include "NudgeHandler.h"

#include "core/VPApp.h"
#include "input/PhysicsSensor.h"
#include "math/HighPassFilter.h"
#include "parts/pintable.h"
#include "physics/cabinet/CabinetNudgeSensor.h"
#include "physics/cabinet/GamepadNudge.h"
#include "physics/cabinet/KeyboardNudge.h"
#include "physics/cabinet/MotionKalmanAxis.h"


namespace VPX::Physics
{

NudgeHandler::NudgeHandler(InputManager* inputManager)
{
   const Settings& settings = g_app->m_settings;
   
   SetKeyboardNudgeMode((KeyboardNudgeMode)settings.GetPlayer_KeyboardNudgeMode());

   const int nNudgeSensors = settings.GetInput_NudgeSensorCount();
   for (int i = 0; i < nNudgeSensors; i++)
   {
      std::unique_ptr<NudgeSensor> sensor;
      AddSensor(sensor);
      auto typePropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Type", i));
      switch (g_app->m_settings.GetInt(typePropId.value()))
      {
      case 0: sensor = std::make_unique<GamepadNudge>(inputManager); break;
      case 1: sensor = std::make_unique<CabinetNudgeSensor>(inputManager); break;
      case 2: sensor = std::make_unique<CabinetNudgeSensor>(inputManager); break;
      default: assert(false); continue;
      }
      sensor->Load(settings, i);
      ReplaceSensor(i, std::move(sensor));
   }
}

void NudgeHandler::ApplyKeyboardImpulse(float angle, float force)
{
   m_keyboardNudgeIndex++;
   m_keyboardNudge->Nudge(angle, force);
}

void NudgeHandler::StepOneMillisecond()
{
   m_keyboardNudge->StepOneMillisecond();
   for (const auto& sensor : m_sensors)
      sensor->StepOneMillisecond();
}

const Vertex2D& NudgeHandler::GetCabinetAcceleration() const
{
   if (m_keyboardNudge->IsActive())
      return m_keyboardNudge->GetCabinetAcceleration();
   for (const auto& sensor : m_sensors)
      if (sensor->IsActive())
         return sensor->GetCabinetAcceleration();
   return m_noNudge;
}

const Vertex2D& NudgeHandler::GetCabinetOffset() const
{
   if (m_keyboardNudge->IsActive())
      return m_keyboardNudge->GetCabinetOffset();
   for (const auto& sensor : m_sensors)
      if (sensor->IsActive())
         return sensor->GetCabinetOffset();
   return m_noNudge;
}

NudgeHandler::KeyboardNudgeMode NudgeHandler::GetKeyboardNudgeMode() const
{
   if (dynamic_cast<PushRetractKeyboardNudge*>(m_keyboardNudge.get()) != nullptr)
      return VP9_PUSH_RETRACT;
   if (dynamic_cast<BoxModelKeyboardNudge*>(m_keyboardNudge.get()) != nullptr)
      return VPX_BOX_MODEL;
   if (dynamic_cast<CabModelKeyboardNudge*>(m_keyboardNudge.get()) != nullptr)
      return VPX_CAB_MODEL;
   assert(false);
   return VPX_CAB_MODEL;
}

void NudgeHandler::SetKeyboardNudgeMode(KeyboardNudgeMode mode)
{
   if (m_keyboardNudge != nullptr && m_keyboardNudgeMode == mode)
      return;
   const float strength = m_keyboardNudge ? m_keyboardNudge->GetStrengthScale() : g_app->m_settings.GetPlayer_KeyboardNudgeStrength();
   m_keyboardNudgeMode = mode;
   switch (m_keyboardNudgeMode)
   {
   case VP9_PUSH_RETRACT: m_keyboardNudge = std::make_unique<PushRetractKeyboardNudge>(strength); break;
   case VPX_BOX_MODEL: m_keyboardNudge = std::make_unique<BoxModelKeyboardNudge>(strength); break;
   case VPX_CAB_MODEL: m_keyboardNudge = std::make_unique<CabModelKeyboardNudge>(strength); break;
   default: assert(false); break;
   };
}

float NudgeHandler::GetKeyboardNudgeStrength() const { return m_keyboardNudge->GetStrengthScale(); }

void NudgeHandler::SetKeyboardNudgeStrength(float strength) { m_keyboardNudge->SetStrengthScale(strength); }

int NudgeHandler::GetSensorCount() const { return static_cast<int>(m_sensors.size()); }

const std::unique_ptr<NudgeSensor>& NudgeHandler::GetSensor(int index) const { return m_sensors[index]; }

void NudgeHandler::AddSensor(std::unique_ptr<NudgeSensor>& sensor)
{
   const int sensorIndex = static_cast<int>(m_sensors.size());

   Settings::GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>("Input"s, std::format("Mapping.Nudge{}.Type", sensorIndex),
      "Nudge Sensor Type"s,
      "Game controller use stick position to evaluate player nudge intent.\nIntent Sensor uses sensor to evaluate player nudge intent while Cabinet Sensor directy maps sensor to emulated nudge (only valid on a real pinball cabinet with high frequency, low latency, noise free sensor on a high speed communication port)."s,
      true, 0, 0, vector { "Game Controller"s, "Intent Sensor"s, "Cabinet Sensor"s }));

   Settings::GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(
      "Input"s, std::format("Mapping.Nudge{}.Strength", sensorIndex), "Nudge Strength"s, "Custom strength factor applied to this sensor."s, true, 0.f, 2.f, 0.f, 1.f));

   Settings::GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(
      "Input"s, std::format("Mapping.Nudge{}.CabWeight", sensorIndex), "Cabinet Weight"s, "Weight of the cabinet (kg). This is needed to compute the nudge force from the acquired acceleration."s, true, 0.f, 200.f, 0.f, 113.f));

   if (sensor)
   {
      sensor->Save(g_app->m_settings, sensorIndex);
      g_app->m_settings.SetInput_NudgeSensorCount(sensorIndex + 1, false);
      g_app->m_settings.Save();
   }

   m_sensors.push_back(std::move(sensor));
}

void NudgeHandler::ReplaceSensor(int index, std::unique_ptr<NudgeSensor> sensor) { m_sensors[index] = std::move(sensor); }

void NudgeHandler::RemoveSensor(int index) { 
   // We must shift all saved mappings with index greater than the suppressed one
   if (index < m_sensors.size() - 1)
   {
      for (size_t i = index + 1; i < m_sensors.size(); i++)
      {
         const auto prepend = std::format("Mapping.Nudge{}.", i);
         const auto prependLen = prepend.length();
         for (const auto id : Settings::GetRegistry().GetPropertyIds())
         {
            const VPX::Properties::PropertyDef* propDef = Settings::GetRegistry().GetProperty(id);
            if (propDef->m_groupId == "Input"s && propDef->m_propId.starts_with(prepend))
            {
               const string newId = std::format("Mapping.Nudge{}.{}", i - 1, propDef->m_propId.substr(prependLen));
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
   g_app->m_settings.SetInput_NudgeSensorCount(static_cast<int>(m_sensors.size() + 1), false);
   g_app->m_settings.Save();
}

bool NudgeHandler::HasSensor(const std::unique_ptr<NudgeSensor>& sensor) const
{
   for (const auto& existing : m_sensors)
      if (existing->IsSameSensor(sensor))
         return true;
   return false;
}

bool NudgeHandler::IsMappedToDevice(uint16_t deviceId) const
{
   for (const auto& sensor : m_sensors)
      if (sensor->IsMappedToDevice(deviceId))
         return true;
   return false;
}

void NudgeHandler::UnmapDevice(uint16_t deviceId)
{
   bool removed = true;
   while (removed)
   {
      removed = false;
      for (size_t i = 0; i < m_sensors.size(); i++)
      {
         if (m_sensors[i]->IsMappedToDevice(deviceId))
         {
            RemoveSensor(static_cast<int>(i));
            removed = true;
            break;
         }
      }
   }
}

void NudgeHandler::SetExternalNudge(bool enableOverride, const Vertex2D& acceleration, const Vertex2D& displacement)
{
   // FIXME implement
}


};