// license:GPLv3+

$input v_worldPos, v_tablePos, v_normal, v_texcoord0
#ifdef STEREO
	$input v_eye
#endif
#ifdef CLIP
	$input v_clipDistance
#endif

#include "common.sh"
#include "ball_shadows.sh"

uniform vec4 staticColor_Alpha;
uniform vec4 lightCenter_doShadow;

#ifdef TEX
SAMPLER2D      (tex_base_color, 0); // base color
#endif


#ifndef CLIP
EARLY_DEPTH_STENCIL
#endif
void main()
{
	#ifdef CLIP
	if (v_clipDistance < 0.0)
		discard;
	#endif
	
	vec4 result = staticColor_Alpha;

	#ifdef TEX
		result *= texture2D(tex_base_color, v_texcoord0);
	#endif

	#ifdef SHADOW
		const vec3 light_dir = v_tablePos.xyz - lightCenter_doShadow.xyz;
		const float light_dist = length(light_dir);
		const float shadow = get_light_ball_shadow(lightCenter_doShadow.xyz, light_dir, light_dist);
		result.rgb *= shadow;
	#endif

	gl_FragColor = result;
}
