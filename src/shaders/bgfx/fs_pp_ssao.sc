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

uniform vec4 AO_scale_timeblur; // actually vec2 extended to vec4

SAMPLER2DSTEREO(tex_fb_filtered,  0); // Framebuffer (filtered)
SAMPLER2DSTEREO(tex_depth,        4); // DepthBuffer
SAMPLER2D      (tex_ao_dither,    5); // AO Dither


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

void main()
{
	const vec2 u = v_texcoord0;
	const vec2 uv0 = u - w_h_height.xy * 0.5 + w_h_height.xy; // half pixel shift in x & y for filter
	const vec2 uv1 = u - w_h_height.xy * 0.5; // dto.

	const float depth0 = texStereoNoLod(tex_depth, u).x;
	BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) //!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
	{
		gl_FragColor = vec4(1.0, 0.,0.,0.);
		return;
	}

	const vec3 ushift = /*hash(uv1) + w_h_height.zw*/ // jitter samples via hash of position on screen and then jitter samples by time //!! see below for non-shifted variant
	                      texNoLod(tex_ao_dither, uv1/(64.0*w_h_height.xy) + w_h_height.zw).xyz; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
	//const float base = 0.0;
	const float area = 0.06; //!!
	const float falloff = 0.0002; //!!
	const int samples = 8/*9*/; //4,8,9,13,16,21,25,32 korobov,fibonacci
	const float samples_float = float(samples);
	const float radius = 0.001+/*frac*/(ushift.z)*0.009; // sample radius
	const float depth_threshold_normal = 0.005;
	const float total_strength = AO_scale_timeblur.x * (/*1.0 for uniform*/0.5 / samples_float);
	#ifdef STEREO
	const vec3 normal = normalize(get_nonunit_normal(depth0, u, v_eye));
	#else
	const vec3 normal = normalize(get_nonunit_normal(depth0, u));
	#endif
	//const vec3 normal = texNoLod(tex_normals, u).xyz *2.0-1.0; // use 8bitRGB pregenerated normals
	const float radius_depth = radius/depth0;

	float occlusion = 0.0;
	UNROLL for(int i=0; i < samples; ++i) {
		const float i_float = float(i);
		const vec2 r = vec2(i_float * (1.0 / samples_float), i_float * (5.0/*2.0*/ / samples_float)); //1,5,2,8,4,13,7,7 korobov,fibonacci //!! could also use progressive/extensible lattice via rad_inv(i)*(1501825329, 359975893) (check precision though as this should be done in double or uint64)
		//const vec3 ray = sphere_sample(fract(r+ushift.xy)); // shift lattice // uniform variant
		const vec2 ray = rotate_to_vector_upper(cos_hemisphere_sample(fract(r+ushift.xy)), normal).xy; // shift lattice
		//!! maybe a bit worse distribution: const vec2 ray = cos_hemisphere_sample(normal,fract(r+ushift.xy)).xy; // shift lattice
		//const float rdotn = dot(ray,normal);
		const vec2 hemi_ray = u + (radius_depth /** sign(rdotn) for uniform*/) * ray.xy;
		const float occ_depth = texStereoNoLod(tex_depth, hemi_ray).x;
		#ifdef STEREO
		const vec3 occ_normal = get_nonunit_normal(occ_depth, hemi_ray, v_eye);
		#else
		const vec3 occ_normal = get_nonunit_normal(occ_depth, hemi_ray);
		#endif
		//const vec3 occ_normal = texNoLod(tex_normals, hemi_ray).xyz *2.0-1.0;  // use 8bitRGB pregenerated normals, can also omit normalization below then
		const float diff_depth = depth0 - occ_depth;
		const float diff_norm = dot(occ_normal,normal);
		occlusion += step(falloff, diff_depth) * /*abs(rdotn)* for uniform*/ (diff_depth < depth_threshold_normal ? (1.0-diff_norm*diff_norm/dot(occ_normal,occ_normal)) : 1.0) * (1.0-smoothstep(falloff, area, diff_depth));
	}
	// weight with result(s) from previous frames
	const float ao = 1.0 - total_strength * occlusion;
	gl_FragColor = vec4((texStereoNoLod(tex_fb_filtered, uv0).x //abuse bilerp for filtering (by using half texel/pixel shift)
	                    +texStereoNoLod(tex_fb_filtered, uv1).x
	                    +texStereoNoLod(tex_fb_filtered, vec2(uv0.x,uv1.y)).x
	                    +texStereoNoLod(tex_fb_filtered, vec2(uv1.x,uv0.y)).x)
		*(0.25*(1.0-AO_scale_timeblur.y))+saturate(ao /*+base*/)*AO_scale_timeblur.y, 0.,0.,0.);
}
