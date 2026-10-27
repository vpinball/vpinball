// license:GPLv3+

#pragma once

#include "math/matrix.h"

namespace VPX::VRSeatedView
{

// Head yaw from the headset right axis, which stays horizontal when looking up (forward axis does not when reclined)
inline float GetHeadYaw(const Matrix3D& headToWorld, float fallback)
{
   const float x = headToWorld.m[0][0];
   const float z = headToWorld.m[0][2];
   return x * x + z * z > 1e-6f ? atan2f(-z, x) : fallback;
}

// Replace the standing placement of the playfield front (frontToWorld) by a tilted placement relative to the recentered head
inline Matrix3D GetTransform(const Matrix3D& frontToWorld, const Matrix3D& seatedOrientation, const Vertex3Ds& headPosition,
   float height, float distance, float tilt)
{
   return Matrix3D::MatrixInverse(frontToWorld)
      * Matrix3D::MatrixTranslate(0.f, CMTOVPU(distance), CMTOVPU(height))
      * Matrix3D::MatrixRotateX(ANGTORAD(tilt))
      * seatedOrientation
      * Matrix3D::MatrixTranslate(headPosition.x, headPosition.y, headPosition.z);
}

}
