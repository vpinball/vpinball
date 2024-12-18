// license:GPLv3+

#include "core/stdafx.h"
#include "captureExt.h"
#include "comdef.h"

#if defined(EXT_CAPTURE)
DXGIRegistry g_DXGIRegistry;
ExtCaptureManager g_ExtCaptureManager;

void StartDMDCapture()
{
   static const vector<string> dmdlist = { "Virtual DMD"s, "pygame"s, "PUPSCREEN1"s, "formDMD"s, "PUPSCREEN5"s };
   g_ExtCaptureManager.StartCapture("DMD"s, &g_pplayer->m_dmdFrame, dmdlist);
}
void StartPUPCapture()
{
   static const vector<string> b2slist = { "PUPSCREEN2"s, "Form1"s /* Old B2S */, "B2S Backglass Server"s, "B2S Background"s, "B2S DMD"s };
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
   for (Capture* capture : m_captures)
   {
      if (capture->m_state == CS_Texture)
      {
         const std::lock_guard<std::mutex> guard(m_captureMutex);
         if (*capture->m_targetTexture != nullptr)
         {
            g_pplayer->m_renderer->m_renderDevice->m_texMan.UnloadTexture(*capture->m_targetTexture);
            delete *capture->m_targetTexture;
            *capture->m_targetTexture = nullptr;
         }
         // Sleaze alert! - ec creates a HBitmap, but we hijack ec's data pointer to dump its data directly into VP's texture
         const HDC all_screen = GetDC(nullptr);
         const int BitsPerPixel = GetDeviceCaps(all_screen, BITSPIXEL);
         const HDC hdc2 = CreateCompatibleDC(all_screen);
         BITMAPINFO info;
         info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
         info.bmiHeader.biWidth = capture->m_width;
         info.bmiHeader.biHeight = -(int)capture->m_height;
         info.bmiHeader.biPlanes = 1;
         info.bmiHeader.biBitCount = (WORD)BitsPerPixel;
         info.bmiHeader.biCompression = BI_RGB;
         capture->m_hBitmap = CreateDIBSection(hdc2, &info, DIB_RGB_COLORS, (void**)&capture->m_data, 0, 0);

         *capture->m_targetTexture = BaseTexture::CreateFromHBitmap(capture->m_hBitmap, g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MaxTexDimension"s, 0));
         capture->m_data = (*capture->m_targetTexture)->data();
         capture->m_state = CS_Capturing;
      }
      else if (capture->m_state == CS_Capturing && capture->m_updated)
      {
         // We do not lock wait on the update thread when pushing the update information to the texture manager to limit the performance impact
         capture->m_updated = false;
         g_pplayer->m_renderer->m_renderDevice->m_texMan.SetDirty(*capture->m_targetTexture);
      }
   }

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

      // search for windows and create output duplication if needed
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
         }
      }

      // Perform the monitor captures
      for (Duplication* duplication : m_duplications)
      {
         if (duplication->m_failed)
            continue;

         bool desktopIsMapped = false;
         bool stagingTexIsMapped = false;
         BYTE* srcData = nullptr;
         int pitch = 0;
         IDXGIResource* desktop_resource = nullptr;
         DXGI_OUTDUPL_FRAME_INFO frame_info;
         HRESULT hr = duplication->m_duplication->AcquireNextFrame(0, &frame_info, &desktop_resource);
         if (hr == DXGI_ERROR_WAIT_TIMEOUT)
         {
            // No new frame available, just skip (we use a 0ms timeout to peek for a new frame on each update, keeping previous capture texture data if none)
            continue;
         }
         else if (FAILED(hr))
         {
            _com_error err(hr);
            std::basic_stringstream<TCHAR> ss;
            ss << "Capture failed with error message: " << err.ErrorMessage();
            ShowError(ss.str().c_str());
         }
         else
         {
            DXGI_MAPPED_RECT mapped_rect;
            hr = duplication->m_duplication->MapDesktopSurface(&mapped_rect);
            if (hr == S_OK)
            {
               desktopIsMapped = true;
               srcData = (BYTE*)mapped_rect.pBits;
               pitch = mapped_rect.Pitch;
            }
            else if (hr == DXGI_ERROR_UNSUPPORTED) // Can not perform a direct desktop resource mapping: we need to copy it from the GPU to a system memory texture
            {
               ID3D11Texture2D* tex = nullptr;
               hr = desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
               if (FAILED(hr))
               {
                  ShowError("Capture: Failed to query the ID3D11Texture2D interface on IDXGIResource.");
                  exit(EXIT_FAILURE);
               }
               D3D11_BOX srcBox;
               srcBox.front = srcBox.right = srcBox.bottom = 0;
               srcBox.back = 1;
               srcBox.left = srcBox.top = UINT32_MAX;
               UINT outWidth = duplication->m_output->m_OutputDesc.DesktopCoordinates.right - duplication->m_output->m_OutputDesc.DesktopCoordinates.left;
               UINT outHeight = duplication->m_output->m_OutputDesc.DesktopCoordinates.bottom - duplication->m_output->m_OutputDesc.DesktopCoordinates.top;
               for (Capture* capture : m_captures)
               {
                  if (capture->m_duplication == duplication)
                  {
                     RECT inputRect;
                     GetWindowRect(capture->m_window, &inputRect);
                     UINT oldWidth = capture->m_width, oldHeight = capture->m_height;
                     capture->m_width = inputRect.right - inputRect.left;
                     capture->m_height = inputRect.bottom - inputRect.top;
                     capture->m_dispLeft = clamp(inputRect.left - duplication->m_output->m_OutputDesc.DesktopCoordinates.left, 0l, outWidth - capture->m_width);
                     capture->m_dispTop = clamp(inputRect.top - duplication->m_output->m_OutputDesc.DesktopCoordinates.top, 0l, outHeight - capture->m_height);
                     if (oldWidth != capture->m_width || oldHeight != capture->m_height)
                        capture->m_state = CS_Texture;
                     srcBox.left = capture->m_dispLeft;
                     srcBox.right = capture->m_dispLeft + capture->m_width;
                     srcBox.top = capture->m_dispTop;
                     srcBox.bottom = capture->m_dispTop + capture->m_height;
                     duplication->m_device->m_D3DContext->CopySubresourceRegion(duplication->m_stagingTex, 0, capture->m_dispLeft, capture->m_dispTop, 0, tex, 0, &srcBox);
                  }
               }
               SAFE_RELEASE(tex);
               D3D11_MAPPED_SUBRESOURCE map;
               hr = duplication->m_device->m_D3DContext->Map(duplication->m_stagingTex, 0, D3D11_MAP_READ, 0, &map);
               if (SUCCEEDED(hr))
               {
                  stagingTexIsMapped = true;
                  srcData = (BYTE*)map.pData;
                  pitch = map.RowPitch;
               }
               else
               {
                  PLOGE << "Capture: Failed to map the staging tex. Cannot access the pixels.";
               }
            }
            else
            {
               _com_error err(hr);
               std::basic_stringstream<TCHAR> ss;
               ss << "MapDesktopSurface failed with error message: " << err.ErrorMessage();
               ShowError(ss.str().c_str());
            }
         }

         if (srcData != nullptr)
         {
            // Get move & dirty rectangles and mark corresponding captures as dirty
            UINT BufSize = frame_info.TotalMetadataBufferSize;
            if (duplication->m_metaDataBufferSize < BufSize)
            {
               delete[] duplication->m_metaDataBuffer;
               duplication->m_metaDataBuffer = new BYTE[BufSize];
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
                     if (capture->m_duplication == duplication && pmr->DestinationRect.left < capright && pmr->DestinationRect.right > capleft && pmr->DestinationRect.top < capbottom
                        && pmr->DestinationRect.bottom > captop)
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
                     if (capture->m_duplication == duplication && r->left < capright && r->right > capleft && r->top < capbottom && r->bottom > captop)
                        capture->m_dirty = true;
                  }
            }

            // Upload data to target (system) textures, they will be uploaded back to the GPU by the texture manager (this is a fairly inefficient implementation)
            for (Capture* capture : m_captures)
            {
               if (capture->m_duplication == duplication && capture->m_state == CS_Capturing && capture->m_dirty)
               {
                  capture->m_dirty = false;
                  capture->m_updated = true;
                  const uint8_t* __restrict sptr = reinterpret_cast<uint8_t*>(srcData) + pitch * capture->m_dispTop;
                  uint8_t* __restrict ddptr = (uint8_t*)capture->m_data;
                  for (unsigned int h = 0; h < capture->m_height; ++h)
                  {
                     // Copy acquired frame, swapping red and blue channel
                     copy_bgra_rgba<false>((unsigned int*)ddptr, (const unsigned int*)sptr + capture->m_dispLeft, capture->m_width);
                     sptr += pitch;
                     ddptr += capture->m_width * 4;
                  }
               }
            }
         }

         // Clean up
         if (desktopIsMapped)
            duplication->m_duplication->UnMapDesktopSurface();
         else if (stagingTexIsMapped)
            duplication->m_device->m_D3DContext->Unmap(duplication->m_stagingTex, 0);
         SAFE_RELEASE(desktop_resource);
         duplication->m_duplication->ReleaseFrame();
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
