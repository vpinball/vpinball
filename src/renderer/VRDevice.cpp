// license:GPLv3+

#include "core/stdafx.h"
#include "VRDevice.h"
#include "core/vpversion.h"

#include "parts/primitive.h"

// MSVC Concurrency Viewer support
// This requires to add the MSVC Concurrency SDK to the project
//#define MSVC_CONCURRENCY_VIEWER
#ifdef MSVC_CONCURRENCY_VIEWER
#include <cvmarkersobj.h>
using namespace Concurrency::diagnostic;
extern marker_series series;
#endif

#if defined(ENABLE_XR) && defined(ENABLE_BGFX)
   #include "bgfx/platform.h"
   #include "bgfx/bgfx.h"
   #include "bx/math.h"
   #include "bx/os.h"
   #include <map>
   #include <vector>
   #include <time.h>

static inline const char* GetXRErrorString(XrInstance xrInstance, XrResult result)
{
   static char string[XR_MAX_RESULT_STRING_SIZE];
   xrResultToString(xrInstance, result, string);
   return string;
}

#define OPENXR_CHECK(x, y)                                                                                                                                                                   \
   {                                                                                                                                                                                         \
      if (const XrResult res = (x); !XR_SUCCEEDED(res))                                                                                                                                      \
      {                                                                                                                                                                                      \
         PLOGE << "ERROR: OPENXR: " << int(res) << " (" << (m_xrInstance ? GetXRErrorString(m_xrInstance, res) : "") << ") " << (y);                                                         \
      }                                                                                                                                                                                      \
   }

inline static float XrRcpSqrt(const float x)
{
   constexpr float SMALLEST_NON_DENORMAL = FLT_MIN; // ( 1U << 23 )
   const float rcp = (x >= SMALLEST_NON_DENORMAL) ? 1.0f / sqrtf(x) : 1.0f;
   return rcp;
}

inline static void XrVector3f_Lerp(XrVector3f* const result, const XrVector3f* const a, const XrVector3f* const b, const float fraction) {
   result->x = a->x + fraction * (b->x - a->x);
   result->y = a->y + fraction * (b->y - a->y);
   result->z = a->z + fraction * (b->z - a->z);
}

inline static void XrVector3f_Scale(XrVector3f* const result, const XrVector3f* const a, const float scaleFactor) {
   result->x = a->x * scaleFactor;
   result->y = a->y * scaleFactor;
   result->z = a->z * scaleFactor;
}

inline static void XrQuaternionf_Lerp(XrQuaternionf* const result, const XrQuaternionf* const a, const XrQuaternionf* const b, const float fraction) {
   const float s = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
   const float fa = 1.0f - fraction;
   const float fb = (s < 0.0f) ? -fraction : fraction;
   const float x = a->x * fa + b->x * fb;
   const float y = a->y * fa + b->y * fb;
   const float z = a->z * fa + b->z * fb;
   const float w = a->w * fa + b->w * fb;
   const float lengthRcp = XrRcpSqrt(x * x + y * y + z * z + w * w);
   result->x = x * lengthRcp;
   result->y = y * lengthRcp;
   result->z = z * lengthRcp;
   result->w = w * lengthRcp;
}

inline static void XrPosef_ToMatrix3D(Matrix3D* result, const XrPosef* pose)
{
   const bx::Quaternion orientation(pose->orientation.x, pose->orientation.y, pose->orientation.z, pose->orientation.w);
   const bx::Quaternion invertOrientation = bx::invert(orientation);
   bx::mtxFromQuaternion(&result->m[0][0], invertOrientation);
   result->Transpose();
   const bx::Quaternion position(pose->position.x, pose->position.y, pose->position.z, 0.f);
   bx::Quaternion invertPosition = bx::invert(position);
   invertPosition = bx::mul(invertPosition, orientation);
   invertPosition = bx::mul(invertOrientation, invertPosition);
   result->m[3][0] += invertPosition.x;
   result->m[3][1] += invertPosition.y;
   result->m[3][2] += invertPosition.z;
}


#ifdef XR_USE_GRAPHICS_API_D3D11
#include "XRD3D11Backend.h"
#endif

#ifdef XR_USE_GRAPHICS_API_VULKAN
#include "XRVulkanBackend.h"
#endif

#endif



#ifdef ENABLE_VR
   vr::IVRSystem* VRDevice::m_pHMD = nullptr;
#endif



VRDevice::VRDevice(const Settings& settings)
{
      // Scene offset (vertical rotation and horizontal shift)
      m_orientation = settings.GetPlayerVR_Orientation();
      m_tablePos.x = settings.GetPlayerVR_TableX();
      m_tablePos.y = settings.GetPlayerVR_TableY();
      // Offset of the playfield from the room ground is defined as an offset from the lockbar, minus bottom glass height and custom adjustment
      // (Note that for OpenVR offset is defined from the ground)
      m_tablePos.z = settings.GetPlayerVR_TableZ();
   #if defined(ENABLE_VR)
      m_slope = settings.GetPlayerVR_Slope();
   #endif
   m_worldDirty = true;

   #if defined(ENABLE_XR)
      // Relative scale factor and positioning
      m_lockbarWidth = settings.GetPlayer_LockbarWidth();
      m_lockbarHeight = settings.GetPlayer_LockbarHeight();

      // Fill out an XrApplicationInfo structure detailing the names and OpenXR version.
      // The application/engine name and version are user-defined. These may help IHVs or runtimes.
      XrApplicationInfo AI;
      strncpy_s(AI.applicationName, XR_MAX_APPLICATION_NAME_SIZE, "Visual Pinball X");
      constexpr uint32_t ver = (VP_VERSION_MAJOR / 10) << 12 | (VP_VERSION_MAJOR % 10) << 8 | (VP_VERSION_MINOR) << 4 | VP_VERSION_REV; // e.g. 0x1081 for 10.8.1
      AI.applicationVersion = ver;
      strncpy_s(AI.engineName, XR_MAX_ENGINE_NAME_SIZE, "");
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
      for (const auto& requestLayer : m_apiLayers)
      {
         for (const auto& layerProperty : apiLayerProperties)
         {
            if (requestLayer == layerProperty.layerName)
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
         for (const auto& extensionProperty : extensionProperties)
         {
            PLOGD << "OpenXR supported extension: " << extensionProperty.extensionName << ", version " << extensionProperty.extensionVersion;
         }
      #endif
      m_activeInstanceExtensions.clear();
      // Add a specific extension to the list of extensions to be enabled, if it is supported.
      auto EnableExtensionIfSupported = [&](const char* extensionName)
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
      // VRDevice is created before bgfx initialization (since it creates the graphic context expected by OpenXR), so bgfx::getRendererType() is not defined at this point.
      // Renderer is determined at compile time based on platform: D3D11 for Windows, Vulkan for Android.
      #if BX_PLATFORM_WINDOWS
         m_rendererType = bgfx::RendererType::Enum::Direct3D11;
         const string gfxBackend = g_pplayer->m_ptable->m_settings.GetPlayer_GfxBackend();
         #ifdef _DEBUG
         if (gfxBackend == "Vulkan"sv)
            m_rendererType = bgfx::RendererType::Enum::Vulkan;
         #else
         if (gfxBackend != "Direct3D11"sv && gfxBackend != "Default"sv)
         {
            PLOGI << "Renderer backend enforced to Direct3D11 as other backends are still experimental and not enabled in release builds";
         }
         #endif
      #elif BX_PLATFORM_ANDROID
         m_rendererType = bgfx::RendererType::Enum::Vulkan;
      #else
         #error "Unsupported platform for OpenXR"
      #endif
      bool hasGraphicBackend = false;
      switch (m_rendererType)
      {
      #ifdef XR_USE_GRAPHICS_API_VULKAN
         case bgfx::RendererType::Enum::Vulkan:
            // According to https://github.khronos.org/OpenXR-Inventory/runtime_extension_support.html all runtimes that support XR_KHR_VULKAN_ENABLE_EXTENSION_NAME do support XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME
            hasGraphicBackend = EnableExtensionIfSupported(XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME);
            break;
      #endif
      #ifdef XR_USE_GRAPHICS_API_D3D11
         case bgfx::RendererType::Enum::Direct3D11: hasGraphicBackend = EnableExtensionIfSupported(XR_KHR_D3D11_ENABLE_EXTENSION_NAME); break;
      #endif
      }
      assert(hasGraphicBackend);
      if (!hasGraphicBackend)
         return;

      m_depthExtensionSupported = EnableExtensionIfSupported(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
      m_colorSpaceExtensionSupported = EnableExtensionIfSupported(XR_FB_COLOR_SPACE_EXTENSION_NAME);
      m_visibilityMaskExtensionSupported = EnableExtensionIfSupported(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME);
      #if BX_PLATFORM_WINDOWS
         m_win32PerfCounterExtensionSupported = EnableExtensionIfSupported(XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME);
      #elif BX_PLATFORM_ANDROID
         m_convertTimespecTimeExtensionSupported = EnableExtensionIfSupported(XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME);
      #endif
      m_passthroughExtensionSupported = EnableExtensionIfSupported(XR_FB_PASSTHROUGH_EXTENSION_NAME);
      #ifdef DEBUG
         m_debugUtilsExtensionSupported = EnableExtensionIfSupported(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
      #endif

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

      #if BX_PLATFORM_WINDOWS
      if (m_win32PerfCounterExtensionSupported)
      {
         OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrConvertTimeToWin32PerformanceCounterKHR", (PFN_xrVoidFunction*)&m_xrConvertTimeToWin32PerformanceCounterKHR),
            "Failed to get xrConvertTimeToWin32PerformanceCounterKHR.");
      }
      #elif BX_PLATFORM_ANDROID
      if (m_convertTimespecTimeExtensionSupported)
      {
         OPENXR_CHECK(
            xrGetInstanceProcAddr(m_xrInstance, "xrConvertTimeToTimespecTimeKHR", (PFN_xrVoidFunction*)&m_xrConvertTimeToTimespecTimeKHR),
            "Failed to get xrConvertTimeToTimespecTimeKHR.");
      }
      #endif
      if (m_visibilityMaskExtensionSupported)
      {
         OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetVisibilityMaskKHR", (PFN_xrVoidFunction*)&xrGetVisibilityMaskKHR), "Failed to get xrGetVisibilityMaskKHR.");
      }
      if (m_debugUtilsExtensionSupported)
      {
         // Fill out a XrDebugUtilsMessengerCreateInfoEXT structure specifying all severities and types.
         // Set the userCallback to OpenXRMessageCallbackFunction().
         XrDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI { XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
         debugUtilsMessengerCI.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
         debugUtilsMessengerCI.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
         debugUtilsMessengerCI.userCallback = (PFN_xrDebugUtilsMessengerCallbackEXT)OpenXRMessageCallbackFunction;
         debugUtilsMessengerCI.userData = nullptr;

         // Load xrCreateDebugUtilsMessengerEXT() function pointer as it is not default loaded by the OpenXR loader.
         PFN_xrCreateDebugUtilsMessengerEXT xrCreateDebugUtilsMessengerEXT;
         OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)&xrCreateDebugUtilsMessengerEXT), "Failed to get InstanceProcAddr.");

         // Finally create and return the XrDebugUtilsMessengerEXT.
         OPENXR_CHECK(xrCreateDebugUtilsMessengerEXT(m_xrInstance, &debugUtilsMessengerCI, &m_debugUtilsMessenger), "Failed to create DebugUtilsMessenger.");
      }

      // Get the instance's properties and log the runtime name and version.
      XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
      OPENXR_CHECK(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get InstanceProperties.");
      PLOGI << "OpenXR Runtime: " << instanceProperties.runtimeName << " - "
                                 << XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << '.'
                                 << XR_VERSION_MINOR(instanceProperties.runtimeVersion) << '.'
                                 << XR_VERSION_PATCH(instanceProperties.runtimeVersion);
   #endif

   #if defined(ENABLE_VR)
      m_pHMD = nullptr;
      m_rTrackedDevicePose = nullptr;
      m_scale = 1.0f; // Scale factor from scene (in VP units) to VR view (in meters)
      if (settings.GetPlayerVR_ScaleToFixedWidth())
      {
         float width;
         g_pplayer->m_ptable->get_Width(&width);
         m_scale = settings.GetPlayerVR_ScaleAbsolute() * 0.01f / width;
      }
      else
         m_scale = VPUTOCM(0.01f) * g_pplayer->m_ptable->m_settings.GetPlayerVR_ScaleRelative();
      if (m_scale < VPUTOCM(0.01f))
         m_scale = VPUTOCM(0.01f); // Scale factor for VPUnits to Meters

      // Initialize VR, this will also override the render buffer size (m_width, m_height) to account for HMD render size and render the 2 eyes simultaneously
      vr::EVRInitError VRError = vr::VRInitError_None;
      if (!m_pHMD) {
         m_pHMD = vr::VR_Init(&VRError, vr::VRApplication_Scene);
         if (VRError != vr::VRInitError_None) {
            m_pHMD = nullptr;
            ShowError("Unable to init VR runtime: "s + vr::VR_GetVRInitErrorAsEnglishDescription(VRError));
         }
         else if (!vr::VRCompositor())
         /*if (VRError != vr::VRInitError_None)*/ {
            m_pHMD = nullptr;
            ShowError("Unable to init VR compositor"); // + vr::VR_GetVRInitErrorAsEnglishDescription(VRError))
         }
      }

      // Move from VP units to meters, and also apply user scene scaling if any
      Matrix3D sceneScale = Matrix3D::MatrixScale(m_scale);

      // Convert from VPX coords to VR (270deg rotation around X axis, and flip x axis)
      Matrix3D coords = Matrix3D::MatrixIdentity();
      coords._11 = -1.f; coords._12 = 0.f; coords._13 =  0.f;
      coords._21 =  0.f; coords._22 = 0.f; coords._23 = -1.f;
      coords._31 =  0.f; coords._32 = 1.f; coords._33 =  0.f;

      float zNear, zFar;
      g_pplayer->m_ptable->ComputeNearFarPlane(coords * sceneScale, m_scale, zNear, zFar);
      zNear = g_pplayer->m_ptable->m_settings.GetPlayerVR_NearPlane() / 100.0f; // Replace near value to allow player to move near parts up to user defined value
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
            m_pfMatProj[i] = coords * sceneScale * proj;
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

         m_pfMatProj[0] = coords * sceneScale * matEye2Head * matProjection;

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

         m_pfMatProj[1] = coords * sceneScale * matEye2Head * matProjection;
      }

      if (vr::k_unMaxTrackedDeviceCount > 0) {
         m_rTrackedDevicePose = new vr::TrackedDevicePose_t[vr::k_unMaxTrackedDeviceCount];
      }
      else {
         std::runtime_error noDevicesFound("No Tracking devices found");
         throw(noDevicesFound);
      }
   #endif
}

VRDevice::~VRDevice()
{
   #if defined(ENABLE_XR)
      // Destroy the reference XrSpace.
      OPENXR_CHECK(xrDestroySpace(m_referenceSpace), "Failed to destroy Space.")

      if (m_passthroughLayer != XR_NULL_HANDLE)
      {
         PFN_xrDestroyPassthroughLayerFB xrDestroyPassthroughLayerFB;
         OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrDestroyPassthroughLayerFB", (PFN_xrVoidFunction*)&xrDestroyPassthroughLayerFB), "Failed to get xrDestroyPassthroughLayerFB.");
         OPENXR_CHECK(xrDestroyPassthroughLayerFB(m_passthroughLayer), "Failed to destroy passthrough layer.");
         m_passthroughLayer = XR_NULL_HANDLE;
      }

      if (m_passthrough != XR_NULL_HANDLE)
      {
         PFN_xrDestroyPassthroughFB xrDestroyPassthroughFB;
         OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrDestroyPassthroughFB", (PFN_xrVoidFunction*)&xrDestroyPassthroughFB), "Failed to get xrDestroyPassthroughFB.");
         OPENXR_CHECK(xrDestroyPassthroughFB(m_passthrough), "Failed to destroy passthrough.");
         m_passthrough = XR_NULL_HANDLE;
      }

      // Destroy the XrSession.
      OPENXR_CHECK(xrDestroySession(m_session), "Failed to destroy Session.");

      m_backend = nullptr;

      if (m_debugUtilsExtensionSupported)
      {
         PFN_xrDestroyDebugUtilsMessengerEXT xrDestroyDebugUtilsMessengerEXT;
         OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrDestroyDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)&xrDestroyDebugUtilsMessengerEXT), "Failed to get InstanceProcAddr.");
         OPENXR_CHECK(xrDestroyDebugUtilsMessengerEXT(m_debugUtilsMessenger), "Failed to destroy DebugUtilsMessenger.");
      }

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

XrBool32 VRDevice::OpenXRMessageCallbackFunction(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageType, const XrDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
   // Lambda to covert an XrDebugUtilsMessageSeverityFlagsEXT to std::string. Bitwise check to concatenate multiple severities to the output string.
   auto GetMessageSeverityString = [](XrDebugUtilsMessageSeverityFlagsEXT messageSeverity) -> std::string
   {
      bool separator = false;

      std::string msgFlags;
      if (messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
      {
         msgFlags += "VERBOSE"sv;
         separator = true;
      }
      if (messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
      {
         if (separator)
            msgFlags += ',';
         msgFlags += "INFO"sv;
         separator = true;
      }
      if (messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
      {
         if (separator)
            msgFlags += ',';
         msgFlags += "WARN"sv;
         separator = true;
      }
      if (messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
      {
         if (separator)
            msgFlags += ',';
         msgFlags += "ERROR"sv;
      }
      return msgFlags;
   };
   // Lambda to covert an XrDebugUtilsMessageTypeFlagsEXT to std::string. Bitwise check to concatenate multiple types to the output string.
   auto GetMessageTypeString = [](XrDebugUtilsMessageTypeFlagsEXT messageType) -> std::string
   {
      bool separator = false;

      std::string msgFlags;
      if (messageType & XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
      {
         msgFlags += "GEN"sv;
         separator = true;
      }
      if (messageType & XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
      {
         if (separator)
            msgFlags += ',';
         msgFlags += "SPEC"sv;
         separator = true;
      }
      if (messageType & XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
      {
         if (separator)
            msgFlags += ',';
         msgFlags += "PERF"sv;
      }
      return msgFlags;
   };

   std::string functionName = (pCallbackData->functionName) ? pCallbackData->functionName : "";
   std::string messageSeverityStr = GetMessageSeverityString(messageSeverity);
   std::string messageTypeStr = GetMessageTypeString(messageType);
   std::string messageId = (pCallbackData->messageId) ? pCallbackData->messageId : "";
   std::string message = (pCallbackData->message) ? pCallbackData->message : "";
   std::stringstream errorMessage;
   errorMessage << functionName << '(' << messageSeverityStr << " / " << messageTypeStr << "): msgNum: " << messageId << " - " << message;
   if (messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
   {
      PLOGE << errorMessage.str();
      // assert(false);
   }
   else
   {
      PLOGI << errorMessage.str();
   }
   return XrBool32();
}

void* VRDevice::GetGraphicContext() const { return m_backend->GetGraphicContext(); }

bgfx::RendererType::Enum VRDevice::GetGraphicContextType() const { return m_backend->GetRendererType(); }

void VRDevice::SetupHMD()
{
   assert(m_backend == nullptr);
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
   XrSystemColorSpacePropertiesFB colorSpaceProperties { XR_TYPE_SYSTEM_COLOR_SPACE_PROPERTIES_FB };
   if (m_colorSpaceExtensionSupported)
      m_systemProperties.next = &colorSpaceProperties;
   OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, m_systemID, &m_systemProperties), "Failed to get SystemProperties.");
   if (m_colorSpaceExtensionSupported)
   {
      PLOGI << "Native XR device colorspace: " << colorSpaceProperties.colorSpace;

      // Set color space to get the same rendering on all HMD and usual desktop play
      PFN_xrEnumerateColorSpacesFB xrEnumerateColorSpacesFB;
      OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrEnumerateColorSpacesFB", (PFN_xrVoidFunction*)&xrEnumerateColorSpacesFB), "Failed to get xrEnumerateColorSpacesFB.");
      PFN_xrSetColorSpaceFB xrSetColorSpaceFB;
      OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrSetColorSpaceFB", (PFN_xrVoidFunction*)&xrSetColorSpaceFB), "Failed to get xrSetColorSpaceFB.");
      uint32_t colorSpaceCount;
      xrEnumerateColorSpacesFB(m_session, 0, &colorSpaceCount, nullptr);
      XrColorSpaceFB* colorSpaces = new XrColorSpaceFB[colorSpaceCount];
      xrEnumerateColorSpacesFB(m_session, colorSpaceCount, &colorSpaceCount, colorSpaces);
      for (uint32_t i = 0; i < colorSpaceCount; i++)
      {
         if (colorSpaces[i] == XR_COLOR_SPACE_REC709_FB)
         {
            xrSetColorSpaceFB(m_session, XR_COLOR_SPACE_REC709_FB);
            break;
         }
      }
   }

   // Gets the View Configuration Types. The first call gets the count of the array that will be returned. The next call fills out the array.
   uint32_t viewConfigurationCount = 0;
   OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, 0, &viewConfigurationCount, nullptr), "Failed to enumerate View Configurations.");
   m_viewConfigurations.resize(viewConfigurationCount);
   OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, viewConfigurationCount, &viewConfigurationCount, m_viewConfigurations.data()), "Failed to enumerate View Configurations.");

   // Pick the first application supported View Configuration Type con supported by the hardware.
   for (const XrViewConfigurationType& viewConfiguration : m_applicationViewConfigurations)
   {
      if (std::ranges::find(m_viewConfigurations.begin(), m_viewConfigurations.end(), viewConfiguration) != m_viewConfigurations.end())
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
      for (const auto environmentBlendMode : m_environmentBlendModes)
      {
         static const char* blendModeNames[] = { "Opaque", "Additive", "Alpha" };
         PLOGD << "OpenXR supported blend mode: " << (1 <= environmentBlendMode && environmentBlendMode < 4 ? blendModeNames[environmentBlendMode - 1] : std::to_string(environmentBlendMode).c_str());
      }
   #endif
   // Pick the first application supported blend mode supported by the hardware.
   for (const XrEnvironmentBlendMode& environmentBlendMode : m_applicationEnvironmentBlendModes)
   {
      if (std::ranges::find(m_environmentBlendModes.begin(), m_environmentBlendModes.end(), environmentBlendMode) != m_environmentBlendModes.end())
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

   // Since we are using texture array rendering, we need target to be stereo views with the same setup
   assert(m_viewConfigurationViews.size() == 2);
   assert(m_viewConfigurationViews[0].recommendedImageRectWidth == m_viewConfigurationViews[1].recommendedImageRectWidth);
   assert(m_viewConfigurationViews[0].recommendedImageRectHeight == m_viewConfigurationViews[1].recommendedImageRectHeight);
   assert(m_viewConfigurationViews[0].recommendedSwapchainSampleCount == m_viewConfigurationViews[1].recommendedSwapchainSampleCount);

   // Let the user choose the down/super sampling
   const float resFactor = g_pplayer ? g_pplayer->m_ptable->m_settings.GetPlayerVR_ResFactor() : -1.f;
   if (resFactor <= 0.1f || resFactor > 10.f)
   {
      m_eyeWidth = m_viewConfigurationViews[0].recommendedImageRectWidth;
      m_eyeHeight = m_viewConfigurationViews[0].recommendedImageRectHeight;
   }
   else
   {
      m_eyeWidth = static_cast<unsigned int>((float)m_viewConfigurationViews[0].maxImageRectWidth * resFactor);
      m_eyeHeight = static_cast<unsigned int>((float)m_viewConfigurationViews[0].maxImageRectHeight * resFactor);
   }
   // Limit to a resolution, under the maximum texture size supported by the GPU
   const bgfx::Caps* caps = bgfx::getCaps();
   if ((static_cast<uint32_t>(m_eyeWidth) >= caps->limits.maxTextureSize) || (static_cast<uint32_t>(m_eyeHeight) >= caps->limits.maxTextureSize))
   {
      PLOGI << "Requested resolution exceed the GPU capability, defaulting to headset recommended resolution";
      m_eyeWidth = m_viewConfigurationViews[0].recommendedImageRectWidth;
      m_eyeHeight = m_viewConfigurationViews[0].recommendedImageRectHeight;
   }
   PLOGI << "Headset recommended resolution: " << m_viewConfigurationViews[0].recommendedImageRectWidth << 'x' << m_viewConfigurationViews[0].recommendedImageRectHeight;
   PLOGI << "Headset maximum resolution: " << m_viewConfigurationViews[0].maxImageRectWidth << 'x' << m_viewConfigurationViews[0].maxImageRectHeight;
   PLOGI << "Selected resolution: " << m_eyeWidth << 'x' << m_eyeHeight;

   // Create graphics backend early so GetGraphicContext() can provide Vulkan handles to BGFX
   #if BX_PLATFORM_WINDOWS || BX_PLATFORM_ANDROID
   if (m_rendererType == bgfx::RendererType::Vulkan)
   {
      PLOGI << "Creating Vulkan backend for OpenXR (before BGFX initialization)";
      m_backend = std::make_unique<XRVulkanBackend>(m_xrInstance, m_systemID);
   }
   #endif
   #if BX_PLATFORM_WINDOWS
   if (m_rendererType == bgfx::RendererType::Direct3D11)
   {
      PLOGI << "Creating DX11 backend for OpenXR (before BGFX initialization)";
      m_backend = std::make_unique<XRD3D11Backend>(m_xrInstance, m_systemID);
   }
   #endif

   assert(m_backend != nullptr);
}

void VRDevice::CreateSession()
{
   assert(m_xrInstance != XR_NULL_HANDLE);
   assert(m_systemID != XR_NULL_SYSTEM_ID);
   assert(m_session == XR_NULL_HANDLE);
   assert(m_backend != nullptr);

   m_visibilityMaskDirty = true;

   // Evaluate scene size to be able to define a good far plane
   // Adjust near/far plane for each projected bounding box
   vector<Vertex3Ds> bounds;
   bounds.reserve(16);
   Vertex3Ds sceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
   Vertex3Ds sceneMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
   for (IEditable* editable : g_pplayer->m_ptable->GetParts())
   {
      bool prevVisibility;
      Primitive* prim = editable->GetItemType() == ItemTypeEnum::eItemPrimitive ? static_cast<Primitive*>(editable) : nullptr;
      if (prim)
      {
         prevVisibility = prim->m_d.m_visible;
         prim->m_d.m_visible = true;
      }
      editable->GetBoundingVertices(bounds, nullptr);
      if (prim)
         prim->m_d.m_visible = prevVisibility;
      for (const auto& v : bounds)
      {
         sceneMin.x = min(sceneMin.x, v.x);
         sceneMin.y = min(sceneMin.y, v.y);
         sceneMin.z = min(sceneMin.z, v.z);
         sceneMax.x = max(sceneMax.x, v.x);
         sceneMax.y = max(sceneMax.y, v.y);
         sceneMax.z = max(sceneMax.z, v.z);
      }
      bounds.clear();
   }
   m_sceneSize = (sceneMax - sceneMin).Length();

   // Create an XrSessionCreateInfo structure.
   XrSessionCreateInfo sessionCI { XR_TYPE_SESSION_CREATE_INFO };
   sessionCI.next = m_backend->GetGraphicsBinding();
   sessionCI.createFlags = 0;
   sessionCI.systemId = m_systemID;
   OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCI, &m_session), "Failed to create Session.");
   assert(m_session);

   // Initialize passthrough if supported (Meta Quest MR feature)
   if (m_passthroughExtensionSupported)
   {
      PFN_xrCreatePassthroughFB xrCreatePassthroughFB;
      OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrCreatePassthroughFB", (PFN_xrVoidFunction*)&xrCreatePassthroughFB), "Failed to get xrCreatePassthroughFB.");

      XrPassthroughCreateInfoFB passthroughCI { XR_TYPE_PASSTHROUGH_CREATE_INFO_FB };
      passthroughCI.flags = XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB;
      OPENXR_CHECK(xrCreatePassthroughFB(m_session, &passthroughCI, &m_passthrough), "Failed to create passthrough.");

      PFN_xrCreatePassthroughLayerFB xrCreatePassthroughLayerFB;
      OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrCreatePassthroughLayerFB", (PFN_xrVoidFunction*)&xrCreatePassthroughLayerFB), "Failed to get xrCreatePassthroughLayerFB.");

      XrPassthroughLayerCreateInfoFB passthroughLayerCI { XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB };
      passthroughLayerCI.passthrough = m_passthrough;
      passthroughLayerCI.purpose = XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB;
      passthroughLayerCI.flags = XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB;
      OPENXR_CHECK(xrCreatePassthroughLayerFB(m_session, &passthroughLayerCI, &m_passthroughLayer), "Failed to create passthrough layer.");

      m_passthroughEnabled = true;
      PLOGI << "Meta Quest passthrough initialized successfully";
   }

   // Fill out an XrReferenceSpaceCreateInfo structure and create a reference XrSpace, specifying an identity pose as the origin and a stage space, defaulting to a local space.
   uint32_t referenceSpaceCount;
   xrEnumerateReferenceSpaces(m_session, 0, &referenceSpaceCount, nullptr);
   XrReferenceSpaceType* referenceSpaces = new XrReferenceSpaceType[referenceSpaceCount];
   xrEnumerateReferenceSpaces(m_session, referenceSpaceCount, &referenceSpaceCount, referenceSpaces);
   XrReferenceSpaceCreateInfo referenceSpaceCI { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
   for (uint32_t i = 0; i < referenceSpaceCount; i++)
      if (referenceSpaces[i] == XR_REFERENCE_SPACE_TYPE_STAGE)
         referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
      else if ((referenceSpaces[i] == XR_REFERENCE_SPACE_TYPE_LOCAL) && (referenceSpaceCI.referenceSpaceType != XR_REFERENCE_SPACE_TYPE_STAGE))
         referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
   referenceSpaceCI.poseInReferenceSpace = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } };
   OPENXR_CHECK(xrCreateReferenceSpace(m_session, &referenceSpaceCI, &m_referenceSpace), "Failed to create ReferenceSpace.");
   delete[] referenceSpaces;

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

   for (int i = 0; i < 2; i++)
   {
      SwapchainInfo& swapchain = i == 0 ? m_colorSwapchainInfo : m_depthSwapchainInfo;
      swapchain.backendFormat = i == 0 ? m_backend->SelectColorSwapchainFormat(formats) : m_backend->SelectDepthSwapchainFormat(formats);
      swapchain.width = m_eyeWidth;
      swapchain.height = m_eyeHeight;
      swapchain.arraySize = static_cast<uint32_t>(m_viewConfigurationViews.size());

      XrSwapchainCreateInfo swapchainCreateInfo { XR_TYPE_SWAPCHAIN_CREATE_INFO };
      swapchainCreateInfo.arraySize = swapchain.arraySize;
      swapchainCreateInfo.format = swapchain.backendFormat;
      swapchainCreateInfo.width = swapchain.width;
      swapchainCreateInfo.height = swapchain.height;
      swapchainCreateInfo.mipCount = 1;
      swapchainCreateInfo.faceCount = 1;
      swapchainCreateInfo.sampleCount = m_viewConfigurationViews[0].recommendedSwapchainSampleCount;
      swapchainCreateInfo.createFlags = 0;
      swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | (i == 0 ? XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT : XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
      OPENXR_CHECK(xrCreateSwapchain(m_session, &swapchainCreateInfo, &swapchain.swapchain), "Failed to create Swapchain");

      uint32_t swapchainImageCount;
      OPENXR_CHECK(xrEnumerateSwapchainImages(swapchain.swapchain, 0, &swapchainImageCount, nullptr), "Failed to enumerate Swapchain Images.");
      XrSwapchainImageBaseHeader* swapchainImages = m_backend->AllocateSwapchainImageData(swapchain.swapchain, i == 0 ? SwapchainType::COLOR : SwapchainType::DEPTH, swapchainImageCount);
      OPENXR_CHECK(xrEnumerateSwapchainImages(swapchain.swapchain, swapchainImageCount, &swapchainImageCount, swapchainImages), "Failed to enumerate Swapchain Images.");
      m_backend->CreateImageViews(swapchain);
   }
   m_swapchainRenderTargets.resize(m_colorSwapchainInfo.imageViews.size() * m_depthSwapchainInfo.imageViews.size(), nullptr);

   auto inputHandler = std::make_unique<XRInputHandler>(g_pplayer->m_pininput, m_xrInstance, m_session);
   m_xrInputHandler = inputHandler.get();
   g_pplayer->m_pininput.AddInputHandler(std::move(inputHandler));
}

void VRDevice::ReleaseSession()
{
   assert(m_session);

   g_pplayer->m_pininput.RemoveInputHandler(m_xrInputHandler);
   m_xrInputHandler = nullptr;

   // Destroy the swapchian render targets, and color/depth image views
   for (auto& rt : m_swapchainRenderTargets)
      delete rt;
   for (const auto& imageView : m_colorSwapchainInfo.imageViews)
      bgfx::destroy(imageView);
   for (const auto& imageView : m_depthSwapchainInfo.imageViews)
      bgfx::destroy(imageView);

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

   DiscardVisibilityMask();
}

void VRDevice::PollEvents()
{
   assert(m_session);

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
      // Visibility mask changed
      case XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR:
      {
         m_visibilityMaskDirty = true;
         break;
      }
      default:
      {
         break;
      }
      }
   }
}

void VRDevice::UpdateVisibilityMask(RenderDevice* rd)
{
   assert(m_session);

   if (m_visibilityMaskExtensionSupported && m_visibilityMaskDirty)
   {
      DiscardVisibilityMask();
      uint32_t indexCount = 0, vertexCount = 0, maxVertexCount = 0;
      for (size_t view = 0; view < m_viewConfigurationViews.size(); view++)
      {
         XrVisibilityMaskKHR visibilityMask { XR_TYPE_VISIBILITY_MASK_KHR };
         xrGetVisibilityMaskKHR(m_session, m_viewConfiguration, (uint32_t)view, XR_VISIBILITY_MASK_TYPE_HIDDEN_TRIANGLE_MESH_KHR, &visibilityMask);
         indexCount += visibilityMask.indexCountOutput;
         vertexCount += visibilityMask.vertexCountOutput;
         maxVertexCount = max(maxVertexCount, visibilityMask.vertexCountOutput);
      }
      if ((indexCount > 0) && (vertexCount > 0))
      {
         std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(rd, indexCount, false, IndexBuffer::FMT_INDEX32);
         std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(rd, vertexCount);
         XrVector2f* const vert2d = new XrVector2f[maxVertexCount];
         uint32_t* indices;
         indexBuffer->Lock(indices);
         Vertex3D_NoTex2* vertices;
         vertexBuffer->Lock(vertices);
         uint32_t vertexOffset = 0;
         for (size_t view = 0; view < m_viewConfigurationViews.size(); view++)
         {
            XrVisibilityMaskKHR visibilityMask { XR_TYPE_VISIBILITY_MASK_KHR };
            xrGetVisibilityMaskKHR(m_session, m_viewConfiguration, (uint32_t)view, XR_VISIBILITY_MASK_TYPE_HIDDEN_TRIANGLE_MESH_KHR, &visibilityMask);
            visibilityMask.indexCapacityInput = visibilityMask.indexCountOutput;
            visibilityMask.vertexCapacityInput = visibilityMask.vertexCountOutput;
            visibilityMask.vertices = vert2d;
            visibilityMask.indices = indices;
            xrGetVisibilityMaskKHR(m_session, m_viewConfiguration, (uint32_t)view, XR_VISIBILITY_MASK_TYPE_HIDDEN_TRIANGLE_MESH_KHR, &visibilityMask);
            for (uint32_t i = 0; i < visibilityMask.vertexCountOutput; i++)
            {
               vertices->x = vert2d[i].x;
               vertices->y = vert2d[i].y;
               vertices->z = -1.f;
               vertices->nx = static_cast<float>(view);
               vertices->ny = 0.f;
               vertices->nz = 0.f;
               vertices->tu = 0.f;
               vertices->tv = 0.f;
               vertices++;
            }
            for (uint32_t i = 0; i < visibilityMask.indexCountOutput; i++)
            {
               (*indices) += vertexOffset;
               indices++;
            }
            vertexOffset += visibilityMask.vertexCountOutput;
         }
         indexBuffer->Unlock();
         vertexBuffer->Unlock();
         delete[] vert2d;
         m_visibilityMask = std::make_shared<MeshBuffer>("VisibilityMask"s, vertexBuffer, indexBuffer, true);
         m_visibilityMaskDirty = false;
         PLOGI << "Headset visibility mask acquired";
      }
      else
      {
         m_visibilityMask = nullptr; 
         m_visibilityMaskDirty = false;
         PLOGI << "Headset visibility mask defined to none (empty mask returned by headset)";
      }
   }
}

void VRDevice::RenderFrame(RenderDevice* rd, const std::function<void(RenderTarget* vrRenderTarget)>& submitFrame)
{
   assert(m_session);

   bool rendered = true;
   if (!m_sessionRunning)
   {
      // FIXME we should perform preview rendering here
      submitFrame(nullptr);
      return;
   }

   // Let OpenXR throttle frame submission and get the XrFrameState for timing and rendering info.
   #ifdef MSVC_CONCURRENCY_VIEWER
   span *tagSpanFF = new span(series, 1, _T("xrWaitFrame"));
   #endif
   g_pplayer->m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_FLIP);
   XrFrameState frameState { XR_TYPE_FRAME_STATE };
   constexpr XrFrameWaitInfo frameWaitInfo { XR_TYPE_FRAME_WAIT_INFO, nullptr };
   OPENXR_CHECK(xrWaitFrame(m_session, &frameWaitInfo, &frameState), "Failed to wait for XR Frame.");
   g_pplayer->m_renderProfiler->ExitProfileSection();
   #ifdef MSVC_CONCURRENCY_VIEWER
   delete tagSpanFF;
   #endif

   // Tell the OpenXR compositor that the application is beginning the frame.
   constexpr XrFrameBeginInfo frameBeginInfo { XR_TYPE_FRAME_BEGIN_INFO, nullptr };
   OPENXR_CHECK(xrBeginFrame(m_session, &frameBeginInfo), "Failed to begin the XR Frame.");

   // Variables for rendering and layer composition.
   RenderLayerInfo renderLayerInfo;
   renderLayerInfo.predictedDisplayTime = frameState.predictedDisplayTime;

   m_predictedDisplayTimestamp = static_cast<float>(usec()) / 1000000.f;
   #if BX_PLATFORM_WINDOWS
   if (m_xrConvertTimeToWin32PerformanceCounterKHR)
   {
      LARGE_INTEGER displayTime;
      m_xrConvertTimeToWin32PerformanceCounterKHR(m_xrInstance, frameState.predictedDisplayTime, &displayTime);
      LARGE_INTEGER now;
      QueryPerformanceCounter(&now);
      LARGE_INTEGER TimerFreq;
      QueryPerformanceFrequency(&TimerFreq);
      m_predictedDisplayTimestamp += static_cast<float>(displayTime.QuadPart - now.QuadPart) / static_cast<float>(TimerFreq.QuadPart);
   }
   #elif BX_PLATFORM_ANDROID
   if (m_xrConvertTimeToTimespecTimeKHR)
   {
      timespec displayTime;
      m_xrConvertTimeToTimespecTimeKHR(m_xrInstance, frameState.predictedDisplayTime, &displayTime);
      timespec now;
      clock_gettime(CLOCK_MONOTONIC, &now);
      time_t sec_diff = displayTime.tv_sec - now.tv_sec;
      long nsec_diff = displayTime.tv_nsec - now.tv_nsec;
      int64_t total_nsec = sec_diff * 1000000000LL + nsec_diff;
      m_predictedDisplayTimestamp += static_cast<float>(total_nsec) / 1000000000.0f;
   }
   #endif

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
      viewLocateInfo.space = m_referenceSpace;
      uint32_t viewCount = 0;
      const XrResult result = xrLocateViews(m_session, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount, views.data());
      if (result != XR_SUCCESS)
      {
         PLOGE << "Failed to locate Views.";
         rendered = false;
      }

      if (rendered)
      {
         // The steps that leads to the matrix stack implemented below are the followings, with first matrix being view, 
         // second being projection, i.e. notation is (view) * (projection):
         // - What would be right: (sceneScale * sceneBasis * VPUToWorldScale * viewOffset * view[eye]) * (projection[eye])
         // - Single view matrix:  (sceneScale * sceneBasis * VPUToWorldScale * viewOffset * medianView) * (inv(medianView) * view[eye] * projection[eye])
         //                        (sceneScale * sceneBasis * viewOffsetInVPU * medianViewInVPU) * (inv(medianViewInVPU) * VPUToWorldScale * view[eye] * projection[eye])
         // - Orthonormal shading: (sceneBasis * viewOffsetInVPU * medianViewInVPU) * (inv(sceneBasis * viewOffsetInVPU * medianViewInVPU) * sceneScale * sceneBasis * viewOffsetInVPU * VPUToWorldScale * view[eye] * projection[eye])
         // - Therefore, we define:
         //   . m_pfWorld = sceneBasis * viewOffsetInVPU
         //   . m_pfMatView = m_pfWorld * medianViewInVPU
         //   . m_pfMatProj[eye] = inv(m_pfMatView) * sceneScale * m_pfWorld * VPUToWorldScale * view[eye] * projection[eye]
         // TODO Note that this is not the way it should be done: shading should use each eye view matrix, leading to slightly different shading per eye
         constexpr float vpuToWorldScale = static_cast<float>(0.0254 * 1.0625 / 50.); // VPU to meters
         constexpr float zNear = 0.1f; // 10cm in front of player
         const float zFar = max(5.f, m_sceneSize * vpuToWorldScale); // This could be fairly optimized for better accuracy (as well as use an optimized depth buffer for rendering)

         // Compute the eye median pose in VPU coordinates to be used as the view point for shading
         XrPosef medianPoseInVPU;
         XrVector3f_Lerp(&medianPoseInVPU.position, &views[0].pose.position, &views[1].pose.position, 0.5f);
         XrQuaternionf_Lerp(&medianPoseInVPU.orientation, &views[0].pose.orientation, &views[1].pose.orientation, 0.5f);
         XrVector3f_Scale(&medianPoseInVPU.position , & medianPoseInVPU.position, 1.f / vpuToWorldScale); // Convert position from meters to VPU
         XrPosef_ToMatrix3D(&m_nextMedianView, &medianPoseInVPU);

         if (m_recenterTable)
         {
            m_recenterTable = false;
            m_orientation = RADTOANG(atan2f(m_nextMedianView.m[0][2], m_nextMedianView.m[0][0]));
            m_tablePos.x = g_app->m_settings.GetPlayer_ScreenPlayerX() - VPUTOCM(medianPoseInVPU.position.x);
            m_tablePos.y = g_app->m_settings.GetPlayer_ScreenPlayerY() - VPUTOCM(medianPoseInVPU.position.z);
            m_tablePos.z = 0.f;
            //m_tablePos.z = abs(m_tablePos.z) > 10.f ? 0.f : m_tablePos.z; // Keep user custom offset except if it seems out of normal range
            m_worldDirty = true;
         }

         // Prepare the view/projection matrices for the 4 space references we use:
         // - Room is the VR room, offseted and rotated to the table orientation (rotation around vertical axis)
         // - Cabinet feet is the same as the room with cabinet scaling applied (to match lockbar width)
         // - Cabinet is the same as the cabinet feet but also applying depth offset (to match lockbar height)
         // - Playfield is the main space reference where the simulation happens (only one to support physics), relative to the cabinet, with inclination and table coordinate system

         // playfield is inclined at a fixed slope that should be defined with the table (it corresponds to the cabinet design),
         // then player adjust the play angle by adjusting the feet casters, so the whole cab rotates.
         const PinTable* const table = g_pplayer->m_ptable;
         const float liveSlope = table->GetPlayfieldSlope();
         if (m_worldDirty || m_slope != liveSlope)
         {
            m_worldDirty = false;
            m_slope = liveSlope;

            // Update fixed scaling, considering lockbar size to be the width of the playfield + 2"1/4
            const float tableWidth = VPUTOCM(table->m_right - table->m_left) + 2.25f * 2.54f;
            m_scale = clamp(m_lockbarWidth / tableWidth, 0.1f, 2.0f);

            // Move table (in VPU coordinates), adjust coord from RH to LH system
            const Matrix3D coords = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
            const Matrix3D rotz = Matrix3D::MatrixRotateZ(ANGTORAD(m_orientation));
            const Matrix3D rotx2 = Matrix3D::MatrixRotateX(ANGTORAD(-90.f));
            const Matrix3D viewOrientation = rotz * rotx2;
            const Matrix3D viewOrientationInv = Matrix3D::MatrixInverse(viewOrientation);

            // The users define in their settings the real world height where they want the top of the lockbar to be.
            // The real world playfield height is then computed by removing the glass distance at the playfield bottom (typically 2 to 3").
            // The m_tablePos allows to slightly adjust this height on a per table basis.
            // In the end Playfield height = m_tablePos.z (User adjustement) + m_lockBatHeight (Real world lockbar height) - glass distance * scale
            const float scaledGlassHeight = m_scale * VPUTOCM(table->m_glassBottomHeight);

            // The table defines the height of the lockbar of its cabinet model, as well as the playfield base inclination.
            // This allows to fit the cabinet & playfield models to the real world space.
            // If user adjust the inclination, then the cab is rotated as it would in real life (still missing the legs stretching a bit using caster adjustments)
            const float groundToPlayfieldHeight = m_scale * table->m_groundToLockbarHeight - m_scale * table->m_glassBottomHeight;
            const float baseSlope = lerp(table->m_angletiltMin, table->m_angletiltMax, table->m_difficulty);

            // Fixed value of 5 cm between playfield bottom and lockbar border
            // We could (should ?) make this a table data but this does not vary that much so this seems fine for the time being
            constexpr float lockbarToPlayfield = 5.f;

            // Before 10.8.1, there weren't multiple space reference, so room used to be inclined to compensate the playfield inclination.
            // This may leads to slight visual artefact for old VR room (that is to say very slightly inclined room).
            const Matrix3D playfieldSlope = Matrix3D::MatrixRotateX(ANGTORAD(liveSlope));
            const Matrix3D playfieldSlopeInv = Matrix3D::MatrixRotateX(-ANGTORAD(liveSlope));
            const Matrix3D tableCoords = Matrix3D::MatrixTranslate(
               - m_scale * (table->m_right - table->m_left) * 0.5f,
               + m_scale * (table->m_bottom - table->m_top),
               0.f);
            const Matrix3D playfieldPos = Matrix3D::MatrixTranslate(
               -CMTOVPU(m_tablePos.x),
                CMTOVPU(m_tablePos.y + lockbarToPlayfield),
                CMTOVPU(m_tablePos.z + m_lockbarHeight - scaledGlassHeight)); 
            const Matrix3D playfieldPosInv = Matrix3D::MatrixInverse(playfieldPos);
            m_pfWorld = coords * tableCoords * playfieldSlope * playfieldPos * viewOrientation;

            const Matrix3D cabinetSlope = playfieldPosInv * Matrix3D::MatrixRotateX(ANGTORAD(liveSlope - baseSlope)) * playfieldPos;
            const Matrix3D pfToCab = viewOrientationInv // Revert view orientation
               * playfieldPosInv * playfieldSlopeInv // Revert playfield slope
               * Matrix3D::MatrixTranslate(
                  -CMTOVPU(m_tablePos.x),
                   CMTOVPU(m_tablePos.y + lockbarToPlayfield),
                   // Cabinet model has its z origin at the feet level, m_groundToLockbarHeight corresponding to the playfield level, so we move it down (to real world ground) then up to match user seyup (where we placed the playfield)
                   CMTOVPU(m_tablePos.z + m_lockbarHeight - scaledGlassHeight) - groundToPlayfieldHeight)
               * cabinetSlope // Apply cabinet slope
               * viewOrientation; // Reapply view orientation
            m_cabWorld = m_pfWorld * pfToCab;

            // Feet are always touching the ground, scaled against the real world vs model defined playfield level
            // Note that since we are rotating the cabinet with its feet, the feet may slightly leave or enter the ground.
            const float feetScale = (m_tablePos.z + m_lockbarHeight - scaledGlassHeight) / VPUTOCM(groundToPlayfieldHeight);
            const Matrix3D pfToFeet = viewOrientationInv // Revert view orientation
               * playfieldPosInv * playfieldSlopeInv // Revert playfield slope
               * Matrix3D::MatrixTranslate(
                  -CMTOVPU(m_tablePos.x),
                   CMTOVPU(m_tablePos.y + lockbarToPlayfield),
                   CMTOVPU(m_tablePos.z)) // Feets are always at z=0 in real world, that is to say ground
               * Matrix3D::MatrixScale(1.f, 1.f, feetScale) // Scale feets in order to match feet bottom to real world floor
               * cabinetSlope // Apply cabinet slope
               * viewOrientation; // Reapply view orientation
            m_feetWorld = m_pfWorld * pfToFeet;

            // Room does not apply the cabinet scaling nor any inclination, as it is the real world room
            const Matrix3D pfToRoom = viewOrientationInv // Revert view orientation
               * playfieldPosInv * playfieldSlopeInv // Revert playfield slope
               * Matrix3D::MatrixTranslate( // Apply table coordinate but without table scale
                   - (1.f - m_scale) * (table->m_right - table->m_left) * 0.5f,
                   + (1.f - m_scale) * (table->m_bottom - table->m_top),
                   0.f)
               * Matrix3D::MatrixTranslate(
                  -CMTOVPU(m_tablePos.x), // For the ease of positioning, align the room to the table view setting, except for z which must stay on ground
                   CMTOVPU(m_tablePos.y + lockbarToPlayfield),
                   CMTOVPU(m_tablePos.z))
               * viewOrientation; // Reapply view orientation
            m_roomWorld = m_pfWorld * pfToRoom;
         }

         // As we only have one view matrix for shading, each eye view is integrated in the projection matrix, by reverting the 'shading' view matrix then
         // applying the eye view matrix, we also apply scale in the projection matrix as it would break shading otherwise (it needs an orthonormal view matrix)

         m_pfMatView = m_pfWorld * m_nextMedianView;
         m_cabMatView = m_cabWorld * m_nextMedianView;
         m_feetMatView = m_feetWorld * m_nextMedianView;
         m_roomMatView = m_roomWorld * m_nextMedianView;

         const Matrix3D sceneScale = Matrix3D::MatrixScale(m_scale);
         const Matrix3D vpuScale = Matrix3D::MatrixScale(vpuToWorldScale);
         const Matrix3D pfBaseView = Matrix3D::MatrixInverse(m_pfMatView) * sceneScale * m_pfWorld * vpuScale;
         const Matrix3D cabBaseView = Matrix3D::MatrixInverse(m_cabMatView) * sceneScale * m_cabWorld * vpuScale;
         const Matrix3D feetBaseView = Matrix3D::MatrixInverse(m_feetMatView) * sceneScale * m_feetWorld * vpuScale;
         const Matrix3D roomBaseView = Matrix3D::MatrixInverse(m_roomMatView) * m_roomWorld * vpuScale;
         for (uint32_t i = 0; i < viewCount; i++)
         {
            XrPosef_ToMatrix3D(&m_nextView[i], &views[i].pose);
            m_nextProj[i].SetPerspectiveFovRH(views[i].fov.angleLeft, views[i].fov.angleRight, views[i].fov.angleDown, views[i].fov.angleUp, zNear, zFar);
            const Matrix3D viewProj = m_nextView[i] * m_nextProj[i];
            m_pfMatProj[i] = pfBaseView * viewProj;
            m_cabMatProj[i] = cabBaseView * viewProj;
            m_feetMatProj[i] = feetBaseView * viewProj;
            m_roomMatProj[i] = roomBaseView * viewProj;
         }

         // Swapchain is acquired, rendered to, and released together for all views as a texture array

         // Resize the layer projection views to match the view count. The layer projection views are used in the layer projection.
         renderLayerInfo.layerProjectionViews.resize(viewCount, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });
         if (m_depthExtensionSupported)
            renderLayerInfo.depthInfoViews.resize(viewCount, { XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR });

         // Acquire and wait for an image from the swapchains (the timeout is infinite)
         uint32_t colorImageIndex = 0;
         uint32_t depthImageIndex = 0;
         constexpr XrSwapchainImageAcquireInfo acquireInfo { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO, nullptr };
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

         // Prepare frame with the acquired views to limit position-visual latency (limit motion sickness)
         // This can't be done earlier since view acquisition is done for the predicted frame display time (which we have only after xrWaitFrame) 
         // and we also need OpenXR to selected color/depth render target from xrAcquireSwapchainImage
         RenderTarget* vrRenderTarget = m_swapchainRenderTargets[colorImageIndex + depthImageIndex * m_colorSwapchainInfo.imageViews.size()];
         if (vrRenderTarget == nullptr)
         {
            const uint16_t nViews = static_cast<uint16_t>(m_viewConfigurationViews.size());
            bgfx::Attachment colorAttachment, depthAttachment;
            colorAttachment.init(m_colorSwapchainInfo.imageViews[colorImageIndex], bgfx::Access::Write, 0, nViews, 0, BGFX_RESOLVE_NONE);
            depthAttachment.init(m_depthSwapchainInfo.imageViews[depthImageIndex], bgfx::Access::Write, 0, nViews, 0, BGFX_RESOLVE_NONE);
            const bgfx::Attachment attachments[] = { colorAttachment, depthAttachment };
            const bgfx::FrameBufferHandle fbh = bgfx::createFrameBuffer(2, attachments);
            vrRenderTarget = new RenderTarget(rd, SurfaceType::RT_STEREO, 
               fbh, colorAttachment.handle, m_colorSwapchainInfo.format, depthAttachment.handle, m_depthSwapchainInfo.format,
               "VRSwapchain [" + std::to_string(colorImageIndex) + '/' + std::to_string(depthImageIndex) + ']',
               m_colorSwapchainInfo.width, m_colorSwapchainInfo.height, colorFormat::RGBA);
            m_swapchainRenderTargets[colorImageIndex + depthImageIndex * m_colorSwapchainInfo.imageViews.size()] = vrRenderTarget;
         }
         submitFrame(vrRenderTarget);

         // Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().
         renderLayerInfo.layerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
         renderLayerInfo.layerProjection.space = m_referenceSpace;
         renderLayerInfo.layerProjection.viewCount = static_cast<uint32_t>(renderLayerInfo.layerProjectionViews.size());
         renderLayerInfo.layerProjection.views = renderLayerInfo.layerProjectionViews.data();

         // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
         constexpr XrSwapchainImageReleaseInfo releaseInfo { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO, nullptr };
         OPENXR_CHECK(xrReleaseSwapchainImage(m_colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
         OPENXR_CHECK(xrReleaseSwapchainImage(m_depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");

         // Add passthrough layer first (background)
         if (m_passthroughEnabled && m_passthroughLayer != XR_NULL_HANDLE)
         {
            renderLayerInfo.layerPassthrough.layerHandle = m_passthroughLayer;
            renderLayerInfo.layerPassthrough.flags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
            renderLayerInfo.layerPassthrough.space = XR_NULL_HANDLE;
            renderLayerInfo.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&renderLayerInfo.layerPassthrough));
         }

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
      submitFrame(nullptr);
}
#endif

void VRDevice::OffsetTable(float dx, float dy, float dz)
{
   m_tablePos.x = clamp(m_tablePos.x + dx, -100.0f, 100.0f);
   m_tablePos.y = clamp(m_tablePos.y + dy, -100.0f, 100.0f);
   m_tablePos.z = clamp(m_tablePos.z + dz, -100.0f, 100.0f);
   m_worldDirty = true;
}

#ifdef ENABLE_VR
bool VRDevice::IsVRinstalled() {
   return vr::VR_IsRuntimeInstalled();
}

bool VRDevice::IsVRturnedOn()
{
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

void VRDevice::SubmitFrame(const std::shared_ptr<Sampler>& leftEye, const std::shared_ptr<Sampler>& rightEye)
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
         ShowError("VRCompositor Submit Left Error " + std::to_string(errorLeft));
      }
      vr::EVRCompositorError errorRight = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
      if (errorRight != vr::VRCompositorError_None)
      {
         ShowError("VRCompositor Submit Right Error " + std::to_string(errorRight));
      }
      #if defined(ENABLE_OPENGL)
         glFlush();
      #endif
      //vr::VRCompositor()->PostPresentHandoff(); // PostPresentHandoff gives mixed results, improved GPU frametime for some, worse CPU frametime for others, troublesome enough to not warrants it's usage for now
}
#endif


void VRDevice::UpdateVRPosition(PartGroupData::SpaceReference spaceRef, ModelViewProj& mvp)
{
   #ifdef ENABLE_XR
      switch (spaceRef)
      {
      case PartGroupData::SpaceReference::SR_PLAYFIELD:
      {
         mvp.SetView(m_pfMatView);
         mvp.SetProj(0, m_pfMatProj[0]);
         mvp.SetProj(1, m_pfMatProj[1]);
         break;
      }
      
      case PartGroupData::SpaceReference::SR_CABINET:
      {
         mvp.SetView(m_cabMatView);
         mvp.SetProj(0, m_cabMatProj[0]);
         mvp.SetProj(1, m_cabMatProj[1]);
         break;
      }

      case PartGroupData::SpaceReference::SR_CABINET_FEET:
      {
         mvp.SetView(m_feetMatView);
         mvp.SetProj(0, m_feetMatProj[0]);
         mvp.SetProj(1, m_feetMatProj[1]);
         break;
      }

      case PartGroupData::SpaceReference::SR_ROOM:
      {
         mvp.SetView(m_roomMatView);
         mvp.SetProj(0, m_roomMatProj[0]);
         mvp.SetProj(1, m_roomMatProj[1]);
         break;
      }
      }

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

      if (m_worldDirty)
      {
         m_worldDirty = false;
         // Locate front left corner of the table in the room -x is to the right, -y is up and -z is back - all units in meters
         const float inv_transScale = 1.0f / (100.0f * m_scale);
         m_pfWorld = Matrix3D::MatrixRotateX(ANGTORAD(-m_slope)) // Tilt playfield
            * Matrix3D::MatrixRotateZ(ANGTORAD(180.f + m_orientation)) // Rotate table around VR height axis
            * Matrix3D::MatrixTranslate(-m_tablePos.x * inv_transScale, m_tablePos.y * inv_transScale, m_tablePos.z * inv_transScale);
      }

      mvp.SetView(m_pfWorld * matView);
      mvp.SetProj(0, m_pfMatProj[0]);
      mvp.SetProj(1, m_pfMatProj[1]);
   #endif
}

void VRDevice::RecenterTable()
{
   #ifdef ENABLE_XR
      m_recenterTable = true;

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
      m_tablePos.x = 100.0f * (headX - c * w + s * h);
      m_tablePos.y = 100.0f * (headY + s * w + c * h);
      m_worldDirty = true;
   #endif
}

void VRDevice::SaveVRSettings(Settings& settings) const
{
   settings.SetPlayerVR_Orientation(m_orientation, false);
   settings.SetPlayerVR_TableX(m_tablePos.x, false);
   settings.SetPlayerVR_TableY(m_tablePos.y, false);
   settings.SetPlayerVR_TableZ(m_tablePos.z, false);
}
