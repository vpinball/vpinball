// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"


// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

uniform vec4 SSR_bumpHeight_fresnelRefl_scale_FS;

SAMPLER2DSTEREO(tex_fb_filtered,  0); // Framebuffer (filtered)
SAMPLER2DSTEREO(tex_depth,        4); // DepthBuffer
SAMPLER2D(tex_ao_dither,    5); // AO Dither

#define tex_fb_unfiltered tex_fb_filtered



#ifdef STEREO
vec3 get_nonunit_normal(const float depth0, const vec2 u, const float v_eye) // use neighboring pixels // quite some tex access by this
#else
vec3 get_nonunit_normal(const float depth0, const vec2 u) // use neighboring pixels // quite some tex access by this
#endif
{
   const float depth1 = texStereoNoLod(tex_depth, vec2(u.x, u.y + w_h_height.y)).x;
   const float depth2 = texStereoNoLod(tex_depth, vec2(u.x + w_h_height.x, u.y)).x;
   return vec3(w_h_height.y * (depth2 - depth0), (depth1 - depth0) * w_h_height.x, w_h_height.y * w_h_height.x); //!!
}

#ifdef STEREO
vec3 approx_bump_normal(const vec2 coords, const vec2 offs, const float scale, const float sharpness, const float v_eye)
#else
vec3 approx_bump_normal(const vec2 coords, const vec2 offs, const float scale, const float sharpness)
#endif
{
    const vec3 lumw = vec3(0.212655,0.715158,0.072187);

    const float lpx = dot(texStereoNoLod(tex_fb_filtered, vec2(coords.x+offs.x,coords.y)).xyz, lumw);
    const float lmx = dot(texStereoNoLod(tex_fb_filtered, vec2(coords.x-offs.x,coords.y)).xyz, lumw);
    const float lpy = dot(texStereoNoLod(tex_fb_filtered, vec2(coords.x,coords.y-offs.y)).xyz, lumw);
    const float lmy = dot(texStereoNoLod(tex_fb_filtered, vec2(coords.x,coords.y+offs.y)).xyz, lumw);

    const float dpx = texStereoNoLod(tex_depth, vec2(coords.x + offs.x, coords.y)).x;
    const float dmx = texStereoNoLod(tex_depth, vec2(coords.x - offs.x, coords.y)).x;
    const float dpy = texStereoNoLod(tex_depth, vec2(coords.x, coords.y + offs.y)).x;
    const float dmy = texStereoNoLod(tex_depth, vec2(coords.x, coords.y - offs.y)).x;

    const vec2 xymult = max(1.0 - vec2(abs(dmx - dpx), abs(dmy - dpy)) * sharpness, 0.0);

    return normalize(vec3(vec2(lmx - lpx,lmy - lpy)*xymult/offs, scale));
}

float normal_fade_factor(const vec3 n)
{
    return min(sqr(1.0-n.z)*0.5 + max(SSR_bumpHeight_fresnelRefl_scale_FS.w == 0.0 ? n.y : n.x,0.0) + abs(SSR_bumpHeight_fresnelRefl_scale_FS.w == 0.0 ? n.x : n.y)*0.5,1.0); // dot(n,vec3(0,0,1))  dot(n,vec3(0,1,0))  dot(n,vec3(1,0,0)) -> penalty for z-axis/up (geometry like playfield), bonus for y-axis (like backwall) and x-axis (like sidewalls)
}

void main()
{
	const vec2 u = v_texcoord0;

	const vec3 color0 = texStereoNoLod(tex_fb_unfiltered, u).xyz; // original pixel

	const float depth0 = texStereoNoLod(tex_depth, u).x;
	BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) //!!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
	{
		gl_FragColor = vec4(color0, 1.0);
		return;
	}

	#ifdef STEREO
	const vec3 normal = normalize(get_nonunit_normal(depth0,u,v_eye));
	vec3 normal_b = approx_bump_normal(u, 0.01 * w_h_height.xy / depth0, depth0 / (0.05*depth0 + 0.0001), 1000.0,v_eye); //!! magic
	#else
	const vec3 normal = normalize(get_nonunit_normal(depth0,u));
	vec3 normal_b = approx_bump_normal(u, 0.01 * w_h_height.xy / depth0, depth0 / (0.05*depth0 + 0.0001), 1000.0); //!! magic
	#endif
	normal_b = normalize(vec3(normal.xy*normal_b.z + normal_b.xy*normal.z, normal.z*normal_b.z));
	normal_b = normalize(mix(normal,normal_b, SSR_bumpHeight_fresnelRefl_scale_FS.x * normal_fade_factor(normal))); // have less impact of fake bump normals on playfield, etc
	
	const vec3 V = normalize(vec3(0.5 - vec2(u.x, 1.0 - u.y), -0.5)); // WTF?! cam is in 0,0,0 but why z=-0.5?

	const float fresnel = (SSR_bumpHeight_fresnelRefl_scale_FS.y + (1.0-SSR_bumpHeight_fresnelRefl_scale_FS.y) * pow(1.0-saturate(dot(V,normal_b)),5.)) // fresnel for falloff towards silhouette
	                     * SSR_bumpHeight_fresnelRefl_scale_FS.z // user scale
	                     * sqr(normal_fade_factor(normal_b/*normal*/)); // avoid reflections on playfield, etc

#if 0 // test code
	gl_FragColor = vec4(0.,sqr(normal_fade_factor(normal_b/*normal*/)),0., 1.0);
	return;
#endif

	BRANCH if(fresnel < 0.01) //!! early out if contribution too low
	{
		gl_FragColor = vec4(color0, 1.0);
		return;
	}

	const int samples = 32; //!! reduce to ~24? would save ~1-4% overall frame time, depending on table
	const float samples_float = float(samples);
	const float ReflBlurWidth = 2.2; //!! magic, small enough to not collect too much, and large enough to have cool reflection effects

	const float ushift = /*hash(v_texcoord0) + w_h_height.zw*/ // jitter samples via hash of position on screen and then jitter samples by time //!! see below for non-shifted variant
	                     /*fract(*/texNoLod(tex_ao_dither, v_texcoord0/(64.0*w_h_height.xy)).z /*+ w_h_height.z)*/; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
	const vec2 offsMul = normal_b.xy * (/*w_h_height.xy*/ vec2(1.0/1920.0,1.0/1080.0) * ReflBlurWidth * (32./float(samples))); //!! makes it more resolution independent?? test with 4xSSAA

	// loop in screen space, simply collect all pixels in the normal direction (not even a depth check done!)
	vec3 refl = vec3(0.,0.,0.);
	float color0w = 0.;
	UNROLL for(int i=1; i</*=*/samples; i++) //!! due to jitter
	{
		const vec2 offs = u + (float(i)+ushift)*offsMul; //!! jitter per pixel (uses blue noise tex)
		const vec3 color = texStereoNoLod(tex_fb_filtered, offs).xyz;

		/*BRANCH if(i==1) // necessary special case as compiler warns/'optimizes' sqrt below into rqsrt?!
		{
			refl += color;
		}
		else
		{*/
			const float w = sqrt(float(i-1)/samples_float); //!! fake falloff for samples more far away
			refl += color*(1.0-w); //!! dampen large color values in addition?
			color0w += w;
		//}
	}

	refl += color0*color0w;
	refl *= 1.0/(samples_float-1.0); //!! -1 due to jitter

	gl_FragColor = vec4(mix(color0,refl, min(fresnel,1.0)), 1.0);
}
