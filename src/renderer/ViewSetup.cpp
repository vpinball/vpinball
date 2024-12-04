// license:GPLv3+

#include "core/stdafx.h"
#include "ViewSetup.h"

ViewSetup::ViewSetup()
{
}

// Update the view setup from the application/table settings and table property for window mode.
// - window position is relative to table playfield (likely at the glass position)
// - player position is defined in the app settings relatively from the bottom center of the screen (to avoid depending on a specific table)
void ViewSetup::SetWindowModeFromSettings(const PinTable* const table)
{
   float realToVirtual = GetRealToVirtualScale(table);
   vec3 playerPos(CMTOVPU(table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerX"s, 0.0f)),
                  CMTOVPU(table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerY"s, 0.0f)),
                  CMTOVPU(table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerZ"s, 70.0f)));
   float inclination = table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenInclination"s, 0.0f);
   float screenBotZ = GetWindowBottomZOFfset(table);
   float screenTopZ = GetWindowTopZOFfset(table);
   const Matrix3D rotx = // Rotate by the angle between playfield and real world horizontal (scale on Y and Z axis are equal and can be ignored)
      Matrix3D::MatrixRotateX(atan2f(screenTopZ - screenBotZ, table->m_bottom) - ANGTORAD(inclination));
   playerPos = rotx.MultiplyVectorNoPerspective(playerPos);
   mViewX = playerPos.x;
   mViewY = playerPos.y;
   mViewZ = playerPos.z + screenBotZ * mSceneScaleY / realToVirtual;
}

void ViewSetup::ApplyTableOverrideSettings(const Settings& settings, const ViewSetupID id)
{
   const string keyPrefix = id == BG_DESKTOP ? "ViewDT"s : id == BG_FSS ? "ViewFSS"s : "ViewCab"s;
   settings.LoadValue(Settings::TableOverride, keyPrefix + "Mode", (int&)mMode);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "ScaleX", mSceneScaleX);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "ScaleY", mSceneScaleY);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "ScaleZ", mSceneScaleZ);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "Rotation", mViewportRotation);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "PlayerX", mViewX);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "PlayerY", mViewY);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "PlayerZ", mViewZ);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "LookAt", mLookAt);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "FOV", mFOV);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "HOfs", mViewHOfs);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "VOfs", mViewVOfs);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "WindowTop", mWindowTopZOfs);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "WindowBot", mWindowBottomZOfs);
   settings.LoadValue(Settings::TableOverride, keyPrefix + "Layback", mLayback);
}

void ViewSetup::SaveToTableOverrideSettings(Settings& settings, const ViewSetupID id) const
{
   const string keyPrefix = id == BG_DESKTOP ? "ViewDT"s : id == BG_FSS ? "ViewFSS"s : "ViewCab"s;
   settings.SaveValue(Settings::TableOverride, keyPrefix + "Mode", mMode, false);
   settings.SaveValue(Settings::TableOverride, keyPrefix + "ScaleX", mSceneScaleX, false);
   settings.SaveValue(Settings::TableOverride, keyPrefix + "ScaleY", mSceneScaleY, false);
   settings.SaveValue(Settings::TableOverride, keyPrefix + "ScaleZ", mSceneScaleZ, false);
   settings.SaveValue(Settings::TableOverride, keyPrefix + "Rotation", mViewportRotation, false);
   if (mMode == VLM_LEGACY || mMode == VLM_CAMERA)
   {
      settings.SaveValue(Settings::TableOverride, keyPrefix + "PlayerX", mViewX, false);
      settings.SaveValue(Settings::TableOverride, keyPrefix + "PlayerY", mViewY, false);
      settings.SaveValue(Settings::TableOverride, keyPrefix + "PlayerZ", mViewZ, false);
      settings.SaveValue(Settings::TableOverride, keyPrefix + "LookAt", mLookAt, false);
      settings.SaveValue(Settings::TableOverride, keyPrefix + "FOV", mFOV, false);
   }
   else
   {
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "PlayerX");
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "PlayerY");
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "PlayerZ");
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "LookAt");
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "FOV");
   }
   if (mMode == VLM_CAMERA || mMode == VLM_WINDOW)
   {
      settings.SaveValue(Settings::TableOverride, keyPrefix + "HOfs", mViewHOfs, false);
      settings.SaveValue(Settings::TableOverride, keyPrefix + "VOfs", mViewVOfs, false);
   }
   else
   {
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "HOfs");
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "VOfs");
   }
   if (mMode == VLM_WINDOW)
   {
      settings.SaveValue(Settings::TableOverride, keyPrefix + "WindowTop", mWindowTopZOfs, false);
      settings.SaveValue(Settings::TableOverride, keyPrefix + "WindowBot", mWindowBottomZOfs, false);
   }
   else
   {
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "WindowTop");
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "WindowBot");
   }
   if (mMode == VLM_LEGACY)
   {
      settings.SaveValue(Settings::TableOverride, keyPrefix + "Layback", mLayback, false);
   }
   else
   {
      settings.DeleteValue(Settings::TableOverride, keyPrefix + "Layback");
   }
}

float ViewSetup::GetWindowTopZOFfset(const PinTable* const table) const
{
   if (mMode == VLM_WINDOW)
      return mWindowTopZOfs;
   else
      return 0.f;
}

float ViewSetup::GetWindowBottomZOFfset(const PinTable* const table) const
{
   // result is in the table coordinate system (so, usually between 0 and table->bottomglassheight)
   if (mMode == VLM_WINDOW)
      return mWindowBottomZOfs;
   else
      return 0.f;
}


float ViewSetup::GetRotation(const int viewportWidth, const int viewportHeight) const
{
   if (mMode == VLM_WINDOW)
   {
      int rotation = ((int)mViewportRotation) - (((int)mViewportRotation) / 360) * 360;
      return (float) (((viewportWidth < viewportHeight ? 0 : 3) * 90 + (rotation < 0 ? rotation + 360 : rotation)) % 360); // 0 / 90 / 180 / 270
   }
   else
   {
      float rot = fmodf(mViewportRotation, 360.f);
      return rot < 0.f ? (rot + 360.f) : rot;
   }
}

float ViewSetup::GetRealToVirtualScale(const PinTable* const table) const
{
   if (mMode == VLM_WINDOW)
   {
      float windowBotZ = GetWindowBottomZOFfset(table), windowTopZ = GetWindowTopZOFfset(table);
      const float screenHeight = table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenWidth"s, 0.0f); // Physical width (always measured in landscape orientation) is the height in window mode
      // const float inc = atan2f(mSceneScaleZ * (windowTopZ - windowBotZ), mSceneScaleY * table->m_bottom);
      const float inc = atan2f(windowTopZ - windowBotZ, table->m_bottom);
      return screenHeight <= 1.f ? 1.f : (VPUTOCM(table->m_bottom) / cosf(inc)) / screenHeight; // Ratio between screen height in virtual world to real world screen height
   }
   else
      return 1.f;
}

void ViewSetup::ComputeMVP(const PinTable* const table, const float aspect, const bool stereo, ModelViewProj& mvp, const vec3& cam, const float cam_inc, const float xpixoff, const float ypixoff)
{
   const float FOV = (mFOV < 1.0f) ? 1.0f : mFOV; // Can't have a real zero FOV, but this will look almost the same
   const bool isLegacy = mMode == VLM_LEGACY;
   const bool isWindow = mMode == VLM_WINDOW;
   float camx = cam.x, camy = cam.y, camz = cam.z;
   float windowBotZ = GetWindowBottomZOFfset(table), windowTopZ = GetWindowTopZOFfset(table);

   // Scale to convert a value expressed in the player 'real' world to our virtual world (where the geometry is defined)
   float realToVirtual = GetRealToVirtualScale(table);

   // Viewport rotation. Window mode does not support free rotation (since we fit the table to the screen)
   float rotation;
   int quadrant;
   if (isWindow)
   {
      quadrant = (int)mViewportRotation - ((int)mViewportRotation / 360) * 360;
      quadrant = (aspect < 1.f ? 0 : 3) + (quadrant < 0 ? quadrant + 360 : quadrant) / 90; // 0 / 90 / 180 / 270
      rotation = ANGTORAD((float)(quadrant * 90));
   }
   else
   {
      rotation = ANGTORAD(mViewportRotation);
   }

   // View angle inclination against playfield. 0 is straight up the playfield.
   float inc;
   switch (mMode)
   {
   case VLM_LEGACY: inc = ANGTORAD(mLookAt) + cam_inc; break;
   case VLM_CAMERA: inc = -M_PIf + atan2f(-mViewY + cam.y - (mLookAt / 100.0f) * table->m_bottom, -mViewZ + cam.z); break;
   //case VLM_WINDOW: inc = atan2f(mSceneScaleZ * (windowTopZ - windowBotZ), mSceneScaleY * table->m_bottom); break;
   case VLM_WINDOW: inc = atan2f(windowTopZ - windowBotZ, table->m_bottom); break;
   }

   if (isLegacy && table->m_BG_enable_FSS)
   {
      // for FSS, force an offset to camy which drops the table down 1/3 of the way.
      // some values to camy have been commented out because I found the default value
      // better and just modify the camz and keep the table design inclination
      // within 50-60 deg and 40-50 FOV in editor.
      // these values were tested against all known video modes upto 1920x1080
      // in landscape and portrait on the display
      camy += cam.y + 500.0f;
      inc += 0.2f;
      const int width = GetSystemMetrics(SM_CXSCREEN);
      const int height = GetSystemMetrics(SM_CYSCREEN);
      if ((aspect > 1.f) && (height < width))
      {
         // layout landscape(game horz) in lanscape(LCD\LED horz)
         //inc += 0.1f;       // 0.05-best, 0.1-good, 0.2-bad > (0.2 terrible original)
         //camy -= 30.0f;     // 70.0f original // 100
         if (aspect > 1.6f)
            camz -= 1170.0f; // 700
         else if (aspect > 1.5f)
            camz -= 1070.0f; // 650
         else if (aspect > 1.4f)
            camz -= 900.0f;  // 580
         else if (aspect > 1.3f)
            camz -= 820.0f;  // 500 // 600
         else
            camz -= 800.0f;  // 480
      }
      else if (height > width)
      {
         // layout portrait(game vert) in portrait(LCD\LED vert)
         if (aspect > 0.6f)
            camz += 10.0f;  // 50
         else if (aspect > 0.5f)
            camz += 300.0f; // 100
         else
            camz += 300.0f; // 200
      }
      // Other layouts are not really supported: landscape(game horz) in portrait(LCD\LED vert), who would but the UI allows for it!
   }

   // Original matrix stack was: [Lb.Rx.Rz.T.S.Rpi] . [P]   ([first] part is view matrix, [second] is projection matrix)
   // This leads to a non orthonormal view matrix since layback and scaling are part of it. This slightly breaks lighting, reflection and stereo. This also break the reflection/lighting since the viewport rotation is part of the view matrix (hence the hack for ball shading in cab mode)
   //
   // To improve this situation, the matrices where adapted to only keep translation & view rotation in the view matrix. A new 'camera' view setup
   // was also added to improve camera setup (real world unit, layback replaced by view offset,...). The final matrix stacks are the following:
   // 'legacy' mode: [T.Rx] . [invRx.invT.Lb.T.Rx.Rz.Rpi.S.P]
   // 'camera' mode: [T.Rx] . [                   Rz.Rpi.S.P]
   //
   // This seems ok for 'camera' mode since the view is orthonormal (it transforms normals and points without changing their length or relative angle).

   const Matrix3D matWorld = Matrix3D::MatrixIdentity();
   mvp.SetModel(matWorld);

   Matrix3D scale, coords, lookat, layback, matView;
   const Matrix3D projTrans = Matrix3D::MatrixTranslate(xpixoff, ypixoff, 0.f); // in-pixel offset for manual oversampling
   const Matrix3D rotz = Matrix3D::MatrixRotateZ(rotation); // Viewport rotation

   vector<Vertex3Ds> bounds, legacy_bounds;
   bounds.reserve(table->m_vedit.size() * 8); // upper bound estimate
   if (isLegacy)
      legacy_bounds.reserve(table->m_vedit.size() * 8); // upper bound estimate
   for (IEditable* editable : table->m_vedit)
      editable->GetBoundingVertices(bounds, isLegacy ? &legacy_bounds : nullptr); // Collect part bounds to fit the legacy mode camera?

   // Compute translation
   // Also setup a dedicated matView (complete matrix stack excepted projection) to be able to compute near and far plane
   if (isLegacy)
   {
      // Layback to skew the view backward
      layback = Matrix3D::MatrixIdentity();
      layback._32 = -tanf(0.5f * ANGTORAD(mLayback));

       // Revert Y and Z axis to convert to D3D coordinate system and perform scaling as a last step
      coords = Matrix3D::MatrixScale(mSceneScaleX, -mSceneScaleY, -mSceneScaleZ);

      const vec3 fit = FitCameraToVertices(legacy_bounds, aspect, rotation, inc, FOV, mViewZ, mLayback);
      legacy_bounds.clear();
      vec3 pos = vec3(mViewX - fit.x + camx, mViewY - fit.y + camy, -fit.z + camz);
      // For some reason I don't get, viewport rotation (rotz) and head inclination (rotx) used to be swapped when in camera mode on desktop.
      // This is no more applied and we always consider this order: rotx * rotz * trans which we swap to apply it as trans * rotx * rotz
      Matrix3D rot = Matrix3D::MatrixRotateZ(-rotation); // convert position to swap trans and rotz (rotz * trans => trans * rotz)
      pos = rot.MultiplyVectorNoPerspective(pos);
      rot = Matrix3D::MatrixRotateX(-inc); // convert position to swap trans and rotx (rotx * trans => trans * rotx)
      pos = rot.MultiplyVectorNoPerspective(pos);
      const Matrix3D trans = Matrix3D::MatrixTranslate(pos.x, pos.y, pos.z);
      const Matrix3D rotx = Matrix3D::MatrixRotateX(inc); // Player head inclination (0 is looking straight to playfield)
      lookat = trans * rotx;
      scale = Matrix3D::MatrixIdentity();
      matView = layback /** scale*/ * trans * rotx * rotz * coords;
   }
   else
   {
      // The transforms are applied in this order: scale, then translation, then X rotation (aka inclination)

      // Scale transform table model into a scaled version, at real world scale: therefore transformations applied afterward are in 'real world scale' (user scale)
      scale = Matrix3D::MatrixTranslate(-0.5f * table->m_right, -0.5f * table->m_bottom, -windowBotZ)
            * Matrix3D::MatrixScale(mSceneScaleX / realToVirtual, mSceneScaleY / realToVirtual, isWindow ? mSceneScaleY / realToVirtual : mSceneScaleZ)
            * Matrix3D::MatrixTranslate(0.5f * table->m_right, 0.5f * table->m_bottom, windowBotZ); // Global scene scale (using bottom center of the playfield as origin)
      // mView is in real world scale (like the actual display size), it is applied after scale which scale the table to the user's real world scale
      Matrix3D trans = Matrix3D::MatrixTranslate(-mViewX + cam.x - 0.5f * table->m_right, -mViewY + cam.y - table->m_bottom, -mViewZ + cam.z);
      Matrix3D rotx = Matrix3D::MatrixRotateX(inc); // Player head inclination (0 is looking straight to playfield)
      coords = Matrix3D::MatrixScale(1.f, -1.f, -1.f); // Revert Y and Z axis to convert to D3D coordinate system
      lookat = trans * rotx;
      matView = scale * trans * rotx * rotz * coords;
   }

   // Compute frustum Z bounds (near/far plane), taking in account base view but also reflected point of view to avoid depth clipping in reflection probes
   float zNear, zFar;
   table->ComputeNearFarPlane(bounds, matView, 1.f, zNear, zFar);
   for (auto probe : table->m_vrenderprobe)
   {
      if (probe->GetType() == RenderProbe::PLANE_REFLECTION)
      {
         vec4 plane;
         probe->GetReflectionPlane(plane);
         Vertex3Ds pn = Vertex3Ds(plane.x, plane.y, plane.z);
         pn.Normalize();
         const Matrix3D reflect = Matrix3D::MatrixPlaneReflection(pn, plane.w);
         const Matrix3D probeView = reflect * matView;
         float zNearProbe, zFarProbe;
         table->ComputeNearFarPlane(bounds, probeView, 1.f, zNearProbe, zFarProbe);
         zNear = min(zNear, zNearProbe);
         zFar = max(zFar, zFarProbe);
      }
   }
   bounds.clear();

   // Compute frustum X/Y bounds
   float xcenter, ycenter, xspan, yspan;
   switch (mMode)
   {
   case VLM_LEGACY:
   {
      xcenter = 0.0f;
      ycenter = 0.0f;
      yspan = zNear * tanf(0.5f * ANGTORAD(FOV));
      xspan = yspan * aspect;
      break;
   }
   case VLM_CAMERA:
   {
      const float ymax = zNear * tanf(0.5f * ANGTORAD(FOV));
      xcenter = zNear * 0.01f * (mViewVOfs * sinf(rotation) - mViewHOfs * cosf(rotation));
      ycenter = zNear * 0.01f * (mViewVOfs * cosf(rotation) + mViewHOfs * sinf(rotation));
      yspan = ymax;
      xspan = ymax * aspect;
      break;
   }
   case VLM_WINDOW:
   {
      // Fit camera to adjusted table bounds, along vertical axis
      const Matrix3D fit = 
           Matrix3D::MatrixTranslate(-0.5f * table->m_right, -0.5f * table->m_bottom, -windowBotZ) // Center of scaling
         * Matrix3D::MatrixScale(1.f / realToVirtual) // We do not apply the scene scale since we want to fit the scaled version of the table as if it was the normal version (otherwise it would reverse the scaling during the fitting)
         * Matrix3D::MatrixTranslate(0.5f * table->m_right, 0.5f * table->m_bottom, windowBotZ) // Reverse center of scaling
         * lookat * rotz * Matrix3D::MatrixScale(1.f, -1.f, -1.f) // Camera pos and inclination, vieport rotation
         * Matrix3D::MatrixPerspectiveFovLH(90.f, 1.0f, zNear, zFar); // For fitting, we use a vertical FOV of 90deg, leading to a yspan of 2, and an aspect ratio of 1, also leading to a xspan of 2
      const float centerAxis = 0.5f * (table->m_left + table->m_right);
      const Vertex3Ds top = fit * Vertex3Ds{centerAxis, table->m_top, windowTopZ};
      const Vertex3Ds bottom = fit * Vertex3Ds{centerAxis, table->m_bottom, windowBotZ};
      const float xmin = zNear * min(bottom.x, top.x), xmax = zNear * max(bottom.x, top.x);
      const float ymin = zNear * min(bottom.y, top.y), ymax = zNear * max(bottom.y, top.y);
      const float screenHeight = table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenWidth"s, 0.0f); // Physical width (always measured in landscape orientation) is the height in window mode
      float offsetScale;
      if ((quadrant & 1) == 0) // 0 & 180
      {
         offsetScale = screenHeight <= 1.f ? 1.f : ((ymax - ymin) / (screenHeight * realToVirtual / mSceneScaleY));
         yspan = 0.5f * (ymax - ymin);
         xspan = yspan * aspect;
      }
      else // 90 & 270
      {
         offsetScale = screenHeight <= 1.f ? 1.f : ((xmax - xmin) / (screenHeight * realToVirtual / mSceneScaleY));
         xspan = 0.5f * (xmax - xmin);
         yspan = xspan / aspect;
      }
      xcenter = 0.5f * (xmin + xmax) + offsetScale * (mViewVOfs * sinf(rotation) - mViewHOfs * cosf(rotation));
      ycenter = 0.5f * (ymin + ymax) + offsetScale * (mViewVOfs * cosf(rotation) + mViewHOfs * sinf(rotation));
      break;
   }
   }

   // Define the view matrix. This matrix MUST be orthonormal (orthogonal axis with a unit length) or shading will be broken
   mvp.SetView(lookat);

   // Apply non uniform scene scaling after shading, in the projection matrix
   Matrix3D invLookAt(lookat);
   invLookAt.Invert();
   scale = invLookAt * scale * lookat;

   // Define the projection matrices
   if (stereo)
   {
      // Stereo mode needs an offcentered projection along the eye axis. Parallax shifted, or toe-in camera are incorrect and 
      // should not be used (See 'Gaze-coupled Perspective for Enhanced Human-Machine Interfaces in Aeronautics' for an explanation)
      // Since the table is scaled to 'real world units' (that is to say same scale as the user measures), we directly use the user settings for IPD,.. without any scaling

      // 63mm is the average distance between eyes (varies from 54 to 74mm between adults, 43 to 58mm for children)
      const float eyeSeparation = MMTOVPU(table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DEyeSeparation"s, 63.0f));

      // Z where the stereo separation is 0:
      // - for cabinet (window) mode, we use the orthogonal distance to the screen (window)
      // - for camera mode, we place the 0 depth line on the lower third of the playfield
      // This way, we don't have negative parallax (objects closer than projection plane) and all the artefact they may cause
      const float zNullSeparation = mMode == VLM_WINDOW ? (mViewZ - windowBotZ) * cosf(inc) + mViewY * sinf(inc)
                                                        : -(lookat * Vertex3Ds{0.5f * (table->m_left + table->m_right), table->m_bottom * 0.66f /* For flipper bats: 1800.f / 2150.f*/, 0.f}).z;
      const float ofs = 0.5f * eyeSeparation * zNear / zNullSeparation;
      const float xOfs = ofs * cosf(rotation);
      const float yOfs = ofs * sinf(rotation);

      // Compute the view orthonormal basis
      const Matrix3D invView(mvp.GetView());
      const vec3 right = invView.GetOrthoNormalRight();

      // Left eye
      Matrix3D eyeShift = Matrix3D::MatrixTranslate(0.5f * eyeSeparation * right);
      Matrix3D proj = Matrix3D::MatrixPerspectiveOffCenterLH(xcenter - xspan + xOfs, xcenter + xspan + xOfs, ycenter - yspan - yOfs, ycenter + yspan - yOfs, zNear, zFar);
      mvp.SetProj(0, scale * eyeShift * rotz * coords * proj * projTrans);

      // Right eye
      eyeShift = Matrix3D::MatrixTranslate(-0.5f * eyeSeparation * right);
      proj = Matrix3D::MatrixPerspectiveOffCenterLH(xcenter - xspan - xOfs, xcenter + xspan - xOfs, ycenter - yspan + yOfs, ycenter + yspan + yOfs, zNear, zFar);
      mvp.SetProj(1, scale * eyeShift * rotz * coords * proj * projTrans);
   }
   else
   {
      const Matrix3D proj = Matrix3D::MatrixPerspectiveOffCenterLH(xcenter - xspan, xcenter + xspan, ycenter - yspan, ycenter + yspan, zNear, zFar);
      mvp.SetProj(0, scale * rotz * coords * proj * projTrans);
   }

   // Apply layback
   if (isLegacy && fabsf(mLayback) > 0.1f)
   {
      // To be backward compatible while having a well behaving view matrix, we compute a view without the layback (which is meaningful with regards to what was used before).
      // We use it for rendering computation. It is reverted by the projection matrix which then apply the old transformation, including layback.
      Matrix3D invView(mvp.GetView());
      invView.Invert();
      const Matrix3D tmp = invView * layback * mvp.GetView();
      mvp.SetProj(0, tmp * mvp.GetProj(0));
      if (stereo) // Real stereo is not really supported for legacy camera mode (it used to be only fake parallax stereo)
         mvp.SetProj(1, tmp * mvp.GetProj(1));
   }

   if (!stereo && mvp.m_nEyes > 1)
      mvp.SetProj(1, mvp.GetProj(0));
}


vec3 ViewSetup::FitCameraToVertices(const vector<Vertex3Ds>& pvvertex3D, const float aspect, const float rotation, const float inclination, 
   const float FOV, const float xlatez, const float layback)
{
   // Determine camera distance
   const float rrotsin = sinf(rotation);
   const float rrotcos = cosf(rotation);
   const float rincsin = sinf(inclination);
   const float rinccos = cosf(inclination);

   const float slopey = tanf(0.5f * ANGTORAD(FOV)); // *0.5 because slope is half of FOV - FOV includes top and bottom

   // Field of view along the axis = atan(tan(yFOV)*width/height)
   // So the slope of x simply equals slopey*width/height

   const float slopex = slopey * aspect;

   float maxyintercept = -FLT_MAX;
   float minyintercept = FLT_MAX;
   float maxxintercept = -FLT_MAX;
   float minxintercept = FLT_MAX;

   Matrix3D laybackTrans = Matrix3D::MatrixIdentity();
   laybackTrans._32 = -tanf(0.5f * ANGTORAD(layback));

   for (size_t i = 0; i < pvvertex3D.size(); ++i)
   {
      Vertex3Ds v = laybackTrans * pvvertex3D[i];

      // Rotate vertex about x axis according to incoming inclination
      float temp = v.y;
      v.y = rinccos * temp - rincsin * v.z;
      v.z = rincsin * temp + rinccos * v.z;

      // Rotate vertex about z axis according to incoming rotation
      temp = v.x;
      v.x = rrotcos * temp - rrotsin * v.y;
      v.y = rrotsin * temp + rrotcos * v.y;

      // Extend slope lines from point to find camera intersection
      maxyintercept = max(maxyintercept, v.y + slopey * v.z);
      minyintercept = min(minyintercept, v.y - slopey * v.z);
      maxxintercept = max(maxxintercept, v.x + slopex * v.z);
      minxintercept = min(minxintercept, v.x - slopex * v.z);
   }

   // PLOGD << "maxy: " << maxyintercept;
   // PLOGD << "miny: " << minyintercept;
   // PLOGD << "maxx: " << maxxintercept;
   // PLOGD << "minx: " << minxintercept;

   // Find camera center in xy plane
   const float ydist = (maxyintercept - minyintercept) / (slopey * 2.0f);
   const float xdist = (maxxintercept - minxintercept) / (slopex * 2.0f);
   return vec3((maxxintercept + minxintercept) * 0.5f, (maxyintercept + minyintercept) * 0.5f, max(ydist, xdist) + xlatez);
}
