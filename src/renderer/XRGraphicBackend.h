// license:GPLv3+

#pragma once

#include "VRDevice.h"

class XRGraphicBackend
{
public:
   virtual ~XRGraphicBackend() = default;

   virtual void* GetGraphicContext() const = 0;
   virtual bgfx::RendererType::Enum GetRendererType() const = 0;
   virtual void* GetGraphicsBinding() = 0;

   virtual XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, VRDevice::SwapchainType type, uint32_t count) = 0;
   virtual void FreeSwapchainImageData(XrSwapchain swapchain) = 0;
   virtual void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) = 0;

   virtual void CreateImageViews(VRDevice::SwapchainInfo& swapchain) = 0;

   virtual const std::vector<int64_t> GetSupportedColorSwapchainFormats() = 0;
   virtual const std::vector<int64_t> GetSupportedDepthSwapchainFormats() = 0;

   int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& formats)
   {
      const std::vector<int64_t>& supportSwapchainFormats = GetSupportedColorSwapchainFormats();
      const std::vector<int64_t>::const_iterator& swapchainFormatIt
         = std::find_first_of(formats.begin(), formats.end(), std::begin(supportSwapchainFormats), std::end(supportSwapchainFormats));
      if (swapchainFormatIt == formats.end())
      {
         PLOGE << "ERROR: Unable to find supported Color Swapchain Format";
         assert(false);
         return 0;
      }
      return *swapchainFormatIt;
   }

   int64_t SelectDepthSwapchainFormat(const std::vector<int64_t>& formats)
   {
      const std::vector<int64_t>& supportSwapchainFormats = GetSupportedDepthSwapchainFormats();
      const std::vector<int64_t>::const_iterator& swapchainFormatIt
         = std::find_first_of(formats.begin(), formats.end(), std::begin(supportSwapchainFormats), std::end(supportSwapchainFormats));
      if (swapchainFormatIt == formats.end())
      {
         PLOGE << "ERROR: Unable to find supported Depth Swapchain Format";
         assert(false);
         return 0;
      }
      return *swapchainFormatIt;
   }
};
