#include "stdafx.h"
#include "VRDevice.h"

// Undefine this if you want to debug VR mode without a VR headset
//#define VR_PREVIEW_TEST

#ifdef ENABLE_VR
vr::IVRSystem* VRDevice::m_pHMD = nullptr;
#endif

VRDevice::VRDevice()
{
   #if defined(ENABLE_VR)
   m_pHMD = nullptr;
   m_rTrackedDevicePose = nullptr;
   m_scale = 1.0f; // Scale factor from scene (in VP units) to VR view (in meters)
   if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleToFixedWidth"s, false))
   {
      float width;
      g_pplayer->m_ptable->get_Width(&width);
      m_scale = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleAbsolute"s, 55.0f) * 0.01f / width;
   }
   else
      m_scale = VPUTOCM(0.01f) * g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleRelative"s, 1.0f);
   if (m_scale <= 0.000001f)
      m_scale = VPUTOCM(0.01f); // Scale factor for VPUnits to Meters

   // Initialize VR, this will also override the render buffer size (m_width, m_height) to account for HMD render size and render the 2 eyes simultaneously
#ifdef VR_PREVIEW_TEST
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
   Matrix3D coords;
   coords.SetIdentity();
   coords._11 = -1.f; coords._12 = 0.f; coords._13 =  0.f;
   coords._21 =  0.f; coords._22 = 0.f; coords._23 = -1.f;
   coords._31 =  0.f; coords._32 = 1.f; coords._33 =  0.f;

   float zNear, zFar;
   g_pplayer->m_ptable->ComputeNearFarPlane(coords * sceneScale, m_scale, zNear, zFar);
   zNear = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "NearPlane"s, 5.0f) / 100.0f; // Replace near value to allow player to move near parts up to user defined value
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
         Matrix3D proj;
         proj.SetPerspectiveFovLH(90.f, 1.f, zNear, zFar);
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

      Matrix3D matEye2Head, matProjection;

      //Calculate left EyeProjection Matrix relative to HMD position
      matEye2Head.SetIdentity();
      for (int i = 0; i < 3; i++)
         for (int j = 0;j < 4;j++)
            matEye2Head.m[j][i] = left_eye_pos.m[i][j];
      matEye2Head.Invert();

      left_eye_proj.m[2][2] = -1.0f;
      left_eye_proj.m[2][3] = -zNear;
      for (int i = 0;i < 4;i++)
         for (int j = 0;j < 4;j++)
            matProjection.m[j][i] = left_eye_proj.m[i][j];

      m_vrMatProj[0] = coords * sceneScale * matEye2Head * matProjection;

      //Calculate right EyeProjection Matrix relative to HMD position
      matEye2Head.SetIdentity();
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

   m_slope = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Slope"s, 6.5f);
   m_orientation = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Orientation"s, 0.0f);
   m_tablex = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableX"s, 0.0f);
   m_tabley = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableY"s, 0.0f);
   m_tablez = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableZ"s, 80.0f);
}

VRDevice::~VRDevice()
{
   #if defined(ENABLE_VR)
   if (m_pHMD)
   {
      vr::VR_Shutdown();
      m_pHMD = nullptr;
   }
   #endif
}

void VRDevice::SaveVRSettings(Settings& settings) const
{
   #if defined(ENABLE_VR) && defined(ENABLE_SDL)
   settings.SaveValue(Settings::PlayerVR, "Slope"s, m_slope);
   settings.SaveValue(Settings::PlayerVR, "Orientation"s, m_orientation);
   settings.SaveValue(Settings::PlayerVR, "TableX"s, m_tablex);
   settings.SaveValue(Settings::PlayerVR, "TableY"s, m_tabley);
   settings.SaveValue(Settings::PlayerVR, "TableZ"s, m_tablez);
   #endif
}

bool VRDevice::IsVRinstalled()
{
   #ifdef ENABLE_VR
      #ifdef VR_PREVIEW_TEST
      return true;
      #else
      return vr::VR_IsRuntimeInstalled();
      #endif
   #else
      return false;
   #endif
}

bool VRDevice::IsVRturnedOn()
{
   #ifdef ENABLE_VR
   #ifdef VR_PREVIEW_TEST
   return true;
   #else
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
   #endif
   #endif
   return false;
}

bool VRDevice::IsVRReady() const
{
   #ifdef ENABLE_VR
   return m_pHMD != nullptr;
   #else
   return false;  
   #endif
}

void VRDevice::SubmitFrame(Sampler* leftEye, Sampler* rightEye)
{
   #if defined(ENABLE_VR) && defined (ENABLE_SDL)
   vr::Texture_t leftEyeTexture = { (void*)(__int64)leftEye->GetCoreTexture(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
   vr::EVRCompositorError errorLeft = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
   if (errorLeft != vr::VRCompositorError_None)
   {
      char msg[128];
      sprintf_s(msg, sizeof(msg), "VRCompositor Submit Left Error %u", errorLeft);
      ShowError(msg);
   }
   vr::Texture_t rightEyeTexture = { (void*)(__int64)rightEye->GetCoreTexture(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
   vr::EVRCompositorError errorRight = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
   if (errorRight != vr::VRCompositorError_None)
   {
      char msg[128];
      sprintf_s(msg, sizeof(msg), "VRCompositor Submit Right Error %u", errorRight);
      ShowError(msg);
   }
   glFlush();
   #endif
}

void VRDevice::UpdateVRPosition(ModelViewProj& mvp)
{
   mvp.SetProj(0, m_vrMatProj[0]);
   mvp.SetProj(1, m_vrMatProj[1]);

   Matrix3D matView;
   matView.SetIdentity();

   #ifdef ENABLE_VR
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
            Matrix3D coords, revCoords;
            coords.SetIdentity();
            coords._11 = -1.f; coords._12 = 0.f; coords._13 =  0.f;
            coords._21 =  0.f; coords._22 = 0.f; coords._23 = -1.f;
            coords._31 =  0.f; coords._32 = 1.f; coords._33 =  0.f;
            revCoords.SetIdentity();
            revCoords._11 = -1.f; revCoords._12 =  0.f; revCoords._13 = 0.f;
            revCoords._21 =  0.f; revCoords._22 =  0.f; revCoords._23 = 1.f;
            revCoords._31 =  0.f; revCoords._32 = -1.f; revCoords._33 = 0.f;
            matView = coords * matView * revCoords;

            break;
         }
      }
   }
   #endif

   // Apply table world position
   if (m_tableWorldDirty)
   {
      m_tableWorldDirty = false;
      Matrix3D rotx, rotz, trans, coords;
      // Tilt playfield.
      rotx.SetRotateX(ANGTORAD(-m_slope));
      // Rotate table around VR height axis
      rotz.SetRotateZ(ANGTORAD(180.f + m_orientation));
      // Locate front left corner of the table in the room -x is to the right, -y is up and -z is back - all units in meters
      const float inv_transScale = 1.0f / (100.0f * m_scale);
      trans.SetTranslation(-m_tablex * inv_transScale, m_tabley * inv_transScale, m_tablez * inv_transScale);
      m_tableWorld = rotx * rotz * trans;
   }
   
   mvp.SetView(m_tableWorld * matView);
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
   const float w = m_scale * (g_pplayer->m_ptable->m_right - g_pplayer->m_ptable->m_left) * 0.5f;
   const float h = m_scale * (g_pplayer->m_ptable->m_bottom - g_pplayer->m_ptable->m_top) + 0.2f;
   float headX = 0.f, headY = 0.f;
   #ifdef ENABLE_VR
   if (IsVRReady())
   {
      m_orientation = -RADTOANG(atan2f(m_hmdPosition.mDeviceToAbsoluteTracking.m[0][2], m_hmdPosition.mDeviceToAbsoluteTracking.m[0][0]));
      if (m_orientation < 0.0f)
         m_orientation += 360.0f;
      headX = m_hmdPosition.mDeviceToAbsoluteTracking.m[0][3];
      headY = -m_hmdPosition.mDeviceToAbsoluteTracking.m[2][3];
   }
   #endif
   const float c = cosf(ANGTORAD(m_orientation));
   const float s = sinf(ANGTORAD(m_orientation));
   m_tablex = 100.0f * (headX - c * w + s * h);
   m_tabley = 100.0f * (headY + s * w + c * h);
   m_tableWorldDirty = true;
}
