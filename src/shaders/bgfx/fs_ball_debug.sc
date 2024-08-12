// license:GPLv3+

$input v_worldPos, v_normal, v_texcoord0
#ifdef STEREO
	$input v_eye
#endif
#ifdef CLIP
	$input v_clipDistance
#endif

#include "bgfx_shader.sh"

#if !defined(CLIP)
EARLY_DEPTH_STENCIL
#endif
void main() {
   #ifdef CLIP
   if (v_clipDistance < 0.0)
      discard;
   #endif
  gl_FragColor = vec4_splat(1.0);
}
