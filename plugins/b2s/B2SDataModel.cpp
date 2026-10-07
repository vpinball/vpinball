// license:GPLv3+

#include <algorithm>
#include <charconv>
#include <cctype>
#include <format>

#include "common.h"
#include "B2SDataModel.h"
#include "B2SServer.h"

#include "tinyxml2/tinyxml2.h"

namespace B2S {

static const tinyxml2::XMLElement* GetNode(const tinyxml2::XMLNode& doc, const std::string& nodePath) noexcept
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

static string GetStringAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName, const string& defVal) noexcept
{
   if (const tinyxml2::XMLElement* node = GetNode(doc, nodePath); node)
   {
      const char* value = node->Attribute(attributeName.c_str());
      if (value)
         return value;
   }
   return defVal;
}

static int GetIntAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName, const int& defVal) noexcept
{
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   return node ? node->IntAttribute(attributeName.c_str(), defVal) : defVal;
}

static bool GetBoolAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName, const bool& defVal) noexcept
{
   return GetIntAttribute(doc, nodePath, attributeName, defVal ? 1 : 0) != 0;
}

static vec4 GetColorAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName, const vec4& defVal) noexcept
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

static VPXTexture GetTextureAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& attributeName) noexcept
{
   if (const tinyxml2::XMLElement* node = GetNode(doc, nodePath); node)
   {
      if (const char* value = node->Attribute(attributeName.c_str()))
      {
         std::string_view valueView { value };
         vector<uint8_t> decoded = base64_decode(valueView.data(), valueView.size());
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
            LOGE(std::format("Failed to decode image at line {} ({})", node->GetLineNum(), path));
         }
         return CreateTexture(decoded.data(), static_cast<int>(decoded.size()));
      }
   }
   return nullptr;
}

static std::shared_ptr<vector<uint8_t>> GetSoundAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath) noexcept
{
   if (const tinyxml2::XMLElement* node = GetNode(doc, nodePath); node)
   {
      // The WAV stream is stored in the 'Stream' attribute ('Value' in some older files)
      const char* value = node->Attribute("Stream");
      if (value == nullptr)
         value = node->Attribute("Value");
      if (value != nullptr)
      {
         std::string_view valueView { value };
         vector<uint8_t> decoded_wav = base64_decode(valueView.data(), valueView.size());
         return std::make_shared<vector<uint8_t>>(std::move(decoded_wav));
      }
   }
   return nullptr;
}

static B2SImage GetImageAttribute(const tinyxml2::XMLNode& doc, const std::string& nodePath) noexcept
{
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   return node ? B2SImage(*node) : B2SImage();
}

template <class T> static vector<T> GetList(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& subNodeName) noexcept
{
   vector<T> list;
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node == nullptr)
      return list;
   for (auto subNode = node->FirstChildElement(subNodeName.c_str()); subNode != nullptr; subNode = subNode->NextSiblingElement(subNodeName.c_str()))
      list.emplace_back(*subNode);
   return list;
}

template <class T> static vector<std::unique_ptr<T>> GetPtrList(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& subNodeName) noexcept
{
   vector<std::unique_ptr<T>> list;
   const tinyxml2::XMLElement* node = GetNode(doc, nodePath);
   if (node == nullptr)
      return list;
   for (auto subNode = node->FirstChildElement(subNodeName.c_str()); subNode != nullptr; subNode = subNode->NextSiblingElement(subNodeName.c_str()))
      list.emplace_back(std::make_unique<T>(*subNode));
   return list;
}

template <class T> static vector<T> GetFilteredList(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& subNodeName, bool isDMD) noexcept
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

template <class T> static vector<std::unique_ptr<T>> GetFilteredPtrList(const tinyxml2::XMLNode& doc, const std::string& nodePath, const std::string& subNodeName, bool isDMD) noexcept
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

static std::vector<std::string> GetStringList(const std::string& str, char delimiter) noexcept
{
   std::vector<std::string> tokens;
   std::istringstream tokenStream(str);
   std::string token;
   while (std::getline(tokenStream, token, delimiter))
      tokens.push_back(token);
   return tokens;
}

static bool StartsWithCaseInsensitive(const std::string_view& str, const std::string_view& prefix) noexcept
{
   if (prefix.length() > str.length()) return false;
   return std::equal(prefix.begin(), prefix.end(), str.begin(), [](char a, char b) { return cLower(a) == cLower(b); });
}

B2SSound::B2SSound(const tinyxml2::XMLNode& root) noexcept
   : m_name(GetStringAttribute(root, ""s, "Name"s, ""s))
   , m_wav(GetSoundAttribute(root, ""s))
{
}


B2SBulb::B2SBulb(const tinyxml2::XMLNode& root) noexcept
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
   , m_snippitRotatingSteps(GetIntAttribute(root, ""s, "SnippitRotatingSteps"s, -1) >= 0
           ? GetIntAttribute(root, ""s, "SnippitRotatingSteps"s, 0)
           : (GetIntAttribute(root, ""s, "SnippitRotatingAngle"s, 0) != 0 ? (360 / GetIntAttribute(root, ""s, "SnippitRotatingAngle"s, 1)) : 0))
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

void B2SBulb::UpdateRotation(float elapsedInS)
{
   if (m_snippitType != B2SSnippitType::SelfRotatingImage)
      return;

   if (const int request = m_rotRequest.exchange(0); request != 0)
   {
      if (request == 1)
      {
         m_rotating = m_snippitRotatingSteps > 0 && m_snippitRotatingInterval > 0;
         m_rotateSlowDown = 0;
         m_slowdownAccMs = 0.f;
         m_rotateRunTillEnd = false;
         m_rotateRunToFirstStep = false;
         m_rotIntervalMs = static_cast<float>(m_snippitRotatingInterval);
      }
      else if (m_rotating)
      {
         switch (m_snippitRotatingStopBehaviour)
         {
         case B2SSnippitRotationStopBehaviour::SpinOff: m_rotateSlowDown = 1; break;
         case B2SSnippitRotationStopBehaviour::RunAnimationTillEnd: m_rotateRunTillEnd = true; break;
         case B2SSnippitRotationStopBehaviour::RunAnimationToFirstStep: m_rotateRunToFirstStep = true; break;
         default: m_rotating = false; break;
         }
      }
   }

   if (!m_rotating)
      return;

   const float direction = (m_snippitRotatingDirection == B2SSnippitRotationDirection::AntiClockwise) ? -1.f : 1.f;
   const float stepAngle = 360.f / static_cast<float>(m_snippitRotatingSteps);
   m_selfRotAngle += direction * stepAngle * (elapsedInS * 1000.f / m_rotIntervalMs);

   bool wrapped = false;
   while (m_selfRotAngle >= 360.f)
   {
      m_selfRotAngle -= 360.f;
      wrapped = true;
   }
   while (m_selfRotAngle < 0.f)
   {
      m_selfRotAngle += 360.f;
      wrapped = true;
   }

   if (m_rotateSlowDown > 0)
   {
      // Spin off: interval grows by 3ms per elapsed step for 25 steps then stops (reference behaviour)
      m_slowdownAccMs += elapsedInS * 1000.f;
      while (m_slowdownAccMs >= m_rotIntervalMs && m_rotateSlowDown <= 25)
      {
         m_slowdownAccMs -= m_rotIntervalMs;
         m_rotIntervalMs += 3.f;
         m_rotateSlowDown++;
      }
      if (m_rotateSlowDown > 25)
      {
         m_rotating = false;
         m_rotateSlowDown = 0;
      }
   }
   else if ((m_rotateRunTillEnd || m_rotateRunToFirstStep) && wrapped)
   {
      // Complete the current revolution then stop on the first step (angle 0)
      m_selfRotAngle = 0.f;
      m_rotating = false;
      m_rotateRunTillEnd = false;
      m_rotateRunToFirstStep = false;
   }
}


B2SImage::B2SImage()
   : m_image(nullptr)
   , m_filename(""s)
   , m_romId(0)
   , m_romIdType(B2SRomIDType::NotDefined)
   , m_romInverted(false)
{
}

B2SImage::B2SImage(const tinyxml2::XMLNode& root) noexcept
   : m_image(GetTextureAttribute(root, ""s, "Value"s))
   , m_filename(GetStringAttribute(root, ""s, "FileName"s, ""s))
   , m_romId(GetIntAttribute(root, ""s, "RomID"s, 0))
   , m_romIdType(static_cast<B2SRomIDType>(GetIntAttribute(root, ""s, "RomIDType"s, 0)))
   , m_romInverted(GetBoolAttribute(root, ""s, "RomInverted"s, false))
{
}

B2SImage::~B2SImage()
{
   DeleteTexture(m_image);
}


B2SReelImage::B2SReelImage(const tinyxml2::XMLNode& root, int setId) noexcept
   : m_name(GetStringAttribute(root, ""s, "Name"s, ""s) + (setId > 0 ? "_"s + std::to_string(setId) : ""s))
   , m_countOfIntermediate(GetIntAttribute(root, ""s, "CountOfIntermediates"s, 0))
   , m_image(GetTextureAttribute(root, ""s, "Image"s))
{
   for (int i = 1; i <= m_countOfIntermediate; i++)
      m_intermediates.push_back(GetTextureAttribute(root, ""s, "IntermediateImage"s + std::to_string(i)));
}

B2SReelImage::~B2SReelImage()
{
   DeleteTexture(m_image);
   for (const VPXTexture tex : m_intermediates)
      DeleteTexture(tex);
}


static vector<std::unique_ptr<B2SReelImage>> GetIlluminatedReelImages(const tinyxml2::XMLNode& root) noexcept
{
   vector<std::unique_ptr<B2SReelImage>> list;
   const tinyxml2::XMLElement* node = GetNode(root, "Reels/IlluminatedImages"s);
   if (node == nullptr)
      return list;
   // Flat illuminated images and per-set images (names suffixed with _setId)
   for (auto img = node->FirstChildElement("IlluminatedImage"); img != nullptr; img = img->NextSiblingElement("IlluminatedImage"))
      list.push_back(std::make_unique<B2SReelImage>(*img));
   for (auto set = node->FirstChildElement("Set"); set != nullptr; set = set->NextSiblingElement("Set"))
   {
      const int setId = set->IntAttribute("ID");
      for (auto img = set->FirstChildElement("IlluminatedImage"); img != nullptr; img = img->NextSiblingElement("IlluminatedImage"))
         list.push_back(std::make_unique<B2SReelImage>(*img, setId));
   }
   return list;
}

B2SReel::B2SReel(const tinyxml2::XMLNode& root) noexcept
   : m_images(GetNode(root, "Reels"s) ? GetPtrList<B2SReelImage>(*GetNode(root, "Reels"s), "Images"s, "Image"s) : vector<std::unique_ptr<B2SReelImage>>())
   , m_illuImages(GetIlluminatedReelImages(root))
{
}

const B2SReelImage* B2SReel::GetImage(const string& name, int index, bool illuminated, int setId) const
{
   // The reel type suffix defines the digit field width: "reel_00" -> 2 digits, "reel_0" -> 1 digit
   string base;
   int width = 0;
   if (name.ends_with("_00"))
   {
      width = 2;
      base = name.substr(0, name.length() - 2);
   }
   else if (name.ends_with("_0"))
   {
      width = 1;
      base = name.substr(0, name.length() - 1);
   }
   else
      return nullptr;
   string imgName;
   if (index < 0)
      imgName = std::format("{}Empty", base);
   else if (width == 2)
      imgName = std::format("{}{:02}", base, index);
   else
      imgName = std::format("{}{:01}", base, index);
   if (illuminated && setId > 0)
      imgName += "_"s + std::to_string(setId);
   const auto& pool = illuminated ? m_illuImages : m_images;
   for (const auto& img : pool)
   {
      if (img->m_name == imgName)
         return img.get();
   }
   return nullptr;
}

void B2SReelDigit::SetTarget(int value, bool animate)
{
   m_target = value;
   if (value < 0 || value >= 10 || m_current < 0 || m_current >= 10)
   {
      m_current = value;
      m_rolling = false;
      m_intermediate = 0;
      m_settle = 0;
      m_accMs = 0.f;
   }
   else if (!m_rolling)
   {
      if (animate && m_current != m_target)
      {
         m_rolling = true;
         m_intermediate = 0;
         m_settle = 0;
      }
      else
         m_current = value;
   }
   // While rolling, only the goal is updated (a roll in progress is never cancelled)
}

void B2SReelDigit::SetIlluminated(bool value, int intermediates)
{
   if (m_illuminated != value)
   {
      m_illuminated = value;
      if (m_rolling)
         m_intermediate = std::max(m_intermediate, intermediates);
   }
}

bool B2SReelDigit::Update(float elapsedInS, int rollingIntervalMs, int intermediates, bool rollUp)
{
   if (!m_rolling)
   {
      // No rolling animation: snap to the target
      m_current = m_target;
      m_intermediate = 0;
      m_settle = 0;
      m_accMs = 0.f;
      return false;
   }

   if (rollingIntervalMs < 10)
      rollingIntervalMs = 101; // B2SReelBox default rolling interval
   if (intermediates < 0)
      intermediates = 3; // Automatic intermediate count
   // Each digit step is spread over intermediate images + advance + settle ticks
   const float stepMs = static_cast<float>(rollingIntervalMs) / static_cast<float>(intermediates + 2);
   m_accMs += elapsedInS * 1000.f;
   bool advanced = false;
   while (m_rolling && m_accMs >= stepMs)
   {
      m_accMs -= stepMs;
      if (m_settle > 0)
      {
         if (--m_settle == 0 && m_current == m_target)
            m_rolling = false;
      }
      else if (m_intermediate < intermediates)
         m_intermediate++;
      else
      {
         m_current += rollUp ? 1 : -1;
         if (m_current > 9)
            m_current = 0;
         else if (m_current < 0)
            m_current = 9;
         m_intermediate = 0;
         m_settle = 2;
         advanced = true;
      }
   }
   return advanced;
}


B2SScore::B2SScore(const tinyxml2::XMLNode& root) noexcept
   : m_id(GetIntAttribute(root, ""s, "ID"s, 0))
   , m_b2sStartDigit(GetIntAttribute(root, ""s, "B2SStartDigit"s, 0))
   , m_b2sScoreType(static_cast<B2SScoreType>(GetIntAttribute(root, ""s, "B2SScoreType"s, 0)))
   , m_b2sPlayerNo(GetIntAttribute(root, ""s, "B2SPlayerNo"s, 0))
   , m_reelType(GetStringAttribute(root, ""s, "ReelType"s, ""s))
   , m_reelIlluLocation(GetIntAttribute(root, ""s, "ReelIlluLocation"s, 0))
   , m_reelIlluIntensity(GetIntAttribute(root, ""s, "ReelIlluIntensity"s, 0))
   , m_reelIlluB2SID(GetIntAttribute(root, ""s, "ReelIlluB2SID"s, 0))
   , m_reelIlluB2SIDType(GetIntAttribute(root, ""s, "ReelIlluB2SIDType"s, 0))
   , m_reelIlluB2SValue(GetIntAttribute(root, ""s, "ReelIlluB2SValue"s, 0))
   , m_reelIlluImageSet(GetIntAttribute(root, ""s, "ReelIlluImageSet"s, 0))
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
   , m_soundNames(
        [&root, this]() -> vector<string>
        {
           vector<string> names;
           for (int i = 1; i <= m_digits; i++)
              names.push_back(GetStringAttribute(root, ""s, "Sound"s + std::to_string(i), ""s));
           return names;
        }())
   , m_ledSegments(
        [this]()
        {
           // ReelType suffix is the segment count: "Dream7LED7" -> 7, "RenderedLED14" -> 14
           size_t pos = m_reelType.find_last_not_of("0123456789");
           if (pos == string::npos || pos == m_reelType.length() - 1)
              return 0;
           const int count = atoi(m_reelType.c_str() + pos + 1);
           if (count == 7 || count == 8)
              return 7;
           if (count == 9 || count == 10)
              return 10;
           if (count == 14)
              return 14;
           return 0;
        }())
   , m_scoreType(StartsWithCaseInsensitive(m_reelType, "dream7"s)  ? B2SScoreRenderer::Dream7
           : StartsWithCaseInsensitive(m_reelType, "rendered"s)    ? B2SScoreRenderer::RenderedLED
           : StartsWithCaseInsensitive(m_reelType, "LED"s)         ? B2SScoreRenderer::LED
           : StartsWithCaseInsensitive(m_reelType, "ImportedLED"s) ? B2SScoreRenderer::ImportedLED
                                                                   : B2SScoreRenderer::Reel)
{
}

vector<int> B2SScore::DistributeScore(int value) const
{
   // Right aligned score on the display digits, keeping its rightmost digits if it does not fit.
   // Reels show leading zeros (matching B2SReelDisplay::SetScore) while LED displays show blanks.
   vector<int> result(static_cast<size_t>(std::max(0, m_digits)), -1);
   const string text = std::to_string(value);
   const size_t count = std::min(text.length(), result.size());
   for (size_t i = 0; i < count; i++)
   {
      const char c = text[text.length() - 1 - i];
      int digit = -1;
      if (c >= '0' && c <= '9')
         digit = c - '0';
      result[result.size() - 1 - i] = digit;
   }
   if (m_scoreType == B2SScoreRenderer::Reel || m_scoreType == B2SScoreRenderer::LED || m_scoreType == B2SScoreRenderer::ImportedLED)
      for (int& digit : result)
         if (digit < 0)
            digit = 0;
   return result;
}

B2SScores::B2SScores(const tinyxml2::XMLNode& root) noexcept
   : m_reelCountOfIntermediates(GetIntAttribute(root, "Scores"s, "ReelCountOfIntermediates"s, 0))
   , m_reelRollingDirection(GetStringAttribute(root, "Scores"s, "ReelRollingDirection"s, "Up"s) == "Up"s ? B2SReelRollingDirection::Up : B2SReelRollingDirection::Down)
   , m_reelRollingInterval(GetIntAttribute(root, "Scores"s, "ReelRollingInterval"s, 0))
{
}


B2SAnimationStep::B2SAnimationStep(const tinyxml2::XMLNode& root) noexcept
   : m_step(GetIntAttribute(root, ""s, "Step"s, 0))
   , m_on(GetStringList(GetStringAttribute(root, ""s, "On"s, ""s), ','))
   , m_waitLoopsAfterOn(GetIntAttribute(root, ""s, "WaitLoopsAfterOn"s, 0))
   , m_off(GetStringList(GetStringAttribute(root, ""s, "Off"s, ""s), ','))
   , m_waitLoopsAfterOff(GetIntAttribute(root, ""s, "WaitLoopsAfterOff"s, 0))
   , m_pulseSwitch(GetIntAttribute(root, ""s, "PulseSwitch"s, 0))
{
}


B2SAnimation::B2SAnimation(const tinyxml2::XMLNode& root) noexcept
   : m_name(GetStringAttribute(root, ""s, "Name"s, ""s))
   , m_dualMode(static_cast<B2SDualMode>(GetIntAttribute(root, ""s, "DualMode"s, 0)))
   , m_interval(GetIntAttribute(root, ""s, "Interval"s, 0))
   , m_loops(GetIntAttribute(root, ""s, "Loops"s, 0))
   , m_idJoin(GetStringAttribute(root, ""s, "IDJoin"s, ""s))
   , m_startAnimationAtBackglassStartup(GetBoolAttribute(root, ""s, "StartAnimationAtBackglassStartup"s, false))
   , m_allLightsOffAtAnimationStart(GetBoolAttribute(root, ""s, "AllLightsOffAtAnimationStart"s, false))
   , m_lightsStateAtAnimationStart(
        static_cast<B2SLightsStateAtAnimationStart>(GetIntAttribute(root, ""s, "LightsStateAtAnimationStart"s, static_cast<int>(B2SLightsStateAtAnimationStart::NoChange))))
   , m_resetLightsAtAnimationEnd(GetBoolAttribute(root, ""s, "ResetLightsAtAnimationEnd"s, false))
   , m_lightsStateAtAnimationEnd(
        static_cast<B2SLightsStateAtAnimationEnd>(GetIntAttribute(root, ""s, "LightsStateAtAnimationEnd"s, static_cast<int>(B2SLightsStateAtAnimationEnd::InvolvedLightsOff))))
   , m_runAnimationTilEnd(GetBoolAttribute(root, ""s, "RunAnimationTilEnd"s, false))
   , m_animationStopBehaviour(static_cast<B2SAnimationStopBehaviour>(GetIntAttribute(root, ""s, "AnimationStopBehaviour"s, static_cast<int>(B2SAnimationStopBehaviour::StopImmediatelly))))
   , m_lockInvolvedLamps(GetBoolAttribute(root, ""s, "LockInvolvedLamps"s, false))
   , m_hideScoreDisplays(GetBoolAttribute(root, ""s, "HideScoreDisplays"s, false))
   , m_bringToFront(GetBoolAttribute(root, ""s, "BringToFront"s, false))
   , m_randomStart(GetBoolAttribute(root, ""s, "RandomStart"s, false))
   , m_randomQuality(GetIntAttribute(root, ""s, "RandomQuality"s, 0))
   , m_animationSteps(GetList<B2SAnimationStep>(root, ""s, "AnimationStep"s))
   , m_runtime(std::make_unique<Runtime>())
{
   // Expand each step into on/off entry actions (matches the reference EntryAction expansion)
   for (const B2SAnimationStep& step : m_animationSteps)
   {
      const bool isOnValid = (!step.m_on.empty() && !step.m_on[0].empty()) || step.m_waitLoopsAfterOn > 0;
      const bool isOffValid = (!step.m_off.empty() && !step.m_off[0].empty()) || step.m_waitLoopsAfterOff > 0;
      int pulseSwitch = step.m_pulseSwitch;
      if (isOnValid)
      {
         m_entryActions.push_back({ step.m_on, step.m_waitLoopsAfterOn, true, isOffValid ? 1 : 0, pulseSwitch });
         pulseSwitch = 0;
      }
      if (isOffValid)
      {
         m_entryActions.push_back({ step.m_off, step.m_waitLoopsAfterOff, false, isOnValid ? -1 : 0, pulseSwitch });
         pulseSwitch = 0;
      }
      if (pulseSwitch > 0)
         m_entryActions.push_back({ {}, 0, true, 0, pulseSwitch });
   }

   // Collect all involved light groups
   for (const EntryAction& entry : m_entryActions)
      for (const string& group : entry.groups)
         if (!group.empty() && std::find(m_lightsInvolved.begin(), m_lightsInvolved.end(), group) == m_lightsInvolved.end())
            m_lightsInvolved.push_back(group);

   // Parse the IDJoin ROM event triggers (lamp/solenoid/GI string, I prefix inverts)
   for (const string& idJoin : GetStringList(m_idJoin, ','))
   {
      if (idJoin.empty())
         continue;
      const auto toInt = [&idJoin](size_t offset) -> int
      {
         int v = 0;
         for (size_t i = offset; i < idJoin.length(); i++)
         {
            if (idJoin[i] < '0' || idJoin[i] > '9')
               return 0;
            v = v * 10 + (idJoin[i] - '0');
         }
         return v;
      };
      const char c0 = static_cast<char>(std::toupper(static_cast<unsigned char>(idJoin[0])));
      const char c1 = idJoin.length() > 1 ? static_cast<char>(std::toupper(static_cast<unsigned char>(idJoin[1]))) : ' ';
      const char c2 = idJoin.length() > 2 ? static_cast<char>(std::toupper(static_cast<unsigned char>(idJoin[2]))) : ' ';
      switch (c0)
      {
      case 'L':
         if (toInt(1) > 0)
            m_romTriggers.push_back({ B2SRomIDType::Lamp, toInt(1), false });
         break;
      case 'S':
         if (toInt(1) > 0)
            m_romTriggers.push_back({ B2SRomIDType::Solenoid, toInt(1), false });
         break;
      case 'G':
         if (c1 == 'I')
         {
            if (toInt(2) > 0)
               m_romTriggers.push_back({ B2SRomIDType::GIString, toInt(2), false });
         }
         else if (toInt(1) > 0)
            m_romTriggers.push_back({ B2SRomIDType::GIString, toInt(1), false });
         break;
      case 'I':
         if (c1 == 'L')
         {
            if (toInt(2) > 0)
               m_romTriggers.push_back({ B2SRomIDType::Lamp, toInt(2), true });
         }
         else if (c1 == 'S')
         {
            if (toInt(2) > 0)
               m_romTriggers.push_back({ B2SRomIDType::Solenoid, toInt(2), true });
         }
         else if (c1 == 'G')
         {
            if (c2 == 'I')
            {
               if (toInt(3) > 0)
                  m_romTriggers.push_back({ B2SRomIDType::GIString, toInt(3), true });
            }
            else if (toInt(2) > 0)
               m_romTriggers.push_back({ B2SRomIDType::GIString, toInt(2), true });
         }
         else if (toInt(1) > 0)
            m_romTriggers.push_back({ B2SRomIDType::Lamp, toInt(1), true });
         break;
      default:
         if (toInt(0) > 0)
            m_romTriggers.push_back({ B2SRomIDType::Lamp, toInt(0), false });
         break;
      }
   }

   m_runtime->triggerValues.resize(m_romTriggers.size(), 0.f);
   m_runtime->triggerPrev.resize(m_romTriggers.size(), false);
   m_runtime->triggerUpdaters.resize(m_romTriggers.size());

   if (m_startAnimationAtBackglassStartup)
      m_runtime->request = 1;
}

void B2SAnimation::BindRomTriggers(const RomTriggerResolver& resolver)
{
   for (size_t i = 0; i < m_romTriggers.size(); i++)
      m_runtime->triggerUpdaters[i] = resolver(m_romTriggers[i].romIdType, m_romTriggers[i].romId, m_romTriggers[i].inverted, &m_runtime->triggerValues[i]);
}

bool B2SAnimation::IsRunning() const { return m_runtime->running; }

void B2SAnimation::Start(bool reverse) { m_runtime->request = reverse ? 2 : 1; }

void B2SAnimation::Stop() { m_runtime->request = 3; }

void B2SAnimation::Update(float elapsedInS, const B2SAnimationEffects& fx)
{
   Runtime& rt = *m_runtime;

   // Poll ROM event triggers (IDJoin): rising edge starts, falling edge stops (random-start animations go through the pool)
   for (size_t i = 0; i < m_romTriggers.size() && i < rt.triggerUpdaters.size(); i++)
   {
      if (rt.triggerUpdaters[i])
         rt.triggerUpdaters[i]();
      const bool on = rt.triggerValues[i] >= 0.5f;
      if (on != rt.triggerPrev[i])
      {
         rt.triggerPrev[i] = on;
         if (m_randomStart)
         {
            if (fx.randomTrigger)
               fx.randomTrigger(m_romTriggers[i].romIdType, m_romTriggers[i].romId, on, this);
         }
         else
         {
            // Trigger edges do not override a pending script request
            int expected = 0;
            rt.request.compare_exchange_strong(expected, on ? 1 : 3);
         }
      }
   }

   // Consume the pending script/trigger request
   if (const int request = rt.request.exchange(0); request != 0)
   {
      if (request == 3)
      {
         if (rt.running)
         {
            // Stop request honors the configured stop behaviour
            switch (m_animationStopBehaviour)
            {
            case B2SAnimationStopBehaviour::RunAnimationTillEnd:
            case B2SAnimationStopBehaviour::RunAnimationToFirstStep:
               if (!rt.stopMeLater)
                  rt.reachedThe0Point = false;
               rt.stopMeLater = true;
               break;
            default: EndRun(fx); break;
            }
         }
      }
      else if (!rt.running) // Starting an already running animation is ignored (reference behavior)
      {
         // Dual backglass: an animation only runs in its declared mode
         if (m_dualMode == B2SDualMode::Both || fx.dualMode == B2SDualMode::Both || m_dualMode == fx.dualMode)
            BeginRun(fx, request == 2);
      }
   }

   if (!rt.running)
      return;

   // Interval is in milliseconds, clamped to avoid a lock up on degenerate animations
   const float intervalInS = static_cast<float>(m_interval > 0 ? m_interval : 1) / 1000.f;
   rt.timeUntilNextStep -= elapsedInS;
   while (rt.running && rt.timeUntilNextStep <= 0.f)
      rt.timeUntilNextStep += static_cast<float>(Tick(fx)) * intervalInS;
}

int B2SAnimation::Tick(const B2SAnimationEffects& fx)
{
   Runtime& rt = *m_runtime;
   const int count = static_cast<int>(m_entryActions.size());
   int waitLoops = 1;

   while (true)
   {
      const int index = !rt.reverse ? rt.ticker + 1 : count - rt.ticker;
      if (index < 1 || index > count)
         break;
      const EntryAction* action = &m_entryActions[index - 1];
      if (action->corrector != 0 && rt.reverse)
      {
         const int corrected = index + action->corrector;
         if (corrected >= 1 && corrected <= count)
            action = &m_entryActions[corrected - 1];
      }
      for (const string& group : action->groups)
         if (!group.empty())
            fx.setGroup(group, action->on);
      if (action->pulseSwitch > 0)
         fx.pulseSwitch(action->pulseSwitch);
      if (action->waitLoops > 0)
      {
         waitLoops = action->waitLoops;
         break;
      }
      rt.ticker++;
      if (rt.ticker >= count)
         break;
   }

   rt.ticker++;
   bool finished = false;
   if (rt.ticker >= count)
   {
      rt.reachedThe0Point = true;
      rt.loopTicker++;
      rt.ticker = 0;
      if (m_loops > 0 && rt.loopTicker >= m_loops)
      {
         rt.loopTicker = 0;
         finished = true;
      }
   }

   if (finished || (rt.stopMeLater && m_animationStopBehaviour == B2SAnimationStopBehaviour::RunAnimationTillEnd && rt.ticker == 0)
      || (rt.stopMeLater && m_animationStopBehaviour == B2SAnimationStopBehaviour::RunAnimationToFirstStep && (rt.ticker == 1 || rt.ticker == 2) && rt.reachedThe0Point))
      EndRun(fx);

   return waitLoops;
}

void B2SAnimation::BeginRun(const B2SAnimationEffects& fx, bool reverse)
{
   Runtime& rt = *m_runtime;
   rt.stopMeLater = false;
   rt.reverse = reverse;
   rt.ticker = 0;
   rt.loopTicker = 0;
   rt.reachedThe0Point = false;
   rt.timeUntilNextStep = static_cast<float>(m_interval > 0 ? m_interval : 1) / 1000.f;

   // snapshot all lights before touching them (restored at animation end when reset is requested)
   if (m_resetLightsAtAnimationEnd || m_lightsStateAtAnimationEnd == B2SLightsStateAtAnimationEnd::LightsReseted)
      rt.lightSnapshot = fx.snapshotAllLights();
   else
      rt.lightSnapshot.clear();

   // maybe switch off all lights or switch on/off the involved lights
   if (m_allLightsOffAtAnimationStart || m_lightsStateAtAnimationStart == B2SLightsStateAtAnimationStart::LightsOff)
      fx.allLightsOff();
   else if (m_lightsStateAtAnimationStart == B2SLightsStateAtAnimationStart::InvolvedLightsOff || m_lightsStateAtAnimationStart == B2SLightsStateAtAnimationStart::InvolvedLightsOn)
      for (const string& group : m_lightsInvolved)
         fx.setGroup(group, m_lightsStateAtAnimationStart == B2SLightsStateAtAnimationStart::InvolvedLightsOn);

   if (m_lockInvolvedLamps)
      for (const string& group : m_lightsInvolved)
         fx.lockGroup(group);
   if (m_hideScoreDisplays)
      fx.setScoreDisplaysHidden(true);

   rt.running = true;
}

void B2SAnimation::EndRun(const B2SAnimationEffects& fx)
{
   Runtime& rt = *m_runtime;
   if (!rt.running)
      return;
   rt.running = false;
   rt.stopMeLater = false;

   if (m_hideScoreDisplays)
      fx.setScoreDisplaysHidden(false);
   if (m_lockInvolvedLamps)
      for (const string& group : m_lightsInvolved)
         fx.unlockGroup(group);
   if (m_resetLightsAtAnimationEnd || m_lightsStateAtAnimationEnd == B2SLightsStateAtAnimationEnd::LightsReseted)
   {
      fx.restoreAllLights(rt.lightSnapshot);
      rt.lightSnapshot.clear();
   }
   else if (m_lightsStateAtAnimationEnd == B2SLightsStateAtAnimationEnd::InvolvedLightsOff || m_lightsStateAtAnimationEnd == B2SLightsStateAtAnimationEnd::InvolvedLightsOn)
      for (const string& group : m_lightsInvolved)
         fx.setGroup(group, m_lightsStateAtAnimationEnd == B2SLightsStateAtAnimationEnd::InvolvedLightsOn);
}

B2STable::B2STable(const tinyxml2::XMLNode& root) noexcept
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
   , m_backglassScores(root)
   , m_dmdScores(root)
   , m_backglassIlluminations(GetFilteredPtrList<B2SBulb>(root, "Illumination"s, "Bulb"s, false))
   , m_backglassAnimations(GetFilteredList<B2SAnimation>(root, "Animations"s, "Animation"s, false))
   , m_dmdIlluminations(GetFilteredPtrList<B2SBulb>(root, "Illumination"s, "Bulb"s, true))
   , m_dmdAnimations(GetFilteredList<B2SAnimation>(root, "Animations"s, "Animation"s, true))
{
   // Score digit numbering: displays without an explicit B2SStartDigit are numbered sequentially
   // over all Score nodes in file order (both backglass and DMD parents share the same counter)
   if (const tinyxml2::XMLElement* scoresNode = GetNode(root, "Scores"s); scoresNode != nullptr)
   {
      int autoDigit = 1;
      for (const tinyxml2::XMLElement* node = scoresNode->FirstChildElement("Score"); node != nullptr; node = node->NextSiblingElement("Score"))
      {
         const bool isDMD = node->Attribute("Parent", "Backglass") == nullptr;
         B2SScore score(*node);
         score.m_resolvedStartDigit = (score.m_b2sStartDigit > 0) ? score.m_b2sStartDigit : autoDigit;
         autoDigit += score.m_digits;
         (isDMD ? m_dmdScores : m_backglassScores).m_scores.push_back(std::move(score));
      }
   }

   // ZOrder: images without ZOrder keep their file order, then ZOrder ascending (drawn on top)
   const auto zsort = [](vector<std::unique_ptr<B2SBulb>>& bulbs)
   {
      std::stable_sort(bulbs.begin(), bulbs.end(),
         [](const std::unique_ptr<B2SBulb>& a, const std::unique_ptr<B2SBulb>& b) { return (a->m_zOrder > 0 ? a->m_zOrder : 0) < (b->m_zOrder > 0 ? b->m_zOrder : 0); });
   };
   zsort(m_backglassIlluminations);
   zsort(m_dmdIlluminations);

   // Animations without a positive interval or without any playable step are dropped (reference behavior)
   const auto dropEmptyAnims = [](vector<B2SAnimation>& animations)
   {
      vector<B2SAnimation> kept;
      for (B2SAnimation& animation : animations)
         if (animation.m_interval > 0 && !animation.IsEmpty())
            kept.push_back(std::move(animation));
      animations.swap(kept);
   };
   dropEmptyAnims(m_backglassAnimations);
   dropEmptyAnims(m_dmdAnimations);

   // Bulbs on the BackglassOnImage ROM channel are baked into the lit background and not drawn
   // separately, unless an animation drives their group (SetThruAnimation in the reference)
   if (m_backglassOnImage.m_romId > 0 && m_backglassOnImage.m_romIdType != B2SRomIDType::NotDefined)
   {
      std::unordered_map<string, bool> animatedGroups;
      for (const B2SAnimation& animation : m_backglassAnimations)
         for (const string& group : animation.GetLightsInvolved())
            animatedGroups[group] = true;
      for (const auto& bulb : m_backglassIlluminations)
      {
         // A B2SID driven bulb matches a Lamp ROM channel, without inversion
         const int romId = bulb->m_b2sId > 0 ? bulb->m_b2sId : bulb->m_romId;
         const B2SRomIDType romIdType = bulb->m_b2sId > 0 ? B2SRomIDType::Lamp : bulb->m_romIdType;
         const bool romInverted = bulb->m_b2sId > 0 ? false : bulb->m_romInverted;
         bulb->m_bakedIntoBackground
            = romId != 0 && romId == m_backglassOnImage.m_romId && romIdType == m_backglassOnImage.m_romIdType && !romInverted && animatedGroups.find(bulb->m_name) == animatedGroups.end();
      }
   }
}

const B2SScore* B2STable::FindScoreDisplay(int displayId) const
{
   for (const B2SScore& score : m_backglassScores.m_scores)
      if (score.m_id == displayId)
         return &score;
   for (const B2SScore& score : m_dmdScores.m_scores)
      if (score.m_id == displayId)
         return &score;
   return nullptr;
}

const B2SScore* B2STable::FindScoreDigitDisplay(int digit) const
{
   for (const B2SScore& score : m_backglassScores.m_scores)
      if (digit >= score.m_resolvedStartDigit && digit < score.m_resolvedStartDigit + score.m_digits)
         return &score;
   for (const B2SScore& score : m_dmdScores.m_scores)
      if (digit >= score.m_resolvedStartDigit && digit < score.m_resolvedStartDigit + score.m_digits)
         return &score;
   return nullptr;
}

uint16_t B2SSegmentCharMask(char c, int ledSegments)
{
   // Character to segment mask, following the Dream7 DisplayCharacter tables (reference SegmentNumber.vb)
   // expressed in the CTLPI/PinMAME luminance bit order used by the modern segment renderer:
   //   7seg : a=0,b=1,c=2,d=3,e=4,f=5,g=6, dp/comma=7
   //   10seg: a..f=0..5, g1/g2 merged=6, i=8, l=9
   //   14seg: a=0,b=1,c=2,d=3,e=4,f=5,g1=6, h=8,i=9,j=10,g2=11,k=12,l=13,m=14, dp=15
   switch (ledSegments)
   {
   case 14:
   {
      switch (c)
      {
      case '0': return 0x143F;
      case '1': return 0x0406;
      case '2': return 0x085B;
      case '3': return 0x080F;
      case '4': return 0x0866;
      case '5': return 0x086D;
      case '6': return 0x087D;
      case '7': return 0x0007;
      case '8': return 0x087F;
      case '9': return 0x086F;
      case 'A':
      case 'a': return 0x0877;
      case 'B':
      case 'b': return 0x280F;
      case 'C':
      case 'c': return 0x0039;
      case 'D':
      case 'd': return 0x220F;
      case 'E':
      case 'e': return 0x0079;
      case 'F':
      case 'f': return 0x0071;
      case 'G':
      case 'g': return 0x083D;
      case 'H':
      case 'h': return 0x0876;
      case 'I':
      case 'i': return 0x2209;
      case 'J':
      case 'j': return 0x001E;
      case 'K':
      case 'k': return 0x4470;
      case 'L':
      case 'l': return 0x0038;
      case 'M':
      case 'm': return 0x0536;
      case 'N':
      case 'n': return 0x4136;
      case 'O':
      case 'o': return 0x003F;
      case 'P':
      case 'p': return 0x0873;
      case 'Q':
      case 'q': return 0x403F;
      case 'R':
      case 'r': return 0x4873;
      case 'S':
      case 's': return 0x086D;
      case 'T':
      case 't': return 0x2201;
      case 'U':
      case 'u': return 0x003E;
      case 'V':
      case 'v': return 0x1430;
      case 'W':
      case 'w': return 0x5036;
      case 'X':
      case 'x': return 0x5500;
      case 'Y':
      case 'y': return 0x2500;
      case 'Z':
      case 'z': return 0x1409;
      case '-': return 0x00C0;
      case '_': return 0x0008;
      case '=': return 0x0048;
      default: return 0;
      }
   }
   case 10:
   {
      switch (c)
      {
      case '0': return 0x003F;
      case '1': return 0x0300;
      case '2': return 0x005B;
      case '3': return 0x004F;
      case '4': return 0x0066;
      case '5': return 0x006D;
      case '6': return 0x007D;
      case '7': return 0x0007;
      case '8': return 0x007F;
      case '9': return 0x006F;
      case 'A':
      case 'a': return 0x0077;
      case 'B':
      case 'b': return 0x007C;
      case 'C':
      case 'c': return 0x0039;
      case 'D':
      case 'd': return 0x005E;
      case 'E':
      case 'e': return 0x0079;
      case 'F':
      case 'f': return 0x0071;
      case '-': return 0x0040;
      default: return 0;
      }
   }
   default:
   {
      switch (c)
      {
      case '0': return 0x003F;
      case '1': return 0x0006;
      case '2': return 0x005B;
      case '3': return 0x004F;
      case '4': return 0x0066;
      case '5': return 0x006D;
      case '6': return 0x007D;
      case '7': return 0x0007;
      case '8': return 0x007F;
      case '9': return 0x006F;
      case 'A':
      case 'a': return 0x0077;
      case 'B':
      case 'b': return 0x007C;
      case 'C':
      case 'c': return 0x0039;
      case 'D':
      case 'd': return 0x005E;
      case 'E':
      case 'e': return 0x0079;
      case 'F':
      case 'f': return 0x0071;
      case '-': return 0x0040;
      case '_': return 0x0008;
      default: return 0;
      }
   }
   }
}

uint16_t B2SSegmentTranslateBitCode(uint32_t bits, int ledSegments)
{
   // Dream7 DisplayBitCode iterates the display's segment list: segment index n reads bit n.
   // Translate from that (Dream7 layout order) to the CTLPI/PinMAME luminance bit order.
   switch (ledSegments)
   {
   case 14:
      // Dream7 order: a,b,c,d,e,f,g1,dot,h,i,j,g2,m,l,k,dot2 -> CTLPI: a,b,c,d,e,f,g1,comma,h,i,j,g2,k,l,m,dp
      return static_cast<uint16_t>(
         (bits & 0x007F) | ((bits & 0x0080) << 8) | (bits & 0x0700) | (bits & 0x0800) | ((bits & 0x1000) << 2) | (bits & 0x2000) | ((bits & 0x4000) >> 2) | (bits & 0x8000));
   case 10:
      // Dream7 order: a,b,c,d,e,f,g1,g2,i,l with g2 also reading bit 6 -> CTLPI 9seg: a..g=0..6, i=8, l=9
      return static_cast<uint16_t>(bits & 0x037F);
   case 7:
   default: return static_cast<uint16_t>(bits & 0x00FF);
   }
}

}

