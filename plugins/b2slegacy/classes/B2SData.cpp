#include "../common.h"
#include "B2SData.h"
#include "B2SSettings.h"
#include "B2SPlayer.h"

#include "B2SReelDisplay.h"
#include "LEDDisplayDigitLocation.h"
#include "../controls/B2SReelBox.h"
#include "../controls/B2SLEDBox.h"
#include "../dream7/Dream7Display.h"
#include "../classes/LEDAreaInfo.h"

namespace B2SLegacy {

B2SData::B2SData(B2SSettings* pB2SSettings, VPXPluginAPI* vpxApi)
   : m_pB2SSettings(pB2SSettings), m_vpxApi(vpxApi)
{
   m_tableType = 0;
   m_dmdType = 0;
   m_grillHeight = 0;
   m_smallGrillHeight = 0;
   m_dmdDefaultLocation = { 0, 0 };
   m_dualBackglass = false;
   m_launchBackglass = true;
   m_backglassVisible = false;
   m_useZOrder = false;
   m_useDMDZOrder = false;
   m_onAndOffImage = false;
   m_offImageVisible = false;
   m_usedTopRomIDType4Authentic = eRomIDType_NotDefined;
   m_usedTopRomIDType4Fantasy = eRomIDType_NotDefined;
   m_usedSecondRomIDType4Authentic = eRomIDType_NotDefined;
   m_usedSecondRomIDType4Fantasy = eRomIDType_NotDefined;
   m_useRotatingImage = false;
   m_useMechRotatingImage = false;

   m_ledCoordMax = 103;

   // add led segments
   constexpr int toleft = 8;
   // 8 segments
   m_led8Seg.push_back({ { 22, 5 }, { 26, 2 }, { 88, 2 }, { 92, 5 }, { 85, 11 }, { 29, 11 } });
   m_led8Seg.push_back({ { 93, 7 }, { 96, 10 }, { 96 - toleft, 46 }, { 93 - toleft, 49 }, { 87 - toleft, 43 }, { 87, 12 } });
   m_led8Seg.push_back({ { 92 - toleft, 51 }, { 95 - toleft, 54 }, { 96 - 2 * toleft, 90 }, { 93 - 2 * toleft, 93 }, { 87 - 2 * toleft, 88 }, { 86 - toleft, 57 } });
   m_led8Seg.push_back({ { 22 - 2 * toleft, 95 }, { 29 - 2 * toleft, 89 }, { 85 - 2 * toleft, 89 }, { 92 - 2 * toleft, 95 }, { 88 - 2 * toleft, 98 }, { 26 - 2 * toleft, 98 } });
   m_led8Seg.push_back({ { 20 - toleft, 51 }, { 26 - toleft, 57 }, { 27 - 2 * toleft, 88 }, { 21 - 2 * toleft, 93 }, { 17 - 2 * toleft, 90 }, { 17 - toleft, 54 } });
   m_led8Seg.push_back({ { 21, 7 }, { 27, 12 }, { 27 - toleft, 43 }, { 21 - toleft, 49 }, { 18 - toleft, 46 }, { 18, 10 } });
   m_led8Seg.push_back({ { 23 - toleft, 50 }, { 27 - toleft, 46 }, { 86 - toleft, 46 }, { 90 - toleft, 50 }, { 86 - toleft, 54 }, { 27 - toleft, 54 } });

   // 10 segments
   m_led10Seg.push_back({ { 22, 5 }, { 26, 2 }, { 88, 2 }, { 92, 5 }, { 85, 11 }, { 72, 11 }, { 67, 6 }, { 62, 11 }, { 29, 11 } });
   m_led10Seg.push_back({ { 93, 7 }, { 96, 10 }, { 96 - toleft, 46 }, { 93 - toleft, 49 }, { 87 - toleft, 43 }, { 87, 12 } });
   m_led10Seg.push_back({ { 92 - toleft, 51 }, { 95 - toleft, 54 }, { 96 - 2 * toleft, 90 }, { 93 - 2 * toleft, 93 }, { 87 - 2 * toleft, 88 }, { 86 - toleft, 57 } });
   m_led10Seg.push_back({ { 22 - 2 * toleft, 95 }, { 29 - 2 * toleft, 89 }, { 61 - 2 * toleft, 89 }, { 66 - 2 * toleft, 94 }, { 71 - 2 * toleft, 89 }, { 85 - 2 * toleft, 89 }, { 92 - 2 * toleft, 95 }, { 88 - 2 * toleft, 98 }, { 26 - 2 * toleft, 98 } });
   m_led10Seg.push_back({ { 20 - toleft, 51 }, { 26 - toleft, 57 }, { 27 - 2 * toleft, 88 }, { 21 - 2 * toleft, 93 }, { 17 - 2 * toleft, 90 }, { 17 - toleft, 54 } });
   m_led10Seg.push_back({ { 21, 7 }, { 27, 12 }, { 27 - toleft, 43 }, { 21 - toleft, 49 }, { 18 - toleft, 46 }, { 18, 10 } });
   m_led10Seg.push_back({ { 23 - toleft, 50 }, { 27 - toleft, 46 }, { 63 - toleft, 46 }, { 68 - toleft, 51 }, { 73 - toleft, 46 }, { 86 - toleft, 46 }, { 90 - toleft, 50 }, { 86 - toleft, 54 }, { 72 - toleft, 54 }, { 67 - toleft, 49 }, { 62 - toleft, 54 }, { 27 - toleft, 54 } });
   m_led10Seg.push_back({ { 67, 9 }, { 71, 13 }, { 71 - toleft, 45 }, { 67 - toleft, 49 }, { 63 - toleft, 45 }, { 63, 13 } });
   m_led10Seg.push_back({ { 68, 7 }, { 72, 11 }, { 72 - toleft, 43 }, { 68 - toleft, 47 }, { 64 - toleft, 43 }, { 64, 7 } });
   m_led10Seg.push_back({ { 66 - toleft, 51 }, { 70 - toleft, 55 }, { 70 - 2 * toleft, 88 }, { 66 - 2 * toleft, 92 }, { 62 - 2 * toleft, 88 }, { 62 - toleft, 51 } });

   // 14 segments
   m_led14Seg.push_back({ { 22, 5 }, { 26, 2 }, { 88, 2 }, { 92, 5 }, { 85, 11 }, { 29, 11 } });
   m_led14Seg.push_back({ { 93, 7 }, { 96, 10 }, { 96 - toleft, 46 }, { 93 - toleft, 49 }, { 87 - toleft, 43 }, { 87, 12 } });
   m_led14Seg.push_back({ { 92 - toleft, 51 }, { 95 - toleft, 54 }, { 96 - 2 * toleft, 90 }, { 93 - 2 * toleft, 93 }, { 87 - 2 * toleft, 88 }, { 86 - toleft, 57 } });
   m_led14Seg.push_back({ { 22 - 2 * toleft, 95 }, { 29 - 2 * toleft, 89 }, { 85 - 2 * toleft, 89 }, { 92 - 2 * toleft, 95 }, { 88 - 2 * toleft, 98 }, { 26 - 2 * toleft, 98 } });
   m_led14Seg.push_back({ { 20 - toleft, 51 }, { 26 - toleft, 57 }, { 27 - 2 * toleft, 88 }, { 21 - 2 * toleft, 93 }, { 17 - 2 * toleft, 90 }, { 17 - toleft, 54 } });
   m_led14Seg.push_back({ { 21, 7 }, { 27, 12 }, { 27 - toleft, 43 }, { 21 - toleft, 49 }, { 18 - toleft, 46 }, { 18, 10 } });
   m_led14Seg.push_back({ { 23 - toleft, 50 }, { 27 - toleft, 46 }, { 52 - toleft, 46 }, { 55 - toleft, 50 }, { 52 - toleft, 54 }, { 27 - toleft, 54 } });
   m_led14Seg.push_back({ { 104 - 2 * toleft, 87 }, { 109 - 2 * toleft, 90 }, { 109 - 2 * toleft, 95 }, { 104 - 2 * toleft, 99 }, { 100 - 2 * toleft, 95 }, { 100 - 2 * toleft, 90 } });
   m_led14Seg.push_back({ { 30, 13 }, { 34, 17 }, { 54 - toleft, 38 }, { 51 - toleft, 43 }, { 48 - toleft, 40 }, { 27, 16 } });
   m_led14Seg.push_back({ { 57, 13 }, { 61, 13 }, { 61 - toleft, 46 }, { 57 - toleft, 48 }, { 53 - toleft, 46 }, { 53, 13 } });
   m_led14Seg.push_back({ { 82, 13 }, { 85, 16 }, { 68 - toleft, 42 }, { 65 - toleft, 44 }, { 63 - toleft, 39 }, { 77, 17 } });
   m_led14Seg.push_back({ { 58 - toleft, 50 }, { 62 - toleft, 46 }, { 86 - toleft, 46 }, { 90 - toleft, 50 }, { 86 - toleft, 54 }, { 62 - toleft, 54 } });
   m_led14Seg.push_back({ { 82 - 2 * toleft, 85 }, { 87 - 2 * toleft, 86 }, { 67 - toleft, 57 }, { 62 - toleft, 57 }, { 62 - toleft, 60 }, { 79 - 2 * toleft, 86 } });
   m_led14Seg.push_back({ { 57 - toleft, 52 }, { 61 - toleft, 54 }, { 61 - 2 * toleft, 88 }, { 57 - 2 * toleft, 88 }, { 53 - 2 * toleft, 88 }, { 53 - toleft, 54 } });
   m_led14Seg.push_back({ { 30 - 2 * toleft, 83 }, { 33 - 2 * toleft, 86 }, { 50 - toleft, 60 }, { 47 - toleft, 57 }, { 42 - toleft, 61 }, { 27 - 2 * toleft, 86 } });
   m_led14Seg.push_back({ { 102 - 2 * toleft, 97 }, { 107 - 2 * toleft, 100 }, { 107 - 2 * toleft, 105 }, { 102 - 2 * toleft, 109 }, { 98 - 2 * toleft, 105 }, { 98 - 2 * toleft, 100 } });

   // 16 segments
   m_led16Seg.push_back({ { 22, 5 }, { 26, 2 }, { 88, 2 }, { 92, 5 }, { 85, 11 }, { 29, 11 } });
}

B2SData::~B2SData()
{
   for (auto& it : m_reels)
      delete it.second;

   for (auto& it : m_reelDisplays)
      delete it.second;

   for (auto& it : m_reelImages)
      if (it.second) m_vpxApi->DeleteTexture(it.second);

   for (auto& it : m_reelIntermediateImages)
      if (it.second) m_vpxApi->DeleteTexture(it.second);

   for (auto& it : m_reelIlluImages)
      if (it.second) m_vpxApi->DeleteTexture(it.second);

   for (auto& it : m_reelIntermediateIlluImages)
      if (it.second) m_vpxApi->DeleteTexture(it.second);

   for (auto& outerIt : m_rotatingImages) {
      for (auto& innerIt : outerIt.second) {
         if (innerIt.second) m_vpxApi->DeleteTexture(innerIt.second);
      }
   }

   for (auto& it : m_sounds)
      delete it.second;

   for (auto& it : m_ledAreas)
      delete it.second;

   for (auto& it : m_ledDisplayDigits)
      delete it.second;

   for (auto& it : m_leds)
      delete it.second;

   for (auto& it : m_ledDisplays)
      delete it.second;

   for (auto& it : m_players)
      delete it.second;
}

void B2SData::Stop()
{
}

void B2SData::ClearAll(bool donotclearnames)
{
   m_infoDirty = true;
   if (!donotclearnames) {
      m_szTableName.clear();
      m_szTableFileName.clear();
      m_szBackglassFileName.clear();
   }
   else {
      m_launchBackglass = true;
      m_backglassVisible = false;
   }
   m_tableType = 0;
   m_dmdType = 0;
   m_grillHeight = 0;
   m_smallGrillHeight = 0;
   m_dmdDefaultLocation = { 0, 0 };
   m_dualBackglass = false;
   m_usedTopRomIDType4Authentic = eRomIDType_NotDefined;
   m_usedTopRomIDType4Fantasy = eRomIDType_NotDefined;
   m_usedSecondRomIDType4Authentic = eRomIDType_NotDefined;
   m_usedSecondRomIDType4Fantasy = eRomIDType_NotDefined;
   m_usedRomLampIDs4Authentic.clear();
   m_usedRomSolenoidIDs4Authentic.clear();
   m_usedRomGIStringIDs4Authentic.clear();
   m_usedRomMechIDs4Authentic.clear();
   m_usedRomLampIDs4Fantasy.clear();
   m_usedRomSolenoidIDs4Fantasy.clear();
   m_usedRomGIStringIDs4Fantasy.clear();
   m_usedRomMechIDs4Fantasy.clear();
   m_usedRomReelLampIDs.clear();
   m_usedAnimationLampIDs.clear();
   m_usedRandomAnimationLampIDs.clear();
   m_usedAnimationSolenoidIDs.clear();
   m_usedRandomAnimationSolenoidIDs.clear();
   m_usedAnimationGIStringIDs.clear();
   m_usedRandomAnimationGIStringIDs.clear();
   m_playerAdded = false;
   m_players.clear();
   // For Each r In Reels : r.Value.Dispose() : Next
   m_reels.clear();
   // For Each rd In ReelDisplays : rd.Value.Dispose() : Next
   m_reelDisplays.clear();
   m_reelImages.clear();
   m_reelIntermediateImages.clear();
   m_reelIlluImages.clear();
   m_reelIntermediateIlluImages.clear();
   m_sounds.clear();
   m_leds.clear();
   m_ledAreas.clear();
   m_ledDisplays.clear();
   m_ledDisplayDigits.clear();
   m_illuminations.clear();
   m_dmdIlluminations.clear();
   m_useIlluminationLocks = false;
   m_illuminationGroups.clear();
   m_illuminationLocks.clear();
   m_useZOrder = false;
   m_useDMDZOrder = false;
   m_zOrderImages.clear();
   m_zOrderDMDImages.clear();
   m_useRotatingImage = false;
   m_useMechRotatingImage = false;
   //For Each r As KeyValuePair(Of Integer, Generic.Dictionary(Of Integer, Image)) In RotatingImages : r.Value.Clear() : Next
   m_rotatingImages.clear();
   m_rotatingPictureBox.clear();
}

std::map<int, vector<B2SBaseBox*>>* B2SData::GetUsedRomLampIDs()
{
   return (m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy) ? &m_usedRomLampIDs4Fantasy : &m_usedRomLampIDs4Authentic;
}

std::map<int, vector<B2SBaseBox*>>* B2SData::GetUsedRomSolenoidIDs()
{
   return (m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy) ? &m_usedRomSolenoidIDs4Fantasy : &m_usedRomSolenoidIDs4Authentic;
}

std::map<int, vector<B2SBaseBox*>>* B2SData::GetUsedRomGIStringIDs()
{
   return (m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy) ? &m_usedRomGIStringIDs4Fantasy : &m_usedRomGIStringIDs4Authentic;
}

std::map<int, vector<B2SBaseBox*>>* B2SData::GetUsedRomMechIDs()
{
   return (m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy) ? &m_usedRomMechIDs4Fantasy : &m_usedRomMechIDs4Authentic;
}

bool B2SData::IsLampsData() const
{
   return (IsUseRomLamps() || IsUseAnimationLamps()) && !m_pB2SSettings->IsAllOff() && !m_pB2SSettings->IsLampsOff();
}

bool B2SData::IsSolenoidsData() const
{
   return (IsUseRomSolenoids() || IsUseAnimationSolenoids()) && !m_pB2SSettings->IsAllOff() && !m_pB2SSettings->IsSolenoidsOff();
}

bool B2SData::IsGIStringsData() const
{
   return (IsUseRomGIStrings() || IsUseAnimationGIStrings()) && !m_pB2SSettings->IsAllOff() && !m_pB2SSettings->IsGIStringsOff();
}

bool B2SData::IsLEDsData() const
{
   return (IsUseLEDs() || IsUseLEDDisplays() || IsUseReels()) && !m_pB2SSettings->IsAllOff() && !m_pB2SSettings->IsLEDsOff();
}

bool B2SData::IsUseRomLamps() const
{
   if (m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy)
      return (!m_usedRomLampIDs4Fantasy.empty() || m_usedTopRomIDType4Fantasy == eRomIDType_Lamp || m_usedSecondRomIDType4Fantasy == eRomIDType_Lamp);
   else
      return (!m_usedRomLampIDs4Authentic.empty() || m_usedTopRomIDType4Authentic == eRomIDType_Lamp || m_usedSecondRomIDType4Authentic == eRomIDType_Lamp);
}

bool B2SData::IsUseRomSolenoids() const
{
   if (m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy)
      return (!m_usedRomSolenoidIDs4Fantasy.empty() || m_usedTopRomIDType4Fantasy == eRomIDType_Solenoid || m_usedSecondRomIDType4Fantasy == eRomIDType_Solenoid);
   else
      return (!m_usedRomSolenoidIDs4Authentic.empty() || m_usedTopRomIDType4Authentic == eRomIDType_Solenoid || m_usedSecondRomIDType4Authentic == eRomIDType_Solenoid);
}

bool B2SData::IsUseRomGIStrings() const
{
   if (m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy)
      return (!m_usedRomGIStringIDs4Fantasy.empty() || m_usedTopRomIDType4Fantasy == eRomIDType_GIString || m_usedSecondRomIDType4Fantasy == eRomIDType_GIString);
   else
      return (!m_usedRomGIStringIDs4Authentic.empty() || m_usedTopRomIDType4Authentic == eRomIDType_GIString || m_usedSecondRomIDType4Authentic == eRomIDType_GIString);
}

bool B2SData::IsUseRomMechs() const
{
   if (m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy)
      return (!m_usedRomMechIDs4Fantasy.empty());
   else
      return (!m_usedRomMechIDs4Authentic.empty());
}

bool B2SData::IsUseAnimationLamps() const
{
   return (!m_usedAnimationLampIDs.empty() || !m_usedRandomAnimationLampIDs.empty());
}

bool B2SData::IsUseAnimationSolenoids() const
{
   return (!m_usedAnimationSolenoidIDs.empty() || !m_usedRandomAnimationSolenoidIDs.empty());
}

bool B2SData::IsUseAnimationGIStrings() const
{
   return (!m_usedAnimationGIStringIDs.empty() || !m_usedRandomAnimationGIStringIDs.empty());
}

bool B2SData::IsUseRomReelLamps() const
{
   return (!m_usedRomReelLampIDs.empty());
}

bool B2SData::IsUseLEDs() const
{
   return (!m_leds.empty());
}

bool B2SData::IsUseLEDDisplays() const
{
   return (!m_ledDisplays.empty());
}

bool B2SData::IsUseReels() const
{
   return (!m_reels.empty());
}

}
