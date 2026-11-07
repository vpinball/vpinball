// license:GPLv3+

#pragma once

#include "XRGraphicBackend.h"

#ifdef XR_USE_GRAPHICS_API_METAL

extern "C" void objc_msgSend(void);
extern "C" void* sel_registerName(const char* str);

class XRMetalBackend final : public XRGraphicBackend
{
public:
   XRMetalBackend(const XrInstance& m_xrInstance, const XrSystemId& m_systemID)
   {
      OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetMetalGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetMetalGraphicsRequirementsKHR), "Failed to get InstanceProcAddr xrGetMetalGraphicsRequirementsKHR.");
      XrGraphicsRequirementsMetalKHR graphicsRequirements { XR_TYPE_GRAPHICS_REQUIREMENTS_METAL_KHR };
      OPENXR_CHECK(xrGetMetalGraphicsRequirementsKHR(m_xrInstance, m_systemID, &graphicsRequirements), "Failed to get Graphics Requirements for Metal.");

      m_device = graphicsRequirements.metalDevice;
      if (m_device)
      {
         ObjcSend(m_device, "retain");
         m_commandQueue = ObjcSend(m_device, "newCommandQueue");
      }

      m_graphicsBinding = { XR_TYPE_GRAPHICS_BINDING_METAL_KHR };
      m_graphicsBinding.commandQueue = m_commandQueue;
   }

   ~XRMetalBackend() override
   {
      if (m_commandQueue)
         ObjcSend(m_commandQueue, "release");
      if (m_device)
         ObjcSend(m_device, "release");
   }

   void* GetGraphicContext() const override { return m_device; }

   void* GetGraphicPlatformQueue() override { return m_commandQueue; }

   bgfx::RendererType::Enum GetRendererType() const override { return bgfx::RendererType::Metal; }

   void* GetGraphicsBinding() override { return &m_graphicsBinding; }

   void CreateImageViews(VRDevice::SwapchainInfo& swapchain) override
   {
      uint64_t flags = BGFX_TEXTURE_RT;
      swapchain.format = bgfx::TextureFormat::Enum::Count;
      switch (swapchain.backendFormat)
      {
      case MTLPixelFormatRGBA8Unorm: swapchain.format = bgfx::TextureFormat::RGBA8; break;
      case MTLPixelFormatRGBA8Unorm_sRGB: swapchain.format = bgfx::TextureFormat::RGBA8; flags |= BGFX_TEXTURE_SRGB; break;
      case MTLPixelFormatDepth32Float: swapchain.format = bgfx::TextureFormat::D32F; flags |= BGFX_TEXTURE_BLIT_DST; break;
      default: PLOGE << "ERROR: Unsupported Metal swapchain format: " << swapchain.backendFormat; return;
      }
      for (size_t i = 0; i < m_swapchainImages[swapchain.swapchain].size(); ++i)
      {
         const bgfx::TextureHandle handle = bgfx::createTexture2D(static_cast<uint16_t>(swapchain.width), static_cast<uint16_t>(swapchain.height), false,
            static_cast<uint16_t>(swapchain.arraySize), swapchain.format, flags, nullptr, reinterpret_cast<uintptr_t>(GetSwapchainImage(swapchain.swapchain, (uint32_t)i)));
         swapchain.imageViews.push_back(handle);
      }
   }

   XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, VRDevice::SwapchainType type, uint32_t count) override
   {
      m_swapchainImages[swapchain].resize(count, { XR_TYPE_SWAPCHAIN_IMAGE_METAL_KHR });
      return reinterpret_cast<XrSwapchainImageBaseHeader*>(m_swapchainImages[swapchain].data());
   }

   void FreeSwapchainImageData(XrSwapchain swapchain) override
   {
      m_swapchainImages[swapchain].clear();
      m_swapchainImages.erase(swapchain);
   }

   void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) override { return m_swapchainImages[swapchain][index].texture; }

   const std::vector<int64_t> GetSupportedColorSwapchainFormats() override { return { MTLPixelFormatRGBA8Unorm_sRGB, MTLPixelFormatRGBA8Unorm }; }

   const std::vector<int64_t> GetSupportedDepthSwapchainFormats() override { return { MTLPixelFormatDepth32Float }; }

private:
   static void* ObjcSend(void* object, const char* selector) { return reinterpret_cast<void* (*)(void*, void*)>(objc_msgSend)(object, sel_registerName(selector)); }

   enum : int64_t
   {
      MTLPixelFormatRGBA8Unorm = 70,
      MTLPixelFormatRGBA8Unorm_sRGB = 71,
      MTLPixelFormatDepth32Float = 252,
   };

   PFN_xrGetMetalGraphicsRequirementsKHR xrGetMetalGraphicsRequirementsKHR = nullptr;
   void* m_device = nullptr;
   void* m_commandQueue = nullptr;
   XrGraphicsBindingMetalKHR m_graphicsBinding {};
   std::map<XrSwapchain, std::vector<XrSwapchainImageMetalKHR>> m_swapchainImages;
};

#endif
