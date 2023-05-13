#include "stdafx.h"
#include "captureExt.h"

// 2 implementations are provided:
// - one basic based on default Windows commands (this is mostly a copy/paste for flasher's video capture code)
// - one based on DXGI capture which should have better performance.
// (See for reference implementation: https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/DXGIDesktopDuplication/cpp/DuplicationManager.cpp)
// The initial implementation of the DXGI (https://github.com/vpinball/vpvr/blob/master/captureExt.cpp) was known to work well. Somewhere in the evolution
// path, bugs were introduced and it is not stable as it used to be. Due to lack of time, this lead to implement and defaults to the the leff efficient
// default Windows implementation.
// 2023.05.13: copy/pasted the DXGI initial implementation back and reenabled it for user testing
#define USE_DXGI 1

#if defined(ENABLE_SDL) && !USE_DXGI
class Capture
{
public:
   Capture(BaseTexture** texture, const vector<string>& windows)
      : m_videoCapTex(texture), m_searchWindows(windows)
   {
      ResetVideoCap();
   }
   ~Capture() { ResetVideoCap(); }

   void ResetVideoCap()
   {
      m_isVideoCap = false;
      if (*m_videoCapTex)
      {
         g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.UnloadTexture(*m_videoCapTex);
         delete *m_videoCapTex;
         *m_videoCapTex = nullptr;
      }
   }

   bool Update()
   {
      if (m_isVideoCap == false)
      { // VideoCap has not started because no sourcewin found
         for (const string& windowtext : m_searchWindows)
         {
            m_videoCapHwnd = FindWindowA(nullptr, windowtext.c_str());
            if (m_videoCapHwnd == nullptr)
               m_videoCapHwnd = FindWindowA(windowtext.c_str(), nullptr);
            if (m_videoCapHwnd != nullptr)
               break;
         }
         if (m_videoCapHwnd == nullptr)
            return false;

         //source videocap found.  lets start!
         GetClientRect(m_videoCapHwnd, &m_videoSourceRect); // Get rect in physical units (pixels)
         m_videoCapWidth = m_videoSourceRect.right - m_videoSourceRect.left;
         m_videoCapHeight = m_videoSourceRect.bottom - m_videoSourceRect.top;
         ResetVideoCap();
         *m_videoCapTex = new BaseTexture(m_videoCapWidth, m_videoCapHeight, BaseTexture::SRGBA);
      }

      // Retrieve the handle to a display device context for the client
      // area of the window.

      const HDC hdcWindow = GetDC(m_videoCapHwnd);

      // Create a compatible DC, which is used in a BitBlt from the window DC.
      const HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

      // Get the client area for size calculation.
      const int pWidth = m_videoCapWidth;
      const int pHeight = m_videoCapHeight;

      // Create a compatible bitmap from the Window DC.
      const HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, pWidth, pHeight);

      // Select the compatible bitmap into the compatible memory DC.
      SelectObject(hdcMemDC, hbmScreen);
      SetStretchBltMode(hdcMemDC, HALFTONE);
      // Bit block transfer into our compatible memory DC.
      m_isVideoCap = StretchBlt(hdcMemDC, 0, 0, pWidth, pHeight, hdcWindow, 0, 0, m_videoSourceRect.right - m_videoSourceRect.left, m_videoSourceRect.bottom - m_videoSourceRect.top, SRCCOPY);
      if (m_isVideoCap)
      {
         // Get the BITMAP from the HBITMAP.
         BITMAP bmpScreen;
         GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

         BITMAPINFOHEADER bi;
         bi.biSize = sizeof(BITMAPINFOHEADER);
         bi.biWidth = bmpScreen.bmWidth;
         bi.biHeight = -bmpScreen.bmHeight;
         bi.biPlanes = 1;
         bi.biBitCount = 32;
         bi.biCompression = BI_RGB;
         bi.biSizeImage = 0;
         bi.biXPelsPerMeter = 0;
         bi.biYPelsPerMeter = 0;
         bi.biClrUsed = 0;
         bi.biClrImportant = 0;

         const DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

         const HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
         char* lpbitmap = (char*)GlobalLock(hDIB);

         // Gets the "bits" from the bitmap, and copies them into a buffer
         // that's pointed to by lpbitmap.
         GetDIBits(hdcWindow, hbmScreen, 0, (UINT)bmpScreen.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

         // copy bitmap pixels to texture, reversing BGR to RGB and adding an opaque alpha channel
         copy_bgra_rgba<true>((unsigned int*)((*m_videoCapTex)->data()), (const unsigned int*)lpbitmap, pWidth * pHeight);

         GlobalUnlock(hDIB);
         GlobalFree(hDIB);

         g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.SetDirty(*m_videoCapTex);
      }

      ReleaseDC(m_videoCapHwnd, hdcWindow);
      DeleteObject(hbmScreen);
      DeleteObject(hdcMemDC);

      return m_isVideoCap;
   }

private:
   const vector<string> m_searchWindows;
   bool m_isVideoCap = false;
   int m_videoCapWidth = 0;
   int m_videoCapHeight = 0;
   RECT m_videoSourceRect;
   HWND m_videoCapHwnd = nullptr;
   BaseTexture** m_videoCapTex = nullptr;
};

static Capture* dmdCapture = nullptr;
static Capture* pupCapture = nullptr;

void captureStop()
{
   delete dmdCapture;
   delete pupCapture;
   dmdCapture = nullptr;
   pupCapture = nullptr;
}

bool captureExternalDMD()
{
   if (dmdCapture == nullptr)
   {
      const vector<string> dmdlist = { "Virtual DMD", "pygame", "PUPSCREEN1", "formDMD", "PUPSCREEN5" };
      dmdCapture = new Capture(&g_pplayer->m_texdmd, dmdlist);
   }
   return dmdCapture->Update();
}

bool capturePUP()
{
   if (pupCapture == nullptr)
   {
      const vector<string> b2slist = { "PUPSCREEN2", "Form1", "B2S Backglass Server", "B2S Background", "B2S DMD" }; // Form1 = old B2S
      pupCapture = new Capture(&g_pplayer->m_texPUP, b2slist);
   }
   return pupCapture->Update();
}

#elif defined(ENABLE_SDL) && USE_DXGI

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
      // Sleaze alert! - ec creates a HBitmap, but we hijack ec's data pointer to dump its data directly into VP's texture
      g_pplayer->m_texdmd = BaseTexture::CreateFromHBitmap(ecDMD.m_HBitmap);
      ecDMD.m_pData = g_pplayer->m_texdmd->data();
      ecDMD.m_ecStage = ecCapturing;
   }
   if (ecPUP.m_ecStage == ecTexture)
   {
      if (g_pplayer->m_texPUP != nullptr)
      {
         g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.UnloadTexture(g_pplayer->m_texPUP);
         delete g_pplayer->m_texPUP;
      }
      g_pplayer->m_texPUP = BaseTexture::CreateFromHBitmap(ecPUP.m_HBitmap);
      ecPUP.m_pData = g_pplayer->m_texPUP->data();
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
   m_pData = nullptr;
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
   UINT i = 0, dx = 0;

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

   // duplication->GetDesc(&m_duplication_desc);

   const HDC all_screen = GetDC(nullptr);
   const int BitsPerPixel = GetDeviceCaps(all_screen, BITSPIXEL);
   const HDC hdc2 = CreateCompatibleDC(all_screen);

   BITMAPINFO info;
   info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   info.bmiHeader.biWidth = m_Width;
   info.bmiHeader.biHeight = -m_Height;
   info.bmiHeader.biPlanes = 1;
   info.bmiHeader.biBitCount = (WORD)BitsPerPixel;
   info.bmiHeader.biCompression = BI_RGB;

   m_HBitmap = CreateDIBSection(hdc2, &info, DIB_RGB_COLORS, (void**)&m_pData, 0, 0);
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
      ShowError("Capture: Received DXGI_ERROR_WAIT_TIMEOUT.");
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
      DXGI_MAPPED_RECT mapped_rect;
      hr = m_duplication->MapDesktopSurface(&mapped_rect);
      if (S_OK == hr) {
         //printf("We got access to the desktop surface\n");
         hr = m_duplication->UnMapDesktopSurface();
         if (S_OK != hr) {
            ShowError("Capture: Failed to unmap the desktop surface after successfully mapping it.");
         }
      }
      else if (DXGI_ERROR_UNSUPPORTED == hr) {
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

         }
         else {
            ShowError("Capture: Failed to map the staging tex. Cannot access the pixels.");
         }

         m_d3d_context->Unmap(m_staging_tex, 0);
      }
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
      }
   }
   UINT BufSize = frame_info.TotalMetadataBufferSize;
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

   const uint8_t* __restrict sptr = reinterpret_cast<uint8_t*>(m_pCapOut->m_srcdata) + m_pCapOut->m_pitch * m_DispTop;
   uint8_t* __restrict ddptr = (uint8_t*)m_pData;

   for (int h = 0; h < m_Height; ++h)
   {
      // Copy acquired frame, swapping red and blue channel
      copy_bgra_rgba<false>((unsigned int*)ddptr, (const unsigned int*)sptr + m_DispLeft, m_Width);
      sptr += m_pCapOut->m_pitch;
      ddptr += m_Width * 4;
      Sleep(0); //!! ??
   }

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

#else
bool captureExternalDMD() { return false; }
bool capturePUP() { return false; }
void captureStop() { }

#endif
