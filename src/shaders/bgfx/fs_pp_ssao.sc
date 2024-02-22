$input v_texcoord0

#include "bgfx_shader.sh"
#include "common.sh"


// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

uniform vec4 AO_scale_timeblur; // actually vec2 extended to vec4

SAMPLER2D(tex_fb_filtered,  0); // Framebuffer (filtered)
SAMPLER2D(tex_depth,        4); // DepthBuffer
SAMPLER2D(tex_ao_dither,    5); // AO Dither


vec3 get_nonunit_normal(const float depth0, const vec2 u) // use neighboring pixels // quite some tex access by this
{
   const float depth1 = texture2DLod(tex_depth, float2(u.x, u.y + w_h_height.y), 0.0).x;
   const float depth2 = texture2DLod(tex_depth, float2(u.x + w_h_height.x, u.y), 0.0).x;
   return vec3(w_h_height.y * (depth2 - depth0), (depth1 - depth0) * w_h_height.x, w_h_height.y * w_h_height.x); //!!
}

vec3 cos_hemisphere_sample(const vec2 t) // u,v in [0..1), returns y-up
{
	const float phi = t.y * (2.0*3.1415926535897932384626433832795);
	const float cosTheta = sqrt(1.0 - t.x);
	const float sinTheta = sqrt(t.x);
	float sp,cp;
	sincos(phi,sp,cp);
	return vec3(cp * sinTheta, cosTheta, sp * sinTheta);
}

vec3 rotate_to_vector_upper(const vec3 vec, const vec3 normal)
{
	if(normal.y > -0.99999)
	{
		const float h = 1.0/(1.0+normal.y);
		const float hz = h*normal.z;
		const float hzx = hz*normal.x;
		return vec3(
			vec.x * (normal.y+hz*normal.z) + vec.y * normal.x - vec.z * hzx,
			vec.y * normal.y - vec.x * normal.x - vec.z * normal.z,
			vec.y * normal.z - vec.x * hzx + vec.z * (normal.y+h*normal.x*normal.x));
	}
	else return -vec;
}

void main()
{
	const vec2 u = v_texcoord0;
	const vec2 uv0 = u - w_h_height.xy * 0.5 + w_h_height.xy; // half pixel shift in x & y for filter
	const vec2 uv1 = u - w_h_height.xy * 0.5; // dto.

	const float depth0 = texture2DLod(tex_depth, u, 0.0).x;
	BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) //!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
	{
		gl_FragColor = vec4(1.0, 0.,0.,0.);
		return;
	}

	const vec3 ushift = /*hash(uv1) + w_h_height.zw*/ // jitter samples via hash of position on screen and then jitter samples by time //!! see below for non-shifted variant
	                      texture2DLod(tex_ao_dither, uv1/(64.0*w_h_height.xy) + w_h_height.zw, 0.0).xyz; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
	//const float base = 0.0;
	const float area = 0.06; //!!
	const float falloff = 0.0002; //!!
	const int samples = 8/*9*/; //4,8,9,13,16,21,25,32 korobov,fibonacci
	const float radius = 0.001+/*frac*/(ushift.z)*0.009; // sample radius
	const float depth_threshold_normal = 0.005;
	const float total_strength = AO_scale_timeblur.x * (/*1.0 for uniform*/0.5 / samples);
	const vec3 normal = normalize(get_nonunit_normal(depth0, u));
	//const vec3 normal = texture2DLod(tex_normals, u, 0.0).xyz *2.0-1.0; // use 8bitRGB pregenerated normals
	const float radius_depth = radius/depth0;

	float occlusion = 0.0;
	UNROLL for(int i=0; i < samples; ++i) {
		const vec2 r = vec2(i*(1.0 / samples), i*(5.0/*2.0*/ / samples)); //1,5,2,8,4,13,7,7 korobov,fibonacci //!! could also use progressive/extensible lattice via rad_inv(i)*(1501825329, 359975893) (check precision though as this should be done in double or uint64)
		//const vec3 ray = sphere_sample(frac(r+ushift.xy)); // shift lattice // uniform variant
		const vec2 ray = rotate_to_vector_upper(cos_hemisphere_sample(frac(r+ushift.xy)), normal).xy; // shift lattice
		//!! maybe a bit worse distribution: const vec2 ray = cos_hemisphere_sample(normal,frac(r+ushift.xy)).xy; // shift lattice
		//const float rdotn = dot(ray,normal);
		const vec2 hemi_ray = u + (radius_depth /** sign(rdotn) for uniform*/) * ray.xy;
		const float occ_depth = texture2DLod(tex_depth, hemi_ray, 0.0).x;
		const vec3 occ_normal = get_nonunit_normal(occ_depth, hemi_ray);
		//const vec3 occ_normal = texture2DLod(tex_normals, hemi_ray, 0.0).xyz *2.0-1.0;  // use 8bitRGB pregenerated normals, can also omit normalization below then
		const float diff_depth = depth0 - occ_depth;
		const float diff_norm = dot(occ_normal,normal);
		occlusion += step(falloff, diff_depth) * /*abs(rdotn)* for uniform*/ (diff_depth < depth_threshold_normal ? (1.0-diff_norm*diff_norm/dot(occ_normal,occ_normal)) : 1.0) * (1.0-smoothstep(falloff, area, diff_depth));
	}
	// weight with result(s) from previous frames
	const float ao = 1.0 - total_strength * occlusion;
	gl_FragColor = vec4((texture2DLod(tex_fb_filtered, uv0, 0.0).x //abuse bilerp for filtering (by using half texel/pixel shift)
				  +texture2DLod(tex_fb_filtered, uv1, 0.0).x
				  +texture2DLod(tex_fb_filtered, vec2(uv0.x,uv1.y), 0.0).x
				  +texture2DLod(tex_fb_filtered, vec2(uv1.x,uv0.y), 0.0).x)
		*(0.25*(1.0-AO_scale_timeblur.y))+saturate(ao /*+base*/)*AO_scale_timeblur.y, 0.,0.,0.);
}