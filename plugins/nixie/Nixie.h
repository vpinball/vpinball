// license:GPLv3+

#pragma once

#include "common.h"

#include "NixieLayoutFile.h"
#include "plugins/ResURIResolver.h"

#include <algorithm>
#include <filesystem>
#include <unordered_dense.h>

namespace Nixie
{

LPI_USE_CPP();
#ifndef LOGD
#define LOGD LPI_LOGD_CPP
#define LOGI LPI_LOGI_CPP
#define LOGW LPI_LOGW_CPP
#define LOGE LPI_LOGE_CPP
#endif

using PinballPlugin::NixieFile::ivec2;
using PinballPlugin::NixieFile::vec3;
using PinballPlugin::NixieFile::vec4;

class Nixie final
{
public:
   Nixie(const MsgPluginAPI* api, unsigned int endpointId, VPXPluginAPI* vpxApi);
   ~Nixie();

   bool HasLayouts(const VPXWindowId window) const { return std::ranges::any_of(m_layouts, [window](const Layout& layout) { return layout.window == window; }); }
   bool IsMatched() const { return m_bestLayout != nullptr && m_bestLayout->unmatchedVisuals == 0; }
   void Load(const std::filesystem::path& path);
   void Reset() { m_layouts.clear(); }

   bool Render(VPXRenderContext2D* ctx);

private:
   void Parse(const std::filesystem::path& path);
   void Select(const VPXWindowId window, const float scoreW, const float scoreH);

   using Fit = PinballPlugin::NixieFile::Fit;
   using VisualType = PinballPlugin::NixieFile::VisualType;
   struct Visual final : PinballPlugin::NixieFile::Visual
   {
      explicit Visual(const PinballPlugin::NixieFile::Visual& parsed)
         : PinballPlugin::NixieFile::Visual(parsed)
      {
      }
      // Live data (not serialized)
      VPXTexture glass = nullptr;
      VPXTexture pictureTex = nullptr; // Borrowed from the shared m_images cache (not owned)
      VPXTexture dmdTex = nullptr;
      // Identity of the frame last uploaded to dmdTex, to avoid a full copy plus a GPU re-upload every frame
      DisplaySrcId uploadedSrc {};
      unsigned int uploadedFrameId = 0;
      bool hasUploadedFrame = false;
   };
   struct Layout
   {
      std::filesystem::path path;
      VPXWindowId window;
      float width, height;
      Fit fit;
      vector<Visual> visuals;
      // Live data
      unsigned int unmatchedVisuals = 0;
      unsigned int matchedVisuals = 0;
      float unfittedPixels = 0.f;
   };

   vector<Layout> m_layouts;
   bool m_invalidBestLayout = true;
   Layout* m_bestLayout = nullptr;

   PinballPlugin::ResURIResolver m_resURIResolver;

   void LoadGlass(Visual& visual);
   VPXTexture LoadTexture(const std::string& relPath); // Load (and cache) an image file next to the selected layout file
   ankerl::unordered_dense::map<string, VPXTexture> m_images; // Keyed by resolved file path

   VPXPluginAPI* const m_vpxApi;
   const MsgPluginAPI* const m_msgApi;
   const unsigned int m_endpointId;

   unsigned int m_onDmdChangedMsgId, m_onSegChangedMsgId;
   static void OnResChanged(const unsigned int msgId, void* userData, void* msgData);
};

}
