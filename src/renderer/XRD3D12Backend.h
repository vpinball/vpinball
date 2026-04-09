// license:GPLv3+

#pragma once

#include "XRGraphicBackend.h"

#ifdef XR_USE_GRAPHICS_API_D3D12

#include <d3d12.h>
#include <dxgi1_6.h>

#define D3D12_CHECK(x, y)                                                                                                                                                                    \
   {                                                                                                                                                                                         \
      const HRESULT result = (x);                                                                                                                                                            \
      if (FAILED(result))                                                                                                                                                                    \
      {                                                                                                                                                                                      \
         PLOGE << "ERROR: D3D12: " << std::hex << "0x" << result << std::dec;                                                                                                                \
         PLOGE << "ERROR: D3D12: " << (y);                                                                                                                                                   \
      }                                                                                                                                                                                      \
   }

#define D3D12_SAFE_RELEASE(p)                                                                                                                                                                \
   {                                                                                                                                                                                         \
      if (p)                                                                                                                                                                                 \
      {                                                                                                                                                                                      \
         (p)->Release();                                                                                                                                                                     \
         (p) = nullptr;                                                                                                                                                                      \
      }                                                                                                                                                                                      \
   }

class XRD3D12Backend final : public XRGraphicBackend
{
public:
   XRD3D12Backend(const XrInstance& m_xrInstance, const XrSystemId& m_systemID)
   {
      xrGetD3D12GraphicsRequirementsKHR = nullptr;
      OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetD3D12GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetD3D12GraphicsRequirementsKHR), "Failed to get InstanceProcAddr xrGetD3D12GraphicsRequirementsKHR.");
      XrGraphicsRequirementsD3D12KHR graphicsRequirements { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR };
      OPENXR_CHECK(xrGetD3D12GraphicsRequirementsKHR(m_xrInstance, m_systemID, &graphicsRequirements), "Failed to get Graphics Requirements for D3D12.");

      static const char* dxgiDllName = "dxgi.dll";
      m_dxgiDll = bx::dlopen(dxgiDllName);
      typedef HRESULT(WINAPI * PFN_CREATE_DXGI_FACTORY)(REFIID _riid, void** _factory);
      PFN_CREATE_DXGI_FACTORY CreateDXGIFactory = (PFN_CREATE_DXGI_FACTORY)bx::dlsym(m_dxgiDll, "CreateDXGIFactory1");

      IDXGIFactory4* factory = nullptr;
      D3D12_CHECK(CreateDXGIFactory(IID_PPV_ARGS(&factory)), "Failed to create DXGI factory.");
      IDXGIAdapter* adapter = nullptr;
      DXGI_ADAPTER_DESC adapterDesc = {};
      for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
      {
         adapter->GetDesc(&adapterDesc);
         if (memcmp(&graphicsRequirements.adapterLuid, &adapterDesc.AdapterLuid, sizeof(LUID)) == 0)
            break; // We have the matching adapter that OpenXR wants.
         adapter = nullptr; // If we don't get a match, reset adapter to nullptr to force a throw.
      }
      OPENXR_CHECK(adapter != nullptr ? XR_SUCCESS : XR_ERROR_VALIDATION_FAILURE, "Failed to find matching graphics adapter from xrGetD3D12GraphicsRequirementsKHR.");

      static const char* d3d12DllName = "d3d12.dll";
      m_d3d12Dll = bx::dlopen(d3d12DllName);
      PFN_D3D12_CREATE_DEVICE D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)bx::dlsym(m_d3d12Dll, "D3D12CreateDevice");
      
      D3D12_CHECK(D3D12CreateDevice(adapter, graphicsRequirements.minFeatureLevel, IID_PPV_ARGS(&m_device)), "Failed to create D3D12 Device.");

      D3D12_COMMAND_QUEUE_DESC queueDesc = {};
      queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
      queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
      D3D12_CHECK(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)), "Failed to create D3D12 Command Queue.");

      D3D12_SAFE_RELEASE(factory);

      m_graphicsBinding = { XR_TYPE_GRAPHICS_BINDING_D3D12_KHR };
      m_graphicsBinding.device = m_device;
      m_graphicsBinding.queue = m_commandQueue;
   }

   ~XRD3D12Backend() override
   {
      D3D12_SAFE_RELEASE(m_commandQueue);
      D3D12_SAFE_RELEASE(m_device);
      if (m_dxgiDll)
         bx::dlclose(m_dxgiDll);
      if (m_d3d12Dll)
         bx::dlclose(m_d3d12Dll);
   }

   void* GetGraphicContext() const override
   {
      return m_device;
   }

   bgfx::RendererType::Enum GetRendererType() const override { return bgfx::RendererType::Direct3D12; }

   void* GetGraphicsBinding() override { return &m_graphicsBinding; }

   void CreateImageViews(VRDevice::SwapchainInfo& swapchain) override
   {
      // TODO BGFX_TEXTURE_BLIT_DST was added since we are blitting the depth instead of directly using it. Remove asap
      uint64_t flags = BGFX_TEXTURE_RT;
      swapchain.format = bgfx::TextureFormat::Enum::Count;
      switch (swapchain.backendFormat)
      { // Convert from values returned from GetSupportedColorSwapchainFormats / GetSupportedDepthSwapchainFormats
      case DXGI_FORMAT_R8G8B8A8_UNORM: swapchain.format = bgfx::TextureFormat::RGBA8; break;
      case DXGI_FORMAT_B8G8R8A8_UNORM: swapchain.format = bgfx::TextureFormat::BGRA8; break;
      case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: swapchain.format = bgfx::TextureFormat::RGBA8; flags |= BGFX_TEXTURE_SRGB; break;
      case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: swapchain.format = bgfx::TextureFormat::BGRA8; flags |= BGFX_TEXTURE_SRGB; break;
      case DXGI_FORMAT_D32_FLOAT: swapchain.format = bgfx::TextureFormat::D32F; flags |= BGFX_TEXTURE_BLIT_DST; break;
      case DXGI_FORMAT_D16_UNORM: swapchain.format = bgfx::TextureFormat::D16; flags |= BGFX_TEXTURE_BLIT_DST; break;
      default: assert(false); break; // Unsupported format
      };
      flags |= BGFX_TEXTURE_RT_WRITE_ONLY;
      for (size_t i = 0; i < m_swapchainImagesMap[swapchain.swapchain].second.size(); i++)
      {
         const bgfx::TextureHandle handle = bgfx::createTexture2D(static_cast<uint16_t>(swapchain.width), static_cast<uint16_t>(swapchain.height), false,
            static_cast<uint16_t>(swapchain.arraySize), swapchain.format, flags, nullptr, reinterpret_cast<uintptr_t>(GetSwapchainImage(swapchain.swapchain, (uint32_t)i)));
         swapchain.imageViews.push_back(handle);
      }
   }

   XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, VRDevice::SwapchainType type, uint32_t count) override
   {
      m_swapchainImagesMap[swapchain].first = type;
      m_swapchainImagesMap[swapchain].second.resize(count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR });
      return reinterpret_cast<XrSwapchainImageBaseHeader*>(m_swapchainImagesMap[swapchain].second.data());
   }
   void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) override { return m_swapchainImagesMap[swapchain].second[index].texture; }
   void FreeSwapchainImageData(XrSwapchain swapchain) override
   {
      m_swapchainImagesMap[swapchain].second.clear();
      m_swapchainImagesMap.erase(swapchain);
   }

   const std::vector<int64_t> GetSupportedColorSwapchainFormats() override { return { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB /*, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB  not supported by BGFX */ }; }
   const std::vector<int64_t> GetSupportedDepthSwapchainFormats() override { return { DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_D16_UNORM }; }

private:
   void* m_dxgiDll = nullptr;
   void* m_d3d12Dll = nullptr;
   ID3D12Device* m_device = nullptr;
   ID3D12CommandQueue* m_commandQueue = nullptr;
   PFN_xrGetD3D12GraphicsRequirementsKHR xrGetD3D12GraphicsRequirementsKHR = nullptr;
   XrGraphicsBindingD3D12KHR m_graphicsBinding {};
   ankerl::unordered_dense::map<XrSwapchain, std::pair<VRDevice::SwapchainType, std::vector<XrSwapchainImageD3D12KHR>>> m_swapchainImagesMap {};
};

#endif
