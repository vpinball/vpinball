#pragma once

#include "Sound.h"

#include "../collections/ControlCollection.h"
#include "../collections/PictureBoxCollection.h"
#include "../collections/ReelBoxCollection.h"
#include "../collections/IlluminationGroupCollection.h"
#include "../collections/ZOrderCollection.h"
#include "../collections/GenericDictionaryIgnoreCase.h"
#include "../collections/AnimationCollection.h"
#include "../../vpinmame/VPinMAMEController.h"

#include "B2SSettings.h"
#include "LEDAreaInfo.h"
#include "B2SPlayer.h"

class B2SSettings;

class B2SData
{
public:
   static B2SData* GetInstance();

   void Stop();
   VPinMAMEController* GetVPinMAME();
   void FreeVPinMAME();
   void ClearAll(bool donotclearnames);
   string GetTableName() const { return m_szTableName; }
   void SetTableName(const string& szTableName) { m_szTableName = szTableName; }
   string GetTableFileName() const { return m_szTableFileName; }
   void SetTableFileName(const string& szTableFileName) { m_szTableFileName = szTableFileName; }
   string GetBackglassFileName() const { return m_szBackglassFileName; }
   void SetBackglassFileName(const string& szBackglassFileName) { m_szBackglassFileName = szBackglassFileName; }
   int GetTableType() const { return m_tableType; }
   void SetTableType(const int tableType) { m_tableType = tableType; }
   int GetDMDType() const { return m_dmdType; }
   void SetDMDType(const int dmdType) { m_dmdType = dmdType; }
   int GetGrillHeight() const { return m_grillHeight; }
   void SetGrillHeight(const int grillHeight) { m_grillHeight = grillHeight; }
   int GetSmallGrillHeight() const { return m_smallGrillHeight; }
   void SetSmallGrillHeight(const int smallGrillHeight) { m_smallGrillHeight = smallGrillHeight; }
   const SDL_Point& GetDMDDefaultLocation() { return m_dmdDefaultLocation; }
   void SetDMDDefaultLocation(const SDL_Point& dmdDefaultLocation) { m_dmdDefaultLocation = dmdDefaultLocation; }
   bool IsDualBackglass() const { return m_dualBackglass; }
   void SetDualBackglass(bool dualBackglass) { m_dualBackglass = dualBackglass; }
   bool IsLaunchBackglass() const { return m_launchBackglass; }
   void SetLaunchBackglass(bool launchBackglass) { m_launchBackglass = launchBackglass; }
   bool IsBackglassVisible() const { return m_backglassVisible; }
   void SetBackglassVisible(bool backglassVisible) { m_backglassVisible = backglassVisible; }
   bool IsUseIlluminationLocks() const { return m_useIlluminationLocks; }
   void SetUseIlluminationLocks(bool useIlluminationLocks) { m_useIlluminationLocks = useIlluminationLocks; }
   bool IsUseZOrder() const { return m_useZOrder; }
   void SetUseZOrder(bool useZOrder) { m_useZOrder = useZOrder; }
   bool IsUseDMDZOrder() const { return m_useDMDZOrder; }
   void SetUseDMDZOrder(bool useDMDZOrder) { m_useDMDZOrder = useDMDZOrder; }
   std::map<int, std::map<int, SDL_Surface*>>* GetRotatingImages() { return &m_rotatingImages; }
   std::map<int, B2SPictureBox*>* GetRotatingPictureBox() { return &m_rotatingPictureBox; }
   IlluminationGroupCollection* GetIlluminationGroups() { return &m_illuminationGroups; }
   GenericDictionaryIgnoreCase<int>* GetIlluminationLocks() { return &m_illuminationLocks; }
   PictureBoxCollection* GetIlluminations() { return &m_illuminations; }
   PictureBoxCollection* GetDMDIlluminations() { return &m_dmdIlluminations; }
   ZOrderCollection* GetZOrderImages() { return &m_zOrderImages; }
   ZOrderCollection* GetZOrderDMDImages() { return &m_zOrderDMDImages; }
   ReelBoxCollection* GetReels() { return &m_reels; }
   std::map<int, B2SReelDisplay*>* GetReelDisplays() { return &m_reelDisplays; }
   B2SPlayer* GetPlayers() { return &m_players; }
   bool IsOnAndOffImage() const { return m_onAndOffImage; }
   void SetOnAndOffImage(bool onAndOffImage) { m_onAndOffImage = onAndOffImage; }
   bool IsOffImageVisible() const { return m_offImageVisible; }
   void SetOffImageVisible(bool offImageVisible) { m_offImageVisible = offImageVisible; }
   bool IsPlayerAdded() const { return m_playerAdded; }
   void SetPlayerAdded(bool playerAdded) { m_playerAdded = playerAdded; }
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomLampIDs();
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomSolenoidIDs();
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomGIStringIDs();
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomMechIDs();
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomLampIDs4Authentic() { return &m_usedRomLampIDs4Authentic; }
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomSolenoidIDs4Authentic() { return &m_usedRomSolenoidIDs4Authentic; }
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomGIStringIDs4Authentic() { return &m_usedRomGIStringIDs4Authentic; }
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomMechIDs4Authentic() { return &m_usedRomMechIDs4Authentic; }
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomLampIDs4Fantasy() { return &m_usedRomLampIDs4Fantasy; }
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomSolenoidIDs4Fantasy() { return &m_usedRomSolenoidIDs4Fantasy; }
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomGIStringIDs4Fantasy() { return &m_usedRomGIStringIDs4Fantasy; }
   std::map<int, vector<B2SBaseBox*>>* GetUsedRomMechIDs4Fantasy() { return &m_usedRomMechIDs4Fantasy; }
   eRomIDType GetUsedTopRomIDType4Authentic() const { return m_usedTopRomIDType4Authentic; }
   void SetUsedTopRomIDType4Authentic(eRomIDType usedTopRomIDType4Authentic) { m_usedTopRomIDType4Authentic = usedTopRomIDType4Authentic; }
   eRomIDType GetUsedTopRomIDType4Fantasy() const { return m_usedTopRomIDType4Fantasy; }
   void SetUsedTopRomIDType4Fantasy(eRomIDType usedTopRomIDType4Fantasy) { m_usedTopRomIDType4Fantasy = usedTopRomIDType4Fantasy; }
   eRomIDType GetUsedSecondRomIDType4Authentic() const { return m_usedSecondRomIDType4Authentic; }
   void SetUsedSecondRomIDType4Authentic(eRomIDType usedSecondRomIDType4Authentic) { m_usedSecondRomIDType4Authentic = usedSecondRomIDType4Authentic; }
   eRomIDType GetUsedSecondRomIDType4Fantasy() const { return m_usedSecondRomIDType4Fantasy; }
   void SetUsedSecondRomIDType4Fantasy(eRomIDType usedSecondRomIDType4Fantasy) { m_usedSecondRomIDType4Fantasy = usedSecondRomIDType4Fantasy; }
   bool IsUseRotatingImage() const { return m_useRotatingImage; }
   void SetUseRotatingImage(bool useRotatingImage) { m_useRotatingImage = useRotatingImage; }
   bool IsUseMechRotatingImage() const { return m_useMechRotatingImage; }
   void SetUseMechRotatingImage(bool useMechRotatingImage) { m_useMechRotatingImage = useMechRotatingImage; }
   GenericDictionaryIgnoreCase<Dream7Display*>* GetLEDDisplays() { return &m_ledDisplays; }
   std::map<int, LEDDisplayDigitLocation*>* GetLEDDisplayDigitLocations() { return &m_ledDisplayDigits; }
   std::map<int, vector<B2SReelBox*>>* GetUsedRomReelLampIDs() { return &m_usedRomReelLampIDs; }
   GenericDictionaryIgnoreCase<SDL_Surface*>* GetReelImages() { return &m_reelImages; }
   GenericDictionaryIgnoreCase<SDL_Surface*>* GetReelIntermediateImages() { return &m_reelIntermediateImages; }
   GenericDictionaryIgnoreCase<SDL_Surface*>* GetReelIlluImages() { return &m_reelIlluImages; }
   GenericDictionaryIgnoreCase<SDL_Surface*>* GetReelIntermediateIlluImages() { return &m_reelIntermediateIlluImages; }
   GenericDictionaryIgnoreCase<Sound*>* GetSounds() { return &m_sounds; }
   GenericDictionaryIgnoreCase<B2SLEDBox*>* GetLEDs() { return &m_leds; }
   GenericDictionaryIgnoreCase<LEDAreaInfo*>* GetLEDAreas() { return &m_ledAreas; }
   std::map<int, LEDDisplayDigitLocation*>* GetLEDDisplayDigits() { return &m_ledDisplayDigits; }
   vector<vector<SDL_FPoint>>* GetLED8Seg() { return &m_led8Seg; }
   vector<vector<SDL_FPoint>>* GetLED10Seg() { return &m_led10Seg; }
   vector<vector<SDL_FPoint>>* GetLED14Seg() { return &m_led14Seg; }
   vector<vector<SDL_FPoint>>* GetLED16Seg() { return &m_led16Seg; }
   AnimationCollection* GetUsedAnimationLampIDs() { return &m_usedAnimationLampIDs; }
   AnimationCollection* GetUsedRandomAnimationLampIDs() { return &m_usedRandomAnimationLampIDs; }
   AnimationCollection* GetUsedAnimationSolenoidIDs() { return &m_usedAnimationSolenoidIDs; }
   AnimationCollection* GetUsedRandomAnimationSolenoidIDs() { return &m_usedRandomAnimationSolenoidIDs; }
   AnimationCollection* GetUsedAnimationGIStringIDs() { return &m_usedAnimationGIStringIDs; }
   AnimationCollection* GetUsedRandomAnimationGIStringIDs() { return &m_usedRandomAnimationGIStringIDs; }
   int GetLEDCoordMax() const { return m_ledCoordMax; }
   bool IsLampsData();
   bool IsSolenoidsData();
   bool IsGIStringsData();
   bool IsLEDsData();
   bool IsUseRomLamps();
   bool IsUseRomSolenoids();
   bool IsUseRomGIStrings();
   bool IsUseRomMechs();
   bool IsUseAnimationLamps();
   bool IsUseAnimationSolenoids();
   bool IsUseAnimationGIStrings();
   bool IsUseRomReelLamps();
   bool IsUseLEDs();
   bool IsUseLEDDisplays();
   bool IsUseReels();
   bool IsValid() const { return m_valid; }
   void SetValid(bool valid) { m_valid = valid; }

private:
   B2SData();
   ~B2SData();

   VPinMAMEController* m_pVPinMAME;
   string m_szTableName;
   string m_szTableFileName;
   string m_szBackglassFileName;
   int m_tableType;
   int m_dmdType;
   int m_grillHeight;
   int m_smallGrillHeight;
   SDL_Point m_dmdDefaultLocation;
   bool m_dualBackglass;
   bool m_launchBackglass;
   bool m_backglassVisible;
   bool m_useIlluminationLocks;
   bool m_useZOrder;
   bool m_useDMDZOrder;
   bool m_onAndOffImage;
   bool m_offImageVisible;
   std::map<int, std::map<int, SDL_Surface*>> m_rotatingImages;
   std::map<int, B2SPictureBox*> m_rotatingPictureBox;
   std::map<int, vector<B2SBaseBox*>> m_usedRomLampIDs4Authentic;
   std::map<int, vector<B2SBaseBox*>> m_usedRomSolenoidIDs4Authentic;
   std::map<int, vector<B2SBaseBox*>> m_usedRomGIStringIDs4Authentic;
   std::map<int, vector<B2SBaseBox*>> m_usedRomMechIDs4Authentic;
   std::map<int, vector<B2SBaseBox*>> m_usedRomLampIDs4Fantasy;
   std::map<int, vector<B2SBaseBox*>> m_usedRomSolenoidIDs4Fantasy;
   std::map<int, vector<B2SBaseBox*>> m_usedRomGIStringIDs4Fantasy;
   std::map<int, vector<B2SBaseBox*>> m_usedRomMechIDs4Fantasy;
   eRomIDType m_usedTopRomIDType4Authentic;
   eRomIDType m_usedTopRomIDType4Fantasy;
   eRomIDType m_usedSecondRomIDType4Authentic;
   eRomIDType m_usedSecondRomIDType4Fantasy;
   ZOrderCollection m_zOrderImages;
   ZOrderCollection m_zOrderDMDImages;
   PictureBoxCollection m_illuminations;
   PictureBoxCollection m_dmdIlluminations;
   IlluminationGroupCollection m_illuminationGroups;
   GenericDictionaryIgnoreCase<int> m_illuminationLocks;
   bool m_useRotatingImage;
   bool m_useMechRotatingImage;
   bool m_infoDirty;
   std::map<int, vector<B2SReelBox*>> m_usedRomReelLampIDs;
   GenericDictionaryIgnoreCase<Dream7Display*> m_ledDisplays;
   ReelBoxCollection m_reels;
   std::map<int, B2SReelDisplay*> m_reelDisplays;
   GenericDictionaryIgnoreCase<SDL_Surface*> m_reelImages;
   GenericDictionaryIgnoreCase<SDL_Surface*> m_reelIntermediateImages;
   GenericDictionaryIgnoreCase<SDL_Surface*> m_reelIlluImages;
   GenericDictionaryIgnoreCase<SDL_Surface*> m_reelIntermediateIlluImages;
   GenericDictionaryIgnoreCase<Sound*> m_sounds;
   GenericDictionaryIgnoreCase<B2SLEDBox*> m_leds;
   GenericDictionaryIgnoreCase<LEDAreaInfo*> m_ledAreas;
   std::map<int, LEDDisplayDigitLocation*> m_ledDisplayDigits;
   B2SPlayer m_players;
   bool m_playerAdded;
   vector<vector<SDL_FPoint>> m_led8Seg;
   vector<vector<SDL_FPoint>> m_led10Seg;
   vector<vector<SDL_FPoint>> m_led14Seg;
   vector<vector<SDL_FPoint>> m_led16Seg;
   int m_ledCoordMax;

   AnimationCollection m_usedAnimationLampIDs;
   AnimationCollection m_usedRandomAnimationLampIDs;
   AnimationCollection m_usedAnimationSolenoidIDs;
   AnimationCollection m_usedRandomAnimationSolenoidIDs;
   AnimationCollection m_usedAnimationGIStringIDs;
   AnimationCollection m_usedRandomAnimationGIStringIDs;

   B2SSettings* m_pB2SSettings;

   bool m_valid;
   static B2SData* m_pInstance;
};
