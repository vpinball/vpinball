// license:GPLv3+

$input a_position
$output v_rayOriginView, v_rayDirView, v_rayOriginObj, v_rayDirObj
#ifdef STEREO
    $output v_eye
#endif

#include "common.sh"

#ifdef STEREO
    uniform vec4 layer;
    uniform mat4 matProjInv[2];
    uniform mat4 ballViewToObj[2];
    #define mProjInv      matProjInv[gl_InstanceID]
    #define mBallViewToObj ballViewToObj[gl_InstanceID]
#else
    uniform mat4 matProjInv;
    uniform mat4 ballViewToObj;
    #define mProjInv      matProjInv
    #define mBallViewToObj ballViewToObj
#endif

void main()
{
    // a_position is a clip space corner of the impostor quad (see Ball::Render), at the nearest projected ball depth
    gl_Position = vec4(a_position, 1.0);
    // Ray through the corner, between its unprojected points at clip depth 0 (the origin) and 1. The projection center is
    // not assumed to be the view space origin, which it is not with scene scale, layback, stereo eye shift or orthographic
    // views. The unprojected w only depends on the clip depth, so the points are linear across the quad and interpolate
    // exactly. The ray is also expressed in unit sphere space (affine, so with the same ray parameter)
    const vec4 p0 = mul(mProjInv, vec4(a_position.xy, 0.0, 1.0));
    const vec4 p1 = mul(mProjInv, vec4(a_position.xy, 1.0, 1.0));
    v_rayOriginView = p0.xyz / p0.w;
    v_rayDirView = p1.xyz / p1.w - v_rayOriginView;
    v_rayOriginObj = mul(mBallViewToObj, vec4(v_rayOriginView, 1.0)).xyz;
    v_rayDirObj = mul(mBallViewToObj, vec4(v_rayDirView, 0.0)).xyz;
    #ifdef STEREO
        gl_Layer = gl_InstanceID;
        v_eye = layer.x + gl_InstanceID;
    #endif
}
