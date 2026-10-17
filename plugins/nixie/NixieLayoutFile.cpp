// license:GPLv3+

#include "NixieLayoutFile.h"

#include "plugins/ControllerPlugin.h"
#include "plugins/PluginStrings.h"

#include <format>
#include <fstream>

using std::string;
using namespace std::string_literals;


namespace PinballPlugin::NixieFile
{

namespace
{

   string TrimLeading(const string& str, const string& whitespace)
   {
      const auto strBegin = str.find_first_not_of(whitespace);
      return (strBegin == string::npos) ? string() : str.substr(strBegin);
   }

   string TrimTrailing(const string& str, const string& whitespace)
   {
      const auto strEnd = str.find_last_not_of(whitespace);
      return (strEnd == string::npos) ? string() : str.substr(0, strEnd + 1);
   }

   bool try_parse_float(const string& str, float& value)
   {
      try
      {
         value = std::stof(str);
         return true;
      }
      catch (...)
      {
         return false;
      }
   }

   bool try_parse_int(const string& str, int& value)
   {
      try
      {
         value = std::stoi(str);
         return true;
      }
      catch (...)
      {
         return false;
      }
   }

}


bool Parse(const std::filesystem::path& path, std::vector<Layout>& layouts, std::string& error)
{
   std::ifstream content(path);
   if (!content.is_open())
   {
      error = "Failed to open Nixie file " + PluginStrings::PathToUTF8(path);
      return false;
   }
#define CHECK_FIELD(check)                                                                                                                         \
   if (!(check))                                                                                                                                   \
   {                                                                                                                                               \
      error = std::format("Invalid field '{}: {}' at line {} in Nixie file {}", key, value, lineIndex, PluginStrings::PathToUTF8(path));             \
      return false;                                                                                                                                \
   }
   static const string whitespace = " \t\r"s;
   Layout layout = {};
   layout.path = path;
   bool hasSection = false;
   std::vector<Layout> parsed;
   const auto endLayout = [&]() -> bool
   {
      if (layout.visuals.empty())
      {
         error = "Layout section without visuals in Nixie file " + PluginStrings::PathToUTF8(path);
         return false;
      }
      for (auto& visual : layout.visuals)
      {
         // Pad is authored in layout pixels [left, top, right, bottom] inside the visual bounds, while the renderer expects
         // fractions of the padded display size (converted here, as Pad may come before Rect)
         if (visual.glassPad.x != 0.f || visual.glassPad.y != 0.f || visual.glassPad.z != 0.f || visual.glassPad.w != 0.f)
         {
            const float displayW = visual.w - visual.glassPad.x - visual.glassPad.z;
            const float displayH = visual.h - visual.glassPad.y - visual.glassPad.w;
            if (displayW <= 0.f || displayH <= 0.f)
            {
               error = "Pad leaves no display area in Nixie file " + PluginStrings::PathToUTF8(path);
               return false;
            }
            visual.glassPad = vec4(visual.glassPad.x / displayW, visual.glassPad.y / displayH, visual.glassPad.z / displayW, visual.glassPad.w / displayH);
         }
         switch (visual.type)
         {
         case VisualType::DMD:
            if (visual.displaySize.x < 0 || visual.displaySize.y < 0)
            {
               error = "DMD display needs Size to be defined in Nixie file " + PluginStrings::PathToUTF8(path);
               return false;
            }
            break;

         case VisualType::Screen:
            if (visual.displaySize.x < 0 || visual.displaySize.y < 0)
            {
               error = "Screen display needs Size to be defined in Nixie file " + PluginStrings::PathToUTF8(path);
               return false;
            }
            break;

         case VisualType::SegDisplay:
            if (visual.nElements == -1)
               visual.nElements = (int)visual.xOffsets.size();
            if (visual.nElements == 0)
            {
               error = "Segment display needs at least one of XPos/NElements to be defined in Nixie file " + PluginStrings::PathToUTF8(path);
               return false;
            }
            if (visual.xOffsets.empty())
            {
               visual.xOffsets.reserve(visual.nElements);
               for (int i = 0; i < visual.nElements; i++)
                  visual.xOffsets.push_back(visual.w * static_cast<float>(i) / static_cast<float>(visual.nElements));
            }
            break;

         case VisualType::Picture:
            if (visual.srcUri.empty())
            {
               error = "Picture visual needs Image to be defined in Nixie file " + PluginStrings::PathToUTF8(path);
               return false;
            }
            break;
         }
      }
      parsed.push_back(layout);
      return true;
   };
   {
      Visual* visual = nullptr;
      const auto parseArray = [](const string& value) -> std::vector<float>
      {
         size_t pos1 = value.find('[');
         if (pos1 == string::npos)
            return std::vector<float>();
         std::vector<float> array;
         while (true)
         {
            size_t pos2 = value.find_first_of("],\n", pos1 + 1);
            if ((pos2 == string::npos) || (value[pos2] == '\n'))
               return std::vector<float>();
            float v;
            if (!try_parse_float(value.substr(pos1 + 1, pos2 - pos1 - 1), v))
               return std::vector<float>();
            array.push_back(v);
            if (value[pos2] == ']')
               return array;
            pos1 = pos2;
         }
      };
      std::string line;
      size_t indentSize = 0;
      unsigned int lineIndex = 0;
      size_t expectedIndent = 0;
      float fValue;
      while (std::getline(content, line))
      {
         lineIndex++;
         line = TrimTrailing(line, whitespace);
         if (line.empty())
            continue;
         const auto afterIndent = line.find_first_not_of(whitespace);
         if ((indentSize == 0) && (afterIndent != 0))
            indentSize = afterIndent;
         if ((indentSize != 0) && ((afterIndent % indentSize) != 0))
         {
            error = std::format("Invalid indentation at line {} in Nixie file {}", lineIndex, PluginStrings::PathToUTF8(path));
            return false;
         }
         size_t indent = indentSize == 0 ? 0 : afterIndent / indentSize;
         if (indent > expectedIndent)
         {
            error = std::format("Invalid indentation ({} while expecting {} at line {}) in Nixie file {}", indent, expectedIndent, lineIndex, PluginStrings::PathToUTF8(path));
            return false;
         }
         if (indent < expectedIndent)
         {
            if (indent <= 2)
               visual = nullptr;
            expectedIndent = indent;
         }
         if (line[afterIndent] == '#')
            continue;

         // A '- <display>' line starts a new layout section targeting the given ancillary window display
         if ((indent == 0) && (line[afterIndent] == '-'))
         {
            if (hasSection && !endLayout())
               return false;
            layout = {};
            layout.path = path;
            visual = nullptr;
            const string target = TrimLeading(string(line.cbegin() + afterIndent + 1, line.cend()), whitespace);
            if (target == "Backglass")
               layout.window = VPXWindowId::VPXWINDOW_Backglass;
            else if (target == "ScoreView")
               layout.window = VPXWindowId::VPXWINDOW_ScoreView;
            else if (target == "Topper")
               layout.window = VPXWindowId::VPXWINDOW_Topper;
            else
            {
               error = std::format("Unknown target display '{}' at line {} in Nixie file {}", target, lineIndex, PluginStrings::PathToUTF8(path));
               return false;
            }
            hasSection = true;
            expectedIndent = 1;
            continue;
         }
         const auto colon = line.find(':');
         if (colon == string::npos)
         {
            error = std::format("Field is missing ':' separator at line {} in Nixie file {}", lineIndex, PluginStrings::PathToUTF8(path));
            return false;
         }
         if (colon == afterIndent)
         {
            error = std::format("Field is missing a key before ':' separator at line {} in Nixie file {}", lineIndex, PluginStrings::PathToUTF8(path));
            return false;
         }
         const string key(line.cbegin() + afterIndent, line.cbegin() + colon);
         string value(line.cbegin() + colon + 1, line.cend());
         value = TrimLeading(value, whitespace);

         if (key == "Width")
         {
            CHECK_FIELD((indent == 1) && try_parse_float(value, fValue) && (fValue >= 1.f));
            layout.width = fValue;
         }
         else if (key == "Height")
         {
            CHECK_FIELD((indent == 1) && try_parse_float(value, fValue) && (fValue >= 1.f));
            layout.height = fValue;
         }
         else if (key == "Fit")
         {
            CHECK_FIELD(indent == 1);
            if (value == "Contain")
               layout.fit = Contain;
            else
            {
               // Unsupported fit mode
               CHECK_FIELD(false);
            }
         }
         if (key == "Visuals")
         {
            CHECK_FIELD(indent == 1);
            expectedIndent = 2;
         }
         else if (key == "- DMD")
         {
            CHECK_FIELD(indent == 2);
            expectedIndent = indent + 1;
            layout.visuals.push_back({ VisualType::DMD });
            visual = &layout.visuals.back();
            visual->srcUri = "ctrl://default/display";
            visual->liveStyle = 1; // Default to Neon Plasma
            visual->tint = vec3(1.f, 1.f, 1.f);
            visual->glassTint = vec3(1.f, 1.f, 1.f);
            visual->glassAmbient = vec3(1.f, 1.f, 1.f);
            visual->glassPad = vec4(0.f, 0.f, 0.f, 0.f);
            visual->glassArea = vec4(0.f, 0.f, 0.f, 0.f);
            visual->displaySize = ivec2(-1, -1);
         }
         else if (key == "- Screen")
         {
            CHECK_FIELD(indent == 2);
            expectedIndent = indent + 1;
            layout.visuals.push_back({ VisualType::Screen });
            visual = &layout.visuals.back();
            visual->srcUri = "ctrl://default/display";
            visual->liveStyle = 1; // Default to LCD
            visual->tint = vec3(1.f, 1.f, 1.f);
            visual->glassTint = vec3(1.f, 1.f, 1.f);
            visual->glassAmbient = vec3(1.f, 1.f, 1.f);
            visual->glassPad = vec4(0.f, 0.f, 0.f, 0.f);
            visual->glassArea = vec4(0.f, 0.f, 0.f, 0.f);
            visual->displaySize = ivec2(-1, -1);
         }
         else if (key == "- SegDisplay")
         {
            CHECK_FIELD(indent == 2);
            expectedIndent = indent + 1;
            layout.visuals.push_back({ VisualType::SegDisplay });
            visual = &layout.visuals.back();
            visual->srcUri = string();
            visual->liveStyle = -1;
            visual->tint = vec3(1.f, 1.f, 1.f);
            visual->glassTint = vec3(1.f, 1.f, 1.f);
            visual->glassAmbient = vec3(1.f, 1.f, 1.f);
            visual->glassPad = vec4(0.f, 0.f, 0.f, 0.f);
            visual->glassArea = vec4(0.f, 0.f, 0.f, 0.f);
            visual->nElements = -1;
            visual->segFamilyHint = VPXSegDisplayHint::Generic;
         }
         else if (key == "- Picture")
         {
            CHECK_FIELD(indent == 2);
            expectedIndent = indent + 1;
            layout.visuals.push_back({ VisualType::Picture });
            visual = &layout.visuals.back();
            visual->srcUri = string();
            visual->tint = vec3(1.f, 1.f, 1.f);
         }
         else if (key == "Rect")
         {
            CHECK_FIELD((indent == 3) && (visual != nullptr));
            const auto rect = parseArray(value);
            CHECK_FIELD(rect.size() == 4);
            visual->x = rect[0];
            visual->y = rect[1];
            visual->w = rect[2];
            visual->h = rect[3];
         }
         else if (key == "Pad")
         {
            CHECK_FIELD((indent == 3) && (visual != nullptr)); // Display pad inside rect
            const auto rect = parseArray(value);
            CHECK_FIELD(rect.size() == 4 && rect[0] >= 0.f && rect[1] >= 0.f && rect[2] >= 0.f && rect[3] >= 0.f);
            visual->glassPad = vec4(rect[0], rect[1], rect[2], rect[3]); // In layout pixels, converted at the end of the layout
         }
         else if (key == "Style")
         {
            CHECK_FIELD((indent == 3) && (visual != nullptr));
            if (visual->type == VisualType::DMD)
            {
               if (value == "Neon Plasma")
                  visual->liveStyle = 1;
               else if (value == "Red LED")
                  visual->liveStyle = 2;
               else if (value == "Green LED")
                  visual->liveStyle = 3;
               else if (value == "Yellow LED")
                  visual->liveStyle = 4;
               else if (value == "Generic Plasma")
                  visual->liveStyle = 5;
               else if (value == "Generic LED")
                  visual->liveStyle = 6;
               else
               {
                  // Invalid style
                  CHECK_FIELD(false);
               }
            }
            else if (visual->type == VisualType::Screen)
            {
               if (value == "LCD")
                  visual->liveStyle = 1;
               else if (value == "CRT")
                  visual->liveStyle = 2;
               else
               {
                  // Invalid style
                  CHECK_FIELD(false);
               }
            }
            else if (visual->type == VisualType::SegDisplay)
            {
               if (value == "Auto")
                  visual->liveStyle = -1;
               else if (value == "Neon Plasma")
                  visual->liveStyle = 0;
               else if (value == "Blue VFD")
                  visual->liveStyle = 1;
               else if (value == "Green VFD")
                  visual->liveStyle = 2;
               else if (value == "Red LED")
                  visual->liveStyle = 3;
               else if (value == "Green LED")
                  visual->liveStyle = 4;
               else if (value == "Yellow LED")
                  visual->liveStyle = 5;
               else if (value == "Generic Plasma")
                  visual->liveStyle = 6;
               else if (value == "Generic LED")
                  visual->liveStyle = 7;
               else
               {
                  // Invalid style
                  CHECK_FIELD(false);
               }
            }
         }
         else if (key == "Src")
         {
            CHECK_FIELD((indent == 3) && (visual != nullptr) && (visual->type != VisualType::Picture)); // Display/segment data source (pictures use Image)
            visual->srcUri = value;
         }
         else if (key == "Size")
         {
            CHECK_FIELD((indent == 3) && (visual != nullptr) && (visual->type == VisualType::DMD || visual->type == VisualType::Screen)); // DMD & Screen size
            const auto size = parseArray(value);
            CHECK_FIELD(size.size() == 2);
            visual->displaySize.x = static_cast<int>(size[0]);
            visual->displaySize.y = static_cast<int>(size[1]);
         }
         else if (key == "Type")
         {
            CHECK_FIELD((indent == 3) && (visual != nullptr) && (visual->type == VisualType::SegDisplay)); // Seg display hardware family hint
            if (value == "Atari")
               visual->segFamilyHint = VPXSegDisplayHint::Atari;
            else if (value == "Bally")
               visual->segFamilyHint = VPXSegDisplayHint::Bally;
            else if (value == "Gottlieb")
               visual->segFamilyHint = VPXSegDisplayHint::Gottlieb;
            else if (value == "Williams")
               visual->segFamilyHint = VPXSegDisplayHint::Williams;
            else
            {
               // Invalid segment family hint
               CHECK_FIELD(false);
            }
         }
         else if (key == "XPos")
         {
            CHECK_FIELD((indent == 3) && (visual != nullptr) && (visual->type == VisualType::SegDisplay)); // Seg display sub element offsets
            visual->xOffsets = parseArray(value);
            CHECK_FIELD(!visual->xOffsets.empty() && visual->xOffsets.size() <= CTLPI_SEG_MAX_DISP_ELEMENTS && (visual->nElements == -1 || (int)visual->xOffsets.size() == visual->nElements));
         }
         else if (key == "NElements")
         {
            CHECK_FIELD((indent == 3) && (visual != nullptr) && (visual->type == VisualType::SegDisplay) && try_parse_int(value, visual->nElements)); // Number of seg display elements
            CHECK_FIELD(visual->nElements > 0 && visual->nElements <= CTLPI_SEG_MAX_DISP_ELEMENTS);
            CHECK_FIELD(visual->xOffsets.empty() || (int)visual->xOffsets.size() == visual->nElements);
         }
         else if (key == "Glass")
         {
            CHECK_FIELD((indent == 3) && (visual != nullptr));
            expectedIndent = 4;
            visual->style = value;
         }
         else if (key == "Tint")
         {
            CHECK_FIELD(((indent == 3) || (indent == 4)) && (visual != nullptr));
            const auto tint = parseArray(value);
            CHECK_FIELD(tint.size() == 3);
            if (indent == 3) // Visual tint (emitter tint for displays, image tint for Image visuals)
            {
               visual->tint.x = tint[0];
               visual->tint.y = tint[1];
               visual->tint.z = tint[2];
            }
            else // Glass tint
            {
               visual->glassTint.x = tint[0];
               visual->glassTint.y = tint[1];
               visual->glassTint.z = tint[2];
            }
         }
         else if (key == "Roughness")
         {
            CHECK_FIELD((indent == 4) && (visual != nullptr) && try_parse_float(value, fValue)); // Glass roughness
            visual->glassRoughness = fValue;
         }
         else if (key == "Image")
         {
            CHECK_FIELD(((indent == 3) || (indent == 4)) && (visual != nullptr));
            if (indent == 3) // Picture image file path
            {
               CHECK_FIELD(visual->type == VisualType::Picture);
               visual->srcUri = value;
            }
            else // Glass image path
               visual->glassPath = value;
         }
         else if (key == "Area")
         {
            CHECK_FIELD((indent == 4) && (visual != nullptr)); // Glass image area
            const auto rect = parseArray(value);
            CHECK_FIELD(rect.size() == 4);
            visual->glassArea.x = rect[0];
            visual->glassArea.y = rect[1];
            visual->glassArea.z = rect[2];
            visual->glassArea.w = rect[3];
         }
         else if (key == "Ambient")
         {
            CHECK_FIELD((indent == 4) && (visual != nullptr)); // Glass image ambient
            const auto rect = parseArray(value);
            CHECK_FIELD(rect.size() == 3)
            visual->glassAmbient.x = rect[0];
            visual->glassAmbient.y = rect[1];
            visual->glassAmbient.z = rect[2];
         }
      }
   }
   if (hasSection && !endLayout())
      return false;
   // Only keep the layouts of a fully valid file, so that a broken file falls back to the next layout source
   if (parsed.empty())
   {
      error = "No layout section in Nixie file " + PluginStrings::PathToUTF8(path);
      return false;
   }
   layouts.insert(layouts.end(), parsed.begin(), parsed.end());
   return true;
#undef CHECK_FIELD
}

}
