$input v_texcoord0

#include "bgfx_shader.sh"


// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

SAMPLER2D(tex_fb_unfiltered,  0); // Framebuffer (unfiltered)
SAMPLER2D(tex_depth,          4); // DepthBuffer



float normpdf(const vec3 v, const float sigma)
{
	return exp(dot(v,v)*(-0.5/(sigma*sigma)))*(0.39894228040143/sigma);
}

float LI(const vec3 l)
{
	return dot(l, vec3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
	//return dot(l, vec3(0.2126, 0.7152, 0.0722));
}

void main()
{
	const float sharpness = 0.625*3.1;

	const vec2 u = v_texcoord0;

	const vec3 e = texture2DLod(tex_fb_unfiltered, u, 0.0).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texture2DLod(tex_depth, u, 0.0).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			gl_FragColor = vec4(e, 1.0);
			return;
		}
	}

	// Bilateral Blur (crippled)
	vec3 final_colour = vec3(0.,0.,0.);
	float Z = 0.0;
	UNROLL for (int j=-2; j <= 2; ++j) // 2 = kernelradius
		UNROLL for (int i=-2; i <= 2; ++i)
		{
			const vec3 cc = texture2DLod(tex_fb_unfiltered, vec2(u.x + i*(w_h_height.x*0.5), u.y + j*(w_h_height.y*0.5)), 0.0).xyz; // *0.5 = 1/kernelradius
			const float factor = normpdf(cc-e, 0.25); // 0.25 = BSIGMA
			Z += factor;
			final_colour += factor*cc;
		}

	// CAS (without Better Diagonals)
	const vec2 um1 = u - w_h_height.xy;
	const vec2 up1 = u + w_h_height.xy;

	const float b = LI(texture2DLod(tex_fb_unfiltered, vec2(u.x, um1.y), 0.0).xyz);
	const float d = LI(texture2DLod(tex_fb_unfiltered, vec2(um1.x, u.y), 0.0).xyz);
	const float f = LI(texture2DLod(tex_fb_unfiltered, vec2(up1.x, u.y), 0.0).xyz);
	const float h = LI(texture2DLod(tex_fb_unfiltered, vec2(u.x, up1.y), 0.0).xyz);
	const float e1 = LI(e);

	const float mnRGB = min(min(min(d, e1), min(f, b)), h);
	const float mxRGB = max(max(max(d, e1), max(f, b)), h);

	// Smooth minimum distance to signal limit divided by smooth max.
	const float rcpMRGB = rcp(mxRGB);
	const float ampRGB = saturate(min(mnRGB, 1.0 - mxRGB) * rcpMRGB);

	vec3 sharpen = (e-final_colour/Z) * sharpness;

	const float gs_sharpen = dot(sharpen, 0.333333333333);
	sharpen = lerp(gs_sharpen, sharpen, 0.5);

	gl_FragColor = vec4(lerp(e, sharpen+e, ampRGB*saturate(sharpness)), 1.0);
}
