// license:GPLv3+

#include <charconv>

#include "common.h"
#include "B2SDataModel.h"

#include "tinyxml2/tinyxml2.h"

namespace B2S {

static const tinyxml2::XMLElement* GetNode(const tinyxml2::XMLNode& doc, const std::string& nodePath)
{
   const tinyxml2::XMLNode* node = &doc;
   size_t pos = 0;
   size_t nextPos = nodePath.find('/', pos);
   while (nextPos != std::string::npos)
   {
      std::string elementName = nodePath.substr(pos, nextPos - pos);
      node = node->FirstChildElement(elementName.c_str());
      if (!node)
         return nullptr;
      pos = nextPos + 1;
      nextPos = nodePath.find('/', pos);
   }
   std::string finalElementName = nodePath.substr(pos);
   if (!finalElementName.empty())
      node = node->FirstChildElement(finalElementName.c_str());
   return node ? node->ToElement() : nullptr;
}

static string GetStringAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName, const string& defVal)
{
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node)
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
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node)
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
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node)
   {
      const char* value = node->Attribute(attributeName.c_str());
      if (value)
      {
         vector<unsigned char> imageData = base64_decode(value);
         return CreateTexture(imageData.data(), static_cast<int>(imageData.size()));
      }
   }
   return nullptr;
}

static std::shared_ptr<vector<uint8_t>> GetSoundAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName)
{
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node)
   {
      const char* value = node->Attribute(attributeName.c_str());
      if (value)
      {
         std::shared_ptr<vector<uint8_t>> pWav = std::make_shared<vector<uint8_t>>();
         vector<unsigned char> wav = base64_decode(value);
         pWav->insert(pWav->begin(), wav.begin(), wav.end());
         return pWav;
      }
   }
   return nullptr;
}

static B2SImage GetImageAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName)
{
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   //return node ? B2SImage(*node) : B2SImage();
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

template <class T> static vector<T> GetFilteredList(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& subNodeName, bool isDMD)
{
   vector<T> list;
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node == nullptr)
      return list;
   for (auto subNode = node->FirstChildElement(subNodeName.c_str()); subNode != nullptr; subNode = subNode->NextSiblingElement(subNodeName.c_str()))
   {
      bool isBackglass = subNode->Attribute("Parent", "Backglass") != 0;
      if (isBackglass == !isDMD)
         list.emplace_back(*subNode);
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


B2SSound::B2SSound(const tinyxml2::XMLNode& root)
   : m_name(GetStringAttribute(root, ""s, "Name"s, ""s))
   , m_wav(GetSoundAttribute(root, ""s, "Value"s))
{
}


B2SBulb::B2SBulb(const tinyxml2::XMLNode& root)
   : m_id(GetIntAttribute(root, ""s, "RomID"s, 0))
   , m_name(GetStringAttribute(root, ""s, "Name"s, ""s))
   , m_b2sId(GetIntAttribute(root, ""s, "B2SID"s, 0))
   , m_b2sValue(GetIntAttribute(root, ""s, "B2SValue"s, 0))
   , m_romId(GetIntAttribute(root, ""s, "RomID"s, 0))
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
   , m_snippitRotatingSteps(GetIntAttribute(root, ""s, "SnippitRotatingSteps"s, 0))
   , m_snippitRotatingAngle(GetIntAttribute(root, ""s, "SnippitRotatingAngle"s, 0))
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
   m_brightness = m_initialState;
}
B2SBulb::B2SBulb(B2SBulb&& other) noexcept
   : m_id(other.m_id)
   , m_name(other.m_name)
   , m_b2sId(other.m_b2sId)
   , m_b2sValue(other.m_b2sValue)
   , m_romId(other.m_romId)
   , m_romIdType(other.m_romIdType)
   , m_romInverted(other.m_romInverted)
   , m_initialState(other.m_initialState)
   , m_dualMode(other.m_dualMode)
   , m_intensity(other.m_intensity)
   , m_zOrder(other.m_zOrder)
   , m_lightColor(other.m_lightColor)
   , m_dodgeColor(other.m_dodgeColor)
   , m_illuminationMode(other.m_illuminationMode)
   , m_visible(other.m_visible)
   , m_locationX(other.m_locationX)
   , m_locationY(other.m_locationY)
   , m_width(other.m_width)
   , m_height(other.m_height)
   , m_isImageSnippit(other.m_isImageSnippit)
   , m_snippitType(other.m_snippitType)
   , m_snippitRotatingSteps(other.m_snippitRotatingSteps)
   , m_snippitRotatingAngle(other.m_snippitRotatingAngle)
   , m_snippitRotatingInterval(other.m_snippitRotatingInterval)
   , m_snippitRotatingDirection(other.m_snippitRotatingDirection)
   , m_snippitRotatingStopBehaviour(other.m_snippitRotatingStopBehaviour)
   , m_image(other.m_image)
   , m_offImage(other.m_offImage)
   , m_text(other.m_text)
   , m_textAlignment(other.m_textAlignment)
   , m_fontName(other.m_fontName)
   , m_fontSize(other.m_fontSize)
   , m_fontStyle(other.m_fontStyle)
   , m_brightness(other.m_brightness)
{
   other.m_image = nullptr; // Ressource is transfered, avoid destruction
   other.m_offImage = nullptr; // Ressource is transfered, avoid destruction
}

B2SBulb::~B2SBulb()
{
   DeleteTexture(m_image);
   DeleteTexture(m_offImage);
}

void B2SBulb::Render(VPXRenderContext2D* ctx) const
{
   m_updateBrightness();
   int bulbW, bulbH;
   if (m_offImage && m_brightness < 1.f)
   {
      GetTextureInfo(m_offImage, &bulbW, &bulbH);
      ctx->DrawImage(ctx, m_offImage, m_lightColor.x, m_lightColor.y, m_lightColor.z, 1.f,
         0.f, 0.f, static_cast<float>(bulbW), static_cast<float>(bulbH),
         static_cast<float>(m_locationX), static_cast<float>(m_locationY), static_cast<float>(m_width), static_cast<float>(m_height));
   }
   GetTextureInfo(m_image, &bulbW, &bulbH);
   ctx->DrawImage(ctx, m_image, m_lightColor.x, m_lightColor.y, m_lightColor.z, m_brightness,
      0.f, 0.f, static_cast<float>(bulbW), static_cast<float>(bulbH),
      static_cast<float>(m_locationX), static_cast<float>(m_locationY), static_cast<float>(m_width), static_cast<float>(m_height));
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
   while (m_timeUntilNextStep < 0.f)
   {
      if (m_reverse)
         m_currentStep = (m_currentStep - 1 + static_cast<int>(m_animationSteps.size())) % static_cast<int>(m_animationSteps.size());
      else
         m_currentStep = (m_currentStep + 1) % static_cast<int>(m_animationSteps.size());



      m_timeUntilNextStep += static_cast<float>(m_interval) / 1000.f;
   }
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
   , m_thumbnailImage(GetImageAttribute(root, "Images/ThumbnailImage"s, "Value"s))
   , m_backglassImage(GetImageAttribute(root, "Images/BackglassImage"s, "Value"s))
   , m_backglassOnImage(GetImageAttribute(root, "Images/BackglassOnImage"s, "Value"s))
   , m_backglassOffImage(GetImageAttribute(root, "Images/BackglassOffImage"s, "Value"s))
   , m_dmdImage(GetImageAttribute(root, "Images/DMDImage"s, "Value"s))
   , m_sounds(GetList<B2SSound>(root, "Sounds"s, "Sound"s))
   , m_backglassIlluminations(GetFilteredList<B2SBulb>(root, "Illumination"s, "Bulb"s, false))
   , m_backglassAnimations(GetFilteredList<B2SAnimation>(root, "Animations"s, "Animation"s, false))
   , m_dmdIlluminations(GetFilteredList<B2SBulb>(root, "Illumination"s, "Bulb"s, true))
   , m_dmdAnimations(GetFilteredList<B2SAnimation>(root, "Animations"s, "Animation"s, true))
{
}

}