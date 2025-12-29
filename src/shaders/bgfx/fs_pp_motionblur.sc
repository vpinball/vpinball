// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"

#define NUM_BALLS 8
uniform vec4 balls[NUM_BALLS];
#ifdef STEREO
	uniform mat4 matProj[2];
	uniform mat4 matProjInv[2];
#else
	uniform mat4 matProj;
	uniform mat4 matProjInv;
#endif
uniform vec4 w_h_height;
#define NSAMPLES_F (w_h_height.w)

SAMPLER2DSTEREO(tex_fb_filtered,  0); // Current Render
SAMPLER2DSTEREO(tex_bloom,        1); // Previous Render
SAMPLER2DSTEREO(tex_depth,        4); // Current Render DepthBuffer

#define tex_prev_render tex_bloom
#define tex_cur_render  tex_fb_filtered

void main()
{
	const int nSamples = int(NSAMPLES_F);
	// gl_FragCoord is from bottomLeft for GL/GLES or topLeft for others, so using any of these 2 lines are the same
	//const vec2 screenUV = vec2(gl_FragCoord.x * w_h_height.x, gl_FragCoord.y * w_h_height.y);
	const vec2 screenUV = v_texcoord0.xy;
	#if TEX_V_IS_UP
		// OpenGL and OpenGL ES use -1..1 NDC mapped to 0..1 depth buffer
		const float screenDepth = texStereoNoLod(tex_depth, screenUV).x * 2. - 1.;
		const vec4 projPos = vec4(screenUV.x * 2. - 1., 1. - (1.0 - screenUV.y) * 2., screenDepth, 1.);
	#else
		const float screenDepth = texStereoNoLod(tex_depth, screenUV).x;
		const vec4 projPos = vec4(screenUV.x * 2. - 1., 1. - screenUV.y * 2., screenDepth, 1.);
	#endif

	// Compute position in world/view space of rendered fragment
	#ifdef STEREO
		vec4 pixelPos = mul(matProjInv[int(v_eye)], projPos);
	#else
		vec4 pixelPos = mul(matProjInv, projPos);
	#endif
	pixelPos.xyz /= pixelPos.w;

	// Compute camera ray, knowing that camera is always at origin in world/view space
	const vec3 rayDirection = normalize(pixelPos.xyz);

	// Integrate over nSamples samples (nSamples+1 positions excluding the previous one which was already part of the previous displayed frame)
	const vec3 ball_pos = balls[0].xyz;
	const vec3 prev_ball_pos = balls[1].xyz;
	const float ball_r2 = balls[0].w * balls[0].w;

	// Start with current ball position, as it allows to detect occluded pixel and select the best background (no ball) color source
	vec3 color = texStereoNoLod(tex_cur_render, screenUV).rgb;
	vec3 noIntersectColor;
	{
		// Optimized ray / sphere intersection for a ray origin at origin (camera is always at origin)
		const float b = dot(rayDirection, ball_pos);
		const float c = dot(ball_pos,     ball_pos) - ball_r2 * 1.15; // 1.15 is a magic number to avoid acne at the border of the ball (wrongly considering there is no intersection when the pixel is partially occluded by the ball)
		const float det = b * b - c;
		if (det < 0.0) {
			// No intersection with current ball pos: background of the ball is available in current render
			noIntersectColor = color;
		} else {
			// Nearest intersection (other intersection is b + sqrt(det) which is always farther from camera with our setup)
			const float intersect_pos_z = (b - sqrt(det)) * rayDirection.z;
			if (pixelPos.z > intersect_pos_z + 1.0) // 1.0 is a magic value to avoid surface acnee
			{
				// Ball is behind something, disable motion blur (not fully correct as it may be a transparent object with depth write enabled)
				gl_FragColor = vec4(color, 1.);
				return;
			}
			// Intersection with current ball pos: background of the ball will be taken from previous render
			noIntersectColor = texStereoNoLod(tex_prev_render, screenUV).rgb;
		}
	}

	// Process interpolated positions from previous (excluded as part of previous frame) to current (excluded as already processed)
	//float weight_sum = shutter_function(0.);
	for (int i = 1; i < nSamples; i++)
	{
		const float mix_pos = float(i) / NSAMPLES_F;
		const vec3 sample_ball_pos = mix(prev_ball_pos, ball_pos, mix_pos);
		const float b = dot(rayDirection,    sample_ball_pos);
		const float c = dot(sample_ball_pos, sample_ball_pos) - ball_r2;
		const float det = b * b - c;
		//const float weight = shutter_function(mix_pos);
		//weight_sum += weight;
		if (det < 0.0) {
			// No intersection: use the selected background render
			color += noIntersectColor; // * weight;
		} else {
			// Intersection: build up render at current sample from corresponding points from previous and current renders
			const vec3 intersect_ofs = (b - sqrt(det)) * rayDirection - sample_ball_pos; // intersection position relative to sphere center
			#ifdef STEREO
				// FIXME v_eye needs to be flat interpolated, but if declared as such in varying.def.sc, DX11 will fail (OpenGL/Vulkan are good)
				vec4 prev_proj = mul(matProj[int(round(v_eye))], vec4(prev_ball_pos + intersect_ofs, 1.0)); // FIXME should use previous MVP
				vec4 cur_proj  = mul(matProj[int(round(v_eye))], vec4(ball_pos      + intersect_ofs, 1.0));
			#else
				vec4 prev_proj = mul(matProj, vec4(prev_ball_pos + intersect_ofs, 1.0)); // FIXME should use previous MVP
				vec4 cur_proj  = mul(matProj, vec4(ball_pos      + intersect_ofs, 1.0));
			#endif
			prev_proj.xy /= prev_proj.w;
			cur_proj.xy  /= cur_proj.w;
			#if TEX_V_IS_UP
				// OpenGL and OpenGL ES have reversed texture coordinate system (0,0 is bottom left)
				const vec3 prev_sample = texStereoNoLod(tex_prev_render, vec2_splat(0.5) + 0.5 * prev_proj.xy).rgb;
				const vec3 cur_sample  = texStereoNoLod(tex_cur_render,  vec2_splat(0.5) + 0.5 * cur_proj.xy ).rgb;
			#else
				const vec3 prev_sample = texStereoNoLod(tex_prev_render, vec2_splat(0.5) + 0.5 * vec2(prev_proj.x, -prev_proj.y)).rgb;
				const vec3 cur_sample  = texStereoNoLod(tex_cur_render,  vec2_splat(0.5) + 0.5 * vec2(cur_proj.x,  -cur_proj.y )).rgb;
			#endif
			color += mix(prev_sample, cur_sample, mix_pos); // * weight;
		}
	}

	//gl_FragColor = vec4(color / weight_sum, 1.0);
	gl_FragColor = vec4(color / NSAMPLES_F, 1.0);
}
