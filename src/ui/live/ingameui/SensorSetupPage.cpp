// license:GPLv3+

#include "core/stdafx.h"

#include "SensorSetupPage.h"
#include "renderer/VRDevice.h"

namespace VPX::InGameUI
{

SensorSetupPage::SensorSetupPage(const InGameUIItem& item)
   : InGameUIPage(item.m_label + " Sensor Setup"s, "Select the hardware sensor you want to use for "s + item.m_label, SaveMode::Global)
   , m_item(item.m_label, item.m_tooltip, item.m_physicsSensor, item.m_physicsSensorTypeMask)
{
   BuildPage();
}

void SensorSetupPage::BuildPage()
{
   ClearItems();

   m_sensors = m_player->m_pininput.GetAllAxis();
   if (m_sensors.empty())
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "No hardware sensor connected"s));
      return;
   }

   if (!m_item.m_physicsSensor->IsMapped())
   {
      uint32_t id = m_sensors[0];
      uint16_t deviceId = id >> 16;
      uint16_t axisId = id & 0xFFFF;
      SensorMapping::Type type = (m_item.m_physicsSensorTypeMask & 1) ? SensorMapping::Type::Position
         : (m_item.m_physicsSensorTypeMask & 2)                       ? SensorMapping::Type::Velocity
                                                                      : SensorMapping::Type::Acceleration;
      m_item.m_physicsSensor->SetMapping(SensorMapping(nullptr, nullptr, deviceId, axisId, type));
   }

   const string prevMapping = m_item.m_physicsSensor->GetMappingString();
   m_item.m_physicsSensor->LoadMapping(m_player->m_ptable->m_settings);
   SensorMapping storedMapping = m_item.m_physicsSensor->GetMapping();
   m_item.m_physicsSensor->SetMapping(prevMapping);
   const SensorMapping& liveMapping = m_item.m_physicsSensor->GetMapping();

   int liveAxis = -1;
   int storedAxis = -1;
   vector<string> axisNames;
   {
      int i = 0;
      for (uint32_t id : m_sensors)
      {
         const uint16_t deviceId = id >> 16;
         const uint16_t axisId = id & 0xFFFF;
         axisNames.emplace_back(m_player->m_pininput.GetDeviceElementName(deviceId, axisId) + " (" + m_player->m_pininput.GetDeviceName(deviceId) + ')');
         if (deviceId == liveMapping.GetDeviceId() && axisId == liveMapping.GetAxisId())
            liveAxis = i;
         if (deviceId == storedMapping.GetDeviceId() && axisId == storedMapping.GetAxisId())
            storedAxis = i;
         i++;
      }
   }
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Hardware Sensor"s, ""s, 0, liveAxis, axisNames), //
      [liveAxis]() { return liveAxis; }, // Live
      [storedAxis]() { return storedAxis; }, // Stored
      [this](int, int v)
      {
         const uint16_t deviceId = m_sensors[v] >> 16;
         const uint16_t axisId = m_sensors[v] & 0xFFFF;
         SensorMapping mapping(nullptr, nullptr, deviceId, axisId, SensorMapping::Type::Velocity);
         mapping.SetDeadZone(m_item.m_physicsSensor->GetMapping().GetDeadZone());
         mapping.SetScale(m_item.m_physicsSensor->GetMapping().GetScale());
         mapping.SetLimit(m_item.m_physicsSensor->GetMapping().GetLimit());
         m_item.m_physicsSensor->SetMapping(mapping);
         BuildPage();
      }, //
      [](Settings&) { /* Fully overwritten in save */ }, // Reset
      [this](int, Settings& settings, bool) { m_item.m_physicsSensor->SaveMapping(settings); })); // Save

   if (m_item.m_physicsSensorTypeMask != 1 && m_item.m_physicsSensorTypeMask != 2 && m_item.m_physicsSensorTypeMask != 4)
   {
      vector<string> names;
      for (int i = 0; i < 3; i++)
         if (m_item.m_physicsSensorTypeMask & (1 << i))
            switch (i)
            {
            case 0: names.emplace_back("Position"s); break;
            case 1: names.emplace_back("Velocity"s); break;
            case 2: names.emplace_back("Acceleration"s); break;
            }
      const int sensorType = liveMapping.GetType() == SensorMapping::Type::Position ? 0 : liveMapping.GetType() == SensorMapping::Type::Velocity ? 1 : 2;
      const int storedSensorType = storedMapping.GetType() == SensorMapping::Type::Position ? 0 : storedMapping.GetType() == SensorMapping::Type::Velocity ? 1 : 2;
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(""s, ""s, "Sensor Type"s, ""s, 0, sensorType, names), //
         [sensorType]() { return sensorType; }, // Live
         [storedSensorType]() { return storedSensorType; }, // Stored
         [this](int, int v)
         {
            const uint16_t deviceId = m_item.m_physicsSensor->GetMapping().GetDeviceId();
            const uint16_t axisId = m_item.m_physicsSensor->GetMapping().GetDeviceId();
            SensorMapping mapping(nullptr, nullptr, deviceId, axisId, v == 0 ? SensorMapping::Type::Position : v == 1 ? SensorMapping::Type::Velocity : SensorMapping::Type::Acceleration);
            mapping.SetDeadZone(m_item.m_physicsSensor->GetMapping().GetDeadZone());
            mapping.SetScale(m_item.m_physicsSensor->GetMapping().GetScale());
            mapping.SetLimit(m_item.m_physicsSensor->GetMapping().GetLimit());
            m_item.m_physicsSensor->SetMapping(mapping);
            BuildPage();
         }, //
         [](Settings&) { /* Already performed in first page item */ }, // Reset
         [](int, Settings&, bool) { /* Already performed in first page item */ })); // Save
   }

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::BoolPropertyDef(""s, ""s, "Reversed axis"s, ""s, m_item.m_physicsSensor->GetMapping().GetScale() < 0.f), //
      [this]() { return m_item.m_physicsSensor->GetMapping().GetScale() < 0.f; }, // Live
      [storedMapping]() { return storedMapping.GetScale() < 0.f; }, // Stored
      [this](bool v)
      {
         float s = fabs(m_item.m_physicsSensor->GetMapping().GetScale());
         m_item.m_physicsSensor->GetMapping().SetScale(v ? -s : s);
         BuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::FloatPropertyDef(""s, ""s, "Dead Zone"s, ""s, 0.f, 0.3f, 0.f, m_item.m_physicsSensor->GetMapping().GetDeadZone()), 100.f,
      "%4.1f %%", //
      [this]() { return m_item.m_physicsSensor->GetMapping().GetDeadZone(); }, // Live
      [storedMapping]() { return storedMapping.GetDeadZone(); }, // Stored
      [this](float, float v)
      {
         m_item.m_physicsSensor->GetMapping().SetDeadZone(v);
         BuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::FloatPropertyDef(""s, ""s, "Gain"s, ""s, 0.f, 5.f, 0.f, fabs(m_item.m_physicsSensor->GetMapping().GetScale())), 100.f,
      "%4.1f %%", //
      [this]() { return fabs(m_item.m_physicsSensor->GetMapping().GetScale()); }, // Live
      [storedMapping]() { return fabs(storedMapping.GetScale()); }, // Stored
      [this](float, float v)
      {
         const bool reversed = m_item.m_physicsSensor->GetMapping().GetScale() < 0.f;
         m_item.m_physicsSensor->GetMapping().SetScale(reversed ? -v : v);
         BuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::FloatPropertyDef(""s, ""s, "Limit"s, ""s, 0.f, 10.f, 0.f, m_item.m_physicsSensor->GetMapping().GetLimit()), 1.f,
      "%4.2f", //
      [this]() { return m_item.m_physicsSensor->GetMapping().GetLimit(); }, // Live
      [storedMapping]() { return storedMapping.GetLimit(); }, // Stored
      [this](float, float v)
      {
         m_item.m_physicsSensor->GetMapping().SetLimit(v);
         BuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save
}

}
