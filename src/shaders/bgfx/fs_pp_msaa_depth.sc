// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "bgfx_shader.sh"

#ifdef STEREO
	#if BGFX_SHADER_LANGUAGE_GLSL
		uniform sampler2DMSArray tex_depth;
		#define texFetch(index) texelFetch(tex_depth, ivec3(gl_FragCoord.x, gl_FragCoord.y, v_eye), index).r
	#else
		Texture2DMSArray<float> tex_depth : register(t0);
		#define texFetch(index) tex_depth.Load(ivec3(int(gl_FragCoord.x), int(gl_FragCoord.y), int(v_eye)), index).r
	#endif
#else
	#if BGFX_SHADER_LANGUAGE_GLSL
		uniform sampler2DMS tex_depth;
		#define texFetch(index) texelFetch(tex_depth, ivec2(gl_FragCoord.xy), index).r
	#else
		Texture2DMS<float> tex_depth : register(t0);
		#define texFetch(index) tex_depth.Load(ivec2(gl_FragCoord.xy), index).r
	#endif
#endif

void main()
{
	// Resolve by simply gathering the first sample (we could also go for min/max/average)
	float depth = texFetch(0);
	gl_FragDepth = depth;
	gl_FragColor = vec4_splat(0.0);
}
