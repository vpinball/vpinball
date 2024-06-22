#ifdef STEREO
$input v_worldPos, v_normal, v_texcoord0, v_eye
#else
$input v_worldPos, v_normal, v_texcoord0
#endif

#include "common.sh"

SAMPLER2D(tex_ball_color, 0);     // base texture (used as a reflection map)

uniform vec4  w_h_disableLighting; 

//!! abuses normal for now
#define trailAlpha (v_normal.x)

uniform vec4 fenvEmissionScale_TexWidth; // vec2 extended to vec4 for BGFX
uniform vec4 cBase_Alpha; //!! 0.04-0.95 in RGB

EARLY_DEPTH_STENCIL void main()
{
   const vec3 ballImageColor = texture2D(tex_ball_color, v_texcoord0.xy).rgb;
   if (w_h_disableLighting.z != 0.)
      gl_FragColor = vec4(ballImageColor, trailAlpha);
   else
      gl_FragColor = vec4((cBase_Alpha.rgb*(0.075*0.25) + ballImageColor)*fenvEmissionScale_TexWidth.x, trailAlpha); //!! just add the ballcolor in, this is a whacky trail anyhow
}
