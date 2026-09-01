#include "FlexDMD.h"
#include "UltraDMD.h"
#include "actors/Group.h"
#include "actors/Label.h"
#include "actors/Frame.h"
#include "actors/Image.h"
#include "actors/ImageSequence.h"
#include "actors/GIFImage.h"
#include "actors/Video.h"
#include "resources/AssetManager.h"

#include <format>

#include <SDL3/SDL_timer.h>

namespace Flex {

FlexDMD::FlexDMD(const MsgPluginAPI* msgApi, unsigned int endpointId, VPXPluginAPI* vpxApi)
   : m_vpxApi(vpxApi)
   , m_endpointId(endpointId)
   , m_dmdProvider(msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG)
   , m_segProvider(msgApi, endpointId, CTLPI_SEG_GET_SRC_MSG, CTLPI_SEG_ON_SRC_CHG_MSG)
{
   m_pStage = new Group(this, "Stage"s);
   m_pStage->SetSize(m_width, m_height);
   m_pAssetManager = new AssetManager(m_vpxApi);
}

FlexDMD::~FlexDMD()
{
   SetRun(false);
   m_pStage->Release();
   DiscardFrames();
   delete m_pAssetManager;
   delete m_pSurface;
   if (m_onDestroyHandler != nullptr)
      m_onDestroyHandler(this);
}

void FlexDMD::SetRun(bool run)
{
   if (run == m_run)
      return;

   m_segProvider.ClearItems();
   m_dmdProvider.ClearItems();

   m_run = run;
   if (m_run) {
      m_lastRenderTick = SDL_GetTicks();
      m_pStage->SetOnStage(true);
      //RenderLoop();
   }
   else {
      //m_pThread->join();
      //delete m_pThread;
      //m_pThread = NULL;
      m_pAssetManager->ClearAll();
      m_pStage->SetOnStage(false);
   }
   
   AdvertiseDisplay();
}

void FlexDMD::SetShow(bool v)
{
   if (m_show == v)
      return;

   m_segProvider.ClearItems();
   m_dmdProvider.ClearItems();

   m_show = v;

   AdvertiseDisplay();
}

void FlexDMD::SetWidth(int w)
{
   if (m_width == w)
      return;

   m_dmdProvider.ClearItems();
   m_segProvider.ClearItems();

   m_width = w;
   m_pStage->SetSize(m_width, m_height);
   DiscardFrames();

   AdvertiseDisplay();
}

void FlexDMD::SetHeight(int h)
{
   if (m_height == h)
      return;

   m_dmdProvider.ClearItems();
   m_segProvider.ClearItems();

   m_height = h;
   m_pStage->SetSize(m_width, m_height);
   DiscardFrames();

   AdvertiseDisplay();
}

void FlexDMD::SetRenderMode(RenderMode renderMode)
{
   if (m_renderMode == renderMode)
      return;

   m_dmdProvider.ClearItems();
   m_segProvider.ClearItems();

   m_renderMode = renderMode;
   DiscardFrames();

   AdvertiseDisplay();
}

DisplayFrame FlexDMD::GetRenderFrame(void* callContext)
{
   auto ctx = static_cast<CallContext*>(callContext);
   FlexDMD* me = ctx->me;
   me->Render();
   if (me->GetRenderMode() == RenderMode_DMD_RGB)
      return { me->m_frameId, me->UpdateRGBFrame() };
   else if ((me->GetRenderMode() == RenderMode_DMD_GRAY_2) || (me->GetRenderMode() == RenderMode_DMD_GRAY_4))
      return { me->m_frameId, me->UpdateLumFP32Frame() };
   assert(false);
   return { 0, nullptr };
}

SegDisplayFrame FlexDMD::GetSegState(void* callContext)
{
   static int sizes[17][14] = {
      {}, // RenderMode_DMD_GRAY_2
      {}, // RenderMode_DMD_GRAY_4
      {}, // RenderMode_DMD_RGB
      { 16, 16 }, // RenderMode_SEG_2x16Alpha
      { 20, 20 }, // RenderMode_SEG_2x20Alpha
      { 7, 7, 7, 7 }, // RenderMode_SEG_2x7Alpha_2x7Num
      { 7, 7, 7, 7, 1, 1, 1, 1 }, // RenderMode_SEG_2x7Alpha_2x7Num_4x1Num
      { 7, 7, 7, 7, 1, 1, 1, 1 }, // RenderMode_SEG_2x7Num_2x7Num_4x1Num
      { 7, 7, 7, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // RenderMode_SEG_2x7Num_2x7Num_10x1Num
      { 7, 7, 7, 7, 1, 1, 1, 1 }, // RenderMode_SEG_2x7Num_2x7Num_4x1Num_gen7
      { 7, 7, 7, 7, 1, 1, 1, 1 }, // RenderMode_SEG_2x7Num10_2x7Num10_4x1Num
      { 6, 6, 6, 6, 1, 1, 1, 1 }, // RenderMode_SEG_2x6Num_2x6Num_4x1Num
      { 6, 6, 6, 6, 1, 1, 1, 1 }, // RenderMode_SEG_2x6Num10_2x6Num10_4x1Num
      { 7, 7, 7, 7 }, // RenderMode_SEG_4x7Num10
      { 6, 6, 6, 6, 1, 1, 1, 1 }, // RenderMode_SEG_6x4Num_4x1Num
      { 7, 7, 1, 1, 1, 1, 16 }, // RenderMode_SEG_2x7Num_4x1Num_1x16Alpha,
      { 16, 16, 7 }, // RenderMode_SEG_1x16Alpha_1x16Num_1x7Num
   };
   auto ctx = static_cast<CallContext*>(callContext);
   FlexDMD* me = ctx->me;
   uint32_t subId = ctx->index;
   int pos = 0;
   static thread_local float segLuminances[16 * 128] = { };
   float* lum = segLuminances;
   for (uint32_t i = 0; i < subId; i++)
   {
      pos += sizes[me->GetRenderMode()][i];
      lum += sizes[me->GetRenderMode()][i] * 16;
   }
   for (int i = 0; i < sizes[me->GetRenderMode()][subId]; i++)
   {
      uint16_t v = me->m_segData[pos + i];
      for (int j = 0; j < 16; j++, v >>= 1)
         lum[i * 16 + j] = (v & 1) ? 1.f : 0.f;
   }
   return { me->m_frameId, lum };
}

void FlexDMD::AdvertiseDisplay()
{
   assert(m_segProvider.GetItems().empty());
   assert(m_dmdProvider.GetItems().empty());

   if (!m_run || !m_show)
      return;

   m_callContexts.clear();
   if (GetRenderMode() == RenderMode_DMD_GRAY_2 || GetRenderMode() == RenderMode_DMD_GRAY_4 || GetRenderMode() == RenderMode_DMD_RGB)
   {
      m_callContexts.emplace_back(this, 0);
      m_dmdProvider.SetItem({ //
         .id = { .endpointId = m_endpointId, .resId = GetId() << 8 },
         .overrideId = { },
         .width = static_cast<unsigned int>(m_width),
         .height = static_cast<unsigned int>(m_height),
         .callContext = &m_callContexts[0],
         .frameFormat = GetRenderMode() == RenderMode_DMD_RGB ? CTLPI_DISPLAY_FORMAT_SRGB888 : CTLPI_DISPLAY_FORMAT_LUM32F,
         .GetRenderFrame = GetRenderFrame });
      return;
   }

   std::vector<SegSrcId> segSrcs;
   auto AddSegSrc = [this, &segSrcs](uint32_t displayIndex, int nDisplays, unsigned int nElements, SegElementType type)
   {
      SegSrcId src = { };
      src.id = { .endpointId = m_endpointId, .resId = (GetId() << 8) | displayIndex };
      src.groupId = { m_endpointId, GetId() };
      src.hardware = CTLPI_SEG_HARDWARE_UNKNOWN;
      src.nElements = nElements;
      for (unsigned int j = 0; j < nElements; j++)
         src.elementType[j] = type;
      src.GetState = GetSegState;
      segSrcs.push_back(src);
      m_callContexts.emplace_back(this, displayIndex);
   };
   switch (GetRenderMode())
   {
   case RenderMode_SEG_2x16Alpha:
      AddSegSrc(0, 2, 16, CTLPI_SEG_LAYOUT_14D);
      AddSegSrc(1, 2, 16, CTLPI_SEG_LAYOUT_14D);
      break;
   case RenderMode_SEG_2x20Alpha:
      AddSegSrc(0, 2, 20, CTLPI_SEG_LAYOUT_14D);
      AddSegSrc(1, 2, 20, CTLPI_SEG_LAYOUT_14D);
      break;
   case RenderMode_SEG_2x7Alpha_2x7Num:
      AddSegSrc(0, 4, 7, CTLPI_SEG_LAYOUT_14D);
      AddSegSrc(1, 4, 7, CTLPI_SEG_LAYOUT_14D);
      AddSegSrc(2, 4, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(3, 4, 7, CTLPI_SEG_LAYOUT_7C);
      break;
   case RenderMode_SEG_2x7Alpha_2x7Num_4x1Num:
      AddSegSrc(0, 6, 7, CTLPI_SEG_LAYOUT_14D);
      AddSegSrc(1, 6, 7, CTLPI_SEG_LAYOUT_14D);
      AddSegSrc(2, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(3, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(4, 6, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(5, 6, 2, CTLPI_SEG_LAYOUT_7C);
      break;
   case RenderMode_SEG_2x7Num_2x7Num_4x1Num:
      AddSegSrc(0, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(1, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(2, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(3, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(4, 6, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(5, 6, 2, CTLPI_SEG_LAYOUT_7C);
      break;
   case RenderMode_SEG_2x7Num_2x7Num_10x1Num:
      AddSegSrc(0, 9, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(1, 9, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(2, 9, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(3, 9, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(4, 9, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(5, 9, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(6, 9, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(7, 9, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(8, 9, 2, CTLPI_SEG_LAYOUT_7C);
      break;
   case RenderMode_SEG_2x7Num_2x7Num_4x1Num_gen7:
      AddSegSrc(0, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(1, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(2, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(3, 6, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(4, 6, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(5, 6, 2, CTLPI_SEG_LAYOUT_7C);
      break;
   case RenderMode_SEG_2x7Num10_2x7Num10_4x1Num:
      AddSegSrc(0, 6, 7, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(1, 6, 7, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(2, 6, 7, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(3, 6, 7, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(4, 6, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(5, 6, 2, CTLPI_SEG_LAYOUT_7C);
      break;
   case RenderMode_SEG_2x6Num_2x6Num_4x1Num:
      AddSegSrc(0, 6, 6, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(1, 6, 6, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(2, 6, 6, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(3, 6, 6, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(4, 6, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(5, 6, 2, CTLPI_SEG_LAYOUT_7C);
      break;
   case RenderMode_SEG_2x6Num10_2x6Num10_4x1Num:
      AddSegSrc(0, 6, 6, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(1, 6, 6, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(2, 6, 6, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(3, 6, 6, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(4, 6, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(5, 6, 2, CTLPI_SEG_LAYOUT_7C);
      break;
   case RenderMode_SEG_4x7Num10:
      AddSegSrc(0, 4, 7, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(1, 4, 7, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(2, 4, 7, CTLPI_SEG_LAYOUT_9C);
      AddSegSrc(3, 4, 7, CTLPI_SEG_LAYOUT_9C);
      break;
   case RenderMode_SEG_6x4Num_4x1Num:
      AddSegSrc(0, 6, 4, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(1, 6, 4, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(2, 6, 4, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(3, 6, 4, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(4, 6, 4, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(5, 6, 4, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(4, 6, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(5, 6, 2, CTLPI_SEG_LAYOUT_7C);
      break;
   case RenderMode_SEG_2x7Num_4x1Num_1x16Alpha:
      AddSegSrc(0, 5, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(1, 5, 7, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(2, 5, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(3, 5, 2, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(4, 5, 16, CTLPI_SEG_LAYOUT_14D);
      break;
   case RenderMode_SEG_1x16Alpha_1x16Num_1x7Num:
      AddSegSrc(0, 3, 16, CTLPI_SEG_LAYOUT_14D);
      AddSegSrc(1, 3, 16, CTLPI_SEG_LAYOUT_7C);
      AddSegSrc(2, 3, 7, CTLPI_SEG_LAYOUT_7C);
      break;
   default: break;
   }
   for (size_t i = 0; i < segSrcs.size(); i++)
      segSrcs[i].callContext = &m_callContexts[i];
   m_segProvider.AddItems(segSrcs);
}

void FlexDMD::Render()
{
   // TODO we could do it on a separate thread and afford a latency of 1 frame to remove all overhead
   uint64_t tick = SDL_GetTicks();
   uint64_t elapsedMs = tick - m_lastRenderTick;
   if ((m_renderLockCount == 0) && elapsedMs > 2)
   {
      m_frameId++;
      m_lum8FrameDirty = m_lumFP32FrameDirty = m_lumFrameDirty = m_rgbFrameDirty = m_rgbaFrameDirty = true;
      m_lastRenderTick = tick;
      m_pStage->Update((float)((double)elapsedMs / 1000.0));
      if (m_pSurface == nullptr)
      {
         SDL_Surface* pSurface = SDL_CreateSurface(m_width, m_height, SDL_PIXELFORMAT_RGB24);
         m_pSurface = new Flex::SurfaceGraphics(pSurface);
      }
      if (m_clear)
      {
         m_pSurface->SetColor(RGB(0, 0, 0));
         m_pSurface->Clear();
      }
      m_pStage->Draw(m_pSurface);
   }
}

uint8_t* FlexDMD::UpdateLum8Frame()
{
   if ((m_lum8Frame != nullptr) && !m_lum8FrameDirty)
      return m_lum8Frame;
   if (m_lum8Frame == nullptr)
      m_lum8Frame = new uint8_t[m_width * m_height];
   UpdateLumFP32Frame();
   if (m_lumFP32Frame == nullptr)
      return m_lum8Frame;
   m_lum8FrameDirty = false;
   const float* __restrict src = m_lumFP32Frame;
   uint8_t* __restrict dst = m_lum8Frame;
   for (int o = 0; o < m_height * m_width; o++)
      *dst++ = static_cast<uint8_t>(static_cast<float>(*src++) * 255.0f);
   return m_lum8Frame;
}

float* FlexDMD::UpdateLumFP32Frame()
{
   if ((m_lumFP32Frame != nullptr) && !m_lumFP32FrameDirty)
      return m_lumFP32Frame;
   if (m_lumFP32Frame == nullptr)
      m_lumFP32Frame = new float[m_width * m_height];
   if (m_pSurface == nullptr)
      return m_lumFP32Frame;
   if (m_renderLockCount > 0)
      return m_lumFP32Frame;
   m_lumFP32FrameDirty = false;
   SDL_Surface* surf = m_pSurface->GetSurface();
   SDL_LockSurface(surf);
   const uint8_t* __restrict pixels = static_cast<const uint8_t*>(surf->pixels);
   float* __restrict dst = m_lumFP32Frame;
   constexpr float scale = static_cast<float>(1.0 / 255.0);
   for (int o = 0; o < m_height * m_width; o++)
   {
      const float r = static_cast<float>(*pixels++);
      const float g = static_cast<float>(*pixels++);
      const float b = static_cast<float>(*pixels++);
      *dst++ = (0.2126f * r + 0.7152f * g + 0.0722f * b) * scale;
   }
   SDL_UnlockSurface(surf);
   return m_lumFP32Frame;
}

void FlexDMD::UpdateLumFrame()
{
   if (!m_lumFrame.empty() && !m_lumFrameDirty)
      return;
   if (m_lumFrame.empty())
      m_lumFrame.resize(m_width * m_height);
   if (m_pSurface == nullptr)
      return;
   if (m_renderLockCount > 0)
      return;
   m_lumFrameDirty = false;
   UpdateLum8Frame();
   const uint8_t* __restrict src = m_lum8Frame;
   uint8_t* __restrict dst = m_lumFrame.data();
   static constexpr uint8_t lum4[] = { 0, 85, 170, 255 };
   static constexpr uint8_t lum16[] = { 0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255 };
   if (m_renderMode == RenderMode_DMD_GRAY_2)
      for (int o = 0; o < m_height*m_width; o++)
         *dst++ = lum4[(*src++) >> 6];

   if (m_renderMode == RenderMode_DMD_GRAY_4)
      for (int o = 0; o < m_height*m_width; o++)
         *dst++ = lum16[(*src++) >> 4];
}

uint8_t* FlexDMD::UpdateRGBFrame()
{
   if ((m_rgbFrame != nullptr) && !m_rgbFrameDirty)
      return m_rgbFrame;
   if (m_rgbFrame == nullptr)
      m_rgbFrame = new uint8_t[m_width * m_height * 3];
   if (m_pSurface == nullptr)
      return m_rgbFrame;
   if (m_renderLockCount > 0)
      return m_rgbFrame;
   m_rgbFrameDirty = false;
   SDL_Surface* surf = m_pSurface->GetSurface();
   SDL_LockSurface(surf);
   memcpy(m_rgbFrame, static_cast<uint8_t*>(surf->pixels), m_width * m_height * 3);
   SDL_UnlockSurface(surf);
   return m_rgbFrame;
}

void FlexDMD::UpdateRGBAFrame()
{
   if (!m_rgbaFrame.empty() && !m_rgbaFrameDirty)
      return;
   if (m_rgbaFrame.empty())
      m_rgbaFrame.resize(m_width * m_height);
   if (m_pSurface == nullptr)
      return;
   if (m_renderLockCount > 0)
      return;
   m_rgbaFrameDirty = false;
   SDL_Surface* surf = m_pSurface->GetSurface();
   SDL_LockSurface(surf);
   const uint8_t* __restrict pixels = static_cast<const uint8_t*>(surf->pixels);
   uint32_t* __restrict dst = m_rgbaFrame.data();
   for (int o = 0; o < m_height*m_width; o++)
      {
         const uint8_t r = *pixels++;
         const uint8_t g = *pixels++;
         const uint8_t b = *pixels++;
         *dst++ = RGB(r, g, b);
      }
   SDL_UnlockSurface(surf);
}

const std::vector<uint32_t>& FlexDMD::GetDmdColoredPixels()
{
   Render();
   UpdateRGBAFrame();
   return m_rgbaFrame;
}

const std::vector<uint8_t>& FlexDMD::GetDmdPixels()
{
   Render();
   UpdateLumFrame();
   return m_lumFrame;
}

void FlexDMD::SetSegments(const std::vector<uint16_t>& segments)
{
   if (memcmp(m_segData, segments.data(), 38 * sizeof(uint16_t)) != 0)
   {
      memcpy(m_segData, segments.data(), 38 * sizeof(uint16_t));
      m_frameId++;
   }
}

Group* FlexDMD::NewGroup(const string& name) { return new Group(this, name); }

Frame* FlexDMD::NewFrame(const string& name) { return new Frame(this, name); }

Label* FlexDMD::NewLabel(const string& Name, Font *Font_, const string& Text) { return new Label(this, Font_, Text,  Name); }

Image* FlexDMD::NewImage(const string& name, const string& image) { return Image::Create(this, m_pAssetManager, image, name); }

UltraDMD* FlexDMD::NewUltraDMD() { return new UltraDMD(this); }

Font* FlexDMD::NewFont(const string& font, uint32_t tint, uint32_t borderTint, int borderSize)
{
   const string tintHex = std::format("{:08X}", ((tint & 0x0000FFu) << 24) | ((tint & 0x00FF00u) << 8) | ((tint & 0xFF0000u) >> 8) | 0xFFu);
   const string borderHex = std::format("{:08X}", ((borderTint & 0x0000FFu) << 24) | ((borderTint & 0x00FF00u) << 8) | ((borderTint & 0xFF0000u) >> 8) | 0xFFu);
   AssetSrc* pAssetSrc = m_pAssetManager->ResolveSrc(font + "&tint=" + tintHex + "&border_size=" + std::to_string(borderSize) + "&border_tint=" + borderHex, nullptr);
   Font* pFont = m_pAssetManager->GetFont(pAssetSrc);
   pAssetSrc->Release();
   return pFont;
}

AnimatedActor* FlexDMD::NewVideo(const string& name, const string& video)
{
   if (video.find('|') != string::npos)
      return (AnimatedActor*)ImageSequence::Create(this, m_pAssetManager, video, name, 30, true);
   else {
      AssetSrc* pAssetSrc = m_pAssetManager->ResolveSrc(video, nullptr);
      AssetType assetType = pAssetSrc->GetAssetType();
      pAssetSrc->Release();

      if (assetType == AssetType_Video)
         return (AnimatedActor*)Video::Create(this, m_pAssetManager, video, name, true);
      else if (assetType == AssetType_GIF)
         return (AnimatedActor*)GIFImage::Create(this, m_pAssetManager, video, name);
      else if (assetType == AssetType_Image)
         return (AnimatedActor*)ImageSequence::Create(this, m_pAssetManager, video, name, 30, true);
   }
   return nullptr;
}

}
