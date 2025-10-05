// license:GPLv3+

#pragma once

#include "SensorMapping.h"
#include "SensorFilter.h"

// A PhysicSensor evaluates a physics state based on sensor mappings.
// It eventually performs needed integration/derivative.
class PhysicsSensor final : public SensorMapping::SensorMappingHandler
{
public:
   PhysicsSensor(class InputManager* eventManager, const string& settingId, const string& label, SensorMapping::Type sensorType, const string& defaultMappings);
   PhysicsSensor(PhysicsSensor&& other) = delete;
   ~PhysicsSensor() override = default;

   const string& GetLabel() const { return m_label; }

   void ClearMapping();
   void LoadMapping(const Settings& settings) { SetMapping(settings.LoadValueWithDefault(Settings::Section::Input, "Mapping."s + m_settingId, m_defaultMappings)); }
   void SetMapping(const string& mappingString);
   void SetMapping(const SensorMapping& mapping);
   void SaveMapping(Settings& settings) const;
   string GetMappingLabel() const;
   string GetMappingString() const;
   bool IsMapped() const { return m_inputMapping != nullptr; }
   SensorMapping& GetMapping() { return *m_inputMapping.get(); }

   void SetFilter(std::unique_ptr<SourcedSensorFilter> filter);

   void OnInputChanged(SensorMapping* mapping) override;
   void Override(float value);

   float GetValue() const { return m_filter->Get(SDL_GetTicksNS()); }

private:
   const string m_settingId;
   const string m_label;
   const string m_defaultMappings;
   const SensorMapping::Type m_sensorType;
   class InputManager* const m_eventManager;
   std::unique_ptr<SensorMapping> m_inputMapping;
   bool m_overriden = false;
   std::shared_ptr<StoreSensorFilter> m_integrator;
   std::unique_ptr<SourcedSensorFilter> m_filter;
};
