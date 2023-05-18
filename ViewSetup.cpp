#include "stdafx.h"
#include "ViewSetup.h"

ViewSetup::ViewSetup()
{
}

void ViewSetup::ComputeMVP(const PinTable* table, const int viewportWidth, const int viewportHeight, const bool stereo, ModelViewProj& mvp, const vec3& cam, const float cam_inc,
   const float xpixoff, const float ypixoff)
{
   const float rotation = ANGTORAD(mViewportRotation);
   const float inclination = ANGTORAD(mLookAt);
   const float FOV = (mFOV < 1.0f) ? 1.0f : mFOV; // Can't have a real zero FOV, but this will look almost the same
   const ViewLayoutMode layoutMode = mMode;
   const bool FSS_mode = table->m_BG_enable_FSS;
   const bool isLegacy = layoutMode == CLM_LEGACY;

   const float aspect = ((float)viewportWidth) / ((float)viewportHeight);

   vector<Vertex3Ds> vvertex3D;
   for (size_t i = 0; i < table->m_vedit.size(); ++i)
      table->m_vedit[i]->GetBoundingVertices(vvertex3D);

   // next 4 def values for layout portrait(game vert) in landscape(screen horz)
   // for FSS, force an offset to camy which drops the table down 1/3 of the way.
   // some values to camy have been commented out because I found the default value
   // better and just modify the camz and keep the table design inclination
   // within 50-60 deg and 40-50 FOV in editor.
   // these values were tested against all known video modes upto 1920x1080
   // in landscape and portrait on the display
   const float camx = cam.x;
   const float camy = cam.y + (isLegacy && FSS_mode ? 500.0f : 0.f);
   float camz = cam.z;
   const float inc = inclination + cam_inc + (isLegacy && FSS_mode ? 0.2f : 0.f);

   if (isLegacy && FSS_mode)
   {
      const int width = GetSystemMetrics(SM_CXSCREEN);
      const int height = GetSystemMetrics(SM_CYSCREEN);

      // layout landscape(game horz) in lanscape(LCD\LED horz)
      if ((viewportWidth > viewportHeight) && (height < width))
      {
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
      else
      {
         // layout potrait(game vert) in portrait(LCD\LED vert)
         if (height > width)
         {
            if (aspect > 0.6f)
            {
               camz += 10.0f;
               //camy += 50.0f;
            }
            else if (aspect > 0.5f)
            {
               camz += 300.0f;
               //camy += 100.0f;
            }
            else
            {
               camz += 300.0f;
               //camy += 200.0f;
            }
         }
         // layout landscape(game horz) in portrait(LCD\LED vert), who would but the UI allows for it!
         else
         {
         }
      }
   }

   // Original matrix stack was: [Lb.Rx.Rz.T.S.Rpi] . [P]   ([first] part is view matrix, [second] is projection matrix)
   // This leads to a non orthonormal view matrix since layback and scaling are part of it. This slightly breaks lighting, reflection and stereo.
   //
   // To improve this situation, a new 'absolute' camera mode was added. The matrix stack was also modified to move scaling (S) and DirectX
   // coordinate system (Rpi) from view to projection matrix. The final matrix stacks are the following:
   // 'relative' mode: [Lb.Rx.Rz.T] . [Rpi.S.P]
   // 'absolute' mode:       [T.Rx] . [Ry.Rpi.S.P]
   //
   // This seems ok for 'absolute' mode since the view is orthonormal (it transforms normals and points without changing their length or relative angle).

   Matrix3D matWorld, matView, matProj[2];
   matWorld.SetIdentity();

   Matrix3D coords, rotx, trans, rotz, proj, projTrans, layback;
   projTrans.SetTranslation((float)((double)xpixoff / (double)viewportWidth), (float)((double)ypixoff / (double)viewportHeight), 0.f); // in-pixel offset for manual oversampling
   coords.SetScaling(mViewportScaleX, -mViewportScaleY, -1.f); // Stretch viewport, also revert Y and Z axis to convert to D3D coordinate system
   rotx.SetRotateX(inc); // Player head inclination
   rotz.SetRotateZ(rotation); // Viewport rotation
   layback.SetIdentity(); // Layback to skew the view backward
   layback._32 = -tanf(0.5f * ANGTORAD(mLayback));

   // Compute translation
   if (isLegacy)
   {
      vec3 fit = FitCameraToVertices(vvertex3D, aspect, rotation, inc, FOV, mViewZ, mLayback);
      vec3 pos = vec3(mViewX - fit.x + camx, mViewY - fit.y + camy, -fit.z + camz);
      // For some reason I don't get, viewport rotation (rotz) and head inclination (rotx) used to be swapped when in camera mode on desktop.
      // This is no more applied and we always consider this order: rotx * rotz * trans which we swap to apply it as trans * rotx * rotz
      Matrix3D rot;
      rot.SetRotateZ(-rotation); // convert position to swap trans and rotz (rotz * trans => trans * rotz)
      rot.TransformVec3(pos);
      rot.SetRotateX(-inc); // convert position to swap trans and rotx (rotx * trans => trans * rotx)
      rot.TransformVec3(pos);
      trans.SetTranslation(pos.x, pos.y, pos.z);
      // Recompute near and far plane (workaround for VP9 FitCameraToVertices bugs), needs a complete matView with DirectX coordinate change
      matView = layback * trans * rotx * rotz * projTrans * coords;
   }
   else
   {
      trans.SetTranslation(-mViewX + cam.x - 0.5f * table->m_right, -mViewY + cam.y - table->m_bottom, -mViewZ + cam.z);
      matView = trans * rotx * rotz * projTrans * coords;
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

   if (isLegacy)
      proj.SetPerspectiveFovLH(FOV, aspect, zNear, zFar);
   else
   {
      // Fit camera to adjusted table bounds
      const vec3 topFitOffset(0.f, 0.f, CMTOVPU(12.5f)); // Make this user adjustable (Note that since we only fit to the bottom, this is more or less useless) ?
      const vec3 bottomFitOffset(0.f, 0.f, CMTOVPU(7.5f)); // Make this user adjustable ?
      Matrix3D fit = trans * rotx * rotz * projTrans * coords * Matrix3D::MatrixPerspectiveFovLH(90.f, 1.f, zNear, zFar);
      Vertex3Ds tl = fit.MultiplyVector(Vertex3Ds(table->m_left - topFitOffset.x, table->m_top - topFitOffset.y, topFitOffset.z));
      Vertex3Ds tr = fit.MultiplyVector(Vertex3Ds(table->m_right + topFitOffset.x, table->m_top - topFitOffset.y, topFitOffset.z));
      Vertex3Ds bl = fit.MultiplyVector(Vertex3Ds(table->m_left - bottomFitOffset.x, table->m_bottom + bottomFitOffset.y, bottomFitOffset.z));
      Vertex3Ds br = fit.MultiplyVector(Vertex3Ds(table->m_right + bottomFitOffset.x, table->m_bottom + bottomFitOffset.y, bottomFitOffset.z));
      //float ymax = m_rznear * max(br.y, max(bl.y, max(tl.y, tr.y)));
      float ymin = zNear * min(br.y, min(bl.y, min(tl.y, tr.y)));
      float xmax = zNear * max(br.x, max(bl.x, max(tl.x, tr.x)));
      //float xmin = m_rznear * min(br.x, min(bl.x, min(tl.x, tr.x)));
      const float ymax2 = zNear * tanf(0.5f * ANGTORAD(FOV));
      const float xmax2 = ymax2 * aspect;
      // First offset to fit bottom of table to bottom of view, then apply user defined vertical offset
      const float ofs = zNear * 0.01f * mLayback;
      const float xofs = ((xmax2 - xmax) + ofs) * sinf(rotation);
      const float yofs = ((ymin + ymax2) + ofs) * cosf(rotation);
      proj.SetPerspectiveOffCenterLH(-xmax2 + xofs, xmax2 + xofs, -ymax2 + yofs, ymax2 + yofs, zNear, zFar);
   }

   matView = trans * rotx;
   matProj[0] = rotz * projTrans * coords * proj;

   // Apply layback
   // To be backward compatible while having a well behaving view matrix, we compute a view without the layback (which is meaningful with regards to what was used before).
   // We use it for rendering computation. It is reverted by the projection matrix which then apply the old transformation, including layback.
   if (isLegacy && abs(mLayback) > 0.1f)
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
      Matrix3D baseView, coords;
      coords.SetScaling(1.f, -1.f, -1.f);
      baseView = coords * invView;
      const vec3 right = baseView.GetOrthoNormalRight();
      const vec3 up = baseView.GetOrthoNormalUp();
      const vec3 dir = baseView.GetOrthoNormalDir();
      const vec3 pos = baseView.GetOrthoNormalPos();
      // Default is to look at the ball (playfield level = 0 + ball radius = 50)
      float camDistance = (50.f - pos.z) / dir.z;
      // Clamp it to a reasonable range, a normal viewing distance being around 80cm between view focus (table) and viewer (depends a lot on the player size & position)
      constexpr float minCamDistance = CMTOVPU(30.f);
      constexpr float maxCamDistance = CMTOVPU(200.f);
      const vec3 at = pos + dir * clamp(camDistance, minCamDistance, maxCamDistance);
      Matrix3D lookat = Matrix3D::MatrixLookAtLH(pos + (halfEyeDist * right), at, up); // Apply offset & rotation to the right eye projection
      matProj[1] = invView * lookat * coords * matProj[0];
      lookat = Matrix3D::MatrixLookAtLH(pos + (-halfEyeDist * right), at, up); // Apply offset & rotation to the left eye projection
      matProj[0] = invView * lookat * coords * matProj[0];
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
