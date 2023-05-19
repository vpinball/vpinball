#include "stdafx.h"
#include "ViewSetup.h"

ViewSetup::ViewSetup()
{
}

void ViewSetup::ComputeMVP(const PinTable* const table, const int viewportWidth, const int viewportHeight, const bool stereo, ModelViewProj& mvp, const vec3& cam, const float cam_inc,
   const float xpixoff, const float ypixoff)
{
   const ViewLayoutMode layoutMode = mMode;
   const float rotation = ANGTORAD(mViewportRotation);
   const float FOV = (mFOV < 1.0f) ? 1.0f : mFOV; // Can't have a real zero FOV, but this will look almost the same
   const bool isLegacy = layoutMode == VLM_LEGACY;
   //const bool isCamera = layoutMode == VLM_CAMERA;
   //const bool isWindow = layoutMode == VLM_WINDOW;
   const float aspect = (float)((double)viewportWidth / (double)viewportHeight);
   float camx = cam.x, camy = cam.y, camz = cam.z;
   // For 'Window' mode, we can either have a perspective projection parallel to the screen then reprojected to a view aligned,
   // or a perspective projection aligned to the view which is then perspective corrected. Both implementations are kept for reference.
   // For the time being, the reprojection postprocess step is not implemented so only screen aligned gives acceptable (distorted) results.
   constexpr bool isWndScreenAligned = true;
   
   // View angle inclination against playfield. 0 is straight up the playfield.
   float inc;
   switch (mMode)
   {
   case VLM_LEGACY: inc = ANGTORAD(mLookAt) + cam_inc; break;
   case VLM_CAMERA: inc = -M_PIf + atan2f(-mViewY + cam.y - (mLookAt / 100.0f) * table->m_bottom, -mViewZ + cam.z); break;
   case VLM_WINDOW: inc = isWndScreenAligned ? atan2f(mWindowTopZOfs - mWindowBottomZOfs, table->m_bottom) // Screen angle, relative to playfield
                                             : (-M_PIf + atan2f(-mViewY + cam.y - (mLookAt / 100.0f) * table->m_bottom, -mViewZ + cam.z)); break;
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
      if ((viewportWidth > viewportHeight) && (height < width))
      {
         // layout landscape(game horz) in lanscape(LCD\LED horz)
         //inc += 0.1f;       // 0.05-best, 0.1-good, 0.2-bad > (0.2 terrible original)
         //camy -= 30.0f;     // 70.0f original // 100
         if (aspect > 1.6f)
            camz -= 1170.0f; // 700
         else if (aspect > 1.5f)
            camz -= 1070.0f; // 650
         else if (aspect > 1.4f)
            camz -= 900.0f; // 580
         else if (aspect > 1.3f)
            camz -= 820.0f; // 500 // 600
         else
            camz -= 800.0f; // 480
      }
      else if (height > width)
      {
         // layout portrait(game vert) in portrait(LCD\LED vert)
         if (aspect > 0.6f)
            camz += 10.0f; // 50
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

   Matrix3D matWorld, matView, matProj[2];
   matWorld.SetIdentity();

   Matrix3D coords, lookat, rotz, proj, projTrans, layback;
   projTrans.SetTranslation((float)((double)xpixoff / (double)viewportWidth), (float)((double)ypixoff / (double)viewportHeight), 0.f); // in-pixel offset for manual oversampling
   coords.SetScaling(isLegacy ? mViewportScaleX : 1.f, isLegacy ? -mViewportScaleY : -1.f, -1.f); // Stretch viewport, also revert Y and Z axis to convert to D3D coordinate system
   rotz.SetRotateZ(rotation); // Viewport rotation
   layback.SetIdentity(); // Layback to skew the view backward
   layback._32 = -tanf(0.5f * ANGTORAD(mLayback));

   // Collect part bounds, to fit the legacy mode camera, and to adjust the near/far planes for all modes
   vector<Vertex3Ds> vvertex3D;
   for (size_t i = 0; i < table->m_vedit.size(); ++i)
      table->m_vedit[i]->GetBoundingVertices(vvertex3D);

   // Compute translation
   // Also setup matView (complete matrix stack excepted projection) to be able to compute near and far plane
   if (isLegacy)
   {
      vec3 fit = FitCameraToVertices(vvertex3D, aspect, rotation, inc, FOV, mViewZ, mLayback);
      vec3 pos = vec3(mViewX - fit.x + camx, mViewY - fit.y + camy, -fit.z + camz);
      // For some reason I don't get, viewport rotation (rotz) and head inclination (rotx) used to be swapped when in camera mode on desktop.
      // This is no more applied and we always consider this order: rotx * rotz * trans which we swap to apply it as trans * rotx * rotz
      Matrix3D rot, trans, rotx;
      rot.SetRotateZ(-rotation); // convert position to swap trans and rotz (rotz * trans => trans * rotz)
      rot.TransformVec3(pos);
      rot.SetRotateX(-inc); // convert position to swap trans and rotx (rotx * trans => trans * rotx)
      rot.TransformVec3(pos);
      trans.SetTranslation(pos.x, pos.y, pos.z);
      rotx.SetRotateX(inc); // Player head inclination (0 is looking straight to playfield)
      lookat = trans * rotx;
      matView = layback * lookat * rotz * projTrans * coords;
   }
   else
   {
      Matrix3D trans, rotx;
      trans.SetTranslation(-mViewX + cam.x - 0.5f * table->m_right, -mViewY + cam.y - table->m_bottom, -mViewZ + cam.z);
      rotx.SetRotateX(inc); // Player head inclination (0 is looking straight to playfield)
      lookat = trans * rotx;
      matView = lookat * rotz * projTrans * coords;
   }

   // Compute near/far plane
   float zNear = FLT_MAX, zFar = -FLT_MAX;
   Matrix3D matWorldView(matWorld);
   matView.Multiply(matWorld, matWorldView);
   for (size_t i = 0; i < vvertex3D.size(); ++i)
   {
      const float tempz = matWorldView.MultiplyVector(vvertex3D[i]).z;
      zNear = min(zNear, tempz);
      zFar = max(zFar, tempz);
   }
   // Avoid near plane below 1 which result in loss of precision and z rendering artefacts
   if (zNear < 1.0f)
      zNear = 1.0f;
   zFar *= 1.01f;
   // Avoid div-0 problem (div by far - near)
   if (zFar <= zNear)
      zFar = zNear + 1.0f;
   // FIXME for the time being, the result is not that good since neither primitives vertices are taken in account, nor reflected geometry, so just add a margin
   zFar += 1000.0f;
   if (fabsf(inc) < 0.0075f) //!! magic threshold, otherwise kicker holes are missing for inclination ~0
      zFar += 10.f;

   switch (layoutMode)
   {
   case VLM_LEGACY:
      proj.SetPerspectiveFovLH(FOV, aspect, zNear, zFar);
      break;
   case VLM_CAMERA:
   {
      const float ymax = zNear * tanf(0.5f * ANGTORAD(FOV));
      const float ymin = -ymax;
      const float xmax = ymax * aspect;
      const float xmin = -xmax;
      const float xcenter = 0.5f * (xmin + xmax) + zNear * 0.01f * (mViewVOfs * sinf(rotation) - mViewHOfs * cosf(rotation));
      const float ycenter = 0.5f * (ymin + ymax) + zNear * 0.01f * (mViewVOfs * cosf(rotation) + mViewHOfs * sinf(rotation));
      const float xspan = 0.5f * (xmax - xmin) / mViewportScaleX;
      const float yspan = 0.5f * (ymax - ymin) / mViewportScaleY;
      proj.SetPerspectiveOffCenterLH(xcenter - xspan, xcenter + xspan, ycenter - yspan, ycenter + yspan, zNear, zFar);
      break;
   }
   case VLM_WINDOW:
   {
      // Fit camera to adjusted table bounds
      Matrix3D fit = lookat * rotz * projTrans * coords * Matrix3D::MatrixPerspectiveFovLH(90.f, 1.f, zNear, zFar);
      Vertex3Ds tl = fit.MultiplyVector(Vertex3Ds(table->m_left - mWindowTopXOfs, table->m_top - mWindowTopYOfs, mWindowTopZOfs));
      Vertex3Ds tr = fit.MultiplyVector(Vertex3Ds(table->m_right + mWindowTopXOfs, table->m_top - mWindowTopYOfs, mWindowTopZOfs));
      Vertex3Ds bl = fit.MultiplyVector(Vertex3Ds(table->m_left - mWindowBottomXOfs, table->m_bottom + mWindowBottomYOfs, mWindowBottomZOfs));
      Vertex3Ds br = fit.MultiplyVector(Vertex3Ds(table->m_right + mWindowBottomXOfs, table->m_bottom + mWindowBottomYOfs, mWindowBottomZOfs));
      const float ymax = zNear * max(br.y, max(bl.y, max(tl.y, tr.y)));
      const float ymin = zNear * min(br.y, min(bl.y, min(tl.y, tr.y)));
      const float xmax = zNear * max(br.x, max(bl.x, max(tl.x, tr.x)));
      const float xmin = zNear * min(br.x, min(bl.x, min(tl.x, tr.x)));
      const float ar = (xmax - xmin) / (ymax - ymin);
      const float xcenter = 0.5f * (xmin + xmax) + zNear * 0.01f * (mViewVOfs * sinf(rotation) - mViewHOfs * cosf(rotation));
      const float ycenter = 0.5f * (ymin + ymax) + zNear * 0.01f * (mViewVOfs * cosf(rotation) + mViewHOfs * sinf(rotation));
      const float xspan = (aspect / ar) * 0.5f * (xmax - xmin) / mViewportScaleX;
      const float yspan =                 0.5f * (ymax - ymin) / mViewportScaleY;
      proj.SetPerspectiveOffCenterLH(xcenter - xspan, xcenter + xspan, ycenter - yspan, ycenter + yspan, zNear, zFar);
      break;
   }
   }

   matView = lookat;
   matProj[0] = rotz * projTrans * coords * proj;

   // Apply layback
   // To be backward compatible while having a well behaving view matrix, we compute a view without the layback (which is meaningful with regards to what was used before).
   // We use it for rendering computation. It is reverted by the projection matrix which then apply the old transformation, including layback.
   if (isLegacy && fabsf(mLayback) > 0.1f)
   {
      Matrix3D invView(matView);
      invView.Invert();
      matProj[0] = (invView * layback * matView) * matProj[0];
   }

   if (stereo)
   {
      // Create eye projection matrices for real stereo (not VR but anaglyph,...)
      // 63mm is the average distance between eyes (varies from 54 to 74mm between adults, 43 to 58mm for children)
      const float stereo3DMS = LoadValueWithDefault(regKey[RegName::Player], "Stereo3DEyeSeparation"s, 63.0f);
      const float halfEyeDist = 0.5f * MMTOVPU(stereo3DMS);
      Matrix3D invView(matView);
      invView.Invert();
      // Compute the view orthonormal basis
      Matrix3D baseCoords;
      baseCoords.SetScaling(1.f, -1.f, -1.f);
      const Matrix3D baseView = baseCoords * invView;
      const vec3 right = baseView.GetOrthoNormalRight();
      const vec3 up = baseView.GetOrthoNormalUp();
      const vec3 dir = baseView.GetOrthoNormalDir();
      const vec3 pos = baseView.GetOrthoNormalPos();
      constexpr StereoEyeMode stereoEyeMode = SEM_TOEIN;
      switch (stereoEyeMode)
      {
      case SEM_PARRALEL:
      {
         Matrix3D eyeShift;
         eyeShift.SetTranslation(halfEyeDist * right); // Right
         matProj[1] = eyeShift * matProj[0];
         eyeShift.SetTranslation(-halfEyeDist * right); // Left
         matProj[0] = eyeShift * matProj[0];
         break;
      }
      case SEM_OFFAXIS:
      {
         Matrix3D eyeShift;
         eyeShift.SetTranslation(halfEyeDist * right); // Right
         matProj[1] = eyeShift * matProj[0];
         eyeShift.SetTranslation(-halfEyeDist * right); // Left
         matProj[0] = eyeShift * matProj[0];
         assert(false); // Missing projection correction
         break;
      }
      case SEM_TOEIN:
      {
         // Default is to look at the ball (playfield level = 0 + ball radius = 50)
         const float camDistance = (50.f - pos.z) / dir.z;
         // Clamp it to a reasonable range, a normal viewing distance being around 80cm between view focus (table) and viewer (depends a lot on the player size & position)
         constexpr float minCamDistance = CMTOVPU(30.f);
         constexpr float maxCamDistance = CMTOVPU(200.f);
         const vec3 at = pos + dir * clamp(camDistance, minCamDistance, maxCamDistance);
         Matrix3D toein_lookat = Matrix3D::MatrixLookAtLH(pos + (halfEyeDist * right), at, up); // Apply offset & rotation to the right eye projection
         matProj[1] = invView * toein_lookat * baseCoords * matProj[0];
         toein_lookat = Matrix3D::MatrixLookAtLH(pos + (-halfEyeDist * right), at, up); // Apply offset & rotation to the left eye projection
         matProj[0] = invView * toein_lookat * baseCoords * matProj[0];
         break;
      }
      default:
         assert(false); // Unsupported stereo eye mode
         matProj[1] = matProj[0];
         break;
      }
   }

   mvp.SetModel(matWorld);
   mvp.SetView(matView);
   for (unsigned int eye = 0; eye < mvp.m_nEyes; eye++)
      mvp.SetProj(eye, matProj[eye]);
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

   Matrix3D laybackTrans;
   laybackTrans.SetIdentity();
   laybackTrans._32 = -tanf(0.5f * ANGTORAD(layback));

   for (size_t i = 0; i < pvvertex3D.size(); ++i)
   {
      Vertex3Ds v = laybackTrans.MultiplyVector(pvvertex3D[i]);

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

   slintf("maxy: %f\n", maxyintercept);
   slintf("miny: %f\n", minyintercept);
   slintf("maxx: %f\n", maxxintercept);
   slintf("minx: %f\n", minxintercept);

   // Find camera center in xy plane
   const float ydist = (maxyintercept - minyintercept) / (slopey * 2.0f);
   const float xdist = (maxxintercept - minxintercept) / (slopex * 2.0f);
   return vec3((maxxintercept + minxintercept) * 0.5f, (maxyintercept + minyintercept) * 0.5f, max(ydist, xdist) + xlatez);
}
