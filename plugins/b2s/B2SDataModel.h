// license:GPLv3+

#pragma once

#include "common.h"
#include "tinyxml2/tinyxml2.h"

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
   B2SImage()
      : m_image(nullptr)
      , m_filename(""s)
      , m_romId(0)
      , m_romIdType(B2SRomIDType::NotDefined)
   {
   }
   B2SImage(B2SImage&& other) noexcept
      : m_image(other.m_image)
      , m_filename(other.m_filename)
      , m_romId(other.m_romId)
      , m_romIdType(other.m_romIdType)
   {
      other.m_image = nullptr; // Ressource is transfered, avoid destruction
   }
   B2SImage(const B2SImage&) = delete;
   B2SImage& operator=(const B2SImage&) = delete;
   B2SImage(const tinyxml2::XMLNode& root);
   ~B2SImage();

public:
   VPXTexture m_image; // Should be const, but needed to be set to null by move constructor (should use an unique_ptr instead ?)
   const string m_filename;
   const int m_romId;
   const B2SRomIDType m_romIdType;
};


class B2SReelImage final
{
public:
   B2SReelImage(const tinyxml2::XMLNode& image);

public:
   const string m_name;
   const VPXTexture m_image;
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
   B2SBulb(const tinyxml2::XMLNode& root);
   B2SBulb(B2SBulb&& other) noexcept;
   B2SBulb(const B2SBulb&) = delete;
   B2SBulb& operator=(const B2SBulb&) = delete;
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
   const int m_intensity;
   const int m_zOrder;
   const vec4 m_lightColor;
   const vec4 m_dodgeColor;
   const int m_illuminationMode;
   const bool m_visible;
   const int m_locationX;
   const int m_locationY;
   const int m_width;
   const int m_height;
   const bool m_isImageSnippit;
   const B2SSnippitType m_snippitType;
   const int m_snippitRotatingSteps;
   const int m_snippitRotatingAngle;
   const int m_snippitRotatingInterval;
   const B2SSnippitRotationDirection m_snippitRotatingDirection;
   const B2SSnippitRotationStopBehaviour m_snippitRotatingStopBehaviour;
   VPXTexture m_image; // Should be const, but needed to be set to null by move constructor (should use an unique_ptr instead ?)
   VPXTexture m_offImage; // Should be const, but needed to be set to null by move constructor (should use an unique_ptr instead ?)
   const string m_text;
   const int m_textAlignment;
   const string m_fontName;
   const int m_fontSize;
   const int m_fontStyle;

   bool m_isLit = false;
};


class B2SSound final
{
public:
   B2SSound(const tinyxml2::XMLNode& root);

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
   B2SAnimationStep(const tinyxml2::XMLNode& root);

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


class B2SAnimation final
{
public:
   B2SAnimation(const tinyxml2::XMLNode& root);

   void Update(float elapsedInS);

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
   bool m_playing = false;
   bool m_reverse = false;
   unsigned int m_currentStep = 0;
   float m_timeUntilNextStep = 0.f;
};


class B2STable final
{
public:
   B2STable(const tinyxml2::XMLNode& root); // Create from the root 'DirectB2SData' node

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
   const B2SImage m_backglassOnImage;
   const B2SImage m_backglassOffImage;
   const B2SImage m_dmdImage;
   const vector<B2SSound> m_sounds;
   const vector<B2SBulb> m_backglassIlluminations;
   const vector<B2SAnimation> m_backglassAnimations;
   const vector<B2SBulb> m_dmdIlluminations;
   const vector<B2SAnimation> m_dmdAnimations;
   // Missing Scores
   // Missing Reels
};
