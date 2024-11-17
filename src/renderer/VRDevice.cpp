// license:GPLv3+

#include "core/stdafx.h"
#include "VRDevice.h"

// Undefine this if you want to debug OpenVR without a VR headset
//#define OPEN_VR_TEST


#if defined(ENABLE_XR) && defined(ENABLE_BGFX)
   #include "bgfx/platform.h"
   #include "bgfx/bgfx.h"
   #include "bx/math.h"
   #include "bx/os.h"

   #define XR_USE_PLATFORM_WIN32

   //#define XR_USE_GRAPHICS_API_VULKAN
   //#define XR_USE_GRAPHICS_API_OPENGL
   //#define XR_USE_GRAPHICS_API_OPENGL_ES
   #define XR_USE_GRAPHICS_API_D3D11
   //#define XR_USE_GRAPHICS_API_D3D12

   #include <openxr/openxr.h>
   #include <openxr/openxr_platform.h>


inline const char* GetXRErrorString(XrInstance xrInstance, XrResult result)
{
   static char string[XR_MAX_RESULT_STRING_SIZE];
   xrResultToString(xrInstance, result, string);
   return string;
}

#define OPENXR_CHECK(x, y)                                                                                                                                                                   \
   {                                                                                                                                                                                         \
      const XrResult result = (x);                                                                                                                                                                 \
      if (!XR_SUCCEEDED(result) && g_pplayer->m_vrDevice)                                                                                                                                    \
      {                                                                                                                                                                                      \
         PLOGE << "ERROR: OPENXR: " << int(result) << "(" << (m_xrInstance ? GetXRErrorString(m_xrInstance, result) : "") << ") " << y;        \
      }                                                                                                                                                                                      \
   }

class GraphicBackend
{
public:
   struct ImageViewCreateInfo
   {
      void* image;
      enum class Type : uint8_t
      {
         RTV,
         DSV,
         SRV,
         UAV
      } type;
      enum class View : uint8_t
      {
         TYPE_1D,
         TYPE_2D,
         TYPE_3D,
         TYPE_CUBE,
         TYPE_1D_ARRAY,
         TYPE_2D_ARRAY,
         TYPE_CUBE_ARRAY,
      } view;
      int64_t format;
      enum class Aspect : uint8_t
      {
         COLOR_BIT = 0x01,
         DEPTH_BIT = 0x02,
         STENCIL_BIT = 0x04
      } aspect;
      uint32_t baseMipLevel;
      uint32_t levelCount;
      uint32_t baseArrayLayer;
      uint32_t layerCount;
   };

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




#include <d3d11_1.h>
#include <dxgi1_6.h>

#define D3D11_CHECK(x, y)                                                                                                                                                                    \
   {                                                                                                                                                                                         \
      const HRESULT result = (x);                                                                                                                                                            \
      if (FAILED(result))                                                                                                                                                                    \
      {                                                                                                                                                                                      \
         PLOGE << "ERROR: D3D11: " << std::hex << "0x" << result << std::dec;                                                                                                                \
         PLOGE << "ERROR: D3D11: " << y;                                                                                                                                                     \
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

class D3D11Backend : public GraphicBackend
{
public:
   D3D11Backend(XrInstance& m_xrInstance, XrSystemId& m_systemID)
   {
      PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR = nullptr;
      OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetD3D11GraphicsRequirementsKHR), "Failed to get InstanceProcAddr xrGetD3D11GraphicsRequirementsKHR.");
      XrGraphicsRequirementsD3D11KHR graphicsRequirements { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
      OPENXR_CHECK(xrGetD3D11GraphicsRequirementsKHR(m_xrInstance, m_systemID, &graphicsRequirements), "Failed to get Graphics Requirements for D3D11.");

		const char* dxgiDllName = "dxgi.dll";
      void* m_dxgiDll = bx::dlopen(dxgiDllName);
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
         adapter = nullptr; // If we don't get a match reset adapter to nullptr to force a throw.
      }
      OPENXR_CHECK(adapter != nullptr ? XR_SUCCESS : XR_ERROR_VALIDATION_FAILURE, "Failed to find matching graphics adapter from xrGetD3D11GraphicsRequirementsKHR.");
      
		const char* d3d11DllName = "d3d11.dll";
      m_d3d11Dll = bx::dlopen(d3d11DllName);
      PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)bx::dlsym(m_d3d11Dll, "D3D11CreateDevice");
      D3D11_CHECK(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, D3D11_CREATE_DEVICE_DEBUG, &graphicsRequirements.minFeatureLevel, 1, D3D11_SDK_VERSION, &m_device, nullptr, &m_immediateContext), "Failed to create D3D11 Device.");

      // Also creates a swapchain for the preview window (and to support RenderDoc captures)
      DXGI_SWAP_CHAIN_DESC1 scd;
      scd.Width = g_pplayer->m_playfieldWnd->GetWidth();
      scd.Height = g_pplayer->m_playfieldWnd->GetHeight();
      scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      scd.Stereo = 0;
      scd.SampleDesc.Count = 1;
      scd.SampleDesc.Quality = 0;
      scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      scd.BufferCount = 2;
      scd.Scaling = DXGI_SCALING_STRETCH;
      scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      scd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
      scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
      DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfd;
      scfd.RefreshRate.Numerator = 1;
      scfd.RefreshRate.Denominator = 60;
      scfd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
      scfd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
      scfd.Windowed = true;
      D3D11_CHECK(factory->CreateSwapChainForHwnd(m_device, g_pplayer->m_playfieldWnd->GetNativeHWND(), &scd, &scfd, NULL, &m_swapChain), "Failed to create preview window swapchain");
      D3D11_SAFE_RELEASE(factory);
   }

   ~D3D11Backend()
   {
      D3D11_SAFE_RELEASE(m_immediateContext);
      D3D11_SAFE_RELEASE(m_device);
      if (m_dxgiDll)
         bx::dlclose(m_dxgiDll);
      if (m_d3d11Dll)
         bx::dlclose(m_d3d11Dll);
   }

   void* GetGraphicsBinding()
   {
      graphicsBinding = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
      graphicsBinding.device = m_device;
      return &graphicsBinding;
   }

   XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, VRDevice::SwapchainType type, uint32_t count)
   {
      swapchainImagesMap[swapchain].first = type;
      swapchainImagesMap[swapchain].second.resize(count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
      return reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImagesMap[swapchain].second.data());
   }

   void* CreateImageView(const ImageViewCreateInfo& imageViewCI)
   {
      if (imageViewCI.type == ImageViewCreateInfo::Type::RTV)
      {
         D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};
         rtvDesc.Format = (DXGI_FORMAT)imageViewCI.format;

         switch (imageViewCI.view)
         {
         case ImageViewCreateInfo::View::TYPE_1D:
         {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
            rtvDesc.Texture1D.MipSlice = imageViewCI.baseMipLevel;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_2D:
         {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = imageViewCI.baseMipLevel;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_3D:
         {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.MipSlice = imageViewCI.baseMipLevel;
            rtvDesc.Texture3D.FirstWSlice = imageViewCI.baseArrayLayer;
            rtvDesc.Texture3D.WSize = imageViewCI.layerCount;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_1D_ARRAY:
         {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
            rtvDesc.Texture1DArray.MipSlice = imageViewCI.baseMipLevel;
            rtvDesc.Texture1DArray.FirstArraySlice = imageViewCI.baseArrayLayer;
            rtvDesc.Texture1DArray.ArraySize = imageViewCI.layerCount;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_2D_ARRAY:
         {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice = imageViewCI.baseMipLevel;
            rtvDesc.Texture2DArray.FirstArraySlice = imageViewCI.baseArrayLayer;
            rtvDesc.Texture2DArray.ArraySize = imageViewCI.layerCount;
            break;
         }
         default:
            assert(false);
            PLOGE << "ERROR: D3D11: Unknown ImageView View.";
            return nullptr;
         }
         ID3D11RenderTargetView* rtv = nullptr;
         D3D11_CHECK(m_device->CreateRenderTargetView((ID3D11Resource*)imageViewCI.image, &rtvDesc, &rtv), "Failed to create ImageView.")
         return rtv;
      }
      else if (imageViewCI.type == ImageViewCreateInfo::Type::DSV)
      {
         D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc {};
         dsvDesc.Format = (DXGI_FORMAT)imageViewCI.format;

         switch (imageViewCI.view)
         {
         case ImageViewCreateInfo::View::TYPE_1D:
         {
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
            dsvDesc.Texture1D.MipSlice = imageViewCI.baseMipLevel;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_2D:
         {
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = imageViewCI.baseMipLevel;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_1D_ARRAY:
         {
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
            dsvDesc.Texture1DArray.MipSlice = imageViewCI.baseMipLevel;
            dsvDesc.Texture1DArray.FirstArraySlice = imageViewCI.baseArrayLayer;
            dsvDesc.Texture1DArray.ArraySize = imageViewCI.layerCount;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_2D_ARRAY:
         {
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.MipSlice = imageViewCI.baseMipLevel;
            dsvDesc.Texture2DArray.FirstArraySlice = imageViewCI.baseArrayLayer;
            dsvDesc.Texture2DArray.ArraySize = imageViewCI.layerCount;
            break;
         }
         default:
            assert(false);
            PLOGE << "ERROR: D3D11: Unknown ImageView View.";
            return nullptr;
         }
         ID3D11DepthStencilView* dsv = nullptr;
         D3D11_CHECK(m_device->CreateDepthStencilView((ID3D11Resource*)imageViewCI.image, &dsvDesc, &dsv), "Failed to create ImageView.")
         return dsv;
      }
      else if (imageViewCI.type == ImageViewCreateInfo::Type::SRV)
      {
         D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};
         srvDesc.Format = (DXGI_FORMAT)imageViewCI.format;

         switch (imageViewCI.view)
         {
         case ImageViewCreateInfo::View::TYPE_1D:
         {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D.MostDetailedMip = imageViewCI.baseMipLevel;
            srvDesc.Texture1D.MipLevels = imageViewCI.levelCount;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_2D:
         {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = imageViewCI.baseMipLevel;
            srvDesc.Texture2D.MipLevels = imageViewCI.levelCount;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_3D:
         {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
            srvDesc.Texture3D.MostDetailedMip = imageViewCI.baseMipLevel;
            srvDesc.Texture3D.MipLevels = imageViewCI.levelCount;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_1D_ARRAY:
         {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray.MostDetailedMip = imageViewCI.baseMipLevel;
            srvDesc.Texture1DArray.MipLevels = imageViewCI.levelCount;
            srvDesc.Texture1DArray.FirstArraySlice = imageViewCI.baseArrayLayer;
            srvDesc.Texture1DArray.ArraySize = imageViewCI.layerCount;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_2D_ARRAY:
         {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.MostDetailedMip = imageViewCI.baseMipLevel;
            srvDesc.Texture2DArray.MipLevels = imageViewCI.levelCount;
            srvDesc.Texture2DArray.FirstArraySlice = imageViewCI.baseArrayLayer;
            srvDesc.Texture2DArray.ArraySize = imageViewCI.layerCount;
            break;
         }
         default:
            assert(false);
            PLOGE << "ERROR: D3D11: Unknown ImageView View.";
            return nullptr;
         }
         ID3D11ShaderResourceView* srv = nullptr;
         D3D11_CHECK(m_device->CreateShaderResourceView((ID3D11Resource*)imageViewCI.image, &srvDesc, &srv), "Failed to create ImageView.")
         return srv;
      }
      else if (imageViewCI.type == ImageViewCreateInfo::Type::UAV)
      {
         D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc {};
         uavDesc.Format = (DXGI_FORMAT)imageViewCI.format;

         switch (imageViewCI.view)
         {
         case ImageViewCreateInfo::View::TYPE_1D:
         {
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
            uavDesc.Texture1D.MipSlice = imageViewCI.baseMipLevel;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_2D:
         {
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = imageViewCI.baseMipLevel;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_3D:
         {
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
            uavDesc.Texture3D.MipSlice = imageViewCI.baseMipLevel;
            uavDesc.Texture3D.FirstWSlice = imageViewCI.baseArrayLayer;
            uavDesc.Texture3D.WSize = imageViewCI.layerCount;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_1D_ARRAY:
         {
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.Texture1DArray.MipSlice = imageViewCI.baseMipLevel;
            uavDesc.Texture1DArray.FirstArraySlice = imageViewCI.baseArrayLayer;
            uavDesc.Texture1DArray.ArraySize = imageViewCI.layerCount;
            break;
         }
         case ImageViewCreateInfo::View::TYPE_2D_ARRAY:
         {
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.Texture2DArray.MipSlice = imageViewCI.baseMipLevel;
            uavDesc.Texture2DArray.FirstArraySlice = imageViewCI.baseArrayLayer;
            uavDesc.Texture2DArray.ArraySize = imageViewCI.layerCount;
            break;
         }
         default:
            assert(false);
            PLOGE << "ERROR: D3D11: Unknown ImageView View.";
            return nullptr;
         }
         ID3D11UnorderedAccessView* uav = nullptr;
         D3D11_CHECK(m_device->CreateUnorderedAccessView((ID3D11Resource*)imageViewCI.image, &uavDesc, &uav), "Failed to create ImageView.")
         return uav;
      }
      else
      {
         assert(false);
         PLOGE << "ERROR: D3D11: Unknown ImageView Type.";
         return nullptr;
      }
   }

   void DestroyImageView(void*& imageView)
   {
      ID3D11View* d3d11ImageView = (ID3D11View*)imageView;
      D3D11_SAFE_RELEASE(d3d11ImageView);
      imageView = nullptr;
   }

   void FreeSwapchainImageData(XrSwapchain swapchain)
   {
      swapchainImagesMap[swapchain].second.clear();
      swapchainImagesMap.erase(swapchain);
   }

   void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) { return swapchainImagesMap[swapchain].second[index].texture; }
   const std::vector<int64_t> GetSupportedColorSwapchainFormats() override { return { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB }; }
   const std::vector<int64_t> GetSupportedDepthSwapchainFormats() override { return { DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_D16_UNORM }; }

   void* m_dxgiDll = nullptr;
   void* m_d3d11Dll = nullptr;
   ID3D11Device* m_device = nullptr;
   ID3D11DeviceContext* m_immediateContext = nullptr;
   IDXGISwapChain1* m_swapChain = nullptr;
   PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR = nullptr;
   XrGraphicsBindingD3D11KHR graphicsBinding {};
   std::unordered_map<XrSwapchain, std::pair<VRDevice::SwapchainType, std::vector<XrSwapchainImageD3D11KHR>>> swapchainImagesMap {};
};

#endif



#ifdef ENABLE_VR
   vr::IVRSystem* VRDevice::m_pHMD = nullptr;
#endif



VRDevice::VRDevice()
{
   #if defined(ENABLE_XR)
      // Fill out an XrApplicationInfo structure detailing the names and OpenXR version.
      // The application/engine name and version are user-definied. These may help IHVs or runtimes.
      XrApplicationInfo AI;
      strncpy(AI.applicationName, "Visual Pinball X", XR_MAX_APPLICATION_NAME_SIZE);
      AI.applicationVersion = 0x1081; // 10.81
      strncpy(AI.engineName, "", XR_MAX_ENGINE_NAME_SIZE);
      AI.engineVersion = 0;
      AI.apiVersion = XR_MAKE_VERSION(1, 0, 0); //XR_CURRENT_API_VERSION;

      // Get all the API Layers from the OpenXR runtime.
      uint32_t apiLayerCount = 0;
      std::vector<XrApiLayerProperties> apiLayerProperties;
      OPENXR_CHECK(xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr), "Failed to enumerate ApiLayerProperties.");
      apiLayerProperties.resize(apiLayerCount, { XR_TYPE_API_LAYER_PROPERTIES });
      OPENXR_CHECK(xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data()), "Failed to enumerate ApiLayerProperties.");

      // Check the requested API layers against the ones from the OpenXR. If found add it to the Active API Layers.
      m_activeAPILayers.clear();
      for (auto& requestLayer : m_apiLayers)
      {
         for (auto& layerProperty : apiLayerProperties)
         {
            if (strcmp(requestLayer.c_str(), layerProperty.layerName) == 0)
            {
               m_activeAPILayers.push_back(requestLayer.c_str());
               break;
            }
         }
      }

      // Get all the Instance Extensions from the OpenXR instance.
      uint32_t extensionCount = 0;
      std::vector<XrExtensionProperties> extensionProperties;
      OPENXR_CHECK(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr), "Failed to enumerate InstanceExtensionProperties.");
      extensionProperties.resize(extensionCount, { XR_TYPE_EXTENSION_PROPERTIES });
      OPENXR_CHECK(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()), "Failed to enumerate InstanceExtensionProperties.");
      #ifdef DEBUG
         for (auto& extensionProperty : extensionProperties)
         {
            PLOGD << "OpenXR supported extension: " << extensionProperty.extensionName << ", version " << extensionProperty.extensionVersion;
         }
      #endif
      m_activeInstanceExtensions.clear();
      // Add a specific extension to the list of extensions to be enabled, if it is supported.
      auto EnableExtentionIfSupported = [&](const char* extensionName)
      {
         for (uint32_t i = 0; i < extensionCount; i++)
         {
            if (strcmp(extensionProperties[i].extensionName, extensionName) == 0)
            {
               m_activeInstanceExtensions.push_back(extensionName);
               return true;
            }
         }
         return false;
      };
      //const bool hasGraphicBackend = EnableExtentionIfSupported(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);
      //const bool hasGraphicBackend = EnableExtentionIfSupported(XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME);
      //const bool hasGraphicBackend = EnableExtentionIfSupported(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
      const bool hasGraphicBackend = EnableExtentionIfSupported(XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
      //const bool hasGraphicBackend = EnableExtentionIfSupported(XR_KHR_D3D12_ENABLE_EXTENSION_NAME);
      assert(hasGraphicBackend);
      // FIXME OpenXR: Strangely enough, performance drops if XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME is enabled
      //m_depthExtensionSupported = EnableExtentionIfSupported(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
      m_colorSpaceExtensionSupported = EnableExtentionIfSupported(XR_FB_COLOR_SPACE_EXTENSION_NAME);
      //EnableExtentionIfSupported(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);

      // Fill out an XrInstanceCreateInfo structure and create an XrInstance.
      XrInstanceCreateInfo instanceCI { XR_TYPE_INSTANCE_CREATE_INFO };
      instanceCI.createFlags = 0;
      instanceCI.applicationInfo = AI;
      instanceCI.enabledApiLayerCount = static_cast<uint32_t>(m_activeAPILayers.size());
      instanceCI.enabledApiLayerNames = m_activeAPILayers.data();
      instanceCI.enabledExtensionCount = static_cast<uint32_t>(m_activeInstanceExtensions.size());
      instanceCI.enabledExtensionNames = m_activeInstanceExtensions.data();
      OPENXR_CHECK(xrCreateInstance(&instanceCI, &m_xrInstance), "Failed to create Instance.");
      if (m_xrInstance == XR_NULL_HANDLE)
      {
         assert(false);
         return;
      }

      // Check that "XR_EXT_debug_utils" is in the active Instance Extensions before creating an XrDebugUtilsMessengerEXT.
      //if (IsStringInVector(m_activeInstanceExtensions, XR_EXT_DEBUG_UTILS_EXTENSION_NAME))
      {
         //m_debugUtilsMessenger = CreateOpenXRDebugUtilsMessenger(m_xrInstance); // From OpenXRDebugUtils.h.
      }

      // Get the instance's properties and log the runtime name and version.
      XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
      OPENXR_CHECK(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get InstanceProperties.");
      PLOGI << "OpenXR Runtime: " << instanceProperties.runtimeName << " - "
                                 << XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
                                 << XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
                                 << XR_VERSION_PATCH(instanceProperties.runtimeVersion);
   #endif

   #if defined(ENABLE_VR)
      m_pHMD = nullptr;
      m_rTrackedDevicePose = nullptr;
      m_scale = 1.0f; // Scale factor from scene (in VP units) to VR view (in meters)
      if (g_pvp->GetActiveTable()->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleToFixedWidth"s, false))
      {
         float width;
         g_pvp->GetActiveTable()->get_Width(&width);
         m_scale = g_pvp->GetActiveTable()->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleAbsolute"s, 55.0f) * 0.01f / width;
      }
      else
         m_scale = VPUTOCM(0.01f) * g_pvp->GetActiveTable()->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleRelative"s, 1.0f);
      if (m_scale < VPUTOCM(0.01f))
         m_scale = VPUTOCM(0.01f); // Scale factor for VPUnits to Meters

      // Initialize VR, this will also override the render buffer size (m_width, m_height) to account for HMD render size and render the 2 eyes simultaneously
      #ifdef OPEN_VR_TEST
         m_pHMD = nullptr;
      #else
         vr::EVRInitError VRError = vr::VRInitError_None;
         if (!m_pHMD) {
            m_pHMD = vr::VR_Init(&VRError, vr::VRApplication_Scene);
            if (VRError != vr::VRInitError_None) {
               m_pHMD = nullptr;
               char buf[1024];
               sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(VRError));
               ShowError(buf);
            }
            else if (!vr::VRCompositor())
            /*if (VRError != vr::VRInitError_None)*/ {
               m_pHMD = nullptr;
               char buf[1024];
               sprintf_s(buf, sizeof(buf), "Unable to init VR compositor");// :% s", vr::VR_GetVRInitErrorAsEnglishDescription(VRError));
               ShowError(buf);
            }
         }
      #endif

      // Move from VP units to meters, and also apply user scene scaling if any
      Matrix3D sceneScale = Matrix3D::MatrixScale(m_scale);

      // Convert from VPX coords to VR (270deg rotation around X axis, and flip x axis)
      Matrix3D coords = Matrix3D::MatrixIdentity();
      coords._11 = -1.f; coords._12 = 0.f; coords._13 =  0.f;
      coords._21 =  0.f; coords._22 = 0.f; coords._23 = -1.f;
      coords._31 =  0.f; coords._32 = 1.f; coords._33 =  0.f;

      float zNear, zFar;
      g_pvp->GetActiveTable()->ComputeNearFarPlane(coords * sceneScale, m_scale, zNear, zFar);
      zNear = g_pvp->GetActiveTable()->m_settings.LoadValueWithDefault(Settings::PlayerVR, "NearPlane"s, 5.0f) / 100.0f; // Replace near value to allow player to move near parts up to user defined value
      zFar *= 1.2f;

      if (m_pHMD == nullptr)
      {
         // Basic debug setup (All OpenVR matrices are left handed, using meter units)
         // For debugging without a headset, the null driver of OpenVR should be enabled. To do so:
         // - Set "enable": true in default.vrsettings from C:\Program Files (x86)\Steam\steamapps\common\SteamVR\drivers\null\resources\settings
         // - Add "activateMultipleDrivers" : true, "forcedDriver" : "null", to "steamvr" section of steamvr.vrsettings from C :\Program Files(x86)\Steam\config
         uint32_t eye_width = 1080, eye_height = 1200; // Oculus Rift resolution
         m_eyeWidth = eye_width;
         m_eyeHeight = eye_height;
         for (int i = 0; i < 2; i++)
         {
            const Matrix3D proj = Matrix3D::MatrixPerspectiveFovLH(90.f, 1.f, zNear, zFar);
            m_vrMatProj[i] = coords * sceneScale * proj;
         }
      }
      else
      {
         uint32_t eye_width, eye_height;
         m_pHMD->GetRecommendedRenderTargetSize(&eye_width, &eye_height);
         m_eyeWidth = eye_width;
         m_eyeHeight = eye_height;
         vr::HmdMatrix34_t left_eye_pos = m_pHMD->GetEyeToHeadTransform(vr::Eye_Left);
         vr::HmdMatrix34_t right_eye_pos = m_pHMD->GetEyeToHeadTransform(vr::Eye_Right);
         vr::HmdMatrix44_t left_eye_proj = m_pHMD->GetProjectionMatrix(vr::Eye_Left, zNear, zFar);
         vr::HmdMatrix44_t right_eye_proj = m_pHMD->GetProjectionMatrix(vr::Eye_Right, zNear, zFar);

         //Calculate left EyeProjection Matrix relative to HMD position
         Matrix3D matEye2Head = Matrix3D::MatrixIdentity();
         for (int i = 0; i < 3; i++)
            for (int j = 0;j < 4;j++)
               matEye2Head.m[j][i] = left_eye_pos.m[i][j];
         matEye2Head.Invert();

         left_eye_proj.m[2][2] = -1.0f;
         left_eye_proj.m[2][3] = -zNear;
         Matrix3D matProjection;
         for (int i = 0;i < 4;i++)
            for (int j = 0;j < 4;j++)
               matProjection.m[j][i] = left_eye_proj.m[i][j];

         m_vrMatProj[0] = coords * sceneScale * matEye2Head * matProjection;

         //Calculate right EyeProjection Matrix relative to HMD position
         matEye2Head = Matrix3D::MatrixIdentity();
         for (int i = 0; i < 3; i++)
            for (int j = 0;j < 4;j++)
               matEye2Head.m[j][i] = right_eye_pos.m[i][j];
         matEye2Head.Invert();

         right_eye_proj.m[2][2] = -1.0f;
         right_eye_proj.m[2][3] = -zNear;
         for (int i = 0;i < 4;i++)
            for (int j = 0;j < 4;j++)
               matProjection.m[j][i] = right_eye_proj.m[i][j];

         m_vrMatProj[1] = coords * sceneScale * matEye2Head * matProjection;
      }

      if (vr::k_unMaxTrackedDeviceCount > 0) {
         m_rTrackedDevicePose = new vr::TrackedDevicePose_t[vr::k_unMaxTrackedDeviceCount];
      }
      else {
         std::runtime_error noDevicesFound("No Tracking devices found");
         throw(noDevicesFound);
      }
   #endif

   m_slope = g_pvp->GetActiveTable()->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Slope"s, 6.5f);
   m_orientation = g_pvp->GetActiveTable()->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Orientation"s, 0.0f);
   m_tablex = g_pvp->GetActiveTable()->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableX"s, 0.0f);
   m_tabley = g_pvp->GetActiveTable()->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableY"s, 0.0f);
   m_tablez = g_pvp->GetActiveTable()->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableZ"s, 80.0f);
}

VRDevice::~VRDevice()
{
   #if defined(ENABLE_XR)
      // Destroy the color and depth image views from GraphicsAPI.
      for (void*& imageView : m_colorSwapchainInfo.imageViews)
         m_backend->DestroyImageView(imageView);
      for (void*& imageView : m_depthSwapchainInfo.imageViews)
         m_backend->DestroyImageView(imageView);

      // Free the Swapchain Image Data.
      if (m_colorSwapchainInfo.swapchain)
         m_backend->FreeSwapchainImageData(m_colorSwapchainInfo.swapchain);
      if (m_depthSwapchainInfo.swapchain)
         m_backend->FreeSwapchainImageData(m_depthSwapchainInfo.swapchain);

      // Destroy the swapchains.
      if (m_colorSwapchainInfo.swapchain)
         OPENXR_CHECK(xrDestroySwapchain(m_colorSwapchainInfo.swapchain), "Failed to destroy Color Swapchain");
      if (m_depthSwapchainInfo.swapchain)
         OPENXR_CHECK(xrDestroySwapchain(m_depthSwapchainInfo.swapchain), "Failed to destroy Depth Swapchain");
      delete m_backend;

      // Destroy the reference XrSpace.
      OPENXR_CHECK(xrDestroySpace(m_localSpace), "Failed to destroy Space.")

      // Destroy the XrSession.
      OPENXR_CHECK(xrDestroySession(m_session), "Failed to destroy Session.");

      // Destroy the XrInstance.
      OPENXR_CHECK(xrDestroyInstance(m_xrInstance), "Failed to destroy Instance.");
   
   #elif defined(ENABLE_VR)
      if (m_pHMD)
      {
         vr::VR_Shutdown();
         m_pHMD = nullptr;
      }
   #endif
}

#ifdef ENABLE_XR
void* VRDevice::GetGraphicContext() const
{
   return m_backend->m_device;
}

void* VRDevice::GetSwapChainBackBuffer(const int index, const bool isDepth) const
{
   assert(0 <= index && index < m_depthSwapchainInfo.imageViews.size());
   if (isDepth)
      return m_depthSwapchainInfo.imageViews[index];
   else
      return m_colorSwapchainInfo.imageViews[index];
}

VRDevice::SwapchainInfo VRDevice::CreateSwapChain(XrSession session, D3D11Backend* backend, SwapchainType type, int64_t format, 
   uint32_t width, uint32_t height, uint32_t arraySize, uint32_t sampleCount, XrSwapchainCreateFlags createFlags, XrSwapchainUsageFlags usageFlags)
{
   VRDevice::SwapchainInfo swapchain;
   swapchain.swapchainFormat = format;
   swapchain.width = width;
   swapchain.height = height;
   swapchain.arraySize = arraySize;

   XrSwapchainCreateInfo swapchainCreateInfo { XR_TYPE_SWAPCHAIN_CREATE_INFO };
   swapchainCreateInfo.arraySize = arraySize;
   swapchainCreateInfo.format = format;
   swapchainCreateInfo.width = width;
   swapchainCreateInfo.height = height;
   swapchainCreateInfo.mipCount = 1;
   swapchainCreateInfo.faceCount = 1;
   swapchainCreateInfo.sampleCount = sampleCount;
   swapchainCreateInfo.createFlags = createFlags;
   swapchainCreateInfo.usageFlags = usageFlags;
   OPENXR_CHECK(xrCreateSwapchain(session, &swapchainCreateInfo, &swapchain.swapchain), "Failed to create Color Swapchain");

   uint32_t swapchainImageCount;
   OPENXR_CHECK(xrEnumerateSwapchainImages(swapchain.swapchain, 0, &swapchainImageCount, nullptr), "Failed to enumerate Swapchain Images.");
   XrSwapchainImageBaseHeader* swapchainImages = backend->AllocateSwapchainImageData(swapchain.swapchain, type, swapchainImageCount);
   OPENXR_CHECK(xrEnumerateSwapchainImages(swapchain.swapchain, swapchainImageCount, &swapchainImageCount, swapchainImages), "Failed to enumerate Swapchain Images.");

   for (uint32_t j = 0; j < swapchainImageCount; j++)
   {
      D3D11Backend::ImageViewCreateInfo imageViewCI;
      imageViewCI.image = backend->GetSwapchainImage(swapchain.swapchain, j);
      imageViewCI.type = type == SwapchainType::COLOR ? D3D11Backend::ImageViewCreateInfo::Type::RTV : D3D11Backend::ImageViewCreateInfo::Type::DSV;
      imageViewCI.view = D3D11Backend::ImageViewCreateInfo::View::TYPE_2D_ARRAY;
      imageViewCI.format = format;
      imageViewCI.aspect = type == SwapchainType::COLOR ? D3D11Backend::ImageViewCreateInfo::Aspect::COLOR_BIT : D3D11Backend::ImageViewCreateInfo::Aspect::DEPTH_BIT;
      imageViewCI.baseMipLevel = 0;
      imageViewCI.levelCount = 1;
      imageViewCI.baseArrayLayer = 0;
      imageViewCI.layerCount = arraySize;
      swapchain.imageViews.push_back(backend->CreateImageView(imageViewCI));
   }

   return swapchain;
}

Matrix3D VRDevice::XRPoseToMatrix3D(XrPosef pose)
{
   Matrix3D view;
   bx::Quaternion orientation(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);
   bx::Quaternion invertOrientation = bx::invert(orientation);
   bx::mtxFromQuaternion(&view.m[0][0], invertOrientation);
   view.Transpose();
   bx::Quaternion position(pose.position.x, pose.position.y, pose.position.z, 0.f);
   bx::Quaternion invertPosition = bx::invert(position);
   invertPosition = bx::mul(invertPosition, orientation);
   invertPosition = bx::mul(invertOrientation, invertPosition);
   view.m[3][0] += invertPosition.x;
   view.m[3][1] += invertPosition.y;
   view.m[3][2] += invertPosition.z;
   return view;
}

void VRDevice::SetupHMD()
{
   assert(m_xrInstance != XR_NULL_HANDLE);

   // Get the XrSystemId from the instance and the supplied XrFormFactor.
   XrSystemGetInfo systemGI{XR_TYPE_SYSTEM_GET_INFO};
   systemGI.formFactor = m_formFactor;
   OPENXR_CHECK(xrGetSystem(m_xrInstance, &systemGI, &m_systemID), "Failed to get SystemID.");
   // if failed with XR_ERROR_FORM_FACTOR_UNAVAILABLE, then the headset is just not detected, we delay/retry
   // FIXME handle other error return codes
   if (m_systemID == XR_NULL_SYSTEM_ID)
      return;

   // Get the System's properties for some general information about the hardware and the vendor.
   OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, m_systemID, &m_systemProperties), "Failed to get SystemProperties.");

   // Gets the View Configuration Types. The first call gets the count of the array that will be returned. The next call fills out the array.
   uint32_t viewConfigurationCount = 0;
   OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, 0, &viewConfigurationCount, nullptr), "Failed to enumerate View Configurations.");
   m_viewConfigurations.resize(viewConfigurationCount);
   OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, viewConfigurationCount, &viewConfigurationCount, m_viewConfigurations.data()), "Failed to enumerate View Configurations.");

   // Pick the first application supported View Configuration Type con supported by the hardware.
   for (const XrViewConfigurationType& viewConfiguration : m_applicationViewConfigurations)
   {
      if (std::find(m_viewConfigurations.begin(), m_viewConfigurations.end(), viewConfiguration) != m_viewConfigurations.end())
      {
         m_viewConfiguration = viewConfiguration;
         break;
      }
   }
   if (m_viewConfiguration == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM)
   {
      PLOGE << "Failed to find a view configuration type. Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO.";
      m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
   }

   // Gets the View Configuration Views. The first call gets the count of the array that will be returned. The next call fills out the array.
   uint32_t viewConfigurationViewCount = 0;
   OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, 0, &viewConfigurationViewCount, nullptr), "Failed to enumerate ViewConfiguration Views.");
   m_viewConfigurationViews.resize(viewConfigurationViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
   OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, viewConfigurationViewCount, &viewConfigurationViewCount, m_viewConfigurationViews.data()), "Failed to enumerate ViewConfiguration Views.");

   // Retrieves the available blend modes. The first call gets the count of the array that will be returned. The next call fills out the array.
   uint32_t environmentBlendModeCount = 0;
   OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_systemID, m_viewConfiguration, 0, &environmentBlendModeCount, nullptr), "Failed to enumerate EnvironmentBlend Modes.");
   m_environmentBlendModes.resize(environmentBlendModeCount);
   OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_systemID, m_viewConfiguration, environmentBlendModeCount, &environmentBlendModeCount, m_environmentBlendModes.data()), "Failed to enumerate EnvironmentBlend Modes.");
   #ifdef DEBUG
      for (auto& environmentBlendMode : m_environmentBlendModes)
      {
         static const char* blendModeNames[] = { "Opaque", "Additive", "Alpha" };
         PLOGD << "OpenXR supported blend mode: " << (environmentBlendMode < 3 ? blendModeNames[environmentBlendMode] : std::to_string(environmentBlendMode).c_str());
      }
   #endif
   // Pick the first application supported blend mode supported by the hardware.
   for (const XrEnvironmentBlendMode& environmentBlendMode : m_applicationEnvironmentBlendModes)
   {
      if (std::find(m_environmentBlendModes.begin(), m_environmentBlendModes.end(), environmentBlendMode) != m_environmentBlendModes.end())
      {
         m_environmentBlendMode = environmentBlendMode;
         break;
      }
   }
   if (m_environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM)
   {
      PLOGE << "Failed to find a compatible blend mode. Defaulting to XR_ENVIRONMENT_BLEND_MODE_OPAQUE.";
      m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
   }
}

void VRDevice::CreateSession()
{
   assert(m_xrInstance != XR_NULL_HANDLE);
   assert(m_systemID != XR_NULL_SYSTEM_ID);
   assert(m_session == XR_NULL_HANDLE);
   m_backend = new D3D11Backend(m_xrInstance, m_systemID);

   // Since we are using texture array rendering, we need target to be stereo views with the same setup
   assert(m_viewConfigurationViews.size() == 2);
   assert(m_viewConfigurationViews[0].recommendedImageRectWidth == m_viewConfigurationViews[1].recommendedImageRectWidth);
   assert(m_viewConfigurationViews[0].recommendedImageRectHeight == m_viewConfigurationViews[1].recommendedImageRectHeight);
   assert(m_viewConfigurationViews[0].recommendedSwapchainSampleCount == m_viewConfigurationViews[1].recommendedSwapchainSampleCount);

   // Let the user choose the down/super sampling
   float resFactor = g_pplayer ? g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ResFactor", -1.f) : -1.f;
   if (resFactor <= 0.1f || resFactor > 10.f)
   {
      m_eyeWidth = m_viewConfigurationViews[0].recommendedImageRectWidth;
      m_eyeHeight = m_viewConfigurationViews[0].recommendedImageRectHeight;
   }
   else
   {
      m_eyeWidth = m_viewConfigurationViews[0].maxImageRectWidth * resFactor;
      m_eyeHeight = m_viewConfigurationViews[0].maxImageRectHeight * resFactor;
   }

   // Create an XrSessionCreateInfo structure.
   XrSessionCreateInfo sessionCI { XR_TYPE_SESSION_CREATE_INFO };
   sessionCI.next = m_backend->GetGraphicsBinding();
   sessionCI.createFlags = 0;
   sessionCI.systemId = m_systemID;
   OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCI, &m_session), "Failed to create Session.");

   // Fill out an XrReferenceSpaceCreateInfo structure and create a reference XrSpace, specifying a Local space with an identity pose as the origin.
   XrReferenceSpaceCreateInfo referenceSpaceCI { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
   referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
   referenceSpaceCI.poseInReferenceSpace = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } };
   OPENXR_CHECK(xrCreateReferenceSpace(m_session, &referenceSpaceCI, &m_localSpace), "Failed to create ReferenceSpace.");

   // Set color space to get the same rendering on all HMD and usual desktop play
   if (m_colorSpaceExtensionSupported)
   {
      PFN_xrSetColorSpaceFB xrSetColorSpaceFB;
      OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrSetColorSpaceFB", (PFN_xrVoidFunction*)&xrSetColorSpaceFB), "Failed to get xrSetColorSpaceFB.");
      xrSetColorSpaceFB(m_session, XR_COLOR_SPACE_REC709_FB);
   }

   // Get the supported swapchain formats as an array of int64_t and ordered by runtime preference.
   uint32_t formatCount = 0;
   OPENXR_CHECK(xrEnumerateSwapchainFormats(m_session, 0, &formatCount, nullptr), "Failed to enumerate Swapchain Formats");
   std::vector<int64_t> formats(formatCount);
   OPENXR_CHECK(xrEnumerateSwapchainFormats(m_session, formatCount, &formatCount, formats.data()), "Failed to enumerate Swapchain Formats");
   if (m_backend->SelectDepthSwapchainFormat(formats) == 0)
   {
      PLOGE << "Failed to find depth format for Swapchain." ;
      assert(false);
   }

   m_colorSwapchainInfo = CreateSwapChain(m_session, m_backend, SwapchainType::COLOR, m_backend->SelectColorSwapchainFormat(formats), m_eyeWidth, m_eyeHeight,
      static_cast<uint32_t>(m_viewConfigurationViews.size()), m_viewConfigurationViews[0].recommendedSwapchainSampleCount, 0, XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT);

   m_depthSwapchainInfo = CreateSwapChain(m_session, m_backend, SwapchainType::DEPTH, m_backend->SelectDepthSwapchainFormat(formats), m_eyeWidth, m_eyeHeight,
      static_cast<uint32_t>(m_viewConfigurationViews.size()), m_viewConfigurationViews[0].recommendedSwapchainSampleCount, 0, XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
}


void VRDevice::PollEvents()
{
   // Poll OpenXR for a new event.
   XrEventDataBuffer eventData { XR_TYPE_EVENT_DATA_BUFFER };
   auto XrPollEvents = [&]() -> bool
   {
      eventData = { XR_TYPE_EVENT_DATA_BUFFER };
      return xrPollEvent(m_xrInstance, &eventData) == XR_SUCCESS;
   };

   while (XrPollEvents())
   {
      switch (eventData.type)
      {
      // Log the number of lost events from the runtime.
      case XR_TYPE_EVENT_DATA_EVENTS_LOST:
      {
         XrEventDataEventsLost* eventsLost = reinterpret_cast<XrEventDataEventsLost*>(&eventData);
         PLOGI << "OPENXR: Events Lost: " << eventsLost->lostEventCount;
         break;
      }
      // Log that an instance loss is pending and shutdown the application.
      case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
      {
         XrEventDataInstanceLossPending* instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending*>(&eventData);
         PLOGI << "OPENXR: Instance Loss Pending at: " << instanceLossPending->lossTime;
         m_sessionRunning = false;
         m_applicationRunning = false;
         break;
      }
      // Log that the interaction profile has changed.
      case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
      {
         XrEventDataInteractionProfileChanged* interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged*>(&eventData);
         PLOGI << "OPENXR: Interaction Profile changed for Session: " << interactionProfileChanged->session;
         if (interactionProfileChanged->session != m_session)
         {
            PLOGI << "XrEventDataInteractionProfileChanged for unknown Session";
            break;
         }
         break;
      }
      // Log that there's a reference space change pending.
      case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
      {
         XrEventDataReferenceSpaceChangePending* referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventData);
         PLOGI << "OPENXR: Reference Space Change pending for Session: " << referenceSpaceChangePending->session;
         if (referenceSpaceChangePending->session != m_session)
         {
            PLOGI << "XrEventDataReferenceSpaceChangePending for unknown Session";
            break;
         }
         break;
      }
      // Session State changes:
      case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
      {
         XrEventDataSessionStateChanged* sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
         if (sessionStateChanged->session != m_session)
         {
            PLOGI << "XrEventDataSessionStateChanged for unknown Session";
            break;
         }

         if (sessionStateChanged->state == XR_SESSION_STATE_READY)
         {
            // SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
            XrSessionBeginInfo sessionBeginInfo { XR_TYPE_SESSION_BEGIN_INFO };
            sessionBeginInfo.primaryViewConfigurationType = m_viewConfiguration;
            OPENXR_CHECK(xrBeginSession(m_session, &sessionBeginInfo), "Failed to begin Session.");
            m_sessionRunning = true;
         }
         if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING)
         {
            // SessionState is stopping. End the XrSession.
            OPENXR_CHECK(xrEndSession(m_session), "Failed to end Session.");
            m_sessionRunning = false;
         }
         if (sessionStateChanged->state == XR_SESSION_STATE_EXITING)
         {
            // SessionState is exiting. Exit the application.
            m_sessionRunning = false;
            m_applicationRunning = false;
         }
         if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING)
         {
            // SessionState is loss pending. Exit the application.
            // It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
            m_sessionRunning = false;
            m_applicationRunning = false;
         }
         // Store state for reference across the application.
         m_sessionState = sessionStateChanged->state;
         break;
      }
      default:
      {
         break;
      }
      }
   }
}

void VRDevice::RenderFrame(std::function<void(bool renderVR)> submitFrame)
{
   bool rendered = true;
   if (!m_sessionRunning)
   {
      // FIXME we should perform preview rendering here
      submitFrame(false);
      m_backend->m_swapChain->Present(0, 0);
      return;
   }

   // Let OpenXR throttle frame submisison and get the XrFrameState for timing and rendering info.
   XrFrameState frameState { XR_TYPE_FRAME_STATE };
   XrFrameWaitInfo frameWaitInfo { XR_TYPE_FRAME_WAIT_INFO };
   OPENXR_CHECK(xrWaitFrame(m_session, &frameWaitInfo, &frameState), "Failed to wait for XR Frame.");

   // Tell the OpenXR compositor that the application is beginning the frame.
   XrFrameBeginInfo frameBeginInfo { XR_TYPE_FRAME_BEGIN_INFO };
   OPENXR_CHECK(xrBeginFrame(m_session, &frameBeginInfo), "Failed to begin the XR Frame.");

   // Variables for rendering and layer composition.
   RenderLayerInfo renderLayerInfo;
   renderLayerInfo.predictedDisplayTime = frameState.predictedDisplayTime;

   // Check that the session is active and that we should render.
   const bool sessionActive = (m_sessionState == XR_SESSION_STATE_SYNCHRONIZED || m_sessionState == XR_SESSION_STATE_VISIBLE || m_sessionState == XR_SESSION_STATE_FOCUSED);
   if (!sessionActive || !frameState.shouldRender)
      rendered = false;

   if (rendered)
   {
      // Locate the views from the view configuration within the (reference) space at the predicted display time.
      std::vector<XrView> views(m_viewConfigurationViews.size(), { XR_TYPE_VIEW });

      XrViewState viewState { XR_TYPE_VIEW_STATE }; // Will contain information on whether the position and/or orientation is valid and/or tracked.
      XrViewLocateInfo viewLocateInfo { XR_TYPE_VIEW_LOCATE_INFO };
      viewLocateInfo.viewConfigurationType = m_viewConfiguration;
      viewLocateInfo.displayTime = renderLayerInfo.predictedDisplayTime;
      viewLocateInfo.space = m_localSpace;
      uint32_t viewCount = 0;
      XrResult result = xrLocateViews(m_session, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount, views.data());
      if (result != XR_SUCCESS)
      {
         PLOGE << "Failed to locate Views.";
         rendered = false;
      }

      if (rendered)
      {
         const float zNear = 0.1f;
         const float zFar = 5.f;
         const float vpuScale = 0.0254f * 1.0625f / 50.f;

         // Compute the eye median pose in VPU coordinates to be used as the view point for shading
         XrPosef medianPose = views[0].pose;
         medianPose.position.x = (medianPose.position.x + views[1].pose.position.x) * 0.5f / vpuScale;
         medianPose.position.y = (medianPose.position.y + views[1].pose.position.y) * 0.5f / vpuScale;
         medianPose.position.z = (medianPose.position.z + views[1].pose.position.z) * 0.5f / vpuScale;
         medianPose.orientation.x = (medianPose.orientation.x + views[1].pose.orientation.x) * 0.5f;
         medianPose.orientation.y = (medianPose.orientation.y + views[1].pose.orientation.y) * 0.5f;
         medianPose.orientation.z = (medianPose.orientation.z + views[1].pose.orientation.z) * 0.5f;
         medianPose.orientation.w = (medianPose.orientation.w + views[1].pose.orientation.w) * 0.5f;
         float length = medianPose.orientation.x * medianPose.orientation.x + medianPose.orientation.y * medianPose.orientation.y + medianPose.orientation.z * medianPose.orientation.z
            + medianPose.orientation.w * medianPose.orientation.w;
         medianPose.orientation.x /= length;
         medianPose.orientation.y /= length;
         medianPose.orientation.z /= length;
         medianPose.orientation.w /= length;
         m_vrMatView = XRPoseToMatrix3D(medianPose);

         // We only keep the residual offset of each eye from the median view position in the projection matrix
         Matrix3D scale = Matrix3D::MatrixScale(vpuScale, vpuScale, vpuScale);
         Matrix3D invView = m_vrMatView;
         invView.Invert();
         for (uint32_t i = 0; i < viewCount; i++)
         {
            Matrix3D view = XRPoseToMatrix3D(views[i].pose);
            Matrix3D proj;
            proj.SetPerspectiveFovRH(views[i].fov.angleLeft, views[i].fov.angleRight, views[i].fov.angleDown, views[i].fov.angleUp, zNear, zFar);
            m_vrMatProj[i] = invView * scale * view * proj;
         }

         // Swapchain is acquired, rendered to, and released together for all views as a texture array

         // Resize the layer projection views to match the view count. The layer projection views are used in the layer projection.
         renderLayerInfo.layerProjectionViews.resize(viewCount, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });
         if (m_depthExtensionSupported)
            renderLayerInfo.depthInfoViews.resize(viewCount, { XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR });

         // Acquire and wait for an image from the swapchains (the timeout is infinite)
         uint32_t colorImageIndex = 0;
         uint32_t depthImageIndex = 0;
         XrSwapchainImageAcquireInfo acquireInfo { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
         OPENXR_CHECK(xrAcquireSwapchainImage(m_colorSwapchainInfo.swapchain, &acquireInfo, &colorImageIndex), "Failed to acquire Image from the Color Swapchian");
         OPENXR_CHECK(xrAcquireSwapchainImage(m_depthSwapchainInfo.swapchain, &acquireInfo, &depthImageIndex), "Failed to acquire Image from the Depth Swapchian");

         XrSwapchainImageWaitInfo waitInfo = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
         waitInfo.timeout = XR_INFINITE_DURATION;
         OPENXR_CHECK(xrWaitSwapchainImage(m_colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Color Swapchain");
         OPENXR_CHECK(xrWaitSwapchainImage(m_depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Depth Swapchain");

         // Use the full range of recommended image size to achieve optimum resolution
         const XrRect2Di imageRect = { { 0, 0 }, { (int32_t)m_colorSwapchainInfo.width, (int32_t)m_colorSwapchainInfo.height } };
         assert(m_colorSwapchainInfo.width == m_depthSwapchainInfo.width);
         assert(m_colorSwapchainInfo.height == m_depthSwapchainInfo.height);

         // Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
         for (uint32_t i = 0; i < viewCount; i++)
         {
            renderLayerInfo.layerProjectionViews[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
            renderLayerInfo.layerProjectionViews[i].pose = views[i].pose;
            renderLayerInfo.layerProjectionViews[i].fov = views[i].fov;
            renderLayerInfo.layerProjectionViews[i].subImage.swapchain = m_colorSwapchainInfo.swapchain;
            renderLayerInfo.layerProjectionViews[i].subImage.imageRect = imageRect;
            renderLayerInfo.layerProjectionViews[i].subImage.imageArrayIndex = i;
            if (m_depthExtensionSupported)
            {
               renderLayerInfo.depthInfoViews[i] = { XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR };
               renderLayerInfo.depthInfoViews[i].minDepth = 0;
               renderLayerInfo.depthInfoViews[i].maxDepth = 1;
               renderLayerInfo.depthInfoViews[i].nearZ = zNear;
               renderLayerInfo.depthInfoViews[i].farZ = zFar;
               renderLayerInfo.depthInfoViews[i].subImage.swapchain = m_depthSwapchainInfo.swapchain;
               renderLayerInfo.depthInfoViews[i].subImage.imageRect = imageRect;
               renderLayerInfo.depthInfoViews[i].subImage.imageArrayIndex = i;
               renderLayerInfo.layerProjectionViews[i].next = &renderLayerInfo.depthInfoViews[i];
            }
         }

         // Reset BGFX backbuffer to point to the view from the swap chain selected by OpenXR
         bgfx::PlatformData pdata;
         pdata.context = m_backend->m_device;
         pdata.backBuffer = m_colorSwapchainInfo.imageViews[colorImageIndex];
         pdata.backBufferDS = m_depthSwapchainInfo.imageViews[depthImageIndex];
         bgfx::setPlatformData(pdata);
         bgfx::reset(static_cast<int32_t>(imageRect.extent.width), static_cast<int32_t>(imageRect.extent.height), BGFX_RESET_NONE, bgfx::TextureFormat::RGBA8);

         // Prepare frame with the acquired views to limit position-visual latency (limit motion sickness)
         // This can't be done earlier since view acquisition is done for the predicted frame display time (which we have only after xrWaitFrame)
         submitFrame(true);

         // Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().
         renderLayerInfo.layerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
         renderLayerInfo.layerProjection.space = m_localSpace;
         renderLayerInfo.layerProjection.viewCount = static_cast<uint32_t>(renderLayerInfo.layerProjectionViews.size());
         renderLayerInfo.layerProjection.views = renderLayerInfo.layerProjectionViews.data();

         // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
         XrSwapchainImageReleaseInfo releaseInfo { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
         OPENXR_CHECK(xrReleaseSwapchainImage(m_colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
         OPENXR_CHECK(xrReleaseSwapchainImage(m_depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");

         renderLayerInfo.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&renderLayerInfo.layerProjection));
      }
   }

   // Tell OpenXR that we are finished with this frame; specifying its display time, environment blending and layers.
   XrFrameEndInfo frameEndInfo { XR_TYPE_FRAME_END_INFO };
   frameEndInfo.displayTime = frameState.predictedDisplayTime;
   frameEndInfo.environmentBlendMode = m_environmentBlendMode;
   frameEndInfo.layerCount = static_cast<uint32_t>(renderLayerInfo.layers.size());
   frameEndInfo.layers = renderLayerInfo.layers.data();
   OPENXR_CHECK(xrEndFrame(m_session, &frameEndInfo), "Failed to end the XR Frame.");

   // Perform preview window rendering only
   if (!rendered)
      submitFrame(false);

   // FIXME Also performs preview window rendering and present (to actually render the preview, but also enables RenderDoc)
   // FIXME this needs to be preoperly cleaned up, splitting the preview output from the VR output
   m_backend->m_swapChain->Present(0, 0);
}
#endif

#ifdef ENABLE_VR
bool VRDevice::IsVRinstalled()
{
   #ifdef OPEN_VR_TEST
      return true;
   #endif
   return vr::VR_IsRuntimeInstalled();
}

bool VRDevice::IsVRturnedOn()
{
   #ifdef OPEN_VR_TEST
      return true;
   #endif
   if (vr::VR_IsHmdPresent())
   {
      vr::EVRInitError VRError = vr::VRInitError_None;
      if (!m_pHMD)
         m_pHMD = vr::VR_Init(&VRError, vr::VRApplication_Background);
      if (VRError == vr::VRInitError_None && vr::VRCompositor()) {
         for (uint32_t device = 0; device < vr::k_unMaxTrackedDeviceCount; device++) {
            if ((m_pHMD->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_HMD)) {
               vr::VR_Shutdown();
               m_pHMD = nullptr;
               return true;
            }
         }
      } else
         m_pHMD = nullptr;
   }
   return false;
}

bool VRDevice::IsVRReady() const
{
   return m_pHMD != nullptr;
}

void VRDevice::SubmitFrame(Sampler* leftEye, Sampler* rightEye)
{
      #if defined(ENABLE_OPENGL)
         vr::Texture_t leftEyeTexture = { (void*)(__int64)leftEye->GetCoreTexture(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
         vr::Texture_t rightEyeTexture = { (void*)(__int64)rightEye->GetCoreTexture(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
      #elif defined(ENABLE_BGFX)
         vr::ETextureType textureType;
         switch (bgfx::getRendererType())
         {
         case bgfx::RendererType::Enum::Direct3D11: textureType = vr::TextureType_DirectX; break;
         case bgfx::RendererType::Enum::Direct3D12: textureType = vr::TextureType_DirectX12; break;
         case bgfx::RendererType::Enum::OpenGL:     textureType = vr::TextureType_OpenGL; break;
         case bgfx::RendererType::Enum::OpenGLES:   textureType = vr::TextureType_OpenGL; break;
         case bgfx::RendererType::Enum::Vulkan:     textureType = vr::TextureType_Vulkan; break;
         case bgfx::RendererType::Enum::Metal:      textureType = vr::TextureType_Metal;
            assert(false); // FIXME implement metal protocol (multi layer texture)
            break;
         default: return;
         }
         vr::Texture_t leftEyeTexture = { (void*)(__int64)leftEye->GetNativeTexture(), textureType, vr::ColorSpace_Gamma };
         vr::Texture_t rightEyeTexture = { (void*)(__int64)rightEye->GetNativeTexture(), textureType, vr::ColorSpace_Gamma };
      #endif
      vr::EVRCompositorError errorLeft = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
      if (errorLeft != vr::VRCompositorError_None)
      {
         char msg[128];
         sprintf_s(msg, sizeof(msg), "VRCompositor Submit Left Error %d", errorLeft);
         ShowError(msg);
      }
      vr::EVRCompositorError errorRight = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
      if (errorRight != vr::VRCompositorError_None)
      {
         char msg[128];
         sprintf_s(msg, sizeof(msg), "VRCompositor Submit Right Error %d", errorRight);
         ShowError(msg);
      }
      #if defined(ENABLE_OPENGL)
         glFlush();
      #endif
      //vr::VRCompositor()->PostPresentHandoff(); // PostPresentHandoff gives mixed results, improved GPU frametime for some, worse CPU frametime for others, troublesome enough to not warrants it's usage for now
}
#endif


void VRDevice::UpdateVRPosition(ModelViewProj& mvp)
{
   #ifdef ENABLE_XR
      if (m_tableWorldDirty)
      {
         m_tableWorldDirty = false;
         // Move table in front of player, adjust coord from RH to LH system (in VPU coordinates)
         Matrix3D trans = Matrix3D::MatrixTranslate(
            -CMTOVPU(m_tablex) - (g_pplayer->m_ptable->m_right - g_pplayer->m_ptable->m_left) * 0.5f,
             CMTOVPU(m_tabley) + (g_pplayer->m_ptable->m_bottom - g_pplayer->m_ptable->m_top) + CMTOVPU(30.f),
            -CMTOVPU(m_tablez));
         Matrix3D coords = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
         Matrix3D rotx = Matrix3D::MatrixRotateX(-ANGTORAD(90.f + m_slope));
         Matrix3D rotz = Matrix3D::MatrixRotateZ(ANGTORAD(m_orientation));
         m_tableWorld = coords * trans * rotz * rotx;
      }

      mvp.SetView(m_tableWorld * m_vrMatView);
      mvp.SetProj(0, m_vrMatProj[0]);
      mvp.SetProj(1, m_vrMatProj[1]);

   #elif defined(ENABLE_VR)
      Matrix3D matView = Matrix3D::MatrixIdentity();

      if (IsVRReady())
      {
         vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
         for (unsigned int device = 0; device < vr::k_unMaxTrackedDeviceCount; device++)
         {
            if ((m_rTrackedDevicePose[device].bPoseIsValid) && (m_pHMD->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_HMD))
            {
               m_hmdPosition = m_rTrackedDevicePose[device];

               // Convert to 4x4 inverse matrix (world to head)
               for (int i = 0; i < 3; i++)
                  for (int j = 0; j < 4; j++)
                     matView.m[j][i] = m_hmdPosition.mDeviceToAbsoluteTracking.m[i][j];
               matView.Invert();

               // Scale translation part
               for (int i = 0; i < 3; i++)
                  matView.m[3][i] /= m_scale;

               // Convert from VPX coords to VR and back (270deg rotation around X axis, and flip x axis)
               Matrix3D coords = Matrix3D::MatrixIdentity();
               coords._11 = -1.f; coords._12 = 0.f; coords._13 =  0.f;
               coords._21 =  0.f; coords._22 = 0.f; coords._23 = -1.f;
               coords._31 =  0.f; coords._32 = 1.f; coords._33 =  0.f;
               Matrix3D revCoords = Matrix3D::MatrixIdentity();
               revCoords._11 = -1.f; revCoords._12 =  0.f; revCoords._13 = 0.f;
               revCoords._21 =  0.f; revCoords._22 =  0.f; revCoords._23 = 1.f;
               revCoords._31 =  0.f; revCoords._32 = -1.f; revCoords._33 = 0.f;
               matView = coords * matView * revCoords;

               break;
            }
         }
      }

      if (m_tableWorldDirty)
      {
         m_tableWorldDirty = false;
         // Locate front left corner of the table in the room -x is to the right, -y is up and -z is back - all units in meters
         const float inv_transScale = 1.0f / (100.0f * m_scale);
         m_tableWorld = Matrix3D::MatrixRotateX(ANGTORAD(-m_slope)) // Tilt playfield
            * Matrix3D::MatrixRotateZ(ANGTORAD(180.f + m_orientation)) // Rotate table around VR height axis
            * Matrix3D::MatrixTranslate(-m_tablex * inv_transScale, m_tabley * inv_transScale, m_tablez * inv_transScale);
      }

      mvp.SetView(m_tableWorld * matView);
      mvp.SetProj(0, m_vrMatProj[0]);
      mvp.SetProj(1, m_vrMatProj[1]);
   #endif
}

void VRDevice::TableUp()
{
   m_tablez += 1.0f;
   if (m_tablez > 250.0f)
      m_tablez = 250.0f;
   m_tableWorldDirty = true;
}

void VRDevice::TableDown()
{
   m_tablez -= 1.0f;
   if (m_tablez < 0.0f)
      m_tablez = 0.0f;
   m_tableWorldDirty = true;
}

void VRDevice::RecenterTable()
{
   #ifdef ENABLE_XR
      // TODO allow scaling of the table ? or just keep everything real world size ?
      m_orientation = RADTOANG(atan2f(m_vrMatView.m[0][2], m_vrMatView.m[0][0]));
      m_tablex = VPUTOCM(m_vrMatView.m[3][0]); // Head X offset
      m_tabley = VPUTOCM(m_vrMatView.m[3][2]); // Head Y offset
   #elif defined(ENABLE_VR)
      float headX = 0.f, headY = 0.f;
      const float w = m_scale * (g_pplayer->m_ptable->m_right - g_pplayer->m_ptable->m_left) * 0.5f;
      const float h = m_scale * (g_pplayer->m_ptable->m_bottom - g_pplayer->m_ptable->m_top) + 0.2f;
      if (IsVRReady())
      {
         m_orientation = -RADTOANG(atan2f(m_hmdPosition.mDeviceToAbsoluteTracking.m[0][2], m_hmdPosition.mDeviceToAbsoluteTracking.m[0][0]));
         if (m_orientation < 0.0f)
            m_orientation += 360.0f;
         headX = m_hmdPosition.mDeviceToAbsoluteTracking.m[0][3];
         headY = -m_hmdPosition.mDeviceToAbsoluteTracking.m[2][3];
      }
      const float c = cosf(ANGTORAD(m_orientation));
      const float s = sinf(ANGTORAD(m_orientation));
      m_tablex = 100.0f * (headX - c * w + s * h);
      m_tabley = 100.0f * (headY + s * w + c * h);
   #endif
   m_tableWorldDirty = true;
}

void VRDevice::SaveVRSettings(Settings& settings) const
{
#if defined(ENABLE_VR) || defined(ENABLE_XR)
   settings.SaveValue(Settings::PlayerVR, "Slope"s, m_slope);
   settings.SaveValue(Settings::PlayerVR, "Orientation"s, m_orientation);
   settings.SaveValue(Settings::PlayerVR, "TableX"s, m_tablex);
   settings.SaveValue(Settings::PlayerVR, "TableY"s, m_tabley);
   settings.SaveValue(Settings::PlayerVR, "TableZ"s, m_tablez);
#endif
}
