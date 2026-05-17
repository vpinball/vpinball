// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class SensorSetupPageSection
{
public:
   void AppendSection(InGameUIPage* page, PhysicsSensor* sensor, string settingId, int sensorTypeMask, const std::function<void()>& rebuildPage);

private:
   InGameUIPage* m_page = nullptr;
   PhysicsSensor* m_sensor = nullptr;
   int m_sensorTypeMask = 0;
   int m_accUnit = -1;
   int m_velUnit = -1;
   string m_settingId;
   vector<uint32_t> m_sensors;
   std::function<void()> m_rebuildPage;
};

}
