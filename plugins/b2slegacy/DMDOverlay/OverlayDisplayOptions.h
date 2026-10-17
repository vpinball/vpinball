// license:GPLv3+

#pragma once

// Display option block shared by the authored display records of the display overlay plugins
// (B2S, B2SLegacy, PUP): an identical copy of this file lives inside the DMDOverlay folder of
// each plugin. It mirrors the per visual option set of Nixie layout files (see
// plugins/nixie/NixieLayoutFile.h): render style, expected source dot size, emitter tint and
// the glass layer (image area, tint, roughness, ambient, pad). It also provides the PagedUI
// item builders editing these options so that all overlay plugins present the same editing
// surface.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>

#include "nixie/NixieLayoutFile.h"
#include "plugins/PagedUIPlugin.h"
#include "plugins/VPXPlugin.h"

namespace PinballPlugin
{

// Persisted rendering options of an authored overlay display (a dot matrix or video
// display rendered on top of an artwork or window). 'style' and the serialized
// style names match the .nix 'Style' field values.
struct OverlayDisplayOptions
{
   int style = 0; // 0 = auto (render style derived from the source hardware); else a style index (1..8)
   int sizeW = 0, sizeH = 0; // Expected source dot size (0 = unset)
   float tintR = 1.f, tintG = 1.f, tintB = 1.f; // Emitter tint
   float glassTintR = 1.f, glassTintG = 1.f, glassTintB = 1.f;
   float glassRoughness = 0.f;
   float glassAmbientR = 1.f, glassAmbientG = 1.f, glassAmbientB = 1.f; // Room lighting on the glass, as persisted (sRGB)
   int glassAreaX = 0, glassAreaY = 0, glassAreaW = 0, glassAreaH = 0; // Used area, in glass image pixels (0 w/h = whole image)
   float glassPadL = 0.f, glassPadT = 0.f, glassPadR = 0.f, glassPadB = 0.f; // Emitter padding from the glass bounds (.nix Pad semantics)
};

// Persisted style values, matching the .nix 'Style' field names; value 0 is 'Auto'
inline constexpr const char* OverlayDisplayStyleNames[] = { "Neon Plasma", "Red LED", "Green LED", "Yellow LED", "Generic Plasma", "Generic LED", "LCD", "CRT" };

inline int OverlayDisplayStyleIndex(const char* name)
{
   if (name == nullptr)
      return 0;
   for (int i = 0; i < 8; i++)
      if (strcmp(name, OverlayDisplayStyleNames[i]) == 0)
         return i + 1;
   return 0;
}

// Maps a persisted style to the render API style; returns false when 'style' is
// 'auto', in which case the caller derives the style from the source hardware
inline bool OverlayDisplayStyleToVPX(int style, VPXDisplayRenderStyle& out)
{
   switch (style)
   {
   case 1: out = VPXDisplayRenderStyle::VPXDMDStyle_Plasma; return true;
   case 2: out = VPXDisplayRenderStyle::VPXDMDStyle_RedLED; return true;
   case 3: out = VPXDisplayRenderStyle::VPXDMDStyle_GreenLED; return true;
   case 4: out = VPXDisplayRenderStyle::VPXDMDStyle_YellowLED; return true;
   case 5: out = VPXDisplayRenderStyle::VPXDMDStyle_GenPlasma; return true;
   case 6: out = VPXDisplayRenderStyle::VPXDMDStyle_GenLED; return true;
   case 7: out = VPXDisplayRenderStyle::VPXDMDStyle_Pixelated; return true;
   case 8: out = VPXDisplayRenderStyle::VPXDMDStyle_CRT; return true;
   default: return false;
   }
}

inline bool SameOverlayDisplayOptions(const OverlayDisplayOptions& a, const OverlayDisplayOptions& b)
{
   return a.style == b.style && a.sizeW == b.sizeW && a.sizeH == b.sizeH //
      && a.tintR == b.tintR && a.tintG == b.tintG && a.tintB == b.tintB //
      && a.glassTintR == b.glassTintR && a.glassTintG == b.glassTintG && a.glassTintB == b.glassTintB //
      && a.glassRoughness == b.glassRoughness //
      && a.glassAmbientR == b.glassAmbientR && a.glassAmbientG == b.glassAmbientG && a.glassAmbientB == b.glassAmbientB //
      && a.glassAreaX == b.glassAreaX && a.glassAreaY == b.glassAreaY && a.glassAreaW == b.glassAreaW && a.glassAreaH == b.glassAreaH //
      && a.glassPadL == b.glassPadL && a.glassPadT == b.glassPadT && a.glassPadR == b.glassPadR && a.glassPadB == b.glassPadB;
}

// Copies the rendering options of a parsed Nixie visual (everything but the bounds
// and the source, mapped by the caller to its own coordinate space and defaults)
inline void ApplyNixieVisualOptions(OverlayDisplayOptions& options, const NixieFile::Visual& visual)
{
   switch (visual.type)
   {
   case NixieFile::VisualType::DMD: options.style = (visual.liveStyle >= 1 && visual.liveStyle <= 6) ? visual.liveStyle : 0; break;
   case NixieFile::VisualType::Screen: options.style = visual.liveStyle == 1 ? 7 : visual.liveStyle == 2 ? 8 : 0; break;
   default: options.style = 0; break;
   }
   options.sizeW = std::max(0, visual.displaySize.x);
   options.sizeH = std::max(0, visual.displaySize.y);
   options.tintR = visual.tint.x;
   options.tintG = visual.tint.y;
   options.tintB = visual.tint.z;
   options.glassTintR = visual.glassTint.x;
   options.glassTintG = visual.glassTint.y;
   options.glassTintB = visual.glassTint.z;
   options.glassRoughness = visual.glassRoughness;
   options.glassAmbientR = visual.glassAmbient.x;
   options.glassAmbientG = visual.glassAmbient.y;
   options.glassAmbientB = visual.glassAmbient.z;
   options.glassAreaX = static_cast<int>(visual.glassArea.x);
   options.glassAreaY = static_cast<int>(visual.glassArea.y);
   options.glassAreaW = static_cast<int>(visual.glassArea.z);
   options.glassAreaH = static_cast<int>(visual.glassArea.w);
   options.glassPadL = visual.glassPad.x;
   options.glassPadT = visual.glassPad.y;
   options.glassPadR = visual.glassPad.z;
   options.glassPadB = visual.glassPad.w;
}

// Serialization helpers for the float lists of the document formats
inline std::string OverlayFormatFloats(const float* values, int count)
{
   std::string result;
   for (int i = 0; i < count; i++)
   {
      if (i != 0)
         result += ',';
      result += std::to_string(values[i]);
      // Compact to shortest round trip representation
      while (!result.empty() && result.back() == '0')
         result.pop_back();
      if (!result.empty() && result.back() == '.')
         result.pop_back();
   }
   return result;
}

inline bool OverlayParseFloats(const char* text, float* values, int count)
{
   if (text == nullptr)
      return false;
   std::string str(text);
   size_t pos = 0;
   for (int i = 0; i < count; i++)
   {
      const size_t end = str.find(',', pos);
      try
      {
         values[i] = std::stof(str.substr(pos, end - pos));
      }
      catch (...)
      {
         return false;
      }
      if (end == std::string::npos)
         return i == count - 1;
      pos = end + 1;
   }
   return pos >= str.size(); // Reject lists with more than 'count' elements
}

// sRGB to linear conversion of a color component (glass ambient is authored/saved
// in sRGB while the render API expects linear values)
inline float OverlayInvsRGB(const float x) { return (x <= 0.04045f) ? (x * (float)(1.0 / 12.92)) : (powf(x * (float)(1.0 / 1.055) + (float)(0.055 / 1.055), 2.4f)); }

namespace OverlayUI
{

   inline int MSGPIAPI FieldIntGet(void* context) { return *static_cast<int*>(context); }
   inline void MSGPIAPI FieldIntSet(void* context, int value) { *static_cast<int*>(context) = value; }
   inline float MSGPIAPI FieldFloatGet(void* context) { return *static_cast<float*>(context); }
   inline void MSGPIAPI FieldFloatSet(void* context, float value) { *static_cast<float*>(context) = value; }

   inline void FillItem(PagedUIItemDef& item, const char* id, const char* label, const char* tooltip)
   {
      item = PagedUIItemDef {};
      item.id = id;
      item.label = label;
      item.tooltip = tooltip;
      item.storage = PAGEDUI_STORAGE_PLUGIN;
   }

   inline void FillIntItem(PagedUIItemDef& item, const char* id, const char* label, const char* tooltip, int* field, int maxVal, int defVal = 0)
   {
      FillItem(item, id, label, tooltip);
      item.type = PAGEDUI_ITEM_INT;
      item.context = field;
      item.intProp.minVal = 0;
      item.intProp.maxVal = maxVal;
      item.intProp.defVal = defVal;
      item.intProp.format = nullptr;
      item.intProp.Get = &FieldIntGet;
      item.intProp.Set = &FieldIntSet;
   }

   inline void FillFloatItem(PagedUIItemDef& item, const char* id, const char* label, const char* tooltip, float* field, float minVal, float maxVal, float defVal)
   {
      FillItem(item, id, label, tooltip);
      item.type = PAGEDUI_ITEM_FLOAT;
      item.context = field;
      item.floatProp.minVal = minVal;
      item.floatProp.maxVal = maxVal;
      item.floatProp.step = 0.01f;
      item.floatProp.defVal = defVal;
      item.floatProp.displayScale = 1.f;
      item.floatProp.format = "%4.2f"; // Display precision must be finer than the step for single step adjustments to apply
      item.floatProp.Get = &FieldFloatGet;
      item.floatProp.Set = &FieldFloatSet;
   }

   // Item counts emitted by the fill functions below: GetItemCount must return the
   // same number of items as GetItems fills, so page item counts must be derived
   // from these. The glass image area fields are only emitted when glassW > 0.
   inline constexpr int OverlayDisplayOptionItemCount = 6;
   inline constexpr int OverlayGlassOptionItemCount(int glassW) { return 12 + (glassW > 0 ? 4 : 0); }

   // Emits the rendering option items of a display: section header, render style
   // enum and emitter tint, then the header of the glass section that separates
   // them from the glass option items (6 items). 'str' interns generated item
   // ids for the duration of the enumeration call.
   inline int FillDisplayOptionItems(PagedUIItemDef* items, const std::function<const char*(std::string)>& str, const char* prefix, OverlayDisplayOptions* o)
   {
      static const char* styleValues[] = { "Auto", "Neon Plasma", "Red LED", "Green LED", "Yellow LED", "Generic Plasma", "Generic LED", "LCD", "CRT" };
      int i = 0;
      FillItem(items[i], nullptr, "Display", "Rendering of the display emitter");
      items[i++].type = PAGEDUI_ITEM_HEADER;
      FillItem(items[i], str(std::string(prefix) + "style"), "Style", "Render style of the display; 'Auto' derives it from the source hardware");
      items[i].type = PAGEDUI_ITEM_ENUM;
      items[i].context = &o->style;
      items[i++].enumProp = { 0, 9, styleValues, 0, &FieldIntGet, &FieldIntSet };
      FillFloatItem(items[i++], str(std::string(prefix) + "tintR"), "Tint - Red", "Tint of the display emitter", &o->tintR, 0.f, 1.f, 1.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "tintG"), "Tint - Green", "Tint of the display emitter", &o->tintG, 0.f, 1.f, 1.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "tintB"), "Tint - Blue", "Tint of the display emitter", &o->tintB, 0.f, 1.f, 1.f);
      FillItem(items[i], nullptr, "Glass", "Glass covering the display");
      items[i++].type = PAGEDUI_ITEM_HEADER;
      return i;
   }

   // Emits the glass layer items except the section header (emitted last by
   // FillDisplayOptionItems) and the image selection itself (emitted by the
   // caller after these items): glass tint, roughness, ambient and pad, then the
   // 'Glass Texture' header and the image area fields when a glass size is passed
   // (12 or 16 items). 'str' interns the generated item ids for the duration of
   // the enumeration call.
   inline int FillDisplayGlassItems(PagedUIItemDef* items, const std::function<const char*(std::string)>& str, const char* prefix, OverlayDisplayOptions* o, int glassW, int glassH)
   {
      int i = 0;
      FillFloatItem(items[i++], str(std::string(prefix) + "glassRoughness"), "Roughness", "Roughness of the glass surface", &o->glassRoughness, 0.f, 1.f, 0.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassTintR"), "Tint - Red", "Tint applied to the glass image", &o->glassTintR, 0.f, 1.f, 1.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassTintG"), "Tint - Green", "Tint applied to the glass image", &o->glassTintG, 0.f, 1.f, 1.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassTintB"), "Tint - Blue", "Tint applied to the glass image", &o->glassTintB, 0.f, 1.f, 1.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassAmbR"), "Ambient - Red", "Room lighting reflected by the glass (sRGB)", &o->glassAmbientR, 0.f, 1.f, 1.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassAmbG"), "Ambient - Green", "Room lighting reflected by the glass (sRGB)", &o->glassAmbientG, 0.f, 1.f, 1.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassAmbB"), "Ambient - Blue", "Room lighting reflected by the glass (sRGB)", &o->glassAmbientB, 0.f, 1.f, 1.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassPadL"), "Emitter padding - Left", "Emitter padding from the glass left bound", &o->glassPadL, 0.f, 1.f, 0.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassPadT"), "Emitter padding - Top", "Emitter padding from the glass top bound", &o->glassPadT, 0.f, 1.f, 0.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassPadR"), "Emitter padding - Right", "Emitter padding from the glass right bound", &o->glassPadR, 0.f, 1.f, 0.f);
      FillFloatItem(items[i++], str(std::string(prefix) + "glassPadB"), "Emitter padding - Bottom", "Emitter padding from the glass bottom bound", &o->glassPadB, 0.f, 1.f, 0.f);
      FillItem(items[i], nullptr, "Glass Texture", "Image applied over the display as its glass surface");
      items[i++].type = PAGEDUI_ITEM_HEADER;
      if (glassW > 0)
      {
         FillIntItem(items[i++], str(std::string(prefix) + "glassAreaX"), "Image area - X", "Area of the glass image covering the display, in image pixels", &o->glassAreaX, glassW);
         FillIntItem(items[i++], str(std::string(prefix) + "glassAreaY"), "Image area - Y", "Area of the glass image covering the display, in image pixels", &o->glassAreaY, glassH);
         FillIntItem(items[i++], str(std::string(prefix) + "glassAreaW"), "Image area - Width", "Area of the glass image covering the display, in image pixels (0 = whole image)",
            &o->glassAreaW, glassW);
         FillIntItem(items[i++], str(std::string(prefix) + "glassAreaH"), "Image area - Height", "Area of the glass image covering the display, in image pixels (0 = whole image)",
            &o->glassAreaH, glassH);
      }
      return i;
   }

}

}
