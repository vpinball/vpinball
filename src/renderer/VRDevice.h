// license:GPLv3+

#pragma once

#if defined(ENABLE_XR)
   #if defined(__STANDALONE__) && !defined(_MSC_VER) // wine's headers (non MSVC standalone builds) define _WIN64 on any 64 bit target
   #pragma push_macro("_WIN64")
   #undef _WIN64
   #endif
   #include "bx/platform.h"
   #if defined(__STANDALONE__) && !defined(_MSC_VER)
   #pragma pop_macro("_WIN64")
   #endif

   #if BX_PLATFORM_WINDOWS
      #define XR_USE_PLATFORM_WIN32
      #define XR_USE_GRAPHICS_API_VULKAN
      #define XR_USE_GRAPHICS_API_OPENGL
      #define XR_USE_GRAPHICS_API_OPENGL_ES
      #define XR_USE_GRAPHICS_API_D3D11
      #define XR_USE_GRAPHICS_API_D3D12
      #define VK_USE_PLATFORM_WIN32_KHR
   #elif BX_PLATFORM_ANDROID
      #define XR_USE_TIMESPEC
      #define XR_USE_PLATFORM_ANDROID
      #define XR_USE_GRAPHICS_API_VULKAN
      //#define XR_USE_GRAPHICS_API_OPENGL_ES
   #elif BX_PLATFORM_LINUX
      #define XR_USE_TIMESPEC
      #define XR_USE_GRAPHICS_API_VULKAN
   #endif


   // OpenXR Dependencies

   #ifdef XR_USE_PLATFORM_ANDROID
   #include <android/native_window.h>
   #include <android/window.h>
   #include <android/native_window_jni.h>
   #endif  // XR_USE_PLATFORM_ANDROID

   #ifdef XR_USE_PLATFORM_WIN32

   #include <winapifamily.h>
   #if !(WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM))
   // Enable desktop partition APIs, such as RegOpenKeyEx, LoadLibraryEx, PathFileExists etc.
   #undef WINAPI_PARTITION_DESKTOP
   #define WINAPI_PARTITION_DESKTOP 1
   #endif

   #ifndef NOMINMAX
   #define NOMINMAX
   #endif  // !NOMINMAX

   #ifndef WIN32_LEAN_AND_MEAN
   #define WIN32_LEAN_AND_MEAN
   #endif  // !WIN32_LEAN_AND_MEAN

   #include <windows.h>
   #include <unknwn.h>

   #endif  // XR_USE_PLATFORM_WIN32

   #ifdef XR_USE_GRAPHICS_API_D3D11
   #include <d3d11.h>
   #endif  // XR_USE_GRAPHICS_API_D3D11

   #ifdef XR_USE_GRAPHICS_API_D3D12
   #include <d3d12.h>
   #endif  // XR_USE_GRAPHICS_API_D3D12

   #ifdef XR_USE_PLATFORM_XLIB
   #include <X11/Xlib.h>
   #include <X11/Xutil.h>
   #endif  // XR_USE_PLATFORM_XLIB

   #ifdef XR_USE_PLATFORM_XCB
   #include <xcb/xcb.h>
   #endif  // XR_USE_PLATFORM_XCB

   #ifdef XR_USE_GRAPHICS_API_OPENGL
   #if defined(XR_USE_PLATFORM_XLIB) || defined(XR_USE_PLATFORM_XCB)
   #include <GL/glx.h>
   #endif  // (XR_USE_PLATFORM_XLIB || XR_USE_PLATFORM_XCB)
   #ifdef XR_USE_PLATFORM_XCB
   #include <xcb/glx.h>
   #endif  // XR_USE_PLATFORM_XCB
   #ifdef XR_USE_PLATFORM_MACOS
   #include <OpenCL/cl_gl_ext.h>
   #endif  // XR_USE_PLATFORM_MACOS
   #endif  // XR_USE_GRAPHICS_API_OPENGL

   #ifdef XR_USE_GRAPHICS_API_OPENGL_ES
   #include <SDL3/SDL_egl.h>
   #endif  // XR_USE_GRAPHICS_API_OPENGL_ES

   #ifdef XR_USE_GRAPHICS_API_VULKAN
   #include <vulkan/vulkan.h>
   #ifdef XR_USE_PLATFORM_ANDROID
   #include <vulkan/vulkan_android.h>
   #endif  // XR_USE_PLATFORM_ANDROID
   #endif  // XR_USE_GRAPHICS_API_VULKAN

   #ifdef XR_USE_PLATFORM_WAYLAND
   #include "wayland-client.h"
   #endif  // XR_USE_PLATFORM_WAYLAND

   #ifdef XR_USE_PLATFORM_EGL
   #include <EGL/egl.h>
   #endif  // XR_USE_PLATFORM_EGL

   #if defined(XR_USE_PLATFORM_XLIB) || defined(XR_USE_PLATFORM_XCB)
   #ifdef Success
   #undef Success
   #endif  // Success

   #ifdef Always
   #undef Always
   #endif  // Always

   #ifdef None
   #undef None
   #endif  // None
   #endif // defined(XR_USE_PLATFORM_XLIB) || defined(XR_USE_PLATFORM_XCB)

   #include <openxr/openxr.h>
   #include <openxr/openxr_platform.h>

   #include "input/XRInputHandler.h"
#endif

#include "math/matrix.h"
#include "parts/PartGroup.h"

class MeshBuffer;

class VRDevice final
{
public:
   VRDevice(const Settings& settings);
   ~VRDevice();

   unsigned int GetEyeWidth() const { return m_eyeWidth; }
   unsigned int GetEyeHeight() const { return m_eyeHeight; }

   bool IsOverridingLockbarWidth() const { return m_overrideLockbarWidth; }
   void SetOverrideLockbarWidth(bool overrideWidth)
   {
      m_overrideLockbarWidth = overrideWidth;
      m_worldDirty = true;
   }
   float GetCustomLockbarWidth() const { return m_customLockbarWidth; }
   void SetCustomLockbarWidth(float width)
   {
      m_customLockbarWidth = width;
      m_worldDirty = true;
   }
   bool IsOverridingLockbarHeight() const { return m_overrideLockbarHeight; }
   void SetOverrideLockbarHeight(bool overrideHeight)
   {
      m_overrideLockbarHeight = overrideHeight;
      m_worldDirty = true;
   }
   float GetCustomLockbarHeight() const { return m_customLockbarHeight; }
   void SetCustomLockbarHeight(float height)
   {
      m_customLockbarHeight = height;
      m_worldDirty = true;
   }
   float GetControllerCabYOffset() const { return m_controllerCabYOffset; }
   void SetControllerCabYOffset(float offset) { m_controllerCabYOffset = offset; }
   float GetControllerCabZOffset() const { return m_controllerCabZOffset; }
   void SetControllerCabZOffset(float offset) { m_controllerCabZOffset = offset; }
   bool IsLockFeetToGround() const { return m_lockFeetToGround; }
   void SetLockFeetToGround(bool lock)
   {
      m_lockFeetToGround = lock;
      if (lock)
         m_seatedTilt = 0.f;
      m_worldDirty = true;
   }
   float GetSeatedTilt() const { return m_seatedTilt; }
   void SetSeatedTilt(float tilt)
   {
      m_seatedTilt = tilt;
      if (tilt != 0.f)
         m_lockFeetToGround = false;
      m_worldDirty = true;
   }

   void OffsetRoom(float dx, float dy, float dz);
   void RecenterRoom();
   float GetRoomYaw() const { return m_roomYaw; }
   const Vertex3Ds& GetRoomOffset() const { return m_roomOffset; }
   const Vertex3Ds& GetTableOffset() const { return m_tableOffset; }
   void SetRoomYaw(float yaw)
   {
      m_roomYaw = yaw;
      m_worldDirty = true;
   }
   void SetRoomOffset(const Vertex3Ds& pos)
   {
      m_roomOffset = pos;
      m_worldDirty = true;
   }
   void SetTableOffset(const Vertex3Ds& pos)
   {
      m_tableOffset = pos;
      m_worldDirty = true;
   }
   void SaveVRSettings(Settings& settings) const;

   void UpdateVRPosition(PartGroupData::SpaceReference spaceRef, ModelViewProj& mvp);

   float GetPredictedDisplayTimestamp() const { return m_predictedDisplayTimestamp; }

   void SetShowUILayer(bool show) { m_showUILayer = show; }

private:
   unsigned int m_eyeWidth = 1080;
   unsigned int m_eyeHeight = 1020;

   float m_cabinetScale = 1.0f;
   bool m_overrideLockbarWidth = true; // Scale virtual cabinet so that its lockbar matches the user defined lockbar width
   float m_customLockbarWidth = 57.0f; // User defined width of the (real world cabinet) lockbar in cm
   bool m_overrideLockbarHeight = true; // Position virtual cabinet so that its lockbar matches the user defined lockbar height
   float m_customLockbarHeight = 85.0f; // User defined height (from ground) of the (real world cabinet) lockbar in cm
   float m_controllerCabYOffset = 0.0f; // Y offset applied when using controller view centering in cm
   float m_controllerCabZOffset = 0.0f; // Z offset applied when using controller view centering in cm
   bool m_lockFeetToGround = true;
   float m_seatedTilt = 0.0f; // Cabinet tilt toward the player for seated/reclined play, in degrees
   float m_roomYaw = 0.0f;
   Vertex3Ds m_roomOffset { 0.f, 0.f, 0.f }; // Room positioning correction in cm, to account for imprecise headset tracking calibration
   Vertex3Ds m_tableOffset { 0.f, 0.f, 0.f }; // Per-table adjustment in cm (y: playfield to lockbar end distance, z: glass height)
   float m_slope = 0.0f;

   float m_predictedDisplayTimestamp = 0.f;

   bool m_worldDirty = true;
   struct Viewpoint
   {
      Matrix3D m_toWorld; // Matrix to transform from this viewpoint to world coordinates
      Matrix3D m_view[2];
   };
   Viewpoint m_spaces[4]; // Indexed by PartGroupData::SpaceReference (SR_PLAYFIELD..SR_ROOM)
   Matrix3D m_roomProj[2];
   Matrix3D m_sceneProj[2];

   bool m_showUILayer = false;

#ifdef ENABLE_XR
public:
   int GetDisplayRefreshRateMode() const { return m_displayRefreshRateMode; }
   void SetDisplayRefreshRateMode(int mode);
   bool IsOpenXRReady() const { return m_xrInstance != XR_NULL_HANDLE; }
   void SetupHMD();
   bool IsOpenXRHMDReady() const { return m_systemID != XR_NULL_SYSTEM_ID; }
   void CreateSession();
   void ReleaseSession();
   void* GetGraphicContext() const;
   void* GetGraphicPlatformQueue() const;
   bgfx::RendererType::Enum GetGraphicContextType() const;
   void PollEvents();
   void RenderFrame(class RenderDevice* rd, const std::function<void(RenderTarget* vrRenderTarget)>& submitFrame);
   RenderTarget* GetUIRenderTarget() const { return m_uiRenderTarget; } // Acquired swapchain target for the UI composition layer (or nullptr if unsupported/not acquired yet)
   void UpdateVisibilityMask(class RenderDevice* rd);
   bool UseDepthBuffer() const { return m_depthExtensionSupported; }
   bgfx::TextureFormat::Enum GetDepthFormat() const { return m_depthSwapchainInfo.format; }

   void DiscardVisibilityMask() { m_visibilityMask = nullptr; }
   std::shared_ptr<MeshBuffer> GetVisibilityMask() const { return m_visibilityMask; }

   Matrix3D* GetVisibilityMaskProjs() { return &m_nextProj[0]; }

   void EnableControllerRecenter(bool enable) { m_controllerRecenterEnabled = enable; }
   bool IsControllerRecenterEnabled() const { return m_controllerRecenterEnabled; }

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
   SwapchainInfo m_uiSwapchainInfo = {};
   std::vector<std::unique_ptr<RenderTarget>> m_swapchainRenderTargets;
   std::vector<std::unique_ptr<RenderTarget>> m_uiRenderTargets;
   RenderTarget* m_uiRenderTarget = nullptr; // Written by the render thread before releasing the frame semaphore, read by the logic thread while recording the frame
   std::vector<XrEnvironmentBlendMode> m_applicationEnvironmentBlendModes = { XR_ENVIRONMENT_BLEND_MODE_OPAQUE, XR_ENVIRONMENT_BLEND_MODE_ADDITIVE };
   std::vector<XrEnvironmentBlendMode> m_environmentBlendModes = {};
   XrEnvironmentBlendMode m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;

   XrSpace m_referenceSpace = XR_NULL_HANDLE;
   XrSpace m_viewSpace = XR_NULL_HANDLE; // Head-locked space for the UI quad layer

   bool m_headsetRecenterPending = false;
   bool m_controllerRecenterEnabled = false;
   XrSpace m_leftControllerSpace = XR_NULL_HANDLE;
   XrSpace m_rightControllerSpace = XR_NULL_HANDLE;

   struct RenderLayerInfo
   {
      XrTime predictedDisplayTime = 0;
      std::vector<XrCompositionLayerBaseHeader*> layers;
      XrCompositionLayerProjection layerProjection = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
      std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
      std::vector<XrCompositionLayerDepthInfoKHR> depthInfoViews;
      XrCompositionLayerQuad layerQuad = { XR_TYPE_COMPOSITION_LAYER_QUAD };
      XrCompositionLayerPassthroughFB layerPassthrough = { XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB };
   };

   bool m_depthExtensionSupported = false;
   bool m_colorSpaceExtensionSupported = false;
   #if BX_PLATFORM_WINDOWS
   bool m_win32PerfCounterExtensionSupported = false;
   PFN_xrConvertTimeToWin32PerformanceCounterKHR m_xrConvertTimeToWin32PerformanceCounterKHR = nullptr;
   #elif BX_PLATFORM_ANDROID || BX_PLATFORM_LINUX
   bool m_convertTimespecTimeExtensionSupported = false;
   PFN_xrConvertTimeToTimespecTimeKHR m_xrConvertTimeToTimespecTimeKHR = nullptr;
   #endif
   bool m_displayRefreshRateExtensionSupported = false;
   PFN_xrGetDisplayRefreshRateFB m_xrGetDisplayRefreshRateFB = nullptr;
   PFN_xrRequestDisplayRefreshRateFB m_xrRequestDisplayRefreshRateFB = nullptr;
   int m_displayRefreshRateMode = 0;
   void ApplyDisplayRefreshRate();

   Matrix3D m_nextProj[2];

   bool m_debugUtilsExtensionSupported = false;
   XrDebugUtilsMessengerEXT m_debugUtilsMessenger = XR_NULL_HANDLE;
   static XrBool32 OpenXRMessageCallbackFunction(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageType, const XrDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

   bool m_visibilityMaskExtensionSupported = false;
   PFN_xrGetVisibilityMaskKHR xrGetVisibilityMaskKHR = nullptr;
   bool m_visibilityMaskDirty = true;
   std::shared_ptr<MeshBuffer> m_visibilityMask;

   bool m_passthroughExtensionSupported = false;
   XrPassthroughFB m_passthrough = XR_NULL_HANDLE;
   XrPassthroughLayerFB m_passthroughLayer = XR_NULL_HANDLE;
   bool m_passthroughEnabled = false;

   bgfx::RendererType::Enum m_rendererType;
   std::unique_ptr<class XRGraphicBackend> m_backend;

   float m_sceneSize = 0.f;

   XRInputHandler* m_xrInputHandler = nullptr;
#endif
};
