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


void main()
{
	// variant with better diagonals

	const float Contrast   = 0.0; // 0..1, Adjusts the range the shader adapts to high contrast (0 is not all the way off).  Higher values = more high contrast sharpening.
	const float Sharpening = 1.0; // 0..1, Adjusts sharpening intensity by averaging the original pixels to the sharpened result.  1.0 is the unmodified default.

	const float2 u = v_texcoord0;

	const float3 e = texture2DLod(tex_fb_unfiltered, u, 0.0).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texture2DLod(tex_depth, u, 0.0).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			gl_FragColor = vec4(e, 1.0);
			return;
		}
	}

	// fetch a 3x3 neighborhood around the pixel 'e',
	//  a b c
	//  d(e)f
	//  g h i
	const float2 um1 = u - w_h_height.xy;
	const float2 up1 = u + w_h_height.xy;

	const float3 a = texture2DLod(tex_fb_unfiltered,        um1          , 0.0).xyz;
	const float3 b = texture2DLod(tex_fb_unfiltered, float2(u.x,   um1.y), 0.0).xyz;
	const float3 c = texture2DLod(tex_fb_unfiltered, float2(up1.x, um1.y), 0.0).xyz;
	const float3 d = texture2DLod(tex_fb_unfiltered, float2(um1.x, u.y  ), 0.0).xyz;
	const float3 g = texture2DLod(tex_fb_unfiltered, float2(um1.x, up1.y), 0.0).xyz; 
	const float3 f = texture2DLod(tex_fb_unfiltered, float2(up1.x, u.y  ), 0.0).xyz;
	const float3 h = texture2DLod(tex_fb_unfiltered, float2(u.x,   up1.y), 0.0).xyz;
	const float3 i = texture2DLod(tex_fb_unfiltered,        up1          , 0.0).xyz;

	// Soft min and max.
	//  a b c             b
	//  d e f * 0.5  +  d e f * 0.5
	//  g h i             h
	// These are 2.0x bigger (factored out the extra multiply).
	float3 mnRGB = min(min(min(d, e), min(f, b)), h);
	const float3 mnRGB2 = min(mnRGB, min(min(a, c), min(g, i)));
	mnRGB += mnRGB2;

	float3 mxRGB = max(max(max(d, e), max(f, b)), h);
	const float3 mxRGB2 = max(mxRGB, max(max(a, c), max(g, i)));
	mxRGB += mxRGB2;

	// Smooth minimum distance to signal limit divided by smooth max.
	const float3 rcpMRGB = rcp(mxRGB);
	float3 ampRGB = saturate(min(mnRGB, 2.0 - mxRGB) * rcpMRGB);

	// Shaping amount of sharpening.
	ampRGB = rsqrt(ampRGB);

	const float peak = -3.0 * Contrast + 8.0;
	const float3 wRGB = -rcp(ampRGB * peak);

	const float3 rcpWeightRGB = rcp(4.0 * wRGB + 1.0);

	//                          0 w 0
	//  Filter shape:           w 1 w
	//                          0 w 0  
	const float3 window = (b + d) + (f + h);
	const float3 outColor = saturate((window * wRGB + e) * rcpWeightRGB);

	gl_FragColor = vec4(lerp(e, outColor, Sharpening), 1.);
}
