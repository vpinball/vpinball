// license:GPLv3+

#pragma once

// Parser for Nixie layout files ('.nix'), also used by other plugins (B2S/PUP
// display overlay authoring) to import layouts with the exact same file semantics.
// See plugins/nixie/layouts for file examples.
//
// A .nix file holds one or more layout sections, each introduced by a '- <window>'
// line (Backglass, ScoreView or Topper) followed by indented fields:
//
//   - ScoreView
//       Width: 128
//       Height: 32
//       Fit: Contain
//       Visuals:
//           - DMD:
//               Rect: [0,0,128,32]
//               Style: Neon Plasma
//               Src: ctrl://default/display
//               Size: [128, 32]
//               Tint: [1,0.5,0.2]
//               Glass:
//                   Tint:      [1,1,1]
//                   Roughness: 0.1
//                   Image:     ./Glass-Dirty-1.webp
//                   Area:      [0,0,1024,256]
//                   Ambient:   [0.4,0.4,0.4]
//           - Picture:
//               Rect: [0,0,64,64]
//               Image: ./logo.png
//               Tint: [1,1,1]
//
// Tint directly under a visual (not inside Glass) tints what the visual emits: the
// dots or segments of DMD, Screen and SegDisplay visuals, the image of a Picture.
// Inside Glass, Tint only applies to the glass layer.
//
// Parsed values keep the authored semantics: visual Rects are expressed in layout
// space with the origin at the top left, and Ambient values are sRGB (consumers
// apply their own conversions when rendering).

#include <filesystem>
#include <string>
#include <vector>

#include "plugins/VPXPlugin.h"

namespace PinballPlugin::NixieFile
{

class ivec2 final
{
public:
   constexpr ivec2() { }
   constexpr ivec2(const int _x, const int _y)
      : x(_x)
      , y(_y)
   {
   }

   int x = 0, y = 0;
};

class vec3 final
{
public:
   constexpr vec3() { }
   constexpr vec3(const float _x, const float _y, const float _z)
      : x(_x)
      , y(_y)
      , z(_z)
   {
   }

   float x = 0.f, y = 0.f, z = 0.f;
};

class vec4 final
{
public:
   constexpr vec4() { }
   constexpr vec4(const float _x, const float _y, const float _z, const float _w)
      : x(_x)
      , y(_y)
      , z(_z)
      , w(_w)
   {
   }

   float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
};

enum Fit
{
   Fill, // Stretch to fill container
   Contain, // Stretch to fit inside container, keeping aspect ratio, eventually causing black bars
   Cover, // Stretch to cover container, keeping aspect ratio, eventually cropping
   None, // No stretching, content is placed at center
   ScaleDown // Same as 'None' unless the content overflow the container, in which situation 'Contain' is applied
};

enum VisualType
{
   DMD,
   Screen,
   SegDisplay,
   Picture
};

struct Visual
{
   VisualType type;
   float x = 0.f, y = 0.f, w = 0.f, h = 0.f; // Bounds in layout space, origin at the layout top left
   // Data source, depending on visual type
   std::string srcUri;
   // For DMD, CRT and segment displays
   std::string style;
   vec3 tint;
   vec4 glassPad;
   vec3 glassTint;
   float glassRoughness = 0.f;
   std::string glassPath;
   vec4 glassArea;
   vec3 glassAmbient; // As authored (sRGB)
   int liveStyle = 0;
   // For segment displays
   VPXSegDisplayHint segFamilyHint = VPXSegDisplayHint::Generic;
   int nElements = 0;
   std::vector<float> xOffsets;
   // For DMD & screen displays
   ivec2 displaySize;
};

struct Layout
{
   std::filesystem::path path;
   VPXWindowId window = VPXWindowId::VPXWINDOW_Backglass;
   float width = 1920.f, height = 1080.f;
   Fit fit = Contain;
   std::vector<Visual> visuals;
};

// Parses a '.nix' file, appending its layouts to 'layouts'. Returns false and fills
// 'error' when the file is invalid, in which case nothing is appended.
bool Parse(const std::filesystem::path& path, std::vector<Layout>& layouts, std::string& error);

}
