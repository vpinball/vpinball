// license:GPLv3+

#include "common.h"
#include "B2SDataModel.h"

#include "tinyxml2/tinyxml2.h"

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
            colorValues.push_back(std::stoi(token));
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

std::vector<std::string> GetStringList(const std::string& str, char delimiter)
{
   std::vector<std::string> tokens;
   std::istringstream tokenStream(str);
   std::string token;
   while (std::getline(tokenStream, token, delimiter))
      tokens.push_back(token);
   return tokens;
}


B2SSound::B2SSound(const tinyxml2::XMLNode& root)
   : m_wav(GetSoundAttribute(root, "", "Value"))
   , m_name(GetStringAttribute(root, "", "Name", ""))
{
}


B2SBulb::B2SBulb(const tinyxml2::XMLNode& root)
   : m_id(GetIntAttribute(root, "", "RomID", 0))
   , m_name(GetStringAttribute(root, "", "Name", ""))
   , m_b2sId(GetIntAttribute(root, "", "B2SID", 0))
   , m_b2sValue(GetIntAttribute(root, "", "B2SValue", 0))
   , m_romId(GetIntAttribute(root, "", "RomID", 0))
   , m_romIdType(static_cast<B2SRomIDType>(GetIntAttribute(root, "", "RomIDType", 0)))
   , m_romInverted(GetBoolAttribute(root, "", "RomInverted", false))
   , m_initialState(GetBoolAttribute(root, "", "InitialState", false))
   , m_dualMode(static_cast<B2SDualMode>(GetIntAttribute(root, "", "DualMode", 0)))
   , m_zOrder(GetIntAttribute(root, "", "ZOrder", 0))
   , m_intensity(GetIntAttribute(root, "", "Intensity", 0))
   , m_lightColor(GetColorAttribute(root, "", "LightColor", vec4(1.f, 1.f, 1.f, 1.f)))
   , m_dodgeColor(GetColorAttribute(root, "", "IlluMode", vec4(0.f, 0.f, 0.f, 1.f)))
   , m_illuminationMode(GetIntAttribute(root, "", "Visible", 0))
   , m_visible(GetBoolAttribute(root, "", "Visible", true))
   , m_locationX(GetIntAttribute(root, "", "LocX", 0))
   , m_locationY(GetIntAttribute(root, "", "LocY", 0))
   , m_width(GetIntAttribute(root, "", "Width", 0))
   , m_height(GetIntAttribute(root, "", "Height", 0))
   , m_isImageSnippit(GetBoolAttribute(root, "", "IsImageSnippit", false))
   , m_snippitType(static_cast<B2SSnippitType>(GetIntAttribute(root, "", "SnippitType", 0)))
   , m_snippitRotatingSteps(GetIntAttribute(root, "", "SnippitRotatingSteps", 0))
   , m_snippitRotatingAngle(GetIntAttribute(root, "", "SnippitRotatingAngle", 0))
   , m_snippitRotatingInterval(GetIntAttribute(root, "", "SnippitRotatingInterval", 0))
   , m_snippitRotatingDirection(static_cast<B2SSnippitRotationDirection>(GetIntAttribute(root, "", "eSnippitRotationDirection", 0)))
   , m_snippitRotatingStopBehaviour(static_cast<B2SSnippitRotationStopBehaviour>(GetIntAttribute(root, "", "SnippitRotatingStopBehaviour", 0)))
   , m_image(GetTextureAttribute(root, "", "Image"))
   , m_offImage(GetTextureAttribute(root, "", "OffImage"))
   , m_text(GetStringAttribute(root, "", "Text", ""))
   , m_textAlignment(GetIntAttribute(root, "", "TextAlignment", 0))
   , m_fontName(GetStringAttribute(root, "", "FontName", ""))
   , m_fontSize(GetIntAttribute(root, "", "FontSize", 0))
   , m_fontStyle(GetIntAttribute(root, "", "FontStyle", 0))
{
   m_isLit = m_initialState;
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
   , m_isLit(other.m_isLit)
{
   other.m_image = nullptr; // Ressource is transfered, avoid destruction
   other.m_offImage = nullptr; // Ressource is transfered, avoid destruction
}

B2SBulb::~B2SBulb()
{
   DeleteTexture(m_image);
   DeleteTexture(m_offImage);
}


B2SImage::B2SImage(const tinyxml2::XMLNode& root)
   : m_image(GetTextureAttribute(root, "", "Value"))
   , m_filename(GetStringAttribute(root, "", "FileName", ""))
   , m_romId(GetIntAttribute(root, "", "RomID", 0))
   , m_romIdType(static_cast<B2SRomIDType>(GetIntAttribute(root, "", "RomIDType", 0)))
{
}

B2SImage::~B2SImage()
{
   DeleteTexture(m_image);
}


B2SAnimationStep::B2SAnimationStep(const tinyxml2::XMLNode& root)
   : m_step(GetIntAttribute(root, "", "Step", 0))
   , m_on(GetStringList(GetStringAttribute(root, "", "On", ""), ','))
   , m_waitLoopsAfterOn(GetIntAttribute(root, "", "WaitLoopsAfterOn", 0))
   , m_off(GetStringList(GetStringAttribute(root, "", "Off", ""), ','))
   , m_waitLoopsAfterOff(GetIntAttribute(root, "", "WaitLoopsAfterOff", 0))
   , m_pulseSwitch(GetIntAttribute(root, "", "PulseSwitch", 0))
{
}


B2SAnimation::B2SAnimation(const tinyxml2::XMLNode& root)
   : m_name(GetStringAttribute(root, "", "Name", ""))
   , m_dualMode(static_cast<B2SDualMode>(GetIntAttribute(root, "", "DualMode", 0)))
   , m_interval(GetIntAttribute(root, "", "Interval", 0))
   , m_loops(GetIntAttribute(root, "", "Loops", 0))
   , m_idJoin(GetStringAttribute(root, "", "IDJoin", ""))
   , m_startAnimationAtBackglassStartup(GetBoolAttribute(root, "", "StartAnimationAtBackglassStartup", false))
   , m_allLightsOffAtAnimationStart(GetBoolAttribute(root, "", "AllLightsOffAtAnimationStart", false))
   , m_lightsStateAtAnimationStart(static_cast<B2SLightsStateAtAnimationStart>(GetIntAttribute(root, "", "LightsStateAtAnimationStart", 0)))
   , m_resetLightsAtAnimationEnd(GetBoolAttribute(root, "", "ResetLightsAtAnimationEnd", false))
   , m_lightsStateAtAnimationEnd(static_cast<B2SLightsStateAtAnimationEnd>(GetIntAttribute(root, "", "LightsStateAtAnimationEnd", 0)))
   , m_runAnimationTilEnd(GetBoolAttribute(root, "", "RunAnimationTilEnd", false))
   , m_animationStopBehaviour(static_cast<B2SAnimationStopBehaviour>(GetIntAttribute(root, "", "AnimationStopBehaviour", 0)))
   , m_lockInvolvedLamps(GetBoolAttribute(root, "", "LockInvolvedLamps", false))
   , m_hideScoreDisplays(GetBoolAttribute(root, "", "HideScoreDisplays", false))
   , m_bringToFront(GetBoolAttribute(root, "", "BringToFront", false))
   , m_randomStart(GetBoolAttribute(root, "", "RandomStart", false))
   , m_randomQuality(GetIntAttribute(root, "", "RandomQuality", 0))
   , m_animationSteps(GetList<B2SAnimationStep>(root, "", "AnimationStep"))
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
         m_currentStep = (m_currentStep - 1 + m_animationSteps.size()) % m_animationSteps.size();
      else
         m_currentStep = (m_currentStep + 1) % m_animationSteps.size();



      m_timeUntilNextStep += static_cast<float>(m_interval) / 1000.f;
   }
}

B2STable::B2STable(const tinyxml2::XMLNode& root)
   : m_version(GetStringAttribute(root, "", "Version", ""))
   , m_name(GetStringAttribute(root, "Name", "Value", ""))
   , m_tableType(GetIntAttribute(root, "TableType", "Value", 0))
   , m_dmdType(static_cast<B2SDMDType>(GetIntAttribute(root, "DMDType", "Value", 0)))
   , m_dmdDefaultLocationX(GetIntAttribute(root, "DMDDefaultLocation", "LocX", 0))
   , m_dmdDefaultLocationY(GetIntAttribute(root, "DMDDefaultLocation", "LocY", 0))
   , m_grillHeight(GetIntAttribute(root, "GrillHeight", "Value", 0))
   , m_grillSmallHeight(GetIntAttribute(root, "GrillHeight", "Small", 0))
   , m_lampsDefaultSkipFrames(GetIntAttribute(root, "LampsDefaultSkipFrames", "Value", 0))
   , m_solenoidsDefaultSkipFrames(GetIntAttribute(root, "SolenoidsDefaultSkipFrames", "Value", 0))
   , m_giStringsDefaultSkipFrames(GetIntAttribute(root, "GIStringsDefaultSkipFrames", "Value", 0))
   , m_ledsDefaultSkipFrames(GetIntAttribute(root, "LEDsDefaultSkipFrames", "Value", 0))
   , m_projectGUID(GetStringAttribute(root, "ProjectGUID", "Value", ""))
   , m_projectGUID2(GetStringAttribute(root, "ProjectGUID2", "Value", ""))
   , m_assemblyGUID(GetStringAttribute(root, "AssemblyGUID", "Value", ""))
   , m_vsName(GetStringAttribute(root, "VSName", "Value", ""))
   , m_dualBackglass(GetIntAttribute(root, "DualBackglass", "Value", false))
   , m_author(GetStringAttribute(root, "Author", "Value", ""))
   , m_artwork(GetStringAttribute(root, "Artwork", "Value", ""))
   , m_gameName(GetStringAttribute(root, "GameName", "Value", ""))
   , m_backglassImage(GetImageAttribute(root, "Images/BackglassImage", "Value"))
   , m_backglassOnImage(GetImageAttribute(root, "Images/BackglassOnImage", "Value"))
   , m_backglassOffImage(GetImageAttribute(root, "Images/BackglassOffImage", "Value"))
   , m_dmdImage(GetImageAttribute(root, "Images/DMDImage", "Value"))
   , m_thumbnailImage(GetImageAttribute(root, "Images/ThumbnailImage", "Value"))
   , m_sounds(GetList<B2SSound>(root, "Sounds", "Sound"))
   , m_backglassIlluminations(GetFilteredList<B2SBulb>(root, "Illumination", "Bulb", false))
   , m_backglassAnimations(GetFilteredList<B2SAnimation>(root, "Animations", "Animation", false))
   , m_dmdIlluminations(GetFilteredList<B2SBulb>(root, "Illumination", "Bulb", true))
   , m_dmdAnimations(GetFilteredList<B2SAnimation>(root, "Animations", "Animation", true))
{
}