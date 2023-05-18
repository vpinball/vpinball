#pragma once

enum ViewSetupID
{
   BG_DESKTOP = 0,
   BG_FULLSCREEN = 1,
   BG_FSS = 2,
   NUM_BG_SETS
};

enum ViewLayoutMode : int
{
   CLM_LEGACY = 0,   // All tables before 10.8 used a viewer position relative to a fitting of a set of bounding vertices (not all parts) with a standard perspective projection skewed by a layback angle
   CLM_CAMERA,       // Position viewer relative to the bottom center of the table, use a standard camera perspective projection, replace layback by a frustrum offset
   CLM_MAGIC_WINDOW  // Position viewer relative to the bottom center of the table, use an oblique surface (re)projection (needs some postprocess to avoid distortion)
};


class ViewSetup final
{
public:
   ViewSetup();

   void ComputeMVP(const PinTable* table, const int viewportWidth, const int viewportHeight, const bool stereo, ModelViewProj& mvp, 
      const vec3& cam = vec3(0.f, 0.f, 0.f), const float cam_inc = 0.f, const float xpixoff = 0.f, const float ypixoff = 0.f);

private:
   vec3 FitCameraToVertices(const vector<Vertex3Ds>& pvvertex3D, const float aspect, const float rotation, const float inclination, const float FOV, const float xlatez, const float layback);

public:
   ViewLayoutMode mMode = CLM_CAMERA;

   // View position (relative to table bounds for legacy mode, relative to the bottom center of the table for others)
   float mViewX = 0.f;
   float mViewY = CMTOVPU(10.f);
   float mViewZ = CMTOVPU(70.f);
   float mLookAt = 0.25f; // Look at expressed as a camera inclination for legacy, or a percent of the table height, starting from bottom (0.25 is around top of slingshots)

   // Viewport adjustments
   float mViewportRotation = 0.f;
   float mViewportScaleX = 1.0f;
   float mViewportScaleY = 1.0f;

   // Camera & Legacy mode properties
   float mFOV = 45.0f; // Field of view, in degrees
   float mLayback = 0.0f; // A skewing angle that deform the table to make it look 'good'

   // Magic Window mode properties
   float mWindowTopXOfs = 0.0f; // Upper window border offset from left and right table bounds
   float mWindowTopYOfs = 0.0f; // Upper window border Y coordinate, relative to table top
   float mWindowTopZOfs = 0.0f; // Upper window border Z coordinate, relative to table playfield Z
   float mWindowBottomXOfs = 0.0f; // Lower window border offset from left and right table bounds
   float mWindowBottomYOfs = 0.0f; // Lower window border Y coordinate, relative to table bottom
   float mWindowBottomZOfs = 0.0f; // Lower window border Z coordinate, relative to table playfield Z
};
