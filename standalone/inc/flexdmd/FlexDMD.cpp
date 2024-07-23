#include "stdafx.h"

#include <algorithm>

#include "FlexDMD.h"
#include "UltraDMD.h"
#include "actors/Group.h"
#include "actors/Image.h"
#include "actors/Frame.h"
#include "actors/Label.h"
#include "actors/ImageSequence.h"
#include "actors/GIFImage.h"
#include "actors/Video.h"

#include "AssetManager.h"

#include "../common/WindowManager.h"

#define FLEXDMD_SETTINGS_WINDOW_X      15
#define FLEXDMD_SETTINGS_WINDOW_Y      30 + 218 + 5 + 75 + 5
#define FLEXDMD_SETTINGS_WINDOW_WIDTH  290
#define FLEXDMD_SETTINGS_WINDOW_HEIGHT 75
#define FLEXDMD_ZORDER                 350

FlexDMD::FlexDMD()
{
   m_width = 128;
   m_height = 32;
   m_frameRate = 60;
   m_runtimeVersion = 1008;
   m_clear = false;
   m_renderLockCount = 0;
   memset(m_segData1, 0, 128 * sizeof(UINT16));
   memset(m_segData2, 0, 128 * sizeof(UINT16));
   m_pGraphics = NULL;
   m_pStage = new Group(this, "Stage");
   m_pStage->AddRef();
   m_renderMode = RenderMode_DMD_GRAY_4;
   m_dmdColor = RGB(255, 88, 32);
   m_pAssetManager = new AssetManager();

   m_pDMDWindow = nullptr;

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindow"s, true)) {
      m_pDMDWindow = new VP::DMDWindow("FlexDMD",
         pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowX"s, FLEXDMD_SETTINGS_WINDOW_X),
         pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowY"s, FLEXDMD_SETTINGS_WINDOW_Y),
         pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowWidth"s, FLEXDMD_SETTINGS_WINDOW_WIDTH),
         pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowHeight"s, FLEXDMD_SETTINGS_WINDOW_HEIGHT),
         FLEXDMD_ZORDER,
         pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowRotation"s, 0));
   }
   else {
      PLOGI.printf("FlexDMD window disabled");
   }

   m_show = true;

   m_pDMD = nullptr;
   m_pRGB24DMD = nullptr;

   m_run = false;
   m_pThread = NULL;
}

FlexDMD::~FlexDMD()
{
   m_run = false;

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
   }

   if (m_pDMDWindow) {
      if (m_pDMDWindow->IsDMDAttached())
         m_pDMDWindow->DetachDMD();

      delete m_pDMDWindow;
   }

   m_pStage->Release();

   delete m_pAssetManager;
}

STDMETHODIMP FlexDMD::get_Version(LONG *pRetVal)
{
   *pRetVal = 9999;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_RuntimeVersion(LONG *pRetVal)
{
   *pRetVal = m_runtimeVersion;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_RuntimeVersion(LONG pRetVal)
{
   m_runtimeVersion = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Run(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_run;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Run(VARIANT_BOOL pRetVal)
{
   if (!m_run && pRetVal == VARIANT_FALSE)
      return S_OK;

   if (m_run && pRetVal == VARIANT_TRUE)
      return S_OK;

   m_run = (pRetVal == VARIANT_TRUE);

   if (m_run) {
      ShowDMD(m_show);

      RenderLoop();
   }
   else {
      PLOGI.printf("Stopping render thread");

      m_pThread->join();
      delete m_pThread;
      m_pThread = NULL;

      ShowDMD(false);
      m_pAssetManager->ClearAll();
   }

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Show(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_show ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Show(VARIANT_BOOL pRetVal)
{
   m_show = (pRetVal == VARIANT_TRUE);

   if (m_run)
      ShowDMD(m_show);

   return S_OK;
}

STDMETHODIMP FlexDMD::get_GameName(BSTR *pRetVal)
{
   CComBSTR Val(m_szGameName.c_str());
   *pRetVal = Val.Detach();

   return S_OK;
}

STDMETHODIMP FlexDMD::put_GameName(BSTR pRetVal)
{
   m_szGameName = MakeString(pRetVal);

   PLOGI.printf("Game name set to %s", m_szGameName.c_str());

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Width(unsigned short *pRetVal)
{
   *pRetVal = m_width;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Width(unsigned short pRetVal)
{
   m_width = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Height(unsigned short *pRetVal)
{
   *pRetVal = m_height;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Height(unsigned short pRetVal)
{
   m_height = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Color(OLE_COLOR *pRetVal)
{
   *pRetVal = m_dmdColor;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Color(OLE_COLOR pRetVal)
{
   m_dmdColor = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_RenderMode(RenderMode *pRetVal)
{
   *pRetVal = m_renderMode;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_RenderMode(RenderMode pRetVal)
{
   m_renderMode = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_ProjectFolder(BSTR *pRetVal)
{
   CComBSTR Val(m_pAssetManager->GetBasePath().c_str());
   *pRetVal = Val.Detach();

   return S_OK;
}

STDMETHODIMP FlexDMD::put_ProjectFolder(BSTR pRetVal)
{
   m_pAssetManager->SetBasePath(MakeString(pRetVal));

   return S_OK;
}

STDMETHODIMP FlexDMD::get_TableFile(BSTR *pRetVal)
{
   CComBSTR Val(m_szTableFile.c_str());
   *pRetVal = Val.Detach();

   return S_OK;
}

STDMETHODIMP FlexDMD::put_TableFile(BSTR pRetVal)
{
   m_szTableFile = MakeString(pRetVal);

   PLOGI.printf("Table file set to %s", m_szTableFile.c_str());

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Clear(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_clear ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Clear(VARIANT_BOOL pRetVal)
{
   m_clear = (pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP FlexDMD::get_DmdColoredPixels(VARIANT* pRetVal)
{
   if (!m_pDMD)
      return S_FALSE;

   if (!m_pRGB24DMD) {
      m_pRGB24DMD = m_pDMD->CreateRGB24DMD(m_width, m_height);
      return S_FALSE;
   }

   const UINT8* pRGB24Data = m_pRGB24DMD->GetData();

   if (!pRGB24Data)
      return S_FALSE;

   const int end = m_pRGB24DMD->GetLength();

   SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, 0, end);
   VARIANT* pData;

   SafeArrayAccessData(psa, (void **)&pData);

   for (int i = 0; i < end; i++) {
      int pos = i * 3;
      UINT8 r = pRGB24Data[pos];
      UINT8 g = pRGB24Data[pos + 1];
      UINT8 b = pRGB24Data[pos + 2];

      V_VT(&pData[i]) = VT_UI4;
      V_UI4(&pData[i]) = r | g << 8 | b << 16;
   }

   SafeArrayUnaccessData(psa);

   V_VT(pRetVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pRetVal) = psa;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_DmdPixels(VARIANT* pRetVal)
{
   if (!m_pDMD)
      return S_FALSE;

   if (!m_pRGB24DMD) {
      m_pRGB24DMD = m_pDMD->CreateRGB24DMD(m_width, m_height);
      return S_FALSE;
   }

   const UINT8* pRGB24Data = m_pRGB24DMD->GetData();

   if (!pRGB24Data)
      return S_FALSE;

   const int end = m_pRGB24DMD->GetLength();

   SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, 0, end);
   VARIANT* pData;

   SafeArrayAccessData(psa, (void **)&pData);

   for (int i = 0; i < end; i++) {
      int pos = i * 3;
      UINT8 r = pRGB24Data[pos];
      UINT8 g = pRGB24Data[pos + 1];
      UINT8 b = pRGB24Data[pos + 2];

      V_VT(&pData[i]) = VT_UI1;
      V_UI1(&pData[i]) = r | g << 8 | b << 16;
   }

   SafeArrayUnaccessData(psa);

   V_VT(pRetVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pRetVal) = psa;

   return S_OK;
}

STDMETHODIMP FlexDMD::putref_Segments(VARIANT rhs)
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

   SAFEARRAY* psa = V_ARRAY(&rhs);

   VARIANT* p;
   SafeArrayAccessData(psa, (void **)&p);

   for (int i = 0; i < size1; i++)
      m_segData1[i] = V_I2(&p[i]);

   for (int i = 0; i < size2; i++)
      m_segData2[i] = V_I2(&p[size1 + i]);

   SafeArrayUnaccessData(psa);

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Stage(IGroupActor **pRetVal)
{
   return m_pStage->QueryInterface(IID_IGroupActor, (void**)pRetVal);
}

STDMETHODIMP FlexDMD::LockRenderThread()
{
   m_renderLockCount++;

   return S_OK;
}

STDMETHODIMP FlexDMD::UnlockRenderThread()
{
   m_renderLockCount--;

   return S_OK;
}

STDMETHODIMP FlexDMD::NewGroup(BSTR Name, IGroupActor **pRetVal)
{
   Group* obj = new Group(this, MakeString(Name));
   return obj->QueryInterface(IID_IGroupActor, (void**)pRetVal);
}

STDMETHODIMP FlexDMD::NewFrame(BSTR Name, IFrameActor **pRetVal)
{
   Frame* obj = new Frame(this, MakeString(Name));
   return obj->QueryInterface(IID_IFrameActor, (void**)pRetVal); 
}

STDMETHODIMP FlexDMD::NewLabel(BSTR Name, IUnknown *Font_,BSTR Text, ILabelActor **pRetVal)
{
   Label* obj = new Label(this,  dynamic_cast<Font*>(Font_), MakeString(Text),  MakeString(Name));
   return obj->QueryInterface(IID_ILabelActor, (void**)pRetVal);
}

STDMETHODIMP FlexDMD::NewVideo(BSTR Name, BSTR video, IVideoActor **pRetVal)
{
   AnimatedActor* obj = NewVideo(MakeString(video), MakeString(Name));
   if (obj)
      return obj->QueryInterface(IID_IVideoActor, (void**)pRetVal);

   *pRetVal = NULL;

   return S_OK;
}

STDMETHODIMP FlexDMD::NewImage(BSTR Name, BSTR image, IImageActor **pRetVal)
{
   Image* obj = Image::Create(this, m_pAssetManager, MakeString(image), MakeString(Name));
   if (obj)
      return obj->QueryInterface(IID_IVideoActor, (void**)pRetVal);

   *pRetVal = NULL;

   return S_OK;
}

STDMETHODIMP FlexDMD::NewFont(BSTR font, OLE_COLOR tint, OLE_COLOR borderTint, LONG borderSize, IUnknown **pRetVal)
{
   return NewFont(MakeString(font), tint, borderTint, borderSize)->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMD::NewUltraDMD(IUltraDMD **pRetVal)
{
   UltraDMD* obj = new UltraDMD(this);
   return obj->QueryInterface(IID_IDMDObject, (void**)pRetVal);
}

void FlexDMD::ShowDMD(bool show)
{
   if (m_pDMDWindow) {
      if (show)
         m_pDMDWindow->Show();
      else
         m_pDMDWindow->Hide();
   }
}

void FlexDMD::RenderLoop()
{
   m_pDMD = new DMDUtil::DMD();

   if (m_pDMDWindow)
      m_pDMDWindow->AttachDMD(m_pDMD, m_width, m_height);

   if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "FindDisplays"s, true))
      m_pDMD->FindDisplays();

   PLOGI.printf("Starting render thread");

   m_pThread = new std::thread([this]() {
      SDL_Surface* pSurface = SDL_CreateRGBSurfaceWithFormat(0, m_width, m_height, 24, SDL_PIXELFORMAT_RGB24);
      m_pGraphics = new VP::SurfaceGraphics(pSurface);

      m_pStage->SetSize(m_width, m_height);
      m_pStage->SetOnStage(true);

      double elapsedMs = 0.0;

      while (m_run) {
         Uint64 startTime = SDL_GetTicks64();

         if (!m_renderLockCount) {
            if (m_clear) {
               m_pGraphics->SetColor(RGB(0, 0, 0));
               m_pGraphics->Clear();
            }

            m_pStage->Update((float)(elapsedMs / 1000.0));
            m_pStage->Draw(m_pGraphics);

            switch (m_renderMode) {
               case RenderMode_DMD_GRAY_2:
                  m_pDMD->UpdateRGB24Data((UINT8*)pSurface->pixels, 2, m_width, m_height, GetRValue(m_dmdColor), GetGValue(m_dmdColor), GetBValue(m_dmdColor));
                  break;
               case RenderMode_DMD_GRAY_4:
                  m_pDMD->UpdateRGB24Data((UINT8*)pSurface->pixels, 4, m_width, m_height, GetRValue(m_dmdColor), GetGValue(m_dmdColor), GetBValue(m_dmdColor));
                  break;
               case RenderMode_DMD_RGB:
                  m_pDMD->UpdateRGB24Data((UINT8*)pSurface->pixels, m_width, m_height);
                  break;
               case RenderMode_SEG_2x16Alpha:
               case RenderMode_SEG_2x20Alpha:
               case RenderMode_SEG_2x7Alpha_2x7Num:
               case RenderMode_SEG_2x7Alpha_2x7Num_4x1Num:
               case RenderMode_SEG_2x7Num_2x7Num_4x1Num:
               case RenderMode_SEG_2x7Num_2x7Num_10x1Num:
               case RenderMode_SEG_2x7Num_2x7Num_4x1Num_gen7:
               case RenderMode_SEG_2x7Num10_2x7Num10_4x1Num:
               case RenderMode_SEG_2x6Num_2x6Num_4x1Num:
               case RenderMode_SEG_2x6Num10_2x6Num10_4x1Num:
               case RenderMode_SEG_4x7Num10:
               case RenderMode_SEG_6x4Num_4x1Num:
               case RenderMode_SEG_2x7Num_4x1Num_1x16Alpha:
               case RenderMode_SEG_1x16Alpha_1x16Num_1x7Num:
                  m_pDMD->UpdateAlphaNumericData((DMDUtil::AlphaNumericLayout)(m_renderMode - 2), m_segData1, m_segData2, GetRValue(m_dmdColor), GetGValue(m_dmdColor), GetBValue(m_dmdColor));
                  break;
               default: break;
            }
         }

         double renderingDuration = SDL_GetTicks64() - startTime;

         int sleepMs = (1000 / m_frameRate) - (int)renderingDuration;

         if (sleepMs > 1)
            SDL_Delay(sleepMs);

         elapsedMs = SDL_GetTicks64() - startTime;

         if (elapsedMs > 4000 / m_frameRate) {
            PLOGI.printf("Abnormally long elapsed time between frames of %fs (rendering lasted %fms, sleeping was %dms), limiting to %dms", 
               elapsedMs / 1000.0, renderingDuration, std::max(0, sleepMs), 4000 / m_frameRate);

            elapsedMs = 4000 / m_frameRate;
         }
      }

      m_pStage->SetOnStage(false);

      SDL_FreeSurface(pSurface);

      delete m_pGraphics;
      m_pGraphics = nullptr;

      if (m_pDMDWindow) {
         if (m_pDMDWindow->IsDMDAttached())
            m_pDMDWindow->DetachDMD();
      }

      m_pRGB24DMD = nullptr;

      delete m_pDMD;
      m_pDMD = nullptr;

      PLOGI.printf("Render thread finished");
   });
}

Font* FlexDMD::NewFont(string font, OLE_COLOR tint, OLE_COLOR borderTint, LONG borderSize)
{
   AssetSrc* pAssetSrc = m_pAssetManager->ResolveSrc(
        font + "&tint=" + color_to_hex(tint) + "&border_size=" + std::to_string(borderSize) + "&border_tint=" + color_to_hex(borderTint), NULL);
   Font* pFont = m_pAssetManager->GetFont(pAssetSrc);
   delete pAssetSrc;

   return pFont;
}

AnimatedActor* FlexDMD::NewVideo(string szVideo, string szName)
{
   if (szVideo.find("|") != string::npos)
      return (AnimatedActor*)ImageSequence::Create(this, m_pAssetManager, szVideo, szName, 30, true);
   else {
      AssetSrc* pAssetSrc = m_pAssetManager->ResolveSrc(szVideo, NULL);
      AssetType assetType = pAssetSrc->GetAssetType();
      delete pAssetSrc;

      if (assetType == AssetType_Video)
         return (AnimatedActor*)Video::Create(this, szVideo, szName, true);
      else if (assetType == AssetType_GIF)
         return (AnimatedActor*)GIFImage::Create(this, m_pAssetManager, szVideo, szName);
      else if (assetType == AssetType_Image)
         return (AnimatedActor*)ImageSequence::Create(this, m_pAssetManager, szVideo, szName, 30, true);
   }

   return NULL;
}