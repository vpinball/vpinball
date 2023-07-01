#include "stdafx.h"
#include "captureExt.h"
#include "comdef.h"

#if defined(ENABLE_SDL)
DXGIRegistry g_DXGIRegistry;
ExtCaptureManager g_ExtCaptureManager;

void StartDMDCapture()
{
   const vector<string> dmdlist = { "Virtual DMD", "pygame", "PUPSCREEN1", "formDMD", "PUPSCREEN5" };
   g_ExtCaptureManager.StartCapture("DMD"s, &g_pplayer->m_texdmd, dmdlist);
}
void StartPUPCapture()
{
   const vector<string> b2slist = { "PUPSCREEN2", "Form1" /* Old B2S */, "B2S Backglass Server", "B2S Background", "B2S DMD" };
   g_ExtCaptureManager.StartCapture("Backglass"s, &g_pplayer->m_texPUP, b2slist);
}
void UpdateExtCaptures() { g_ExtCaptureManager.Update(); }
void StopCaptures() { g_ExtCaptureManager.Stop(); }
bool HasDMDCapture() { return g_ExtCaptureManager.GetState("DMD"s) == ExtCaptureManager::CaptureState::CS_Capturing; }
bool HasPUPCapture() { return g_ExtCaptureManager.GetState("Backglass"s) == ExtCaptureManager::CaptureState::CS_Capturing; }


#include <thread>

ExtCaptureManager::CaptureState ExtCaptureManager::GetState(const string& name) const
{
   for (Capture* capture : m_captures)
      if (capture->m_name == name)
         return capture->m_state;
   return CS_Undeclared;
}

void ExtCaptureManager::StartCapture(const string& name, BaseTexture** targetTexture, const vector<string>& windowlist)
{
   m_captureMutex.lock();
   Capture* const capture = new Capture();
   m_captures.push_back(capture);
   capture->m_name = name;
   capture->m_targetTexture = targetTexture;
   capture->m_searchWindows = windowlist;
   capture->m_state = CS_Searching;
   if (!m_capturing)
   {
      m_capturing = true;
      m_captureThread = std::thread(&ExtCaptureManager::UpdateThread, this);
   }
   m_captureMutex.unlock();
}

void ExtCaptureManager::Stop()
{
   {
      const std::lock_guard<std::mutex> guard(m_captureMutex);
      m_capturing = false;
      m_updateCV.notify_one();
   }
   if (m_captureThread.joinable())
      m_captureThread.join();
   for (Duplication* duplication : m_duplications)
   {
      SAFE_RELEASE(duplication->m_duplication);
      SAFE_RELEASE(duplication->m_stagingTex);
      delete[] duplication->m_metaDataBuffer;
      delete duplication;
   }
   m_duplications.clear();
   for (Capture* capture : m_captures)
   {
      delete capture;
   }
   m_captures.clear();
}

void ExtCaptureManager::Update()
{
   //OutputDebugString("lock Upd\n");
   m_captureMutex.lock();
   for (Capture* capture : m_captures)
   {
      if (capture->m_state == CS_Texture)
      {
         if (*capture->m_targetTexture != nullptr)
         {
            g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.UnloadTexture(*capture->m_targetTexture);
            delete *capture->m_targetTexture;
            *capture->m_targetTexture = nullptr;
         }
         // Sleaze alert! - ec creates a HBitmap, but we hijack ec's data pointer to dump its data directly into VP's texture
         *capture->m_targetTexture = BaseTexture::CreateFromHBitmap(capture->m_hBitmap);
         capture->m_data = (*capture->m_targetTexture)->data();
         capture->m_state = CS_Capturing;
      }
      else if (capture->m_state == CS_Capturing && capture->m_updated)
      {
         capture->m_updated = false;
         g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.SetDirty(*capture->m_targetTexture);
      }
   }
   //OutputDebugString("Unlock Upd\n");
   m_captureMutex.unlock();

   //OutputDebugString("Signal Upd\n");
   m_updateCV.notify_one();
}

// (See for reference implementation: https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/DXGIDesktopDuplication/cpp/DuplicationManager.cpp)

void ExtCaptureManager::UpdateThread()
{
   while (m_capturing)
   {
      //OutputDebugString("Lock UpdThread\n");
      m_captureMutex.lock();
      // Perform the monitor captures
      for (Duplication* duplication : m_duplications)
      {
         duplication->m_srcData = nullptr;
         IDXGIResource* desktop_resource = nullptr;
         ID3D11Texture2D* tex = nullptr;
         DXGI_OUTDUPL_FRAME_INFO frame_info;
         HRESULT hr = duplication->m_duplication->AcquireNextFrame(2500, &frame_info, &desktop_resource);
         if (DXGI_ERROR_ACCESS_LOST == hr)
         {
            ShowError("Capture: Received DXGI_ERROR_ACCESS_LOST.");
         }
         else if (DXGI_ERROR_WAIT_TIMEOUT == hr)
         {
            ShowError("Capture: Received DXGI_ERROR_WAIT_TIMEOUT.");
         }
         else if (DXGI_ERROR_INVALID_CALL == hr)
         {
            ShowError("Capture: Received DXGI_ERROR_INVALID_CALL.");
         }
         else if (S_OK == hr)
         {
            //printf("Yay we got a frame.\n");
            hr = desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
            if (S_OK != hr)
            {
               ShowError("Capture: Failed to query the ID3D11Texture2D interface on IDXGIResource.");
               exit(EXIT_FAILURE);
            }
            DXGI_MAPPED_RECT mapped_rect;
            hr = duplication->m_duplication->MapDesktopSurface(&mapped_rect);
            if (S_OK == hr)
            {
               //printf("We got access to the desktop surface\n");
               hr = duplication->m_duplication->UnMapDesktopSurface();
               if (S_OK != hr)
               {
                  ShowError("Capture: Failed to unmap the desktop surface after successfully mapping it.");
               }
            }
            else if (DXGI_ERROR_UNSUPPORTED == hr)
            {
               duplication->m_device->m_D3DContext->CopyResource(duplication->m_stagingTex, tex);
               D3D11_MAPPED_SUBRESOURCE map;
               const HRESULT map_result = duplication->m_device->m_D3DContext->Map(duplication->m_stagingTex, 0, D3D11_MAP_READ, 0, &map);
               if (S_OK == map_result)
               {
                  //printf("Mapped the staging tex; we can access the data now.\n");
                  //printf("RowPitch: %u, DepthPitch: %u, %02X, %02X, %02X\n", map.RowPitch, map.DepthPitch, data[0], data[1], data[2]);
                  duplication->m_srcData = (unsigned char*)map.pData;
                  duplication->m_pitch = map.RowPitch;
               }
               else
               {
                  ShowError("Capture: Failed to map the staging tex. Cannot access the pixels.");
               }
               duplication->m_device->m_D3DContext->Unmap(duplication->m_stagingTex, 0);
            }
            else if (DXGI_ERROR_INVALID_CALL == hr)
            {
               ShowError("Capture: MapDesktopSurface returned DXGI_ERROR_INVALID_CALL.");
            }
            else if (DXGI_ERROR_ACCESS_LOST == hr)
            {
               ShowError("Capture: MapDesktopSurface returned DXGI_ERROR_ACCESS_LOST.");
            }
            else if (E_INVALIDARG == hr)
            {
               ShowError("Capture: MapDesktopSurface returned E_INVALIDARG.");
            }
            else
            {
               ShowError("Capture: MapDesktopSurface returned an unknown error.");
            }
         }

         // Get move & dirty rectangles and marked corresponding captures as dirty
         UINT BufSize = frame_info.TotalMetadataBufferSize;
         if (duplication->m_metaDataBufferSize < BufSize)
         {
            delete[] duplication->m_metaDataBuffer;
            duplication->m_metaDataBuffer = new char[BufSize];
            duplication->m_metaDataBufferSize = BufSize;
         }
         hr = duplication->m_duplication->GetFrameMoveRects(BufSize, reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(duplication->m_metaDataBuffer), &BufSize);
         if (SUCCEEDED(hr))
         {
            DXGI_OUTDUPL_MOVE_RECT* pmr = (DXGI_OUTDUPL_MOVE_RECT*)duplication->m_metaDataBuffer;
            for (size_t i = 0; i < BufSize / sizeof(DXGI_OUTDUPL_MOVE_RECT); i++, pmr++)
            {
               for (Capture* capture : m_captures)
               {
                  const int capleft = capture->m_dispLeft, captop = capture->m_dispTop;
                  const int capright = capture->m_dispLeft + capture->m_width;
                  const int capbottom = capture->m_dispTop + capture->m_height;
                  if (pmr->DestinationRect.left < capright && pmr->DestinationRect.right > capleft && pmr->DestinationRect.top < capbottom && pmr->DestinationRect.bottom > captop)
                     capture->m_dirty = true;
               }
            }
         }
         BufSize = frame_info.TotalMetadataBufferSize;
         hr = duplication->m_duplication->GetFrameDirtyRects(BufSize, reinterpret_cast<RECT*>(duplication->m_metaDataBuffer), &BufSize);
         if (SUCCEEDED(hr))
         {
            const RECT* r = (RECT*)duplication->m_metaDataBuffer;
            for (size_t i = 0; i < BufSize / sizeof(RECT); ++i, ++r)
               for (Capture* capture : m_captures)
               {
                  const int capleft = capture->m_dispLeft, captop = capture->m_dispTop;
                  const int capright = capture->m_dispLeft + capture->m_width;
                  const int capbottom = capture->m_dispTop + capture->m_height;
                  if (r->left < capright && r->right > capleft && r->top < capbottom && r->bottom > captop)
                     capture->m_dirty = true;
               }
         }

         // Clean up
         SAFE_RELEASE(tex);
         SAFE_RELEASE(desktop_resource);
         hr = duplication->m_duplication->ReleaseFrame(); // We must release the frame.
         if (S_OK != hr)
         {
            // std::cout << "FAILED TO RELEASE " << hr << std::endl;
         }
      }

      // Initialize (search for windows, create output duplication) or copy captured data to the capture texture
      for (Capture* capture : m_captures)
      {
         if (capture->m_state == CS_Searching)
         {
            // Find the window handle
            if (capture->m_window == nullptr)
            {
               for (const string& windowtext : capture->m_searchWindows)
               {
                  capture->m_window = FindWindowA(nullptr, windowtext.c_str());
                  if (capture->m_window == nullptr)
                     capture->m_window = FindWindowA(windowtext.c_str(), nullptr);
                  if (capture->m_window != nullptr)
                     break;
               }
               if (capture->m_window == nullptr)
                  continue;
               capture->m_delay = clock() + 1 * CLOCKS_PER_SEC;
            }

            // Wait one second after finding the window handle
            if (clock() < capture->m_delay)
               continue;

            // Find the DXGI output (per monitor) and associated DX11 device (per adapter) this window rely on.
            DXGIRegistry::Output* output = g_DXGIRegistry.GetForWindow(capture->m_window);
            if (output == nullptr)
            {
               capture->m_state = CS_Failure;
               PLOGE << "Failed to get DXGI output for '" << capture->m_name << "' capture";
               continue;
            }
            DXGIRegistry::Device* device = g_DXGIRegistry.GetDevice(output->m_Adapter);
            if (device == nullptr)
            {
               capture->m_state = CS_Failure;
               PLOGE << "Failed to get DX11 device for '" << capture->m_name << "' capture";
               continue;
            }

            // Create duplication object if we do not have one for this DXGI output (one duplication per DXGI output, that is to said, one duplication per monitor)
            for (Duplication* duplication : m_duplications)
               if (duplication->m_output == output)
                  capture->m_duplication = duplication;
            if (capture->m_duplication == nullptr)
            {
               capture->m_duplication = new Duplication();
               capture->m_duplication->m_output = output;
               capture->m_duplication->m_device = device;
               m_duplications.push_back(capture->m_duplication);
               HRESULT hr = output->m_Output1->DuplicateOutput(device->m_D3DDevice, &capture->m_duplication->m_duplication);
               if (S_OK != hr || capture->m_duplication->m_duplication == nullptr)
               {
                  _com_error err(hr);
                  PLOGE << "Failed to create DXGI Output duplication for '" << capture->m_name << "' capture, error: " << err.ErrorMessage();
                  ShowError("Capture: Failed to create the duplication output.");
                  capture->m_state = CS_Failure;
                  capture->m_duplication->m_failed = true;
                  continue;
               }
               D3D11_TEXTURE2D_DESC tex_desc;
               tex_desc.Width = output->m_OutputDesc.DesktopCoordinates.right - output->m_OutputDesc.DesktopCoordinates.left;
               tex_desc.Height = output->m_OutputDesc.DesktopCoordinates.bottom - output->m_OutputDesc.DesktopCoordinates.top;
               tex_desc.MipLevels = 1;
               tex_desc.ArraySize = 1; /* When using a texture array. */
               tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; /* This is the default data when using desktop duplication, see https://msdn.microsoft.com/en-us/library/windows/desktop/hh404611(v=vs.85).aspx */
               tex_desc.SampleDesc.Count = 1; /* MultiSampling, we can use 1 as we're just downloading an existing one. */
               tex_desc.SampleDesc.Quality = 0; /* "" */
               tex_desc.Usage = D3D11_USAGE_STAGING;
               tex_desc.BindFlags = 0;
               tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
               tex_desc.MiscFlags = 0;
               hr = device->m_D3DDevice->CreateTexture2D(&tex_desc, nullptr, &capture->m_duplication->m_stagingTex);
               if (S_OK != hr)
               {
                  _com_error err(hr);
                  PLOGE << "Failed to create duplication target for '" << capture->m_name << "' capture, error: " << err.ErrorMessage();
                  ShowError("Capture: Failed to create the 2D texture, error: " + std::to_string(hr));
                  capture->m_state = CS_Failure;
                  capture->m_duplication->m_failed = true;
                  continue;
               }
            }
            if (capture->m_duplication->m_failed)
            {
               PLOGE << "Duplication target for '" << capture->m_name << "' has failed.";
               capture->m_state = CS_Failure;
               continue;
            }

            // Gather information for this specific window capture and create a DIB for the data
            RECT inputRect;
            GetWindowRect(capture->m_window, &inputRect);
            capture->m_dispLeft = inputRect.left - output->m_OutputDesc.DesktopCoordinates.left;
            capture->m_dispTop = inputRect.top - output->m_OutputDesc.DesktopCoordinates.top;
            capture->m_width = inputRect.right - inputRect.left;
            capture->m_height = inputRect.bottom - inputRect.top;

            const HDC all_screen = GetDC(nullptr);
            const int BitsPerPixel = GetDeviceCaps(all_screen, BITSPIXEL);
            const HDC hdc2 = CreateCompatibleDC(all_screen);
            BITMAPINFO info;
            info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            info.bmiHeader.biWidth = capture->m_width;
            info.bmiHeader.biHeight = -capture->m_height;
            info.bmiHeader.biPlanes = 1;
            info.bmiHeader.biBitCount = (WORD)BitsPerPixel;
            info.bmiHeader.biCompression = BI_RGB;
            capture->m_hBitmap = CreateDIBSection(hdc2, &info, DIB_RGB_COLORS, (void**)&capture->m_data, 0, 0);

            capture->m_state = CS_Texture;
         }
         else if (capture->m_state == CS_Capturing && capture->m_dirty)
         {
            capture->m_dirty = false;
            capture->m_updated = true;
            const uint8_t* __restrict sptr = reinterpret_cast<uint8_t*>(capture->m_duplication->m_srcData) + capture->m_duplication->m_pitch * capture->m_dispTop;
            uint8_t* __restrict ddptr = (uint8_t*)capture->m_data;
            for (int h = 0; h < capture->m_height; ++h)
            {
               // Copy acquired frame, swapping red and blue channel
               copy_bgra_rgba<false>((unsigned int*)ddptr, (const unsigned int*)sptr + capture->m_dispLeft, capture->m_width);
               sptr += capture->m_duplication->m_pitch;
               ddptr += capture->m_width * 4;
            }
         }
      }
      //OutputDebugString("Unlock UpdThread\n");
      m_captureMutex.unlock();

      // Wait for main thread to request the next update
      {
         //OutputDebugString("Wait UpdThread\n");
         std::unique_lock updateLock(m_updateMutex);
         m_updateCV.wait(updateLock);
         //OutputDebugString("Signaled UpdThread\n");
      }
   }
}

#else
void StartDMDCapture() { }
void StartPUPCapture() { }
void UpdateExtCaptures() { }
void StopCaptures() { }
bool HasDMDCapture() { return false; }
bool HasPUPCapture() { return false; }

#endif
