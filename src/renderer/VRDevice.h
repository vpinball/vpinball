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
   float m_scale = 1.0f;
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
   void* GetGraphicContext() const;
   void* GetSwapChainBackBuffer(const int index, const bool isDepth) const;
   void PollEvents();
   void RenderFrame(std::function<void(bool renderVR)> submitFrame);

   enum class SwapchainType : uint8_t
   {
      COLOR,
      DEPTH
   };

private:
   XrInstance m_xrInstance = XR_NULL_HANDLE;
   std::vector<const char*> m_activeAPILayers = {};
   std::vector<const char*> m_activeInstanceExtensions = {};
   std::vector<std::string> m_apiLayers = {};

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

   struct SwapchainInfo
   {
      XrSwapchain swapchain = XR_NULL_HANDLE;
      int64_t swapchainFormat = 0;
      std::vector<void*> imageViews;
      uint32_t width { 0 };
      uint32_t height { 0 };
      uint32_t arraySize { 0 };
   };
   SwapchainInfo m_colorSwapchainInfo = {};
   SwapchainInfo m_depthSwapchainInfo = {};
   SwapchainInfo CreateSwapChain(XrSession session, class D3D11Backend* backend, SwapchainType type, int64_t format, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t sampleCount, XrSwapchainCreateFlags createFlags, XrSwapchainUsageFlags usageFlags);
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
   static Matrix3D XRPoseToMatrix3D(XrPosef pose);

   bool m_depthExtensionSupported = false;
   bool m_colorSpaceExtensionSupported = false;

   class D3D11Backend* m_backend = nullptr;
#endif
};
