// license:GPLv3+

#pragma once

#include "common.h"

#include "core/ResURIResolver.h"

#include <filesystem>
#include <unordered_dense.h>

namespace ScoreView {
   
class vec2i
{
public:
   constexpr vec2i() { }
   constexpr vec2i(const int _x, const int _y)
      : x(_x)
      , y(_y)
   {
   }

   int x, y;
};

class vec3
{
public:
   constexpr vec3() { }
   constexpr vec3(const float _x, const float _y, const float _z)
      : x(_x)
      , y(_y)
      , z(_z)
   {
   }

   float x, y, z;
};

class vec4
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

   float x, y, z, w;
};

class ScoreView final
{
public:
   ScoreView(MsgPluginAPI* api, unsigned int endpointId, VPXPluginAPI* vpxApi);
   ~ScoreView();

   bool HasLayouts() const { return !m_layouts.empty(); }
   bool IsMatched() const { return m_bestLayout != nullptr && m_bestLayout->unmatchedVisuals == 0; }
   void Load(const string& path);
   void Reset() { m_layouts.clear(); }

   bool Render(VPXRenderContext2D* ctx);

private:
   void Parse(const std::filesystem::path& path, std::istream& content);
   void Select(const float scoreW, const float scoreH);

   enum Fit
   {
      Fill,       // Stretch to fill container
      Contain,    // Stretch to fit inside container, keeping aspect ratio, eventually causing black bars
      Cover,      // Stretch to cover container, keeping aspect ratio, eventually cropping
      None,       // No stretching, content is placed at center
      ScaleDown   // Same as 'None' unless the content overflow the container, in which situation 'Contain' is applied
   };
   enum VisualType
   {
      DMD, SegDisplay, Image
   };
   struct Visual
   {
      VisualType type;
      float x, y, w, h;
      // Data source, depending on visual type
      string srcUri;
      // For DMD, CRT and segment displays
      string style;
      vec3 tint;
      vec4 glassPad;
      vec3 glassTint;
      float glassRoughness;
      string glassPath;
      vec4 glassArea;
      vec3 glassAmbient;
      // For segment displays
      VPXSegDisplayHint segFamilyHint;
      int nElements;
      std::vector<float> xOffsets;
      // For DMD displays
      vec2i dmdSize;
      // Live data (not serialized)
      VPXTexture glass;
      VPXTexture dmdTex;
      int liveStyle;
   };
   struct Layout
   {
      std::filesystem::path path;
      float width, height;
      Fit fit;
      vector<Visual> visuals;
      // Live data
      unsigned int unmatchedVisuals;
      unsigned int matchedVisuals;
      float unfittedPixels;
   };

   vector<Layout> m_layouts;
   bool m_invalidBestLayout = true;
   Layout* m_bestLayout = nullptr;

   ResURIResolver m_resURIResolver;

   void LoadGlass(Visual& visual);
   ankerl::unordered_dense::map<string, VPXTexture> m_images;

   VPXPluginAPI* const m_vpxApi;
   MsgPluginAPI* const m_msgApi;
   const unsigned int m_endpointId;

   unsigned int m_onDmdChangedMsgId, m_onSegChangedMsgId;
   static void OnResChanged(const unsigned int msgId, void* userData, void* msgData);
};

}