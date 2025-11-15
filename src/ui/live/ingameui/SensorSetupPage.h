// license:GPLv3+

#pragma once

namespace VPX::InGameUI
{

class SensorSetupPage final : public InGameUIPage
{
public:
   SensorSetupPage(const InGameUIItem& item);

   void SaveGlobally() override;
   void SaveTableOverride() override;

private:
   void BuildPage();
   InGameUIItem m_item;
   vector<uint32_t> m_sensors;
};

}
