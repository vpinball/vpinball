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
   VRDevice(const Settings& settings);
   ~VRDevice();

   unsigned int GetEyeWidth() const { return m_eyeWidth; }
   unsigned int GetEyeHeight() const { return m_eyeHeight; }
   
   float GetLockbarWidth() const { return m_lockbarWidth; }
   void SetLockbarWidth(float width) { m_lockbarWidth = width; m_worldDirty = true; }

   void OffsetTable(float dx, float dy, float dz);
   void RecenterTable();
   float GetSceneOrientation() const { return m_orientation; }
   const Vertex3Ds& GetSceneOffset() const { return m_tablePos; }
   void SetSceneOrientation(float orientation) { m_orientation = orientation; m_worldDirty = true; }
   void SetSceneOffset(const Vertex3Ds& pos) { m_tablePos = pos; m_worldDirty = true; }
   void SaveVRSettings(Settings& settings) const;

   void UpdateVRPosition(PartGroupData::SpaceReference spaceRef, ModelViewProj& mvp);

#ifndef ENABLE_XR
   float GetPredictedDisplayDelayInS() const { return 0.f; } // Unsupported as OpenVR is planned for deprecation and removal
#endif

private:
   unsigned int m_eyeWidth = 1080;
   unsigned int m_eyeHeight = 1020;

   float m_scale = 1.0f;
   float m_lockbarWidth = 57.0f; // Real world width of the lockbar in cm
#ifdef ENABLE_BGFX
   float m_lockbarHeight = 85.0f; // Real world height (from ground) of the lockbar in cm
#endif
   float m_orientation = 0.0f;
   Vertex3Ds m_tablePos;
   float m_slope = 0.0f;

   bool m_worldDirty = true;
   Matrix3D m_pfWorld;
   Matrix3D m_pfMatView;
   Matrix3D m_pfMatProj[2];
   Matrix3D m_cabWorld;
   Matrix3D m_cabMatView;
   Matrix3D m_cabMatProj[2];
   Matrix3D m_feetWorld;
   Matrix3D m_feetMatView;
   Matrix3D m_feetMatProj[2];
   Matrix3D m_roomWorld;
   Matrix3D m_roomMatView;
   Matrix3D m_roomMatProj[2];

#ifdef ENABLE_VR
public:
   static bool IsVRinstalled();
   static bool IsVRturnedOn();
   bool IsVRReady() const;
   void SubmitFrame(const std::shared_ptr<Sampler>& leftEye, const std::shared_ptr<Sampler>& rightEye);

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
   void ReleaseSession();
   void* GetGraphicContext() const;
   bgfx::RendererType::Enum GetGraphicContextType() const;
   void PollEvents();
   void RenderFrame(class RenderDevice* rd, std::function<void(RenderTarget* vrRenderTarget)> submitFrame);
   void UpdateVisibilityMask(class RenderDevice* rd);
   bool UseDepthBuffer() const { return m_depthExtensionSupported; }
   bgfx::TextureFormat::Enum GetDepthFormat() const { return m_depthSwapchainInfo.format; }

   void DiscardVisibilityMask() { m_visibilityMask = nullptr; }
   std::shared_ptr<MeshBuffer> GetVisibilityMask() const { return m_visibilityMask; }

   float GetPredictedDisplayDelayInS() const { return m_predictedDisplayDelayInS; }

   Matrix3D* GetVisibilityMaskProjs() { return &m_nextProj[0]; }

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

   XrSpace m_referenceSpace = XR_NULL_HANDLE;
   struct RenderLayerInfo
   {
      XrTime predictedDisplayTime = 0;
      std::vector<XrCompositionLayerBaseHeader*> layers;
      XrCompositionLayerProjection layerProjection = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
      std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
      std::vector<XrCompositionLayerDepthInfoKHR> depthInfoViews;
      XrCompositionLayerPassthroughFB layerPassthrough = { XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB };
   };

   bool m_depthExtensionSupported = false;

   bool m_colorSpaceExtensionSupported = false;

   bool m_win32PerfCounterExtensionSupported = false;
   float m_predictedDisplayDelayInS = 0.f;
   Matrix3D m_nextMedianView;
   Matrix3D m_nextView[2];
   Matrix3D m_nextProj[2];

   bool m_debugUtilsExtensionSupported = false;
   XrDebugUtilsMessengerEXT m_debugUtilsMessenger = XR_NULL_HANDLE;
   static XrBool32 OpenXRMessageCallbackFunction(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageType, const XrDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

   bool m_visibilityMaskExtensionSupported = false;
   PFN_xrGetVisibilityMaskKHR xrGetVisibilityMaskKHR;
   bool m_visibilityMaskDirty = true;
   std::shared_ptr<MeshBuffer> m_visibilityMask;

   bool m_passthroughExtensionSupported = false;
   XrPassthroughFB m_passthrough = XR_NULL_HANDLE;
   XrPassthroughLayerFB m_passthroughLayer = XR_NULL_HANDLE;
   bool m_passthroughEnabled = false;

   bgfx::RendererType::Enum m_rendererType;
   std::unique_ptr<class XRGraphicBackend> m_backend;

   bool m_recenterTable = false;
   float m_sceneSize = 0.f;
#endif
};
