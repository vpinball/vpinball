// license:GPLv3+

#pragma once

#ifdef ENABLE_VR
   #include <openvr.h>
#elif defined(ENABLE_XR)
   #include <openxr/openxr.h>
#endif

class VRDevice final
{
public:
   VRDevice();
   ~VRDevice();

   int GetEyeWidth() const { return m_eyeWidth; }
   int GetEyeHeight() const { return m_eyeHeight; }
   void UpdateVRPosition(ModelViewProj& mvp);

   void TableUp();
   void TableDown();
   void RecenterTable();
   void SaveVRSettings(Settings& settings) const;

private:
   int m_eyeWidth = 1080;
   int m_eyeHeight = 1020;
   float m_slope, m_orientation, m_tablex, m_tabley, m_tablez;
   Matrix3D m_vrMatView;
   Matrix3D m_vrMatProj[2];
   Matrix3D m_tableWorld;
   bool m_tableWorldDirty = true;

#ifdef ENABLE_VR
public:
   static bool IsVRinstalled();
   static bool IsVRturnedOn();
   bool IsVRReady() const;
   void SubmitFrame(Sampler* leftEye, Sampler* rightEye);

private:
   float m_scale = 1.0f;
   static vr::IVRSystem* m_pHMD;
   vr::TrackedDevicePose_t m_hmdPosition;
   vr::TrackedDevicePose_t* m_rTrackedDevicePose = nullptr;
#endif

#ifdef ENABLE_XR
public:
   bool IsOpenXRReady() const { return m_xrInstance != XR_NULL_HANDLE; }
   void SetupHMD();
   bool IsOpenXRHMDReady() const { return m_systemID != XR_NULL_SYSTEM_ID; }
   void CreateSession();
   void ReleaseSession();
   void* GetGraphicContext() const;
   void PollEvents();
   void RenderFrame(class RenderDevice* rd, std::function<void(RenderTarget* vrRenderTarget)> submitFrame);
   void UpdateVisibilityMask(class RenderDevice* rd);
   bool UseDepthBuffer() const { return m_depthExtensionSupported; }
   bgfx::TextureFormat::Enum GetDepthFormat() const { return m_depthSwapchainInfo.format; };

   void DiscardVisibilityMask() { delete m_visibilityMask; m_visibilityMask = nullptr; };
   MeshBuffer* GetVisibilityMask() const { return m_visibilityMask; };
   Matrix3D m_visibilityMaskProj[2];

   enum class SwapchainType : uint8_t
   {
      COLOR,
      DEPTH
   };

   struct SwapchainInfo
   {
      XrSwapchain swapchain = XR_NULL_HANDLE;
      uint32_t width = 0;
      uint32_t height = 0;
      uint32_t arraySize = 0;
      bool isDepth = false;
      int64_t backendFormat = 0;
      bgfx::TextureFormat::Enum format;
      std::vector<bgfx::TextureHandle> imageViews;
   };

private:
   XrInstance m_xrInstance = XR_NULL_HANDLE;
   std::vector<const char*> m_activeAPILayers;
   std::vector<const char*> m_activeInstanceExtensions;
   std::vector<std::string> m_apiLayers;

   XrFormFactor m_formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
   XrSystemId m_systemID = {};
   XrSystemProperties m_systemProperties = { XR_TYPE_SYSTEM_PROPERTIES };

   XrSession m_session = {};
   XrSessionState m_sessionState = XR_SESSION_STATE_UNKNOWN;
   bool m_applicationRunning = true;
   bool m_sessionRunning = false;

   std::vector<XrViewConfigurationType> m_applicationViewConfigurations = { XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO };
   std::vector<XrViewConfigurationType> m_viewConfigurations;
   XrViewConfigurationType m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
   std::vector<XrViewConfigurationView> m_viewConfigurationViews;

   SwapchainInfo m_colorSwapchainInfo = {};
   SwapchainInfo m_depthSwapchainInfo = {};
   std::vector<RenderTarget*> m_swapchainRenderTargets = {};
   std::vector<XrEnvironmentBlendMode> m_applicationEnvironmentBlendModes = { XR_ENVIRONMENT_BLEND_MODE_OPAQUE, XR_ENVIRONMENT_BLEND_MODE_ADDITIVE };
   std::vector<XrEnvironmentBlendMode> m_environmentBlendModes = {};
   XrEnvironmentBlendMode m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;

   XrSpace m_localSpace = XR_NULL_HANDLE;
   struct RenderLayerInfo
   {
      XrTime predictedDisplayTime = 0;
      std::vector<XrCompositionLayerBaseHeader*> layers;
      XrCompositionLayerProjection layerProjection = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
      std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
      std::vector<XrCompositionLayerDepthInfoKHR> depthInfoViews;
   };
   static Matrix3D XRPoseToMatrix3D(const XrPosef& pose);

   bool m_depthExtensionSupported = false;

   bool m_colorSpaceExtensionSupported = false;

   bool m_debugUtilsExtensionSupported = false;
   XrDebugUtilsMessengerEXT m_debugUtilsMessenger = XR_NULL_HANDLE;
   static XrBool32 OpenXRMessageCallbackFunction(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageType, const XrDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

   bool m_visibilityMaskExtensionSupported = false;
   PFN_xrGetVisibilityMaskKHR xrGetVisibilityMaskKHR;
   bool m_visibilityMaskDirty = true;
   MeshBuffer* m_visibilityMask = nullptr;

   class XRGraphicBackend* m_backend = nullptr;

   float m_sceneSize = 0.f;
#endif
};
