// license:GPLv3+

#pragma once

class LiveUI;

class InGameUI final
{
public:
   explicit InGameUI(LiveUI &liveUI);
   ~InGameUI();

   void Open();
   bool IsOpened() const { return m_isOpened; }
   void Update();
   void Close();

private:
   // UI Context
   LiveUI &m_liveUI;
   VPinball *m_app;
   Player *m_player;
   PinTable *m_table; // The edited table
   PinTable *m_live_table; // The live copy of the edited table being played by the player (all properties can be changed at any time by the script)
   class PinInput *m_pininput;
   Renderer *m_renderer;

   void DisableStaticPrepass() { if (!m_isOpened || m_staticPrepassDisabled) return; m_renderer->DisableStaticPrePass(true); m_staticPrepassDisabled = true; }

   // State
   bool m_isOpened = false;
   enum TweakType
   {
      TT_Int,
      TT_Float,
      TT_Set
   };
   struct TweakOption
   {
      TweakType type;
      float min, max, step, def;
      string name, unit;
      vector<string> options;
      TweakOption(TweakType _type, float _min, float _max, float _step, float _def, const string& _name, const string& _unit, std::initializer_list<string> _options): 
         type(_type), min(_min), max(_max), step(_step), def(_def), name(_name), unit(_unit), options(_options) { }
   };
   enum TweakPage { TP_Info, TP_Rules, TP_PointOfView, TP_VRPosition, TP_TableOption, TP_Plugin00 };
   enum BackdropSetting
   {
      BS_Page,
      // Point of View
      BS_ViewMode, BS_LookAt, BS_FOV, BS_Layback, BS_ViewHOfs, BS_ViewVOfs, BS_XYZScale, BS_XScale, BS_YScale, BS_ZScale, BS_XOffset, BS_YOffset, BS_ZOffset, BS_WndTopZOfs, BS_WndBottomZOfs,
      // VR position
      BS_VROrientation, BS_VRX, BS_VRY, BS_VRZ, BS_AR_VR, BS_VRScale,
      // Table tweaks & Custom table defined options (must be the last of this enum)
      BS_Volume, BS_BackglassVolume, BS_PlayfieldVolume, BS_DayNight, BS_Difficulty, BS_Tonemapper, BS_Exposure, BS_Custom
   };
   uint32_t m_lastTweakKeyDown = 0;
   int m_activeTweakIndex = 0;
   int m_activeTweakPageIndex = 0;
   vector<TweakPage> m_tweakPages;
   int m_tweakState[BS_Custom + 100] = {}; // 0 = unmodified, 1 = modified, 2 = resetted
   vector<BackdropSetting> m_tweakPageOptions;
   float m_tweakScroll = 0.f;
   uint32_t m_openTimeMs = 0; // Used for timed splash overlays
   bool m_staticPrepassDisabled = false;
   void HandleTweakInput();
   void UpdateTweakPage();
   PinInput::InputState m_prevInputState { 0 };
};
