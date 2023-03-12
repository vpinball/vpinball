#include "stdafx.h"
#include "captureExt.h"

// See for reference implementation: https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/DXGIDesktopDuplication/cpp/DuplicationManager.cpp)

#ifndef ENABLE_SDL

bool captureExternalDMD() { return false; }
bool capturePUP() { return false; }
void captureStartup() { }
void captureStop() { }

#else

#include <thread>

// The capture thread will do most of the capture work, it must:
// 1. Find DMD or PUP windows, if enabled.
// 2. Prepare DXGI capture interfaces
// 3. Signal that capture is ready, texture size is now available.
// 4. Wait for main thread to create texture.
// 5. Fill texture data periodically.

static ExtCapture ecDMD, ecPUP;
//std::map<void*, ExtCapture> ecDyn;
static bool StopCapture;
static std::thread threadCap;

static std::mutex mtx;
static std::condition_variable cv;

outputmaptype ExtCapture::m_duplicatormap;
capturelisttype ExtCapture::m_allCaptures;

// Call from VP's rendering loop.   Prepares textures once the sizes are detected by the 
// capture thread. 

void captureCheckTextures()
{
   if (ecDMD.m_ecStage == ecTexture)
   {
      if (g_pplayer->m_texdmd != nullptr)
      {
         g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.UnloadTexture(g_pplayer->m_texdmd);
         delete g_pplayer->m_texdmd;
      }
      g_pplayer->m_texdmd = ecDMD.m_texture;
      ecDMD.m_ecStage = ecCapturing;
   }
   if (ecPUP.m_ecStage == ecTexture)
   {
      if (g_pplayer->m_texPUP != nullptr)
      {
         g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.UnloadTexture(g_pplayer->m_texPUP);
         delete g_pplayer->m_texPUP;
      }
      g_pplayer->m_texPUP = ecPUP.m_texture;
      ecPUP.m_ecStage = ecCapturing;
   }
}

void captureThread()
{
   SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);

   while (!StopCapture)
   {
      for (auto it = ExtCapture::m_duplicatormap.begin(); it != ExtCapture::m_duplicatormap.end(); ++it)
      {
         it->second->AcquireFrame();
      }
      if (ecDMD.m_ecStage == ecSearching)
         ecDMD.SearchWindow();
      if (ecDMD.m_ecStage == ecCapturing)
      {
         if (ecDMD.GetFrame())
            g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.SetDirty(g_pplayer->m_texdmd);
      }

      if (ecPUP.m_ecStage == ecSearching)
      {
         ecPUP.SearchWindow();
      }
      if (ecPUP.m_ecStage == ecCapturing)
      {
         if (ecPUP.GetFrame())
            g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.SetDirty(g_pplayer->m_texPUP);
      }
      std::unique_lock<std::mutex> lck(mtx);

      cv.wait(lck);
      Sleep(16);
   }
}

void captureStartup()
{
   const vector<string> dmdlist = { "Virtual DMD", "pygame", "PUPSCREEN1", "formDMD", "PUPSCREEN5" };
   ecDMD.Setup(dmdlist);
   const vector<string> puplist = { "PUPSCREEN2", "Form1", "B2S Backglass Server", "B2S Background", "B2S DMD" }; // Form1 = old B2S
   ecPUP.Setup(puplist);
   ecDMD.m_ecStage = g_pplayer->m_capExtDMD ? ecSearching : ecFailure;
   ecPUP.m_ecStage = g_pplayer->m_capPUP ? ecSearching : ecFailure;
   StopCapture = false;
   std::thread t(captureThread);
   threadCap = move(t);
}

void captureStop()
{
   StopCapture = true;
   cv.notify_one();
   if (threadCap.joinable())
      threadCap.join();
   ExtCapture::Dispose();
   ecDMD.m_ecStage = ecPUP.m_ecStage = ecUninitialized;
}

bool capturePUP()
{
   if (ecPUP.m_ecStage == ecUninitialized)
   {
      captureStartup();
      return false;
   }
   if (ecPUP.m_ecStage == ecFailure)
      return false;
   captureCheckTextures();
   cv.notify_one();

   return (ecPUP.m_ecStage == ecCapturing);
}

bool captureExternalDMD()
{
   if (ecDMD.m_ecStage == ecUninitialized)
   {
      captureStartup();
      return false;
   }
   if (ecDMD.m_ecStage == ecFailure)
      return false;
   captureCheckTextures();
   cv.notify_one();

   return (ecDMD.m_ecStage == ecCapturing);
}

void ExtCapture::SearchWindow()
{
   HWND target = nullptr;

   for (const string& windowtext : m_searchWindows)
   {
      target = FindWindowA(nullptr, windowtext.c_str());
      if (target == nullptr)
         target = FindWindowA(windowtext.c_str(), nullptr);
      if (target != nullptr)
         break;
   }
   if (target != nullptr)
   {
      if (m_delay == 0)
         m_delay = clock() + 1 * CLOCKS_PER_SEC;

      if (clock() < m_delay)
         return;

      RECT r;
      GetWindowRect(target, &r);
      m_ecStage = SetupCapture(r) ? ecTexture : ecFailure;
   }
}

void ExtCapture::Setup(const vector<string>& windowlist)
{
   m_ecStage = ecUninitialized;
   m_delay = 0;
   m_searchWindows = windowlist;
}

bool ExtCapture::SetupCapture(const RECT& inputRect)
{
   m_Width = inputRect.right - inputRect.left;
   m_Height = inputRect.bottom - inputRect.top;

   if (m_Adapter)
   {
      m_Adapter->Release();
      m_Adapter = nullptr;
   }
   if (m_Output)
   {
      m_Output->Release();
      m_Output = nullptr;
   }

   /* Retrieve a IDXGIFactory that can enumerate the adapters. */
   IDXGIFactory1* factory = nullptr;
   HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory));

   if (FAILED(hr))
      return false;

   /* Enumerate the adapters.*/
   unsigned int i = 0, dx = 0;

   POINT pt;
   pt.x = inputRect.left;
   pt.y = inputRect.top;

   bool found = false;
   while (!found && DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &m_Adapter)) {
      ++i;
      if (m_Adapter)
      {
         dx = 0;
         while (!found && DXGI_ERROR_NOT_FOUND != m_Adapter->EnumOutputs(dx, &m_Output)) {
            ++dx;
            if (m_Output)
            {
               hr = m_Output->GetDesc(&m_outputdesc);
               if (hr == S_OK && PtInRect(&m_outputdesc.DesktopCoordinates, pt))
               {
                  found = true;
                  m_DispLeft = pt.x - m_outputdesc.DesktopCoordinates.left;
                  m_DispTop = pt.y - m_outputdesc.DesktopCoordinates.top;
               }
               else
               {
                  m_Output->Release();
                  m_Output = nullptr;
               }
            }
         }
         if (!found)
         {
            m_Adapter->Release();
            m_Adapter = nullptr;
         }
      }
   }
   if (!found)
      return false;

   const std::tuple<int, int> idx = std::make_tuple(dx, i);
   const outputmaptype::iterator it = m_duplicatormap.find(idx);
   if (it != m_duplicatormap.end())
   {
      m_pCapOut = it->second;
   }
   else
   {
      m_pCapOut = new ExtCaptureOutput(this);

      hr = D3D11CreateDevice(m_Adapter, /* Adapter: The adapter (video card) we want to use. We may use NULL to pick the default adapter. */
         D3D_DRIVER_TYPE_UNKNOWN,  /* DriverType: We use the GPU as backing device. */
         nullptr,                  /* Software: we're using a D3D_DRIVER_TYPE_HARDWARE so it's not applicaple. */
         NULL,                     /* Flags: maybe we need to use D3D11_CREATE_DEVICE_BGRA_SUPPORT because desktop duplication is using this. */
         nullptr,                  /* Feature Levels (ptr to array):  what version to use. */
         0,                        /* Number of feature levels. */
         D3D11_SDK_VERSION,        /* The SDK version, use D3D11_SDK_VERSION */
         &m_pCapOut->m_d3d_device, /* OUT: the ID3D11Device object. */
         &m_d3d_feature_level,     /* OUT: the selected feature level. */
         &m_pCapOut->m_d3d_context); /* OUT: the ID3D11DeviceContext that represents the above features. */

      if (S_OK != hr) {
         ShowError("Capture: Failed to create the D3D11 Device.");
         if (E_INVALIDARG == hr) {
            ShowError("Capture: E_INVALIDARG for D3D11CreateDevice."); // Did you pass an adapter + a driver which is not the UNKNOWN driver?
         }
         return false;
      }

      hr = m_Output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&m_Output1);
      if (S_OK != hr) {
         ShowError("Capture: Failed to query the IDXGIOutput1 interface.");
         return false;
      }

      hr = m_Output1->DuplicateOutput(m_pCapOut->m_d3d_device, &m_pCapOut->m_duplication);
      if (S_OK != hr) {
         ShowError("Capture: Failed to create the duplication output.");
         return false;
      }

      if (nullptr == m_pCapOut->m_duplication) {
         ShowError("Capture: Duplication var is nullptr.");
         return false;
      }

      /* Create the staging texture that we need to download the pixels from gpu. */
      D3D11_TEXTURE2D_DESC tex_desc;
      tex_desc.Width = m_outputdesc.DesktopCoordinates.right - m_outputdesc.DesktopCoordinates.left;
      tex_desc.Height = m_outputdesc.DesktopCoordinates.bottom - m_outputdesc.DesktopCoordinates.top;
      tex_desc.MipLevels = 1;
      tex_desc.ArraySize = 1; /* When using a texture array. */
      tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; /* This is the default data when using desktop duplication, see https://msdn.microsoft.com/en-us/library/windows/desktop/hh404611(v=vs.85).aspx */
      tex_desc.SampleDesc.Count = 1; /* MultiSampling, we can use 1 as we're just downloading an existing one. */
      tex_desc.SampleDesc.Quality = 0; /* "" */
      tex_desc.Usage = D3D11_USAGE_STAGING;
      tex_desc.BindFlags = 0;
      tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
      tex_desc.MiscFlags = 0;

      hr = m_pCapOut->m_d3d_device->CreateTexture2D(&tex_desc, nullptr, &m_pCapOut->m_staging_tex);
      if (E_INVALIDARG == hr) {
         ShowError("Capture: E_INVALIDARG while trying to create the texture.");
         return false;
      }
      else if (S_OK != hr) {
         ShowError("Capture: Failed to create the 2D texture, error: " + std::to_string(hr));
         return false;
      }
      m_duplicatormap[idx] = m_pCapOut;
   }
   m_allCaptures.push_back(this);

   m_texture = new BaseTexture(m_Width, m_Height, BaseTexture::Format::SRGBA);

   return true;
}


void ExtCaptureOutput::AcquireFrame()
{
   m_srcdata = nullptr;
   IDXGIResource* desktop_resource = nullptr;
   ID3D11Texture2D* tex = nullptr;

   DXGI_OUTDUPL_FRAME_INFO frame_info;
   HRESULT hr = m_duplication->AcquireNextFrame(2500, &frame_info, &desktop_resource);

   if (DXGI_ERROR_ACCESS_LOST == hr) {
      ShowError("Capture: Received DXGI_ERROR_ACCESS_LOST.");
   }
   else if (DXGI_ERROR_WAIT_TIMEOUT == hr) {
      // Silently discard timeouts
      // ShowError("Capture: Received DXGI_ERROR_WAIT_TIMEOUT.");
      return;
   }
   else if (DXGI_ERROR_INVALID_CALL == hr) {
      ShowError("Capture: Received DXGI_ERROR_INVALID_CALL.");
   }
   else if (S_OK == hr) {
      //printf("Yay we got a frame.\n");
      hr = desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
      if (S_OK != hr) {
         ShowError("Capture: Failed to query the ID3D11Texture2D interface on IDXGIResource.");
         exit(EXIT_FAILURE);
      }
      /* DXGI_MAPPED_RECT mapped_rect;
      hr = m_duplication->MapDesktopSurface(&mapped_rect);
      if (S_OK == hr) {
         //printf("We got access to the desktop surface\n");
         hr = m_duplication->UnMapDesktopSurface();
         if (S_OK != hr) {
            ShowError("Capture: Failed to unmap the desktop surface after successfully mapping it.");
         }
      }
      else if (DXGI_ERROR_UNSUPPORTED == hr) {*/
         m_d3d_context->CopyResource(m_staging_tex, tex);

         D3D11_MAPPED_SUBRESOURCE map;
         const HRESULT map_result = m_d3d_context->Map(m_staging_tex,          /* Resource */
            0,                    /* Subresource */
            D3D11_MAP_READ,       /* Map type. */
            0,                    /* Map flags. */
            &map);

         if (S_OK == map_result) {
            m_srcdata = (unsigned char*)map.pData;
            //printf("Mapped the staging tex; we can access the data now.\n");
            //printf("RowPitch: %u, DepthPitch: %u, %02X, %02X, %02X\n", map.RowPitch, map.DepthPitch, data[0], data[1], data[2]);
            m_pitch = map.RowPitch;

            const uint8_t* __restrict sptr = reinterpret_cast<uint8_t*>(m_srcdata) + m_pitch * m_cap->m_DispTop;
            uint8_t* __restrict ddptr = (uint8_t*)m_cap->m_texture->data();

            for (int h = 0; h < m_cap->m_Height; ++h)
            {
               // Copy acquired frame, swapping red and blue channel and removing alpha channel
               copy_bgra_rgba<true>((unsigned int*)ddptr, (const unsigned int*)sptr + m_cap->m_DispLeft, m_cap->m_Width);
               sptr += m_pitch;
               ddptr += m_cap->m_Width * 4;
            }

            // To check that the capture is ok
            //stbi_write_png("D:/PUPCapture.png"s.c_str(), m_cap->m_Width, m_cap->m_Height, 4, m_cap->m_pData, m_cap->m_Width * 4);
            //exit(0);
         }
         else {
            ShowError("Capture: Failed to map the staging tex. Cannot access the pixels.");
         }

         m_d3d_context->Unmap(m_staging_tex, 0);
      /*}
      else if (DXGI_ERROR_INVALID_CALL == hr) {
         ShowError("Capture: MapDesktopSurface returned DXGI_ERROR_INVALID_CALL.");
      }
      else if (DXGI_ERROR_ACCESS_LOST == hr) {
         ShowError("Capture: MapDesktopSurface returned DXGI_ERROR_ACCESS_LOST.");
      }
      else if (E_INVALIDARG == hr) {
         ShowError("Capture: MapDesktopSurface returned E_INVALIDARG.");
      }
      else {
         ShowError("Capture: MapDesktopSurface returned an unknown error.");
      }*/
   }
   unsigned int BufSize = frame_info.TotalMetadataBufferSize;
   if (m_MetaDataBufferSize < BufSize)
   {
      delete [] m_MetaDataBuffer;
      m_MetaDataBuffer = new char[BufSize];
      m_MetaDataBufferSize = BufSize;
   }

   // Get move rectangles
   hr = m_duplication->GetFrameMoveRects(BufSize, reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(m_MetaDataBuffer), &BufSize);
   if (SUCCEEDED(hr))
   {
      DXGI_OUTDUPL_MOVE_RECT* pmr = (DXGI_OUTDUPL_MOVE_RECT*)m_MetaDataBuffer;
      for (size_t i = 0;i < BufSize / sizeof(DXGI_OUTDUPL_MOVE_RECT);i++, pmr++)
      {
         for (auto it = ExtCapture::m_allCaptures.begin(); it != ExtCapture::m_allCaptures.end(); ++it)
         {
            const int capleft = (*it)->m_DispLeft;
            const int captop = (*it)->m_DispTop;
            const int capright = (*it)->m_DispLeft + (*it)->m_Width;
            const int capbottom = (*it)->m_DispTop + (*it)->m_Height;

            if (pmr->DestinationRect.left < capright && pmr->DestinationRect.right > capleft &&
               pmr->DestinationRect.top < capbottom && pmr->DestinationRect.bottom > captop)
               (*it)->m_bDirty = true;
         }
      }
   }

   BufSize = frame_info.TotalMetadataBufferSize;

   // Get dirty rectangles
   hr = m_duplication->GetFrameDirtyRects(BufSize, reinterpret_cast<RECT*>(m_MetaDataBuffer), &BufSize);
   if (SUCCEEDED(hr))
   {
      const RECT* r = (RECT*)m_MetaDataBuffer;
      for (size_t i = 0;i < BufSize / sizeof(RECT);++i, ++r)
         for (auto it = ExtCapture::m_allCaptures.begin(); it != ExtCapture::m_allCaptures.end(); ++it)
         {
            const int capleft = (*it)->m_DispLeft;
            const int captop = (*it)->m_DispTop;
            const int capright = (*it)->m_DispLeft + (*it)->m_Width;
            const int capbottom = (*it)->m_DispTop + (*it)->m_Height;

            if (r->left < capright && r->right > capleft &&
               r->top < capbottom && r->bottom > captop)
               (*it)->m_bDirty = true;
         }
   }
   /* Clean up */
   {
      if (nullptr != tex) {
         tex->Release();
         tex = nullptr;
      }

      if (nullptr != desktop_resource) {
         desktop_resource->Release();
         desktop_resource = nullptr;
      }

      /* We must release the frame. */
      hr = m_duplication->ReleaseFrame();
      if (S_OK != hr) {
         return;
         // std::cout << "FAILED TO RELEASE " << hr << std::endl;
      }
   }
}

bool ExtCapture::GetFrame()
{
   if (!m_bDirty)
      return false;

   m_bDirty = false;

   return true;
}

void ExtCapture::Dispose()
{
   for (auto it = m_duplicatormap.begin(); it != m_duplicatormap.end(); ++it)
   {
      it->second->m_duplication->Release();
      it->second->m_d3d_context->Release();
      it->second->m_d3d_device->Release();
      delete it->second;
   }
   m_duplicatormap.clear();
   m_allCaptures.clear();
}
#endif
