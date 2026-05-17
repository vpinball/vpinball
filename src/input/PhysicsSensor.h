// license:GPLv3+

#pragma once

#include "SensorMapping.h"

// A PhysicSensor evaluates a physics state based on sensor mappings.
class PhysicsSensor final : public SensorMapping::SensorMappingHandler
{
public:
   PhysicsSensor(class InputManager* eventManager, const string& label, SensorMapping::Type sensorType);
   PhysicsSensor(PhysicsSensor&& other) = delete;
   ~PhysicsSensor() override = default;

   const string& GetLabel() const { return m_label; }
   const SensorMapping::Type GetType() const { return m_sensorType; }

   void ClearMapping();
   void LoadMapping(const Settings& settings, const string& settingId);
   void SaveMapping(Settings& settings, const string& settingId) const;
   void SetMapping(const string& mappingString);
   void SetMapping(const SensorMapping& mapping);
   string GetMappingLabel() const;
   string GetMappingString() const;
   bool IsMapped() const { return m_inputMapping != nullptr; }
   float GetValue() const { return IsMapped() ? m_inputMapping->GetValue() : 0.f; }
   SensorMapping& GetMapping() { return *m_inputMapping.get(); }
   const SensorMapping& GetMapping() const { return *m_inputMapping.get(); }

   void OnInputChanged(SensorMapping* mapping) override;

private:
   const string m_label;
   const SensorMapping::Type m_sensorType;
   class InputManager* const m_eventManager;
   std::unique_ptr<SensorMapping> m_inputMapping;
};
