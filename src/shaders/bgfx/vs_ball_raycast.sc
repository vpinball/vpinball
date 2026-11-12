// license:GPLv3+

$input a_position
$output v_tablePos
#ifdef STEREO
    $output v_eye
#endif

#include "common.sh"

#ifdef STEREO
    uniform vec4 layer;
    uniform mat4 matProjInv[2];
#else
    uniform mat4 matProjInv;
#endif

void main()
{
    // a_position carries the clip space corner of the impostor quad (built in Ball::Render from
    // the projected bounds of the ball), with z set to the nearest projected ball depth.
    gl_Position = vec4(a_position, 1.0);
    // The camera ray for the analytic raycast is rebuilt by unprojecting the corner through the
    // inverse projection. Interpolating this unprojected point across the quad gives the exact
    // (unnormalized) view space ray direction at each fragment.
    #ifdef STEREO
        const vec4 viewPt = mul(matProjInv[gl_InstanceID], vec4(a_position, 1.0));
        gl_Layer = gl_InstanceID;
        v_eye = layer.x + gl_InstanceID;
    #else
        const vec4 viewPt = mul(matProjInv, vec4(a_position, 1.0));
    #endif
    v_tablePos = viewPt.xyz;
}
