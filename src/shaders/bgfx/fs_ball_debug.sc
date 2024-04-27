#ifdef STEREO
$input v_worldPos, v_normal, v_texcoord0, v_eye
#else
$input v_worldPos, v_normal, v_texcoord0
#endif

#include "bgfx_shader.sh"

EARLY_DEPTH_STENCIL void main()
{
  gl_FragColor = vec4_splat(1.0);
}
