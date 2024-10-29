// license:GPLv3+

$input v_worldPos
#ifdef STEREO
	$input v_eye
#endif
#ifdef CLIP
	$input v_clipDistance
#endif

#include "common.sh"

SAMPLER2D(tex_ball_color, 0);     // base texture (used as a reflection map)

uniform vec4  w_h_disableLighting; 

#define trailAlpha (v_worldPos.z)

uniform vec4 fenvEmissionScale_TexWidth; // vec2 extended to vec4 for BGFX
uniform vec4 cBase_Alpha; //!! 0.04-0.95 in RGB

#if !defined(CLIP)
EARLY_DEPTH_STENCIL
#endif
void main() {
   #ifdef CLIP
   if (v_clipDistance < 0.0)
      discard;
   #endif
   const vec3 ballImageColor = texture2D(tex_ball_color, v_worldPos.xy).rgb;
   if (w_h_disableLighting.z != 0.)
      gl_FragColor = vec4(ballImageColor, trailAlpha);
   else
      gl_FragColor = vec4((cBase_Alpha.rgb*(0.075*0.25) + ballImageColor)*fenvEmissionScale_TexWidth.x, trailAlpha); //!! just add the ballcolor in, this is a whacky trail anyhow
}
