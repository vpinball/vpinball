#ifdef STEREO
$input v_worldPos, v_tablePos, v_normal, v_texcoord0, v_eye
#else
$input v_worldPos, v_tablePos, v_normal, v_texcoord0
#endif

#include "common.sh"

uniform vec4 cBase_Alpha;
uniform vec4 alphaTestValue; // FIXME Actually float but extended to vec4 for BGFX (and we could use the builtin)

#ifdef TEX
SAMPLER2D(tex_base_color, 0);
void main() {
   vec4 pixel = texture2D(tex_base_color, v_texcoord0);
   if (pixel.a <= alphaTestValue.x)
      discard;
   gl_FragColor = pixel * cBase_Alpha;
}

#else
EARLY_DEPTH_STENCIL void main() {
   gl_FragColor = cBase_Alpha;
}

#endif
