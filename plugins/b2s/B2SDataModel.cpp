// license:GPLv3+

#include <charconv>
#include <format>

#include "common.h"
#include "B2SDataModel.h"
#include "B2SServer.h"

#include "tinyxml2/tinyxml2.h"

namespace B2S {

static const tinyxml2::XMLElement* GetNode(const tinyxml2::XMLNode& doc, const std::string& nodePath)
{
   const tinyxml2::XMLNode* node = &doc;
   size_t pos = 0;
   size_t nextPos = nodePath.find('/', pos);
   while (nextPos != std::string::npos)
   {
      const std::string elementName = nodePath.substr(pos, nextPos - pos);
      node = node->FirstChildElement(elementName.c_str());
      if (!node)
         return nullptr;
      pos = nextPos + 1;
      nextPos = nodePath.find('/', pos);
   }
   if (const std::string finalElementName = nodePath.substr(pos); !finalElementName.empty())
      node = node->FirstChildElement(finalElementName.c_str());
   return node ? node->ToElement() : nullptr;
}

static string GetStringAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName, const string& defVal)
{
   if (const tinyxml2::XMLElement* node = GetNode(doc, nodePath); node)
   {
      const char* value = node->Attribute(attributeName.c_str());
      if (value)
         return value;
   }
   return defVal;
}

static int GetIntAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName, const int& defVal)
{
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   return node ? node->IntAttribute(attributeName.c_str(), defVal) : defVal;
}

static bool GetBoolAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName, const bool& defVal)
{
   return GetIntAttribute(doc, nodePath, attributeName, defVal ? 1 : 0) != 0;
}

static vec4 GetColorAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName, const vec4& defVal)
{
   if (const tinyxml2::XMLElement* node = GetNode(doc, nodePath); node)
   {
      const char* value = node->Attribute(attributeName.c_str());
      if (value)
      {
         std::istringstream ss(value);
         string token;
         vector<int> colorValues;
         while (std::getline(ss, token, '.'))
         {
            int result;
            std::from_chars(token.c_str(), token.c_str() + token.length(), result);
            colorValues.push_back(result);
         }
         if (colorValues.size() == 3)
            return vec4(static_cast<float>(colorValues[0]) / 255.f, static_cast<float>(colorValues[1]) / 255.f, static_cast<float>(colorValues[2]) / 255.f, 1.f);
      }
   }
   return defVal;
}

static VPXTexture GetTextureAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName)
{
   if (const tinyxml2::XMLElement* node = GetNode(doc, nodePath); node)
   {
      const char* value = node->Attribute(attributeName.c_str());
      if (value)
      {
         vector<uint8_t> decoded = base64_decode(value, strlen(value));
         if (decoded.empty())
         {
            string path;
            const tinyxml2::XMLNode* parent = node;
            while (parent != nullptr)
            {
               if (parent->ToElement() != nullptr)
                  path = "/"s + parent->ToElement()->Name() + path;
               parent = parent->Parent();
            }
            LOGE("Failed to decode image at line %d (%s)", node->GetLineNum(), path.c_str());
         }
         return CreateTexture(decoded.data(), static_cast<int>(decoded.size()));
      }
   }
   return nullptr;
}

static std::shared_ptr<vector<uint8_t>> GetSoundAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName)
{
   if (const tinyxml2::XMLElement* node = GetNode(doc, nodePath); node)
   {
      const char* value = node->Attribute(attributeName.c_str());
      if (value)
      {
         vector<uint8_t> decoded_wav = base64_decode(value, strlen(value));
         return std::make_shared<vector<uint8_t>>(std::move(decoded_wav));
      }
   }
   return nullptr;
}

static B2SImage GetImageAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath)
{
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   return B2SImage(*node);
}

template <class T> static vector<T> GetList(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& subNodeName)
{
   vector<T> list;
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node == nullptr)
      return list;
   for (auto subNode = node->FirstChildElement(subNodeName.c_str()); subNode != nullptr; subNode = subNode->NextSiblingElement(subNodeName.c_str()))
      list.emplace_back(*subNode);
   return list;
}

template <class T> static vector<std::unique_ptr<T>> GetPtrList(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& subNodeName)
{
   vector<std::unique_ptr<T>> list;
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node == nullptr)
      return list;
   for (auto subNode = node->FirstChildElement(subNodeName.c_str()); subNode != nullptr; subNode = subNode->NextSiblingElement(subNodeName.c_str()))
      list.emplace_back(std::make_unique<T>(*subNode));
   return list;
}

template <class T> static vector<T> GetFilteredList(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& subNodeName, bool isDMD)
{
   vector<T> list;
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node == nullptr)
      return list;
   for (auto subNode = node->FirstChildElement(subNodeName.c_str()); subNode != nullptr; subNode = subNode->NextSiblingElement(subNodeName.c_str()))
   {
      bool isBackglass = subNode->Attribute("Parent", "Backglass") != nullptr;
      if (isBackglass == !isDMD)
         list.emplace_back(*subNode);
   }
   return list;
}

template <class T> static vector<std::unique_ptr<T>> GetFilteredPtrList(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& subNodeName, bool isDMD)
{
   vector<std::unique_ptr<T>> list;
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node == nullptr)
      return list;
   for (auto subNode = node->FirstChildElement(subNodeName.c_str()); subNode != nullptr; subNode = subNode->NextSiblingElement(subNodeName.c_str()))
   {
      bool isBackglass = subNode->Attribute("Parent", "Backglass") != nullptr;
      if (isBackglass == !isDMD)
         list.emplace_back(std::make_unique<T>(*subNode));
   }
   return list;
}

static std::vector<std::string> GetStringList(const std::string& str, char delimiter)
{
   std::vector<std::string> tokens;
   std::istringstream tokenStream(str);
   std::string token;
   while (std::getline(tokenStream, token, delimiter))
      tokens.push_back(token);
   return tokens;
}

static bool StartsWithCaseInsensitive(const std::string& str, const std::string& prepend) { return string_to_lower(str).starts_with(string_to_lower(prepend)); }


B2SSound::B2SSound(const tinyxml2::XMLNode& root)
   : m_name(GetStringAttribute(root, ""s, "Name"s, ""s))
   , m_wav(GetSoundAttribute(root, ""s, "Value"s))
{
}


B2SBulb::B2SBulb(const tinyxml2::XMLNode& root)
   : m_id(GetIntAttribute(root, ""s, "RomID"s, 0))
   , m_name(GetStringAttribute(root, ""s, "Name"s, ""s))
   , m_b2sId(GetIntAttribute(root, ""s, "B2SID"s, -1))
   , m_b2sValue(GetIntAttribute(root, ""s, "B2SValue"s, 0))
   , m_romId(GetIntAttribute(root, ""s, "RomID"s, -1))
   , m_romIdType(static_cast<B2SRomIDType>(GetIntAttribute(root, ""s, "RomIDType"s, 0)))
   , m_romInverted(GetBoolAttribute(root, ""s, "RomInverted"s, false))
   , m_initialState(GetBoolAttribute(root, ""s, "InitialState"s, false))
   , m_dualMode(static_cast<B2SDualMode>(GetIntAttribute(root, ""s, "DualMode"s, 0)))
   , m_intensity(GetIntAttribute(root, ""s, "Intensity"s, 0))
   , m_zOrder(GetIntAttribute(root, ""s, "ZOrder"s, 0))
   , m_lightColor(GetColorAttribute(root, ""s, "LightColor"s, vec4(1.f, 1.f, 1.f, 1.f)))
   , m_dodgeColor(GetColorAttribute(root, ""s, "DodgeColor"s, vec4(0.f, 0.f, 0.f, 1.f)))
   , m_illuminationMode(GetIntAttribute(root, ""s, "IlluMode"s, 0))
   , m_visible(GetBoolAttribute(root, ""s, "Visible"s, true))
   , m_locationX(GetIntAttribute(root, ""s, "LocX"s, 0))
   , m_locationY(GetIntAttribute(root, ""s, "LocY"s, 0))
   , m_width(GetIntAttribute(root, ""s, "Width"s, 0))
   , m_height(GetIntAttribute(root, ""s, "Height"s, 0))
   , m_isImageSnippit(GetBoolAttribute(root, ""s, "IsImageSnippit"s, false))
   , m_snippitType(static_cast<B2SSnippitType>(GetIntAttribute(root, ""s, "SnippitType"s, 0)))
   , m_snippitRotatingSteps(GetIntAttribute(root, ""s, "SnippitRotatingAngle"s, 0) != 0 ? (360 / GetIntAttribute(root, ""s, "SnippitRotatingAngle"s, 1)) : GetIntAttribute(root, ""s, "SnippitRotatingSteps"s, 0))
   , m_snippitRotatingInterval(GetIntAttribute(root, ""s, "SnippitRotatingInterval"s, 0))
   , m_snippitRotatingDirection(static_cast<B2SSnippitRotationDirection>(GetIntAttribute(root, ""s, "eSnippitRotationDirection"s, 0)))
   , m_snippitRotatingStopBehaviour(static_cast<B2SSnippitRotationStopBehaviour>(GetIntAttribute(root, ""s, "SnippitRotatingStopBehaviour"s, 0)))
   , m_image(GetTextureAttribute(root, ""s, "Image"s))
   , m_offImage(GetTextureAttribute(root, ""s, "OffImage"s))
   , m_text(GetStringAttribute(root, ""s, "Text"s, ""s))
   , m_textAlignment(GetIntAttribute(root, ""s, "TextAlignment"s, 0))
   , m_fontName(GetStringAttribute(root, ""s, "FontName"s, ""s))
   , m_fontSize(GetIntAttribute(root, ""s, "FontSize"s, 0))
   , m_fontStyle(GetIntAttribute(root, ""s, "FontStyle"s, 0))
{
   if (m_snippitType == B2SSnippitType::MechRotatingImage)
      m_brightness = 1.f;
   else
      m_brightness = m_initialState ? 1.f : 0.f;
}

B2SBulb::~B2SBulb()
{
   DeleteTexture(m_image);
   DeleteTexture(m_offImage);
}


B2SImage::B2SImage(const tinyxml2::XMLNode& root)
   : m_image(GetTextureAttribute(root, ""s, "Value"s))
   , m_filename(GetStringAttribute(root, ""s, "FileName"s, ""s))
   , m_romId(GetIntAttribute(root, ""s, "RomID"s, 0))
   , m_romIdType(static_cast<B2SRomIDType>(GetIntAttribute(root, ""s, "RomIDType"s, 0)))
{
}

B2SImage::~B2SImage()
{
   DeleteTexture(m_image);
}


B2SReelImage::B2SReelImage(const tinyxml2::XMLNode& root)
   : m_name(GetStringAttribute(root, ""s, "Name"s, ""s))
   , m_countOfIntermediate(GetIntAttribute(root, ""s, "CountOfIntermediates"s, 0))
   , m_image(GetTextureAttribute(root, ""s, "Image"s))
{
}

B2SReelImage::~B2SReelImage()
{
   DeleteTexture(m_image);
}


B2SReel::B2SReel(const tinyxml2::XMLNode& root)
   : m_images(GetNode(root, "Reels"s) ? GetPtrList<B2SReelImage>(*GetNode(root, "Reels"s), "Images"s, "Image"s) : vector<std::unique_ptr<B2SReelImage>>())
{
}

B2SReelImage* B2SReel::GetImage(const string& name, int index) const
{
   string imgName;
   if (name.ends_with("_00"))
   {
      if (index < 0)
         imgName = std::format("{}Empty", name.substr(0, name.length() - 2));
      else
         imgName = std::format("{}{:02}", name.substr(0, name.length() - 2), index);
   }
   else if (name.ends_with("_0"))
   {
      if (index < 0)
         imgName = std::format("{}Empty", name.substr(0, name.length() - 1));
      else
         imgName = std::format("{}{:01}", name.substr(0, name.length() - 1), index);
   }
   else
      return nullptr;
   for (const auto& img : m_images)
   {
      if (img->m_name == imgName)
      {
         return img.get();
      }
   }
   return nullptr;
}


B2SScore::B2SScore(const tinyxml2::XMLNode& root)
   : m_id(GetIntAttribute(root, ""s, "ID"s, 0))
   , m_b2sStartDigit(GetIntAttribute(root, ""s, "B2SStartDigit"s, 0))
   , m_b2sScoreType(static_cast<B2SScoreType>(GetIntAttribute(root, ""s, "B2SScoreType"s, 0)))
   , m_b2sPlayerNo(GetIntAttribute(root, ""s, "B2SPlayerNo"s, 0))
   , m_reelType(GetStringAttribute(root, ""s, "ReelType"s, ""s))
   , m_reelIlluLocation(GetIntAttribute(root, ""s, "ReelIlluLocation"s, 0))
   , m_reelIlluIntensity(GetIntAttribute(root, ""s, "B2SStartDigit"s, 0))
   , m_reelIlluB2SID(GetIntAttribute(root, ""s, "ReelIlluB2SID"s, 0))
   , m_reelIlluB2SIDType(GetIntAttribute(root, ""s, "ReelIlluB2SIDType"s, 0))
   , m_reelIlluB2SValue(GetIntAttribute(root, ""s, "ReelIlluB2SValue"s, 0))
   , m_reelLitColor(GetColorAttribute(root, ""s, "ReelLitColor"s, vec4(1.f, 1.f, 1.f, 1.f)))
   , m_reelDarkColor(GetColorAttribute(root, ""s, "ReelDarkColor"s, vec4(0.f, 0.f, 0.f, 1.f)))
   , m_glow(GetIntAttribute(root, ""s, "Glow"s, 0))
   , m_thickness(GetIntAttribute(root, ""s, "Thickness"s, 0))
   , m_shear(GetIntAttribute(root, ""s, "Shear"s, 0))
   , m_digits(GetIntAttribute(root, ""s, "Digits"s, 0))
   , m_spacing(GetIntAttribute(root, ""s, "Spacing"s, 0))
   , m_displayState(GetIntAttribute(root, ""s, "DisplayState"s, 0))
   , m_locX(GetIntAttribute(root, ""s, "LocX"s, 0))
   , m_locY(GetIntAttribute(root, ""s, "LocY"s, 0))
   , m_width(GetIntAttribute(root, ""s, "Width"s, 0))
   , m_height(GetIntAttribute(root, ""s, "Height"s, 0))
   , m_soundName(GetStringAttribute(root, ""s, "Sound"s, "stille"s))
   , m_scoreType(StartsWithCaseInsensitive(m_reelType, "dream7"s)  ? B2SScoreRenderer::Dream7
           : StartsWithCaseInsensitive(m_reelType, "rendered"s)    ? B2SScoreRenderer::RenderedLED
           : StartsWithCaseInsensitive(m_reelType, "LED"s)         ? B2SScoreRenderer::LED
           : StartsWithCaseInsensitive(m_reelType, "ImportedLED"s) ? B2SScoreRenderer::ImportedLED
                                                                   : B2SScoreRenderer::Reel
   )
{
}

B2SScores::B2SScores(const tinyxml2::XMLNode& root, const bool isDMD)
   : m_reelCountOfIntermediates(GetIntAttribute(root, "Scores"s, "ReelCountOfIntermediates"s, 0))
   , m_reelRollingDirection(GetStringAttribute(root, "Scores"s, "ReelRollingDirection"s, "Up"s) == "Up"s ? B2SReelRollingDirection::Up : B2SReelRollingDirection::Down)
   , m_reelRollingInterval(GetIntAttribute(root, "Scores"s, "ReelRollingInterval"s, 0))
   , m_scores(GetFilteredList<B2SScore>(root, "Scores"s, "Score"s, isDMD))
{
}


B2SAnimationStep::B2SAnimationStep(const tinyxml2::XMLNode& root)
   : m_step(GetIntAttribute(root, ""s, "Step"s, 0))
   , m_on(GetStringList(GetStringAttribute(root, ""s, "On"s, ""s), ','))
   , m_waitLoopsAfterOn(GetIntAttribute(root, ""s, "WaitLoopsAfterOn"s, 0))
   , m_off(GetStringList(GetStringAttribute(root, ""s, "Off"s, ""s), ','))
   , m_waitLoopsAfterOff(GetIntAttribute(root, ""s, "WaitLoopsAfterOff"s, 0))
   , m_pulseSwitch(GetIntAttribute(root, ""s, "PulseSwitch"s, 0))
{
}


B2SAnimation::B2SAnimation(const tinyxml2::XMLNode& root)
   : m_name(GetStringAttribute(root, ""s, "Name"s, ""s))
   , m_dualMode(static_cast<B2SDualMode>(GetIntAttribute(root, ""s, "DualMode"s, 0)))
   , m_interval(GetIntAttribute(root, ""s, "Interval"s, 0))
   , m_loops(GetIntAttribute(root, ""s, "Loops"s, 0))
   , m_idJoin(GetStringAttribute(root, ""s, "IDJoin"s, ""s))
   , m_startAnimationAtBackglassStartup(GetBoolAttribute(root, ""s, "StartAnimationAtBackglassStartup"s, false))
   , m_allLightsOffAtAnimationStart(GetBoolAttribute(root, ""s, "AllLightsOffAtAnimationStart"s, false))
   , m_lightsStateAtAnimationStart(static_cast<B2SLightsStateAtAnimationStart>(GetIntAttribute(root, ""s, "LightsStateAtAnimationStart"s, 0)))
   , m_resetLightsAtAnimationEnd(GetBoolAttribute(root, ""s, "ResetLightsAtAnimationEnd"s, false))
   , m_lightsStateAtAnimationEnd(static_cast<B2SLightsStateAtAnimationEnd>(GetIntAttribute(root, ""s, "LightsStateAtAnimationEnd"s, 0)))
   , m_runAnimationTilEnd(GetBoolAttribute(root, ""s, "RunAnimationTilEnd"s, false))
   , m_animationStopBehaviour(static_cast<B2SAnimationStopBehaviour>(GetIntAttribute(root, ""s, "AnimationStopBehaviour"s, 0)))
   , m_lockInvolvedLamps(GetBoolAttribute(root, ""s, "LockInvolvedLamps"s, false))
   , m_hideScoreDisplays(GetBoolAttribute(root, ""s, "HideScoreDisplays"s, false))
   , m_bringToFront(GetBoolAttribute(root, ""s, "BringToFront"s, false))
   , m_randomStart(GetBoolAttribute(root, ""s, "RandomStart"s, false))
   , m_randomQuality(GetIntAttribute(root, ""s, "RandomQuality"s, 0))
   , m_animationSteps(GetList<B2SAnimationStep>(root, ""s, "AnimationStep"s))
{
   if (m_startAnimationAtBackglassStartup)
      m_playing = true;
}

void B2SAnimation::Update(float elapsedInS)
{
   if (!m_playing)
      return;

   m_timeUntilNextStep -= elapsedInS;
   // FIXME this can lock up as m_timeUntilNextStep can have invalid values
   /* while (m_timeUntilNextStep < 0.f)
   {
      if (m_reverse)
         m_currentStep = (m_currentStep - 1 + static_cast<int>(m_animationSteps.size())) % static_cast<int>(m_animationSteps.size());
      else
         m_currentStep = (m_currentStep + 1) % static_cast<int>(m_animationSteps.size());



      m_timeUntilNextStep += static_cast<float>(m_interval) / 1000.f;
   } */
}

B2STable::B2STable(const tinyxml2::XMLNode& root)
   : m_version(GetStringAttribute(root, ""s, "Version"s, ""s))
   , m_name(GetStringAttribute(root, "Name"s, "Value"s, ""s))
   , m_tableType(GetIntAttribute(root, "TableType"s, "Value"s, 0))
   , m_dmdType(static_cast<B2SDMDType>(GetIntAttribute(root, "DMDType"s, "Value"s, 0)))
   , m_dmdDefaultLocationX(GetIntAttribute(root, "DMDDefaultLocation"s, "LocX"s, 0))
   , m_dmdDefaultLocationY(GetIntAttribute(root, "DMDDefaultLocation"s, "LocY"s, 0))
   , m_grillHeight(GetIntAttribute(root, "GrillHeight"s, "Value"s, 0))
   , m_grillSmallHeight(GetIntAttribute(root, "GrillHeight"s, "Small"s, 0))
   , m_lampsDefaultSkipFrames(GetIntAttribute(root, "LampsDefaultSkipFrames"s, "Value"s, 0))
   , m_solenoidsDefaultSkipFrames(GetIntAttribute(root, "SolenoidsDefaultSkipFrames"s, "Value"s, 0))
   , m_giStringsDefaultSkipFrames(GetIntAttribute(root, "GIStringsDefaultSkipFrames"s, "Value"s, 0))
   , m_ledsDefaultSkipFrames(GetIntAttribute(root, "LEDsDefaultSkipFrames"s, "Value"s, 0))
   , m_projectGUID(GetStringAttribute(root, "ProjectGUID"s, "Value"s, ""s))
   , m_projectGUID2(GetStringAttribute(root, "ProjectGUID2"s, "Value"s, ""s))
   , m_assemblyGUID(GetStringAttribute(root, "AssemblyGUID"s, "Value"s, ""s))
   , m_vsName(GetStringAttribute(root, "VSName"s, "Value"s, ""s))
   , m_dualBackglass(GetIntAttribute(root, "DualBackglass"s, "Value"s, false))
   , m_author(GetStringAttribute(root, "Author"s, "Value"s, ""s))
   , m_artwork(GetStringAttribute(root, "Artwork"s, "Value"s, ""s))
   , m_gameName(GetStringAttribute(root, "GameName"s, "Value"s, ""s))
   , m_thumbnailImage(GetImageAttribute(root, "Images/ThumbnailImage"s))
   , m_backglassImage(GetImageAttribute(root, "Images/BackglassImage"s))
   , m_backglassOnImage(GetImageAttribute(root, "Images/BackglassOnImage"s))
   , m_backglassOffImage(GetImageAttribute(root, "Images/BackglassOffImage"s))
   , m_dmdImage(GetImageAttribute(root, "Images/DMDImage"s))
   , m_sounds(GetList<B2SSound>(root, "Sounds"s, "Sound"s))
   , m_reels(root)
   , m_backglassScores(root, false)
   , m_dmdScores(root, true)
   , m_backglassIlluminations(GetFilteredPtrList<B2SBulb>(root, "Illumination"s, "Bulb"s, false))
   , m_backglassAnimations(GetFilteredList<B2SAnimation>(root, "Animations"s, "Animation"s, false))
   , m_dmdIlluminations(GetFilteredPtrList<B2SBulb>(root, "Illumination"s, "Bulb"s, true))
   , m_dmdAnimations(GetFilteredList<B2SAnimation>(root, "Animations"s, "Animation"s, true))
{
}

}