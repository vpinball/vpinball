// license:GPLv3+

#include "core/stdafx.h"

#include "SensorSetupPage.h"
#include "renderer/VRDevice.h"

namespace VPX::InGameUI
{

SensorSetupPage::SensorSetupPage(const InGameUIItem& item)
   : InGameUIPage(item.m_label + " Sensor Setup", "Select the hardware sensor you want to use for " + item.m_label, SaveMode::Global)
   , m_item(item.m_label, item.m_tooltip, item.m_physicsSensor, item.m_physicsSensorTypeMask)
{
   BuildPage();
}

void SensorSetupPage::SaveGlobally()
{
   InGameUIPage::SaveGlobally();
   BuildPage();
}

void SensorSetupPage::SaveTableOverride()
{
   InGameUIPage::SaveTableOverride();
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

   vector<string> sensorTypeNames;
   SensorMapping::Type defaultSensorType = SensorMapping::Type::Position;
   for (int i = 0; i < 3; i++)
      if (m_item.m_physicsSensorTypeMask & (1 << i))
         switch (i)
         {
         case 0:
            sensorTypeNames.emplace_back("Position"s);
            defaultSensorType = SensorMapping::Type::Position;
            break;
         case 1:
            sensorTypeNames.emplace_back("Velocity"s);
            defaultSensorType = SensorMapping::Type::Velocity;
            break;
         case 2:
            sensorTypeNames.emplace_back("Acceleration"s);
            defaultSensorType = SensorMapping::Type::Acceleration;
            break;
         }
   auto findTypeIndex = [&](SensorMapping::Type type)
   {
      for (size_t i = 0; i < sensorTypeNames.size(); i++)
      {
         if ((type == SensorMapping::Type::Position && sensorTypeNames[i] == "Position") || (type == SensorMapping::Type::Velocity && sensorTypeNames[i] == "Velocity")
            || (type == SensorMapping::Type::Acceleration && sensorTypeNames[i] == "Acceleration"))
         {
            return static_cast<int>(i);
         }
      }
      return 0;
   };

   const string prevMapping = m_item.m_physicsSensor->GetMappingString();
   m_item.m_physicsSensor->LoadMapping(m_player->m_ptable->m_settings);
   std::unique_ptr<SensorMapping> storedMapping = m_item.m_physicsSensor->IsMapped() ? std::make_unique<SensorMapping>(m_item.m_physicsSensor->GetMapping()) : nullptr;
   m_item.m_physicsSensor->SetMapping(prevMapping);

   std::unique_ptr<SensorMapping> liveMapping = m_item.m_physicsSensor->IsMapped() ? std::make_unique<SensorMapping>(m_item.m_physicsSensor->GetMapping()) : nullptr;

   int liveAxis = 0;
   int storedAxis = 0;
   vector<string> axisNames;
   axisNames.emplace_back("Unmapped");
   {
      int i = 1;
      for (uint32_t id : m_sensors)
      {
         const uint16_t deviceId = id >> 16;
         const uint16_t axisId = id & 0xFFFF;
         axisNames.emplace_back(m_player->m_pininput.GetDeviceElementName(deviceId, axisId) + " (" + m_player->m_pininput.GetDeviceName(deviceId) + ')');
         if (liveMapping && deviceId == liveMapping->GetDeviceId() && axisId == liveMapping->GetAxisId())
            liveAxis = i;
         if (storedMapping && deviceId == storedMapping->GetDeviceId() && axisId == storedMapping->GetAxisId())
            storedAxis = i;
         i++;
      }
   }

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, sensorTypeNames.size() > 1 ? "Hardware Sensor"s : (sensorTypeNames[0] + " Sensor"), ""s, false, 0, liveAxis, axisNames),
      [liveAxis]() { return liveAxis; }, // Live
      [liveAxis, storedAxis](Settings& settings) { return storedAxis >= 0 ? storedAxis : liveAxis; }, // Stored
      [this, defaultSensorType](int, int v)
      {
         const uint16_t deviceId = m_sensors[v - 1] >> 16;
         const uint16_t axisId = m_sensors[v - 1] & 0xFFFF;
         SensorMapping mapping(nullptr, nullptr, deviceId, axisId, m_item.m_physicsSensor->IsMapped() ? m_item.m_physicsSensor->GetMapping().GetType() : defaultSensorType);
         if (m_item.m_physicsSensor->IsMapped())
         {
            mapping.SetDeadZone(m_item.m_physicsSensor->GetMapping().GetDeadZone());
            mapping.SetScale(m_item.m_physicsSensor->GetMapping().GetScale());
            mapping.SetLimit(m_item.m_physicsSensor->GetMapping().GetLimit());
         }
         m_item.m_physicsSensor->SetMapping(mapping);
         BuildPage();
      }, //
      [](Settings&) { /* Fully overwritten in save */ }, // Reset
      [this](int, Settings& settings, bool) { m_item.m_physicsSensor->SaveMapping(settings); })); // Save

   if (liveMapping == nullptr)
      return;

   // If the mapping allow more than one type, allow to change it, otherwise, wanr the user it's fixed
   if (sensorTypeNames.size() > 1)
   {
      const int sensorType = findTypeIndex(liveMapping->GetType());
      const int storedSensorType = storedMapping ? findTypeIndex(storedMapping->GetType()) : sensorType;
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(""s, ""s, "Sensor Type"s, ""s, false, 0, sensorType, sensorTypeNames), //
         [sensorType]() { return sensorType; }, // Live
         [storedSensorType](Settings&) { return storedSensorType; }, // Stored
         [this, sensorTypeNames](int, int v)
         {
            const uint16_t deviceId = m_item.m_physicsSensor->GetMapping().GetDeviceId();
            const uint16_t axisId = m_item.m_physicsSensor->GetMapping().GetDeviceId();
            SensorMapping mapping(nullptr, nullptr, deviceId, axisId,
               sensorTypeNames[v] == "Position" ? SensorMapping::Type::Position
                  : sensorTypeNames[v] == "Velocity" ? SensorMapping::Type::Velocity
                                     : SensorMapping::Type::Acceleration);
            mapping.SetDeadZone(m_item.m_physicsSensor->GetMapping().GetDeadZone());
            mapping.SetScale(m_item.m_physicsSensor->GetMapping().GetScale());
            mapping.SetLimit(m_item.m_physicsSensor->GetMapping().GetLimit());
            m_item.m_physicsSensor->SetMapping(mapping);
            BuildPage();
         }, //
         [](Settings&) { /* Already performed in first page item */ }, // Reset
         [](int, Settings&, bool) { /* Already performed in first page item */ })); // Save
   }

   const float liveScale = liveMapping->GetScale();
   const float storedScale = storedMapping ? storedMapping->GetScale() : 1.f;
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::BoolPropertyDef(""s, ""s, "Reversed axis"s, ""s, false, false), //
      [liveScale]() { return liveScale < 0.f; }, // Live
      [storedScale](Settings&) { return storedScale < 0.f; }, // Stored
      [this, liveScale](bool v)
      {
         float s = fabs(liveScale);
         m_item.m_physicsSensor->GetMapping().SetScale(v ? -s : s);
         BuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save

   const float liveDeadZone = liveMapping->GetDeadZone();
   const float storedDeadZone = storedMapping ? storedMapping->GetDeadZone() : 0.f;
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::FloatPropertyDef(""s, ""s, "Dead Zone"s, ""s, false, 0.f, 0.3f, 0.f, 0.f), 100.f,
      "%4.1f %%", //
      [liveDeadZone]() { return liveDeadZone; }, // Live
      [storedDeadZone](Settings&) { return storedDeadZone; }, // Stored
      [this](float, float v)
      {
         m_item.m_physicsSensor->GetMapping().SetDeadZone(v);
         BuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::FloatPropertyDef(""s, ""s, "Gain"s, ""s, false, 0.f, 5.f, 0.f, 1.f), 100.f,
      "%4.1f %%", //
      [liveScale]() { return fabs(liveScale); }, // Live
      [storedScale](Settings&) { return fabs(storedScale); }, // Stored
      [this, liveScale](float, float v)
      {
         const bool reversed = liveScale < 0.f;
         m_item.m_physicsSensor->GetMapping().SetScale(reversed ? -v : v);
         BuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save

   float liveLimit = liveMapping->GetLimit();
   float storedLimit = storedMapping ? storedMapping->GetLimit() : 1.f;
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::FloatPropertyDef(""s, ""s, "Limit"s, ""s, false, 0.f, 10.f, 0.f, 1.f), 1.f,
      "%4.2f", //
      [liveLimit]() { return liveLimit; }, // Live
      [storedLimit](Settings&) { return storedLimit; }, // Stored
      [this](float, float v)
      {
         m_item.m_physicsSensor->GetMapping().SetLimit(v);
         BuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save
}

}
