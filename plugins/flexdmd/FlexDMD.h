#pragma once

#include "common.h"
#include "SurfaceGraphics.h"
#include "plugins/VPXPlugin.h"
#include "resources/AssetManager.h"
#include "actors/Group.h"

namespace Flex {

class Frame;
class Image;
class Label;
class UltraDMD;
class AnimatedActor;

typedef enum
{
   RenderMode_DMD_GRAY_2 = 0,
   RenderMode_DMD_GRAY_4 = 1,
   RenderMode_DMD_RGB = 2,
   RenderMode_SEG_2x16Alpha = 3,
   RenderMode_SEG_2x20Alpha = 4,
   RenderMode_SEG_2x7Alpha_2x7Num = 5,
   RenderMode_SEG_2x7Alpha_2x7Num_4x1Num = 6,
   RenderMode_SEG_2x7Num_2x7Num_4x1Num = 7,
   RenderMode_SEG_2x7Num_2x7Num_10x1Num = 8,
   RenderMode_SEG_2x7Num_2x7Num_4x1Num_gen7 = 9,
   RenderMode_SEG_2x7Num10_2x7Num10_4x1Num = 10,
   RenderMode_SEG_2x6Num_2x6Num_4x1Num = 11,
   RenderMode_SEG_2x6Num10_2x6Num10_4x1Num = 12,
   RenderMode_SEG_4x7Num10 = 13,
   RenderMode_SEG_6x4Num_4x1Num = 14,
   RenderMode_SEG_2x7Num_4x1Num_1x16Alpha = 15,
   RenderMode_SEG_1x16Alpha_1x16Num_1x7Num = 16
} RenderMode;

class FlexDMD final
{
public:
   FlexDMD(const MsgPluginAPI* msgApi, unsigned int endpointId, VPXPluginAPI* vpxApi);
   ~FlexDMD();

   PSC_IMPLEMENT_REFCOUNT()

   static int32_t GetVersion() { return 1009; }
   int32_t GetRuntimeVersion() const { return m_runtimeVersion; }
   void SetRuntimeVersion(int32_t v) { m_runtimeVersion = v; }

   bool GetRun() const { return m_run; }
   void SetRun(bool run);

   bool GetShow() const { return m_show; }
   void SetShow(bool v);

   const string& GetGameName() const { return m_szGameName; }
   void SetGameName(const string& name) { m_szGameName = name; }

   const string& GetTableFile() const { return m_pAssetManager->GetTableFile(); }
   void SetTableFile(const string& name) { m_pAssetManager->SetTableFile(name); }

   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   void SetWidth(int w);
   void SetHeight(int h);

   RenderMode GetRenderMode() const { return m_renderMode; }
   void SetRenderMode(RenderMode renderMode);

   const string& GetProjectFolder() const { return m_pAssetManager->GetBasePath(); }
   void SetProjectFolder(const string& folder) { m_pAssetManager->SetBasePath(folder); }

   bool GetClear() const { return m_clear; }
   void SetClear(bool v) { m_clear = v; }

   void Render();
   const std::vector<uint32_t>& GetDmdColoredPixels();
   const std::vector<uint8_t>& GetDmdPixels();

   void SetSegments(const std::vector<uint16_t>& segments);

   void LockRenderThread() { m_renderLockCount++; }
   void UnlockRenderThread() { m_renderLockCount--; }

   Group* GetStage() const { m_pStage->AddRef(); return m_pStage; }

   Group* NewGroup(const string& name);
   Frame* NewFrame(const string& name);
   Image* NewImage(const string& name, const string& image);
   Label* NewLabel(const string& name, Font* Font, const string& text);
   UltraDMD* NewUltraDMD();
   Font* NewFont(const string& szFont, uint32_t tint, uint32_t borderTint, int borderSize);
   AnimatedActor* NewVideo(const string& name, const string& video);

   uint32_t GetColor() const { return m_dmdColor; }
   void SetColor(uint32_t dmdColor) { m_dmdColor = dmdColor; }

   AssetManager* GetAssetManager() const { return m_pAssetManager; }

   void SetId(uint32_t id) { m_id = id; }
   uint32_t GetId() const { return m_id; }

   void SetOnDestroyHandler(const std::function<void(FlexDMD*)>& handler) { m_onDestroyHandler = handler; }

   SurfaceGraphics* GetGraphics() const { return m_pSurface; }

private:
   std::function<void(FlexDMD*)> m_onDestroyHandler;
   PinballPlugin::Controller::CtrlItemProvider<DisplaySrcId> m_dmdProvider;
   PinballPlugin::Controller::CtrlItemProvider<SegSrcId> m_segProvider;

   void AdvertiseDisplay();
   struct CallContext
   {
      FlexDMD* me;
      unsigned int index;
   };
   std::vector<CallContext> m_callContexts;
   static SegDisplayFrame GetSegState(void* callContext);
   static DisplayFrame GetRenderFrame(void* callContext);
   uint8_t* UpdateRGBFrame();
   uint8_t* UpdateLum8Frame();
   float* UpdateLumFP32Frame();

   void DiscardFrames()
   {
      assert(m_dmdProvider.GetItems().empty());
      delete m_pSurface; m_pSurface = nullptr;
      delete[] m_rgbFrame; m_rgbFrame = nullptr;
      delete[] m_lum8Frame; m_lum8Frame = nullptr;
      delete[] m_lumFP32Frame; m_lumFP32Frame = nullptr;
      m_rgbaFrame.clear();
      m_lumFrame.clear();
   }

   VPXPluginAPI* m_vpxApi = nullptr;
   const unsigned int m_endpointId;

   uint8_t* m_rgbFrame = nullptr;
   bool m_rgbFrameDirty = true;

   void UpdateRGBAFrame();
   std::vector<uint32_t> m_rgbaFrame;
   bool m_rgbaFrameDirty = true;

   uint8_t* m_lum8Frame = nullptr;
   bool m_lum8FrameDirty = true;

   float* m_lumFP32Frame = nullptr;
   bool m_lumFP32FrameDirty = true;

   void UpdateLumFrame();
   std::vector<uint8_t> m_lumFrame;
   bool m_lumFrameDirty = true;

   string m_szGameName;
   uint64_t m_lastRenderTick = 0;
   unsigned int m_frameId = 0;
   int32_t m_runtimeVersion = 1008;
   bool m_clear = false;
   int m_renderLockCount = 0;
   uint16_t m_segData[128] = {};
   int m_width = 128;
   int m_height = 32;
   Group* m_pStage;
   RenderMode m_renderMode = RenderMode_DMD_GRAY_4;
   uint32_t m_dmdColor = RGB(0xFF, 0x58, 0x20);
   AssetManager* m_pAssetManager;
   bool m_show = true;
   bool m_run = false;
   uint32_t m_id = 0;
   SurfaceGraphics* m_pSurface = nullptr;

   //std::thread* m_pThread;
   //void RenderLoop();
};

}