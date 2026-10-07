// license:GPLv3+

#pragma once

#include <atomic>
#include <unordered_map>

#include "common.h"
#include "tinyxml2/tinyxml2.h"

namespace B2S {

enum class B2SRomIDType
{
   NotDefined = 0,
   Lamp = 1,
   Solenoid = 2,
   GIString = 3,
   Mech = 4
};


class B2SImage final
{
public:
   B2SImage();
   explicit B2SImage(const tinyxml2::XMLNode& root) noexcept;
   ~B2SImage();

public:
   const VPXTexture m_image;
   const string m_filename;
   const int m_romId;
   const B2SRomIDType m_romIdType;
   const bool m_romInverted;

   float m_brightness = 0.f;
   std::function<void()> m_romUpdater = []() { };
};


class B2SReelImage final
{
public:
   explicit B2SReelImage(const tinyxml2::XMLNode& image, int setId = 0) noexcept;
   ~B2SReelImage();

   // Rolling intermediate image n (1..CountOfIntermediates), nullptr if not available
   VPXTexture GetIntermediate(int n) const { return (n >= 1 && n <= static_cast<int>(m_intermediates.size())) ? m_intermediates[static_cast<size_t>(n - 1)] : nullptr; }

public:
   const string m_name;
   const int m_countOfIntermediate;
   const VPXTexture m_image;
   vector<VPXTexture> m_intermediates; // IntermediateImage1..CountOfIntermediates shown while the reel rolls to the next digit
};


class B2SReel final
{
public:
   explicit B2SReel(const tinyxml2::XMLNode& root) noexcept;

   // name is the reel type ("reel_00"/"reel_0"), index the digit (-1 = empty).
   // When illuminated, images are taken from the illuminated pool, with a _setId name suffix when setId > 0.
   const B2SReelImage* GetImage(const string& name, int index, bool illuminated = false, int setId = 0) const;

public:
   const vector<std::unique_ptr<B2SReelImage>> m_images;
   const vector<std::unique_ptr<B2SReelImage>> m_illuImages; // Illuminated image sets (names carry their _setId suffix)
};


// Runtime state of a rolling reel digit (render thread only). Digits roll one step per
// interval, showing the intermediate images of the current digit in between, until they
// reach their target value.
class B2SReelDigit final
{
public:
   // Set the digit to display. When animate is false, or for non digit values (blank), the change is instant
   void SetTarget(int value, bool animate);
   // Advances the rolling animation; returns true each time a reel step completes (=> play the reel sound)
   bool Update(float elapsedInS, int rollingIntervalMs, int intermediates, bool rollUp);
   int Current() const { return m_current; }
   int Intermediate() const { return m_intermediate; } // 0 = digit image, n > 0 = intermediate image n
   bool IsRolling() const { return m_rolling; }
   // Illuminated image selection changed: skip the remaining intermediate steps (B2SReelBox.Illuminated behaviour)
   void SetIlluminated(bool value, int intermediates);
   // True while this digit still has to cross the 9->0 (up) or 0->9 (down) boundary to reach its target.
   // More significant digits wait for the rollover before rolling (display carry behaviour)
   bool HasPendingWrap(bool rollUp) const { return m_rolling && (rollUp ? (m_current > m_target) : (m_current < m_target)); }

private:
   int m_current = 0;
   int m_target = 0;
   int m_intermediate = 0;
   int m_settle = 0; // Settle ticks between two digit steps
   float m_accMs = 0.f;
   bool m_rolling = false;
   bool m_illuminated = false;
};


enum class B2SScoreRenderer
{
   Reel, Dream7, LED, RenderedLED, ImportedLED
};


enum class B2SScoreType
{
   NotUsed = 0,
   Scores = 1,
   Credits = 2
};


class B2SScore final
{
public:
   explicit B2SScore(const tinyxml2::XMLNode& root) noexcept;

   // Distribute a score value over this display's digits (right aligned, keeping
   // its rightmost digits if it does not fit). Returns one value per digit:
   // digit value (0-9) or -1 for a blank digit (leading padding for LED displays).
   vector<int> DistributeScore(int value) const;

   // DisplayState=1 marks the score display as initially hidden
   bool IsHidden() const { return m_displayState == 1; }

public:
   const int m_id;
   const int m_b2sStartDigit;
   int m_resolvedStartDigit = 0; // Effective first digit (m_b2sStartDigit or auto-assigned)
   const B2SScoreType m_b2sScoreType;
   const int m_b2sPlayerNo;
   const string m_reelType;
   const int m_reelIlluLocation;
   const int m_reelIlluIntensity;
   const int m_reelIlluB2SID;
   const int m_reelIlluB2SIDType;
   const int m_reelIlluB2SValue;
   const int m_reelIlluImageSet; // ReelIlluImageSet: illuminated image set index (0 = none)
   const vec4 m_reelLitColor;
   const vec4 m_reelDarkColor;
   const int m_glow;
   const int m_thickness;
   const int m_shear;
   const int m_digits;
   const int m_spacing;
   const int m_displayState;
   const int m_locX;
   const int m_locY;
   const int m_width;
   const int m_height;
   const string m_soundName;
   const vector<string> m_soundNames; // Per-digit reel sounds (Sound1..SoundN attributes), "" means default, "stille" means silent
   const int m_ledSegments; // Segment count from the ReelType suffix (Dream7LEDx/RenderedLEDx): 7, 10 or 14, else 0

   const B2SScoreRenderer m_scoreType;

   float m_reelIllu = 0.f; // Current ROM value of the ReelIlluB2SID lamp (render thread)
   std::function<void()> m_reelIlluUpdater = []() { };
};


enum class B2SReelRollingDirection
{
   Up = 0,
   Down = 1,
};


class B2SScores final
{
public:
   explicit B2SScores(const tinyxml2::XMLNode& root) noexcept;

public:
   const int m_reelCountOfIntermediates;
   const B2SReelRollingDirection m_reelRollingDirection;
   const int m_reelRollingInterval;
   vector<B2SScore> m_scores; // filled by B2STable (digit numbering spans both parents in file order)
};


enum class B2SSnippitType
{
   StandardImage = 0,
   SelfRotatingImage = 1,
   MechRotatingImage = 2
};


enum class B2SSnippitRotationDirection
{
   Clockwise = 0,
   AntiClockwise = 1
};


enum class B2SSnippitRotationStopBehaviour
{
   SpinOff = 0,
   StopImmediatelly = 1,
   RunAnimationTillEnd = 2,
   RunAnimationToFirstStep = 3
};


enum class B2SDualMode
{
   Both = 0,
   Authentic = 1,
   Fantasy = 2
};


enum class B2SDualMode2
{
   NotSet = 0,
   Authentic = 1,
   Fantasy = 2
};


class B2SBulb final
{
public:
   explicit B2SBulb(const tinyxml2::XMLNode& root) noexcept;
   ~B2SBulb();

public:
   const int m_id;
   const string m_name;
   const int m_b2sId;
   const int m_b2sValue;
   const int m_romId;
   const B2SRomIDType m_romIdType;
   const bool m_romInverted;
   const bool m_initialState;
   const B2SDualMode m_dualMode;
   const int m_intensity; // Unused property in original B2S
   const int m_zOrder;
   const vec4 m_lightColor;
   const vec4 m_dodgeColor;
   const int m_illuminationMode;
   const bool m_visible;
   int m_locationX; // Mutable: can be repositioned through B2SSetPos
   int m_locationY;
   const int m_width;
   const int m_height;
   const bool m_isImageSnippit; // Image snippit have their initial state applied before others on startup, didn't find any other difference
   const B2SSnippitType m_snippitType;
   const int m_snippitRotatingSteps;
   const int m_snippitRotatingInterval;
   const B2SSnippitRotationDirection m_snippitRotatingDirection;
   const B2SSnippitRotationStopBehaviour m_snippitRotatingStopBehaviour;
   const VPXTexture m_image;
   const VPXTexture m_offImage;
   const string m_text;
   const int m_textAlignment;
   const string m_fontName;
   const int m_fontSize;
   const int m_fontStyle;

public:
   // Self-rotating image runtime. Start/Stop are thread safe requests consumed by UpdateRotation (render thread)
   void StartRotation() { m_rotRequest = 1; }
   void StopRotation() { m_rotRequest = 2; }
   void UpdateRotation(float elapsedInS);
   bool IsRotating() const { return m_rotating; }
   float GetRotationAngle() const { return m_selfRotAngle; }

public:
   std::function<void()> m_romUpdater = []() { };
   float m_brightness = 0.f;
   float m_mechRot = 0.f;
   float m_romOn = 0.f; // ROM on/off state for self-rotating images (drives rotation start/stop)
   bool m_spinDriverOn = false; // Last driving state applied to self-rotation (render thread only)
   bool m_bakedIntoBackground = false; // Bulb shares the BackglassOnImage ROM channel: it is drawn as part of the background, not as a separate bulb

private:
   std::atomic<int> m_rotRequest { 0 }; // 1 = start, 2 = stop
   bool m_rotating = false;
   float m_selfRotAngle = 0.f;
   float m_rotIntervalMs = 0.f;
   float m_slowdownAccMs = 0.f;
   int m_rotateSlowDown = 0;
   bool m_rotateRunTillEnd = false;
   bool m_rotateRunToFirstStep = false;
};


class B2SSound final
{
public:
   explicit B2SSound(const tinyxml2::XMLNode& root) noexcept;

public:
   const string m_name;
   const std::shared_ptr<vector<uint8_t>> m_wav;
};


enum class B2SDMDType
{
   NotDefined = 0,
   NoB2SDMD = 1,
   B2SAlwaysOnSecondMonitor = 2,
   B2SAlwaysOnThirdMonitor = 3,
   B2SOnSecondOrThirdMonitor = 4
};


class B2SAnimationStep final
{
public:
   explicit B2SAnimationStep(const tinyxml2::XMLNode& root) noexcept;

public:
   const int m_step;
   const vector<string> m_on;
   const int m_waitLoopsAfterOn;
   const vector<string> m_off;
   const int m_waitLoopsAfterOff;
   const int m_pulseSwitch;
};


enum class B2SLightsStateAtAnimationStart
{
   Undefined = 0,
   InvolvedLightsOff = 1,
   InvolvedLightsOn = 2,
   LightsOff = 3,
   NoChange = 4
};


enum class B2SLightsStateAtAnimationEnd
{
   Undefined = 0,
   InvolvedLightsOff = 1,
   InvolvedLightsOn = 2,
   LightsReseted = 3,
   NoChange = 4
};


enum class B2SAnimationStopBehaviour
{
   Undefined = 0,
   StopImmediatelly = 1,
   RunAnimationTillEnd = 2,
   RunAnimationToFirstStep = 3
};


class B2SAnimation;

// Effect interface used by the animation engine, provided by the server through the renderer
struct B2SAnimationEffects
{
   std::function<void(const string& group, bool on)> setGroup;
   std::function<float(const string& group)> getGroup;
   std::function<void(const string& group)> lockGroup;
   std::function<void(const string& group)> unlockGroup;
   std::function<void(int switchId)> pulseSwitch;
   std::function<void(bool hidden)> setScoreDisplaysHidden;
   std::function<void()> allLightsOff;
   std::function<std::unordered_map<string, float>()> snapshotAllLights;
   std::function<void(const std::unordered_map<string, float>&)> restoreAllLights;
   // Edge event on a ROM trigger of a RandomStart animation (handled by the renderer which owns the animation pool)
   std::function<void(B2SRomIDType romIdType, int romId, bool start, B2SAnimation* self)> randomTrigger;
   // Active dual-backglass mode; Both when the table does not define a dual backglass (no filtering applied)
   B2SDualMode dualMode = B2SDualMode::Both;
};


class B2SAnimation final
{
public:
   explicit B2SAnimation(const tinyxml2::XMLNode& root) noexcept;
   B2SAnimation(B2SAnimation&&) noexcept = default;

   // Animation runtime, driven by the renderer once per frame
   void Update(float elapsedInS, const B2SAnimationEffects& fx);
   bool IsRunning() const;
   void Start(bool reverse = false); // Thread safe script-side request
   void Stop(); // Thread safe script-side request

   // ROM event trigger parsed from IDJoin (lamp/solenoid/GI string, optionally inverted)
   struct RomTrigger
   {
      B2SRomIDType romIdType;
      int romId;
      bool inverted;
   };
   const vector<RomTrigger>& GetRomTriggers() const { return m_romTriggers; }
   const vector<string>& GetLightsInvolved() const { return m_lightsInvolved; }

   // True when the animation has no playable content (mirrors the reference which drops such animations)
   bool IsEmpty() const { return m_entryActions.empty(); }

   // Rebind ROM trigger state readers (called by the renderer when the ROM state sources change)
   using RomTriggerResolver = std::function<std::function<void()>(B2SRomIDType romIdType, int romId, bool inverted, float* target)>;
   void BindRomTriggers(const RomTriggerResolver& resolver);

public:
   const string m_name;
   const B2SDualMode m_dualMode;
   const int m_interval;
   const int m_loops;
   const string m_idJoin;
   const bool m_startAnimationAtBackglassStartup;
   const bool m_allLightsOffAtAnimationStart;
   const B2SLightsStateAtAnimationStart m_lightsStateAtAnimationStart;
   const bool m_resetLightsAtAnimationEnd;
   const B2SLightsStateAtAnimationEnd m_lightsStateAtAnimationEnd;
   const bool m_runAnimationTilEnd;
   const B2SAnimationStopBehaviour m_animationStopBehaviour;
   const bool m_lockInvolvedLamps;
   const bool m_hideScoreDisplays;
   const bool m_bringToFront;
   const bool m_randomStart;
   const int m_randomQuality;
   const vector<B2SAnimationStep> m_animationSteps;

private:
   struct EntryAction
   {
      vector<string> groups;
      int waitLoops; // Interval multiplier waited after this action (0 = same tick as next action)
      bool on;
      int corrector; // Reverse playback mapping to the matching counterpart action
      int pulseSwitch;
   };
   vector<EntryAction> m_entryActions; // Expanded steps (on/off pairs)
   vector<string> m_lightsInvolved;
   vector<RomTrigger> m_romTriggers;

   struct Runtime
   {
      std::atomic<int> request { 0 }; // 1=start forward, 2=start reverse, 3=stop
      std::atomic<bool> running { false };
      bool reverse = false;
      bool stopMeLater = false;
      bool reachedThe0Point = false;
      int ticker = 0;
      int loopTicker = 0;
      float timeUntilNextStep = 0.f;
      std::unordered_map<string, float> lightSnapshot;
      vector<float> triggerValues;
      vector<bool> triggerPrev;
      vector<std::function<void()>> triggerUpdaters;
   };
   std::unique_ptr<Runtime> m_runtime;

   void BeginRun(const B2SAnimationEffects& fx, bool reverse);
   void EndRun(const B2SAnimationEffects& fx);
   int Tick(const B2SAnimationEffects& fx); // returns the number of interval loops to wait
};


class B2STable final
{
public:
   explicit B2STable(const tinyxml2::XMLNode& root) noexcept; // Create from the root 'DirectB2SData' node

public:
   // Find the score display matching the given display id (searches both backglass and DMD displays)
   const B2SScore* FindScoreDisplay(int displayId) const;
   // Find the score display owning the given resolved digit index (1-based, both backglass and DMD displays)
   const B2SScore* FindScoreDigitDisplay(int digit) const;

public:
   const string m_version;
   const string m_name;
   const int m_tableType;
   const B2SDMDType m_dmdType;
   const int m_dmdDefaultLocationX;
   const int m_dmdDefaultLocationY;
   const int m_grillHeight;
   const int m_grillSmallHeight;
   const int m_lampsDefaultSkipFrames;
   const int m_solenoidsDefaultSkipFrames;
   const int m_giStringsDefaultSkipFrames;
   const int m_ledsDefaultSkipFrames;
   const string m_projectGUID;
   const string m_projectGUID2;
   const string m_assemblyGUID;
   const string m_vsName;
   const bool m_dualBackglass;
   const string m_author;
   const string m_artwork;
   const string m_gameName;
   const B2SImage m_thumbnailImage;
   const B2SImage m_backglassImage;
   B2SImage m_backglassOnImage;
   B2SImage m_backglassOffImage;
   const B2SImage m_dmdImage;
   const vector<B2SSound> m_sounds;
   const B2SReel m_reels;
   B2SScores m_backglassScores;
   B2SScores m_dmdScores;
   vector<std::unique_ptr<B2SBulb>> m_backglassIlluminations;
   vector<B2SAnimation> m_backglassAnimations;
   vector<std::unique_ptr<B2SBulb>> m_dmdIlluminations;
   vector<B2SAnimation> m_dmdAnimations;
   // Missing Scores
};


// LED script API helpers (B2SSetLED/B2SSetLEDDisplay): convert a character or a Dream7 segment bit
// code to a 16 bit luminance mask matching the CTLPI/PinMAME bit order for the display's layout
// (ledSegments is the ReelType suffix: 7, 10 or 14).
uint16_t B2SSegmentCharMask(char c, int ledSegments);
uint16_t B2SSegmentTranslateBitCode(uint32_t bits, int ledSegments);
}
