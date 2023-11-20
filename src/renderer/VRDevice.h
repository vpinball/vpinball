#pragma once

#ifdef ENABLE_VR
#include <openvr.h>
#endif

class VRDevice final
{
public:
   VRDevice();
   ~VRDevice();
   void SaveVRSettings(Settings& settings) const;
   
   void UpdateVRPosition(ModelViewProj& mvp);
   void TableUp();
   void TableDown();
   void RecenterTable();

   bool IsVRReady() const;
   int GetEyeWidth() const { return m_eyeWidth; }
   int GetEyeHeight() const { return m_eyeHeight; }
   
   static bool IsVRinstalled();
   static bool IsVRturnedOn();
   
   void SubmitFrame(Sampler* leftEye, Sampler* rightEye);

private:
   int m_eyeWidth, m_eyeHeight; // Eye resolution
   float m_scale = 1.0f;
   float m_slope, m_orientation, m_tablex, m_tabley, m_tablez;
   Matrix3D m_vrMatProj[2];
   Matrix3D m_tableWorld;
   bool m_tableWorldDirty = true;

   #ifdef ENABLE_VR
   static vr::IVRSystem* m_pHMD;
   vr::TrackedDevicePose_t m_hmdPosition;
   vr::TrackedDevicePose_t* m_rTrackedDevicePose = nullptr;
   #endif
};
