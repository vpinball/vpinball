// license:GPLv3+

#pragma once

#include "XRGraphicBackend.h"

#ifdef XR_USE_GRAPHICS_API_D3D11

#include <d3d11_1.h>
#include <dxgi1_6.h>

#define D3D11_CHECK(x, y)                                                                                                                                                                    \
   {                                                                                                                                                                                         \
      const HRESULT result = (x);                                                                                                                                                            \
      if (FAILED(result))                                                                                                                                                                    \
      {                                                                                                                                                                                      \
         PLOGE << "ERROR: D3D11: " << std::hex << "0x" << result << std::dec;                                                                                                                \
         PLOGE << "ERROR: D3D11: " << (y);                                                                                                                                                   \
      }                                                                                                                                                                                      \
   }

#define D3D11_SAFE_RELEASE(p)                                                                                                                                                                \
   {                                                                                                                                                                                         \
      if (p)                                                                                                                                                                                 \
      {                                                                                                                                                                                      \
         (p)->Release();                                                                                                                                                                     \
         (p) = nullptr;                                                                                                                                                                      \
      }                                                                                                                                                                                      \
   }

class XRD3D11Backend final : public XRGraphicBackend
{
public:
   XRD3D11Backend(const XrInstance& m_xrInstance, const XrSystemId& m_systemID)
   {
      xrGetD3D11GraphicsRequirementsKHR = nullptr;
      OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetD3D11GraphicsRequirementsKHR), "Failed to get InstanceProcAddr xrGetD3D11GraphicsRequirementsKHR.");
      XrGraphicsRequirementsD3D11KHR graphicsRequirements { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
      OPENXR_CHECK(xrGetD3D11GraphicsRequirementsKHR(m_xrInstance, m_systemID, &graphicsRequirements), "Failed to get Graphics Requirements for D3D11.");

      static const char* dxgiDllName = "dxgi.dll";
      m_dxgiDll = bx::dlopen(dxgiDllName);
      typedef HRESULT(WINAPI * PFN_CREATE_DXGI_FACTORY)(REFIID _riid, void** _factory);
      PFN_CREATE_DXGI_FACTORY CreateDXGIFactory = (PFN_CREATE_DXGI_FACTORY)bx::dlsym(m_dxgiDll, "CreateDXGIFactory1");

      IDXGIFactory4* factory = nullptr;
      D3D11_CHECK(CreateDXGIFactory(IID_PPV_ARGS(&factory)), "Failed to create DXGI factory.");
      IDXGIAdapter* adapter = nullptr;
      DXGI_ADAPTER_DESC adapterDesc = {};
      for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
      {
         adapter->GetDesc(&adapterDesc);
         if (memcmp(&graphicsRequirements.adapterLuid, &adapterDesc.AdapterLuid, sizeof(LUID)) == 0)
            break; // We have the matching adapter that OpenXR wants.
         adapter = nullptr; // If we don't get a match, reset adapter to nullptr to force a throw.
      }
      OPENXR_CHECK(adapter != nullptr ? XR_SUCCESS : XR_ERROR_VALIDATION_FAILURE, "Failed to find matching graphics adapter from xrGetD3D11GraphicsRequirementsKHR.");

      static const char* d3d11DllName = "d3d11.dll";
      m_d3d11Dll = bx::dlopen(d3d11DllName);
      PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)bx::dlsym(m_d3d11Dll, "D3D11CreateDevice");
      #ifdef DEBUG
      {
         // Try to create the device with the debug layer and fallback to default DX11 if DX SDK is not installed.
         HRESULT result = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, D3D11_CREATE_DEVICE_DEBUG, &graphicsRequirements.minFeatureLevel, 1, D3D11_SDK_VERSION, &m_device, nullptr, &m_immediateContext);
         if (result == DXGI_ERROR_SDK_COMPONENT_MISSING)
            result = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, 0, &graphicsRequirements.minFeatureLevel, 1, D3D11_SDK_VERSION, &m_device, nullptr, &m_immediateContext);
         if (FAILED(result))
         {
            PLOGE << "ERROR: D3D11: " << std::hex << "0x" << result << std::dec;
            PLOGE << "ERROR: D3D11: Failed to create D3D11 Device.";
         }
      }
      #else
         D3D11_CHECK(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, 0, &graphicsRequirements.minFeatureLevel, 1, D3D11_SDK_VERSION, &m_device, nullptr, &m_immediateContext), "Failed to create D3D11 Device.");
      #endif
      D3D11_SAFE_RELEASE(factory);

      m_graphicsBinding = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
      m_graphicsBinding.device = m_device;
   }

   ~XRD3D11Backend() override
   {
      D3D11_SAFE_RELEASE(m_immediateContext);
      D3D11_SAFE_RELEASE(m_device);
      if (m_dxgiDll)
         bx::dlclose(m_dxgiDll);
      if (m_d3d11Dll)
         bx::dlclose(m_d3d11Dll);
   }

   void* GetGraphicContext() const override
   {
      return m_device;
   }

   bgfx::RendererType::Enum GetRendererType() const override { return bgfx::RendererType::Direct3D11; }

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
      m_swapchainImagesMap[swapchain].second.resize(count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
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
   void* m_d3d11Dll = nullptr;
   ID3D11Device* m_device = nullptr;
   ID3D11DeviceContext* m_immediateContext = nullptr;
   PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR = nullptr;
   XrGraphicsBindingD3D11KHR m_graphicsBinding {};
   ankerl::unordered_dense::map<XrSwapchain, std::pair<VRDevice::SwapchainType, std::vector<XrSwapchainImageD3D11KHR>>> m_swapchainImagesMap {};
};

#endif
