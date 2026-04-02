// license:GPLv3+

#pragma once

#include "XRGraphicBackend.h"

#ifdef XR_USE_GRAPHICS_API_VULKAN

#include <vulkan/vulkan.h>

#ifdef XR_USE_PLATFORM_ANDROID
#include <vulkan/vulkan_android.h>
#endif

class LibVulkan
{
public:
   static const LibVulkan& GetInstance()
   {
      static LibVulkan lib;
      return lib;
   }

   PFN_vkCreateInstance _vkCreateInstance;
   PFN_vkDestroyInstance _vkDestroyInstance;
   PFN_vkGetPhysicalDeviceFeatures _vkGetPhysicalDeviceFeatures;
   PFN_vkGetPhysicalDeviceQueueFamilyProperties _vkGetPhysicalDeviceQueueFamilyProperties;
   PFN_vkGetInstanceProcAddr _vkGetInstanceProcAddr;
   PFN_vkCreateDevice _vkCreateDevice;
   PFN_vkDestroyDevice _vkDestroyDevice;
   PFN_vkGetDeviceQueue _vkGetDeviceQueue;
   PFN_vkDeviceWaitIdle _vkDeviceWaitIdle;

private:
   void* m_vulkan1Dll;
   ~LibVulkan()
   {
      if (m_vulkan1Dll)
         bx::dlclose(m_vulkan1Dll);
   }
   LibVulkan()
   {
#if BX_PLATFORM_WINDOWS
      m_vulkan1Dll = bx::dlopen(
#if BX_PLATFORM_WINDOWS
         "vulkan-1.dll"
#elif BX_PLATFORM_ANDROID
         "libvulkan.so"
#elif BX_PLATFORM_OSX
         "libMoltenVK.dylib"
#else
         "libvulkan.so.1"
#endif // BX_PLATFORM_*
      );
      if (m_vulkan1Dll == nullptr)
         return;
      _vkCreateInstance = (PFN_vkCreateInstance)bx::dlsym(m_vulkan1Dll, "vkCreateInstance");
      _vkDestroyInstance = (PFN_vkDestroyInstance)bx::dlsym(m_vulkan1Dll, "vkDestroyInstance");
      _vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)bx::dlsym(m_vulkan1Dll, "vkGetPhysicalDeviceFeatures");
      _vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)bx::dlsym(m_vulkan1Dll, "vkGetPhysicalDeviceQueueFamilyProperties");
      _vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)bx::dlsym(m_vulkan1Dll, "vkGetInstanceProcAddr");
      _vkCreateDevice = (PFN_vkCreateDevice)bx::dlsym(m_vulkan1Dll, "vkCreateDevice");
      _vkDestroyDevice = (PFN_vkDestroyDevice)bx::dlsym(m_vulkan1Dll, "vkDestroyDevice");
      _vkGetDeviceQueue = (PFN_vkGetDeviceQueue)bx::dlsym(m_vulkan1Dll, "vkGetDeviceQueue");
      _vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)bx::dlsym(m_vulkan1Dll, "vkDeviceWaitIdle");
#else
      _vkCreateInstance = &vkCreateInstance;
      _vkDestroyInstance = &vkDestroyInstance;
      _vkGetPhysicalDeviceFeatures = &vkGetPhysicalDeviceFeatures;
      _vkGetPhysicalDeviceQueueFamilyProperties = &vkGetPhysicalDeviceQueueFamilyProperties;
      _vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
      _vkCreateDevice = &vkCreateDevice;
      _vkDestroyDevice = &vkDestroyDevice;
      _vkGetDeviceQueue = &vkGetDeviceQueue;
      _vkDeviceWaitIdle = &vkDeviceWaitIdle;
#endif
   }
};


class XRVulkanBackend final : public XRGraphicBackend
{
public:
   XRVulkanBackend(const XrInstance& xrInstance, const XrSystemId& systemID)
      : m_xrInstance(xrInstance)
      , m_systemID(systemID)
   {
      assert(m_vulkan._vkCreateDevice != nullptr);
      assert(m_xrInstance != XR_NULL_HANDLE);
      assert(m_systemID != XR_NULL_SYSTEM_ID);

      PLOGI << "Initializing Vulkan OpenXR backend (creating Vulkan instance/device for OpenXR)";

      XrResult result;

      std::vector<const char*> instanceExtensions;
      instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
      // For the time being, we are not creating any additional swapchain when using the Vulkan backend (so no preview window)
      if (false)
      {
         instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if BX_PLATFORM_ANDROID
         instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif BX_PLATFORM_LINUX
         instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
         instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
         instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif BX_PLATFORM_WINDOWS
         instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif BX_PLATFORM_OSX
         instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif BX_PLATFORM_NX
         instanceExtensions.push_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
#endif
      }
      PLOGI << "Requested Vulkan instance extensions: ";
      for (auto ext : instanceExtensions)
         PLOGI << "\t" << ext;

      // Get Vulkan graphics requirements
      XrGraphicsRequirementsVulkan2KHR graphicsRequirements { XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR };
      PFN_xrGetVulkanGraphicsRequirements2KHR xrGetVulkanGraphicsRequirements2KHR;
      xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsRequirements2KHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsRequirements2KHR);
      result = xrGetVulkanGraphicsRequirements2KHR(m_xrInstance, m_systemID, &graphicsRequirements);
      if (result != XR_SUCCESS)
      {
         PLOGE << "Failed to get Vulkan version requirement for OpenXR: " << result;
         return;
      }
      PLOGI << "Min Vulkan API version: " << XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported) << '.' << XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported) << '.'
            << XR_VERSION_PATCH(graphicsRequirements.minApiVersionSupported);
      PLOGI << "Max Vulkan API version: " << XR_VERSION_MAJOR(graphicsRequirements.maxApiVersionSupported) << '.' << XR_VERSION_MINOR(graphicsRequirements.maxApiVersionSupported) << '.'
            << XR_VERSION_PATCH(graphicsRequirements.maxApiVersionSupported);

      // Create Vulkan instance with the required version
      VkApplicationInfo appInfo = {};
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = "VPinball";
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.pEngineName = "bgfx";
      appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.apiVersion = VK_MAKE_VERSION(XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported), XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported),
         XR_VERSION_PATCH(graphicsRequirements.minApiVersionSupported));

      VkInstanceCreateInfo instanceInfo { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
      instanceInfo.pApplicationInfo = &appInfo;
      instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
      instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

      XrVulkanInstanceCreateInfoKHR vulkanInstanceCreateInfo { XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR };
      vulkanInstanceCreateInfo.systemId = m_systemID;
      vulkanInstanceCreateInfo.pfnGetInstanceProcAddr = m_vulkan._vkGetInstanceProcAddr;
      vulkanInstanceCreateInfo.createFlags = 0;
      vulkanInstanceCreateInfo.vulkanCreateInfo = &instanceInfo;

      VkResult vulkanResult;
      PFN_xrCreateVulkanInstanceKHR xrCreateVulkanInstanceKHR;
      xrGetInstanceProcAddr(xrInstance, "xrCreateVulkanInstanceKHR", (PFN_xrVoidFunction*)&xrCreateVulkanInstanceKHR);
      result = xrCreateVulkanInstanceKHR(m_xrInstance, &vulkanInstanceCreateInfo, &m_instance, &vulkanResult);
      if (result != XR_SUCCESS)
      {
         PLOGE << "Failed to create Vulkan instance for OpenXR: " << result;
         return;
      }

      // Get Vulkan physical device
      XrVulkanGraphicsDeviceGetInfoKHR deviceGetInfo { XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR };
      deviceGetInfo.systemId = m_systemID;
      deviceGetInfo.vulkanInstance = m_instance;

      PFN_xrGetVulkanGraphicsDevice2KHR xrGetVulkanGraphicsDevice2KHR;
      xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsDevice2KHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsDevice2KHR);
      xrGetVulkanGraphicsDevice2KHR(m_xrInstance, &deviceGetInfo, &m_physicalDevice);

      // Create Vulkan device
      uint32_t queueFamilyCount = 0;
      m_vulkan._vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
      std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
      m_vulkan._vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

      m_queueFamilyIndex = UINT32_MAX;
      for (uint32_t i = 0; i < queueFamilyCount; i++)
      {
         if (queueFamilies[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
         {
            m_queueFamilyIndex = i;
            break;
         }
      }

      float queuePriorities[1] = { 0.0f };
      VkDeviceQueueCreateInfo dcqi;
      dcqi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      dcqi.pNext = NULL;
      dcqi.flags = 0;
      dcqi.queueFamilyIndex = m_queueFamilyIndex;
      dcqi.queueCount = 1;
      dcqi.pQueuePriorities = queuePriorities;

      VkPhysicalDeviceFeatures availableFeatures {};
      m_vulkan._vkGetPhysicalDeviceFeatures(m_physicalDevice, &availableFeatures);

      VkPhysicalDeviceFeatures deviceFeatures {};
      if (availableFeatures.samplerAnisotropy)
         deviceFeatures.samplerAnisotropy = VK_TRUE;
      if (availableFeatures.textureCompressionETC2)
         deviceFeatures.textureCompressionETC2 = VK_TRUE;
      if (availableFeatures.textureCompressionASTC_LDR)
         deviceFeatures.textureCompressionASTC_LDR = VK_TRUE;
      if (availableFeatures.fillModeNonSolid)
         deviceFeatures.fillModeNonSolid = VK_TRUE;
      if (availableFeatures.independentBlend)
         deviceFeatures.independentBlend = VK_TRUE;
      if (availableFeatures.depthClamp)
         deviceFeatures.depthClamp = VK_TRUE;
      if (availableFeatures.depthBiasClamp)
         deviceFeatures.depthBiasClamp = VK_TRUE;

      std::vector<const char*> deviceExtensions;
      deviceExtensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
      deviceExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
      //deviceExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
      //deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); // For preview swapchain
      PLOGI << "Requested device extensions: ";
      for (auto ext : deviceExtensions)
         PLOGI << "\t" << ext;

      VkDeviceCreateInfo deviceInfo { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
      deviceInfo.queueCreateInfoCount = 1;
      deviceInfo.pQueueCreateInfos = &dcqi;
      deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
      deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
      deviceInfo.pEnabledFeatures = &deviceFeatures;

      XrVulkanDeviceCreateInfoKHR deviceCreateInfo { XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR };
      deviceCreateInfo.systemId = m_systemID;
      deviceCreateInfo.pfnGetInstanceProcAddr = m_vulkan._vkGetInstanceProcAddr;
      deviceCreateInfo.vulkanPhysicalDevice = m_physicalDevice;
      deviceCreateInfo.createFlags = 0;
      deviceCreateInfo.vulkanCreateInfo = &deviceInfo;

      PFN_xrCreateVulkanDeviceKHR xrCreateVulkanDeviceKHR;
      xrGetInstanceProcAddr(xrInstance, "xrCreateVulkanDeviceKHR", (PFN_xrVoidFunction*)&xrCreateVulkanDeviceKHR);
      result = xrCreateVulkanDeviceKHR(m_xrInstance, &deviceCreateInfo, &m_device, &vulkanResult);
      if (result != XR_SUCCESS || vulkanResult != VK_SUCCESS)
      {
         PLOGE << "Failed to create Vulkan device: " << result << ", " << vulkanResult;
         return;
      }

      m_graphicsBinding = { XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
      m_graphicsBinding.instance = m_instance;
      m_graphicsBinding.physicalDevice = m_physicalDevice;
      m_graphicsBinding.device = m_device;
      m_graphicsBinding.queueFamilyIndex = m_queueFamilyIndex;
      m_graphicsBinding.queueIndex = 0;
   }

   ~XRVulkanBackend() override
   {
      if (m_device != VK_NULL_HANDLE)
      {
         m_vulkan._vkDeviceWaitIdle(m_device);
         m_vulkan._vkDestroyDevice(m_device, nullptr);
      }
      if (m_instance != VK_NULL_HANDLE)
      {
         m_vulkan._vkDestroyInstance(m_instance, nullptr);
      }
   }

   void* GetGraphicContext() const override { return (void*)m_device; }

   bgfx::RendererType::Enum GetRendererType() const override { return bgfx::RendererType::Vulkan; }

   void* GetGraphicsBinding() override { return &m_graphicsBinding; }

   void CreateImageViews(VRDevice::SwapchainInfo& swapchain) override
   {
      uint64_t flags = BGFX_TEXTURE_RT;
      swapchain.format = bgfx::TextureFormat::Enum::Count;
      switch (swapchain.backendFormat)
      {
      case VK_FORMAT_R8G8B8A8_UNORM: swapchain.format = bgfx::TextureFormat::RGBA8; break;
      case VK_FORMAT_R8G8B8A8_SRGB:
         swapchain.format = bgfx::TextureFormat::RGBA8;
         flags |= BGFX_TEXTURE_SRGB;
         break;
      case VK_FORMAT_D32_SFLOAT:
         swapchain.format = bgfx::TextureFormat::D32F;
         flags |= BGFX_TEXTURE_BLIT_DST;
         break;
      case VK_FORMAT_D24_UNORM_S8_UINT:
         swapchain.format = bgfx::TextureFormat::D24S8;
         flags |= BGFX_TEXTURE_BLIT_DST;
         break;
      case VK_FORMAT_D16_UNORM:
         swapchain.format = bgfx::TextureFormat::D16;
         flags |= BGFX_TEXTURE_BLIT_DST;
         break;
      default: PLOGE << "ERROR: Unsupported Vulkan swapchain format: " << swapchain.backendFormat; return;
      }
      flags |= BGFX_TEXTURE_RT_WRITE_ONLY;
      for (size_t i = 0; i < m_swapchainImages[swapchain.swapchain].size(); ++i)
      {
         const bgfx::TextureHandle handle = bgfx::createTexture2D(static_cast<uint16_t>(swapchain.width), static_cast<uint16_t>(swapchain.height), false,
            static_cast<uint16_t>(swapchain.arraySize), swapchain.format, flags, nullptr, reinterpret_cast<uintptr_t>(GetSwapchainImage(swapchain.swapchain, (uint32_t)i)));
         swapchain.imageViews.push_back(handle);
      }
   }

   XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, VRDevice::SwapchainType type, uint32_t count) override
   {
      m_swapchainImages[swapchain].resize(count, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
      return reinterpret_cast<XrSwapchainImageBaseHeader*>(m_swapchainImages[swapchain].data());
   }

   void FreeSwapchainImageData(XrSwapchain swapchain) override
   {
      m_swapchainImages[swapchain].clear();
      m_swapchainImages.erase(swapchain);
   }

   void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) override { return (void*)(uintptr_t)m_swapchainImages[swapchain][index].image; }

   const std::vector<int64_t> GetSupportedColorSwapchainFormats() override { return { VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM }; }

   const std::vector<int64_t> GetSupportedDepthSwapchainFormats() override { return { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM }; }

private:
   const LibVulkan& m_vulkan = LibVulkan::GetInstance();
   XrInstance m_xrInstance;
   XrSystemId m_systemID;
   VkInstance m_instance = VK_NULL_HANDLE;
   VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
   VkDevice m_device = VK_NULL_HANDLE;
   uint32_t m_queueFamilyIndex = 0;
   XrGraphicsBindingVulkanKHR m_graphicsBinding {};
   std::map<XrSwapchain, std::vector<XrSwapchainImageVulkanKHR>> m_swapchainImages;
};

#endif
