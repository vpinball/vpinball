// license:GPLv3+

#include "core/stdafx.h"
#include "SensorSetupPage.h"

#include "parts/Collection.h"
#include "renderer/VRDevice.h"

namespace VPX::InGameUI
{

void SensorSetupPageSection::AppendSection(InGameUIPage* page, PhysicsSensor* sensor, string settingId, int sensorTypeMask, const std::function<void()>& rebuildPage)
{
   m_page = page;
   m_sensor = sensor;
   m_sensorTypeMask = sensorTypeMask;
   m_settingId = settingId;
   m_rebuildPage = rebuildPage;

   vector<string> sensorTypeNames;
   SensorMapping::Type defaultSensorType = SensorMapping::Type::Position;
   for (int i = 0; i < 3; i++)
      if (m_sensorTypeMask & (1 << i))
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

   const string prevMapping = m_sensor->GetMappingString();
   m_sensor->LoadMapping(m_page->m_player->m_ptable->m_settings, m_settingId);
   std::unique_ptr<SensorMapping> storedMapping = m_sensor->IsMapped() ? std::make_unique<SensorMapping>(m_sensor->GetMapping()) : nullptr;
   m_sensor->SetMapping(prevMapping);

   std::unique_ptr<SensorMapping> liveMapping = m_sensor->IsMapped() ? std::make_unique<SensorMapping>(m_sensor->GetMapping()) : nullptr;

   int liveAxis = 0;
   int storedAxis = 0;
   vector<string> axisNames;
   axisNames.emplace_back("Unmapped"s);
   m_sensors = m_page->m_player->m_pininput.GetAllAxis();
   {
      int i = 1;
      for (auto it = m_sensors.begin(); it != m_sensors.end();)
      {
         const uint32_t id = *it;
         const uint16_t deviceId = id >> 16;
         const uint16_t axisId = id & 0xFFFF;
         const bool connected = m_page->m_player->m_pininput.IsDeviceConnected(deviceId);
         if (!connected && !m_page->m_player->m_pininput.IsDeviceMapped(deviceId))
         {
            it = m_sensors.erase(it);
         }
         else
         {
            axisNames.emplace_back(std::format(
               "{} ({}){}", m_page->m_player->m_pininput.GetDeviceElementName(deviceId, axisId), m_page->m_player->m_pininput.GetDeviceName(deviceId), connected ? "" : " [Disconnected]"));
            if (liveMapping && deviceId == liveMapping->GetDeviceId() && axisId == liveMapping->GetAxisId())
               liveAxis = i;
            if (storedMapping && deviceId == storedMapping->GetDeviceId() && axisId == storedMapping->GetAxisId())
               storedAxis = i;
            it++;
            i++;
         }
      }
   }

   m_page
      ->AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(
            ""s, ""s, sensorTypeNames.size() > 1 ? "Hardware Sensor"s : (sensorTypeNames[0] + " Sensor"), "Select analog input to use."s, false, 0, liveAxis, axisNames),
         [liveAxis]() { return liveAxis; }, // Live
         [liveAxis, storedAxis](const Settings& settings) { return storedAxis >= 0 ? storedAxis : liveAxis; }, // Stored
         [this, defaultSensorType](int, int v)
         {
            if (v == 0)
            {
               m_sensor->ClearMapping();
            }
            else
            {
               const uint16_t deviceId = m_sensors[v - 1] >> 16;
               const uint16_t axisId = m_sensors[v - 1] & 0xFFFF;
               SensorMapping mapping(nullptr, nullptr, deviceId, axisId, m_sensor->IsMapped() ? m_sensor->GetMapping().GetType() : defaultSensorType);
               if (m_sensor->IsMapped())
               {
                  mapping.SetDeadZone(m_sensor->GetMapping().GetDeadZone());
                  mapping.SetScale(m_sensor->GetMapping().GetScale());
                  mapping.SetLimit(m_sensor->GetMapping().GetLimit());
               }
               m_sensor->SetMapping(mapping);
            }
            m_rebuildPage();
         }, //
         [](Settings&) { /* Fully overwritten in save */ }, // Reset
         [this](int, Settings& settings, bool)
         {
            m_sensor->SaveMapping(settings, m_settingId);
            m_rebuildPage();
         }))
      .m_excludeFromDefault = true; // Save

   if (liveMapping == nullptr)
      return;

   // If the mapping allow more than one type, allow to change it, otherwise, warn the user it's fixed
   if (sensorTypeNames.size() > 1)
   {
      const int sensorType = findTypeIndex(liveMapping->GetType());
      const int storedSensorType = storedMapping ? findTypeIndex(storedMapping->GetType()) : sensorType;
      m_page
         ->AddItem(std::make_unique<InGameUIItem>(
            VPX::Properties::EnumPropertyDef(""s, ""s, "Sensor Type"s, "Select the type of data acquired by the selected sensor."s, false, 0, sensorType, sensorTypeNames), //
            [sensorType]() { return sensorType; }, // Live
            [storedSensorType](const Settings&) { return storedSensorType; }, // Stored
            [this, sensorTypeNames](int, int v)
            {
               const uint16_t deviceId = m_sensor->GetMapping().GetDeviceId();
               const uint16_t axisId = m_sensor->GetMapping().GetAxisId();
               SensorMapping mapping(nullptr, nullptr, deviceId, axisId,
                  sensorTypeNames[v] == "Position"      ? SensorMapping::Type::Position
                     : sensorTypeNames[v] == "Velocity" ? SensorMapping::Type::Velocity
                                                        : SensorMapping::Type::Acceleration);
               mapping.SetDeadZone(m_sensor->GetMapping().GetDeadZone());
               mapping.SetScale(m_sensor->GetMapping().GetScale());
               mapping.SetLimit(m_sensor->GetMapping().GetLimit());
               m_sensor->SetMapping(mapping);
               m_rebuildPage();
            }, //
            [](Settings&) { /* Already performed in first page item */ }, // Reset
            [](int, Settings&, bool) { /* Already performed in first page item */ }))
         .m_excludeFromDefault = true; // Save
   }

   // Scale is defined as a custom scale applied on a selected unit.
   const float liveScale = liveMapping->GetScale();
   const float storedScale = storedMapping ? storedMapping->GetScale() : 1.f;
   bool isCustomScale = true;
   if (liveMapping->GetType() == SensorMapping::Type::Acceleration)
   {
      // Accelerations must be provided to the engine in m/s^2 (acquired value x scale => m/s^2)
      // We propose some default scales, that is to say 1/2/4/8g (which is what Pinscape boards propose)
      constexpr float g = 9.80665f;
      if (m_accUnit < 0)
         m_accUnit = abs(liveScale - g) < 0.01f ? 1 : abs(liveScale - 2.f * g) < 0.01f ? 2 : abs(liveScale - 4.f * g) < 0.01f ? 3 : abs(liveScale - 8.f * g) < 0.01f ? 4 : 0;
      isCustomScale = m_accUnit == 0;
      m_page->AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(
            ""s, ""s, "Sensor unit"s, "Unit used by the sensor or custom scaling."s, false, 0, 0, vector { "Custom unit"s, "1 Gravity"s, "2 Gravity"s, "4 Gravity"s, "8 Gravity"s }),
         [this]() { return m_accUnit; }, // Live
         [this](const Settings& settings) { return m_accUnit; }, // Stored
         [this, liveScale, g](int, int v)
         {
            m_accUnit = v;
            const float sign = liveScale < 0.f ? -1.f : 1.f;
            switch (v)
            {
            case 1: m_sensor->GetMapping().SetScale(sign * g); break;
            case 2: m_sensor->GetMapping().SetScale(sign * 2.f * g); break;
            case 3: m_sensor->GetMapping().SetScale(sign * 4.f * g); break;
            case 4: m_sensor->GetMapping().SetScale(sign * 8.f * g); break;
            }
            m_rebuildPage();
         },
         [](Settings& settings) { /* Already performed in first page item */ }, // Reset
         [](int v, Settings& settings, bool isTableOverride) { /* Already performed in first page item */ })); // Save
   }
   else if (liveMapping->GetType() == SensorMapping::Type::Velocity)
   {
      // Velocities must be provided to the engine in m/s (acquired value x scale => m/s)
      // We propose some default scales, that is to say 20mm/s (which is what Pinscape boards use)
      if (m_velUnit < 0)
         m_velUnit = abs(liveScale - 0.020f) < 0.01f ? 1 :  0;
      isCustomScale = m_velUnit == 0;
      m_page->AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(""s, ""s, "Sensor unit"s, "Unit used by the sensor or custom scaling."s, false, 0, 0, vector { "Custom unit"s, "20 mm/s"s }),
         [this]() { return m_velUnit; }, // Live
         [this](const Settings& settings) { return m_velUnit; }, // Stored
         [this, liveScale](int, int v)
         {
            m_velUnit = v;
            const float sign = liveScale < 0.f ? -1.f : 1.f;
            switch (v)
            {
            case 1: m_sensor->GetMapping().SetScale(sign * 20.f); break;
            }
            m_rebuildPage();
         },
         [](Settings& settings) { /* Already performed in first page item */ }, // Reset
         [](int v, Settings& settings, bool isTableOverride) { /* Already performed in first page item */ })); // Save
   }
   else if (liveMapping->GetType() == SensorMapping::Type::Position)
   {
      // Positions are unit less:
      // . plunger is somewhat ackward with +1 for fully retracted and -1 being either symmetric (linear mode) or a custom scaling (legacy)
      // . nudge only uses position for gamepad nudging where the units does not mean anything (it is just used to evaluate the player intent)
   }
   if (isCustomScale)
   {
      m_page->AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::FloatPropertyDef(""s, ""s, "Gain"s, "Scale the acquired value by the selected scale."s, false, 0.f, 5.f, 0.f, 1.f), 100.f,
         "%4.1f %%", //
         [liveScale]() { return fabs(liveScale); }, // Live
         [storedScale](const Settings&) { return fabs(storedScale); }, // Stored
         [this, liveScale](float, float v)
         {
            const bool reversed = liveScale < 0.f;
            m_sensor->GetMapping().SetScale(reversed ? -v : v);
            m_rebuildPage();
         }, //
         [](Settings&) { /* Already performed in first page item */ }, // Reset
         [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save
   }

   m_page->AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::BoolPropertyDef(""s, ""s, "Reversed axis"s, "Toggle axis direction"s, false, false), //
      [liveScale]() { return liveScale < 0.f; }, // Live
      [storedScale](const Settings&) { return storedScale < 0.f; }, // Stored
      [this, liveScale](bool v)
      {
         float s = fabs(liveScale);
         m_sensor->GetMapping().SetScale(v ? -s : s);
         m_rebuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save

   const float liveDeadZone = liveMapping->GetDeadZone();
   const float storedDeadZone = storedMapping ? storedMapping->GetDeadZone() : 0.f;
   m_page->AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::FloatPropertyDef(""s, ""s, "Dead Zone"s, "Relative amount of the axis range to nullify to avoid noise at rest position"s, false, 0.f, 0.3f, 0.f, 0.f), 100.f,
      "%4.1f %%", //
      [liveDeadZone]() { return liveDeadZone; }, // Live
      [storedDeadZone](const Settings&) { return storedDeadZone; }, // Stored
      [this](float, float v)
      {
         m_sensor->GetMapping().SetDeadZone(v);
         m_rebuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save

   float liveLimit = liveMapping->GetLimit();
   float storedLimit = storedMapping ? storedMapping->GetLimit() : 1.f;
   m_page->AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::FloatPropertyDef(""s, ""s, "Limit"s, "Allow to clamp the acquisition (applied before scaling the result by the gain)"s, false, 0.f, 1.f, 0.f, 1.f), 1.f,
      "%4.2f", //
      [liveLimit]() { return liveLimit; }, // Live
      [storedLimit](const Settings&) { return storedLimit; }, // Stored
      [this](float, float v)
      {
         m_sensor->GetMapping().SetLimit(v);
         m_rebuildPage();
      }, //
      [](Settings&) { /* Already performed in first page item */ }, // Reset
      [](float, Settings&, bool) { /* Already performed in first page item */ })); // Save
}

}
