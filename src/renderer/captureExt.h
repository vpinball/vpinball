// license:GPLv3+

#pragma once

#include "renderer/typedefs3D.h"

#include <vector>
#include <string>
#include <mutex>

void StartDMDCapture();
void StartPUPCapture();
void UpdateExtCaptures();
void StopCaptures();
bool HasDMDCapture();
bool HasPUPCapture();

// Disabled for DX9 since it causes conflict with DX9
#ifdef EXT_CAPTURE
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>

class DXGIRegistry
{
public:
   struct Output
   {
      IDXGIAdapter1*          m_Adapter     = nullptr;
      IDXGIOutput*            m_Output      = nullptr;
      IDXGIOutput1*           m_Output1     = nullptr;
      //IDXGIOutput5*           m_Output5     = nullptr;
      DXGI_OUTPUT_DESC        m_OutputDesc;
   };

   struct Device
   {
      IDXGIAdapter1*       m_Adapter    = nullptr;
      ID3D11Device*        m_D3DDevice  = nullptr;
      ID3D11DeviceContext* m_D3DContext = nullptr;
      D3D_FEATURE_LEVEL    m_D3DFeatureLevel;
   };

   ~DXGIRegistry()
   {
      ReleaseAll();
   }

   Device* GetDevice(IDXGIAdapter1* adapter)
   {
      DXGI_ADAPTER_DESC adapterDesc;
      adapter->GetDesc(&adapterDesc);
      for (Device* dev : m_devices)
      {
         DXGI_ADAPTER_DESC devAdapterDesc;
         dev->m_Adapter->GetDesc(&devAdapterDesc);
         if (adapterDesc.AdapterLuid.LowPart == devAdapterDesc.AdapterLuid.LowPart && adapterDesc.AdapterLuid.HighPart == devAdapterDesc.AdapterLuid.HighPart)
            return dev;
      }

      Device* dev = new Device();
      dev->m_Adapter = adapter;
      HRESULT hr = D3D11CreateDevice(adapter, /* Adapter: The adapter (video card) we want to use. We may use NULL to pick the default adapter. */
         D3D_DRIVER_TYPE_UNKNOWN,  /* DriverType: We use the GPU as backing device. */
         nullptr,                  /* Software: we're using a D3D_DRIVER_TYPE_HARDWARE so it's not applicaple. */
         NULL,                     /* Flags: maybe we need to use D3D11_CREATE_DEVICE_BGRA_SUPPORT because desktop duplication is using this. */
         nullptr,                  /* Feature Levels (ptr to array):  what version to use. */
         0,                        /* Number of feature levels. */
         D3D11_SDK_VERSION,        /* The SDK version, use D3D11_SDK_VERSION */
         &dev->m_D3DDevice,        /* OUT: the ID3D11Device object. */
         &dev->m_D3DFeatureLevel,  /* OUT: the selected feature level. */
         &dev->m_D3DContext);      /* OUT: the ID3D11DeviceContext that represents the above features. */

      PLOGI << "Direct3D11 device created for adapter '" << adapterDesc.Description << '\'';

      m_devices.push_back(dev);
      return dev;
   }

   Output* GetForWindow(HWND wnd)
   {
      HMONITOR monitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTONULL);
      if (wnd == NULL)
         return nullptr;

      for (Output* output : m_outputs)
         if (output->m_OutputDesc.Monitor == monitor)
            return output;

      IDXGIFactory1* factory = nullptr;
      HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory));
      if (FAILED(hr))
          return nullptr;

      bool found = false;
      UINT adapterIndex = 0;
      Output* out = new Output();
      while (!found && factory->EnumAdapters1(adapterIndex, &out->m_Adapter) != DXGI_ERROR_NOT_FOUND)
      {
         UINT outputIndex = 0;
         while (!found && out->m_Adapter->EnumOutputs(outputIndex, &out->m_Output) != DXGI_ERROR_NOT_FOUND)
         {
         if (out->m_Output)
         {
            hr = out->m_Output->GetDesc(&out->m_OutputDesc);
            if (hr == S_OK && out->m_OutputDesc.Monitor == monitor)
            {
               if (adapterIndex > 0)
                  PLOGI << "DXGI output was found on another adapter than the desktop's one";
               found = true;
            }
            else
               SAFE_RELEASE(out->m_Output);
         }
         outputIndex++;
         }
         if (!found)
         {
            SAFE_RELEASE(out->m_Adapter);
            adapterIndex++;
         }
      }
      SAFE_RELEASE(factory);

      if (!found)
      {
         PLOGE << "Failed to create find DXGI output for the requested window";
         delete out;
         return nullptr;
      }

      // Query later API version of the interface to have access to duplication methods
      out->m_Output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&out->m_Output1);
      //out->m_Output->QueryInterface(__uuidof(IDXGIOutput5), (void**)&out->m_Output5);

      m_outputs.push_back(out);
      PLOGI << "DXGI output was created for output '" << out->m_OutputDesc.DeviceName << "' on adapter #" << adapterIndex;
      return out;
   }

   void ReleaseAll()
   {
      for (Output* out : m_outputs)
      {
         SAFE_RELEASE(out->m_Output);
         SAFE_RELEASE(out->m_Adapter);
         delete out;
      }
      m_outputs.clear();
      for (Device* dev : m_devices)
      {
         SAFE_RELEASE(dev->m_D3DContext);
         SAFE_RELEASE(dev->m_D3DDevice);
         delete dev;
      }
      m_devices.clear();
   }

private:
   vector<Output*> m_outputs;
   vector<Device*> m_devices;
};

extern DXGIRegistry g_DXGIRegistry;



class ExtCaptureManager
{
public:
   enum CaptureState
   {
      CS_Undeclared, // State of an unkown capture
      CS_Uninitialized, // Initial state
      CS_Searching, // Searching for the target window to capture
      CS_Texture, // Waiting for main render thread to create the texture
      CS_Capturing, // Everything ok, capture in progress
      CS_Failure // Failed (no going back)
   };
   CaptureState GetState(const string& name) const;
   void StartCapture(const string& name, BaseTexture** targetTexture, const vector<string>& windowlist);
   void Update();
   void Stop();

   ~ExtCaptureManager() { Stop(); }

private:
   struct Capture;
   struct Duplication
   {
      bool m_failed = false;
      DXGIRegistry::Output* m_output = nullptr;
      DXGIRegistry::Device* m_device = nullptr;
      IDXGIOutputDuplication* m_duplication = nullptr;
      ID3D11Texture2D* m_stagingTex = nullptr;
      BYTE* m_metaDataBuffer = nullptr;
      unsigned int m_metaDataBufferSize = 0;
   };
   struct Capture
   {
      string m_name;
      CaptureState m_state = CS_Uninitialized;
      vector<string> m_searchWindows;
      Duplication* m_duplication;
      BaseTexture** m_targetTexture;
      HWND m_window;
      HBITMAP m_hBitmap;
      void* m_data = nullptr;
      int m_delay = 0;
      bool m_dirty = false; // Data needs to be updated from monitor capture
      bool m_updated = false; // Target texture needs to be reuploaded to GPU
      UINT m_dispTop = 0, m_dispLeft = 0;
      UINT m_width = 0, m_height = 0;
   };

   void UpdateThread();

   bool m_capturing;
   vector<Capture*> m_captures;
   vector<Duplication*> m_duplications;
   std::thread m_captureThread;
   std::mutex m_captureMutex;
   std::mutex m_updateMutex;
   std::condition_variable m_updateCV;
};

extern ExtCaptureManager g_ExtCaptureManager;

#endif
