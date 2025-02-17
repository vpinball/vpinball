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

#include <sstream>
#include <iomanip>

#include <SDL3/SDL_timer.h>

FlexDMD::FlexDMD()
{
   m_pStage = new Group(this, "Stage"s);
   m_pStage->SetSize(static_cast<float>(m_width), static_cast<float>(m_height));
   m_pAssetManager = new AssetManager();
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
   if (m_show)
      OnDMDChanged();
}

void FlexDMD::Render()
{
   // TODO we could do it on a separate thread and afford a latency of 1 frame to remove all overhead
   Uint64 tick = SDL_GetTicks();
   Uint64 elapsedMs = tick - m_lastRenderTick;
   if ((m_renderLockCount == 0) && elapsedMs > 2)
   {
      m_frameId++;
      m_lum8FrameDirty = m_lumFrameDirty = m_rgbFrameDirty = m_rgbaFrameDirty = true;
      m_lastRenderTick = tick;
      m_pStage->Update((float)(elapsedMs / 1000.0));
      if (m_pSurface == nullptr)
      {
         SDL_Surface* pSurface = SDL_CreateSurface(m_width, m_height, SDL_PIXELFORMAT_RGB24);
         m_pSurface = new VP::SurfaceGraphics(pSurface);
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
   if (m_pSurface == nullptr)
      return m_lum8Frame;
   if (m_renderLockCount > 0)
      return m_lum8Frame;
   m_lum8FrameDirty = false;
   SDL_Surface* surf = m_pSurface->GetSurface();
   SDL_LockSurface(surf);
   const uint8_t* __restrict pixels = static_cast<uint8_t*>(surf->pixels);
   uint8_t* __restrict dst = m_lum8Frame;
   for (int o = 0; o < m_height*m_width; o++)
      {
         float r = static_cast<float>(*pixels++);
         float g = static_cast<float>(*pixels++);
         float b = static_cast<float>(*pixels++);
         *dst++ = static_cast<uint8_t>(0.2126f * r + 0.7152f * g + 0.0722f * b);
      }
   SDL_UnlockSurface(surf);
   return m_lum8Frame;
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
   for (int o = 0; o < m_height*m_width; o++)
         if (m_renderMode == RenderMode_DMD_GRAY_2)
            *dst++ = lum4[(*src++) >> 6];
         else if (m_renderMode == RenderMode_DMD_GRAY_4)
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
   const uint8_t* __restrict pixels = static_cast<uint8_t*>(surf->pixels);
   uint32_t* __restrict dst = m_rgbaFrame.data();
   for (int o = 0; o < m_height*m_width; o++)
      {
         uint8_t r = *pixels++;
         uint8_t g = *pixels++;
         uint8_t b = *pixels++;
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
   int size1 = 0;
   int size2 = 0;

   switch (m_renderMode) {
      case RenderMode_SEG_2x16Alpha:
         size1 = 2 * 16;
         break;
      case RenderMode_SEG_2x20Alpha:
         size1 = 2 * 20;
         break;
      case RenderMode_SEG_2x7Alpha_2x7Num:
         size1 = 2 * 7 + 2 * 7;
         break;
      case RenderMode_SEG_2x7Alpha_2x7Num_4x1Num:
         size1 = 2 * 7 + 2 * 7 + 4;
         break;
      case RenderMode_SEG_2x7Num_2x7Num_4x1Num:
         size1 = 2 * 7 + 2 * 7 + 4;
         break;
      case RenderMode_SEG_2x7Num_2x7Num_10x1Num:
         size1 = 2 * 7 + 2 * 7 + 4;
         size2 = 6;
         break;
      case RenderMode_SEG_2x7Num_2x7Num_4x1Num_gen7:
         size1 = 2 * 7 + 2 * 7 + 4;
         break;
      case RenderMode_SEG_2x7Num10_2x7Num10_4x1Num:
         size1 = 2 * 7 + 2 * 7 + 4;
         break;
      case RenderMode_SEG_2x6Num_2x6Num_4x1Num:
         size1 = 2 * 6 + 2 * 6 + 4;
         break;
      case RenderMode_SEG_2x6Num10_2x6Num10_4x1Num:
         size1 = 2 * 6 + 2 * 6 + 4;
         break;
      case RenderMode_SEG_4x7Num10:
         size1 = 4 * 7;
         break;
      case RenderMode_SEG_6x4Num_4x1Num:
         size1 = 6 * 4 + 4;
         break;
      case RenderMode_SEG_2x7Num_4x1Num_1x16Alpha:
         size1 = 2 * 7 + 4 + 1;
         break;
      case RenderMode_SEG_1x16Alpha_1x16Num_1x7Num:
         size1 = 1 + 1 + 7;
         break;
      default:
         break;
   }
   for (int i = 0; i < size1; i++)
      m_segData1[i] = segments[i];
   for (int i = 0; i < size2; i++)
      m_segData2[i] = segments[size1 + i];
}

Group* FlexDMD::NewGroup(const string& name) { return new Group(this, name); }

Frame* FlexDMD::NewFrame(const string& name) { return new Frame(this, name); }

Label* FlexDMD::NewLabel(const string& Name, Font *Font_, const string& Text) { return new Label(this, Font_, Text,  Name); }

Image* FlexDMD::NewImage(const string& name, const string& image) { return Image::Create(this, m_pAssetManager, image, name); }

UltraDMD* FlexDMD::NewUltraDMD() { return new UltraDMD(this); }

Font* FlexDMD::NewFont(const string& font, uint32_t tint, uint32_t borderTint, int borderSize)
{
   std::stringstream tintHex, borderHex;
   tintHex << std::setfill('0') << std::setw(8) << std::hex << (((tint & 0x0000FF) << 24) | ((tint & 0x00FF00) << 8) | ((tint & 0xFF0000) >> 8) | 0xFF);
   borderHex << std::setfill('0') << std::setw(8) << std::hex << (((borderTint & 0x0000FF) << 24) | ((borderTint & 0x00FF00) << 8) | ((borderTint & 0xFF0000) >> 8) | 0xFF);
   AssetSrc* pAssetSrc = m_pAssetManager->ResolveSrc(font + "&tint=" + tintHex.str() + "&border_size=" + std::to_string(borderSize) + "&border_tint=" + borderHex.str(), nullptr);
   Font* pFont = m_pAssetManager->GetFont(pAssetSrc);
   delete pAssetSrc;
   return pFont;
}

AnimatedActor* FlexDMD::NewVideo(const string& szVideo, const string& szName)
{
   if (szVideo.find('|') != string::npos)
      return (AnimatedActor*)ImageSequence::Create(this, m_pAssetManager, szVideo, szName, 30, true);
   else {
      AssetSrc* pAssetSrc = m_pAssetManager->ResolveSrc(szVideo, nullptr);
      AssetType assetType = pAssetSrc->GetAssetType();
      delete pAssetSrc;

      if (assetType == AssetType_Video)
         return (AnimatedActor*)Video::Create(this, szVideo, szName, true);
      else if (assetType == AssetType_GIF)
         return (AnimatedActor*)GIFImage::Create(this, m_pAssetManager, szVideo, szName);
      else if (assetType == AssetType_Image)
         return (AnimatedActor*)ImageSequence::Create(this, m_pAssetManager, szVideo, szName, 30, true);
   }
   return nullptr;
}
