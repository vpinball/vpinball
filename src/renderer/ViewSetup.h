// license:GPLv3+

#pragma once

enum ViewSetupID
{
   BG_DESKTOP = 0,
   BG_FULLSCREEN = 1,
   BG_FSS = 2,
   BG_INVALID = 3,
   NUM_BG_SETS
};

enum ViewLayoutMode : int
{
   VLM_LEGACY = 0,   // All tables before 10.8 used a viewer position relative to a fitting of a set of bounding vertices (not all parts) with a standard perspective projection skewed by a layback angle
   VLM_CAMERA,       // Position viewer relative to the bottom center of the table, use a standard camera perspective projection, replace layback by a frustum offset
   VLM_WINDOW        // Position viewer relative to the bottom center of the screen, use an oblique surface (re)projection (would need some postprocess to limit distortion)
};

class ViewSetup final
{
public:
   ViewSetup();

   void ApplyTableOverrideSettings(const Settings& settings, const ViewSetupID id);
   void SaveToTableOverrideSettings(Settings& settings, const ViewSetupID id) const;
   void SetWindowModeFromSettings(const PinTable* const table);

   float GetWindowTopZOFfset(const PinTable* const table) const;
   float GetWindowBottomZOFfset(const PinTable* const table) const;
   float GetRealToVirtualScale(const PinTable* const table) const;
   float GetRotation(const int viewportWidth, const int viewportHeight) const;

   void ComputeMVP(const PinTable* const table, const float aspect, const bool stereo, ModelViewProj& mvp,
      const vec3& cam = vec3(0.f, 0.f, 0.f), const float cam_inc = 0.f, const float xpixoff = 0.f, const float ypixoff = 0.f);

private:
   static vec3 FitCameraToVertices(const vector<Vertex3Ds>& pvvertex3D, const float aspect, const float rotation, const float inclination, const float FOV, const float xlatez, const float layback);

public:
   ViewLayoutMode mMode = VLM_LEGACY;

   // Overall scene scale
   float mSceneScaleX = 1.0f;
   float mSceneScaleY = 1.0f;
   float mSceneScaleZ = 1.0f;

   // View position:
   // - relative to table bounds for legacy mode
   // - relative to the bottom center of the table for camera and window mode
   float mViewX = 0.f;
   float mViewY = CMTOVPU(20.f);
   float mViewZ = CMTOVPU(70.f);

   // Look At (Camera & Legacy only):
   // - expressed ad a camera inclination for legacy mode, 
   // - expressed as a look at toward a percent of the table height, starting from bottom (0.25 is around top of slingshots) for camera mode,
   float mLookAt = 0.25f;

   // Viewport adjustments
   float mViewportRotation = 0.f;

   // View properties
   float mFOV = 45.0f; // Camera & Legacy: Field of view, in degrees
   float mLayback = 0.0f; // Legacy: A skewing angle that deform the table to make it look 'good'
   float mViewHOfs = 0.0f; // Camera & Window: horizontal frustum offset
   float mViewVOfs = 0.0f; // Camera & Window: vertical frustum offset

   // Magic Window mode properties
   float mWindowTopZOfs = CMTOVPU(20.0f); // Upper window border Z coordinate, relative to table playfield Z
   float mWindowBottomZOfs = CMTOVPU(7.5f); // Lower window border Z coordinate, relative to table playfield Z
};
