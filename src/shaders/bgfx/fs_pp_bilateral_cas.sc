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

SAMPLER2DSTEREO(tex_fb_unfiltered,  0); // Framebuffer (unfiltered)
SAMPLER2DSTEREO(tex_depth,          4); // DepthBuffer



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
	const float balance = 0.0;         // [0..1]

	const vec2 u = v_texcoord0;

	const vec3 mid = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			gl_FragColor = vec4(mid, 1.0);
			return;
		}
	}
	
	const ARRAY_BEGIN(vec3, e, 9)
		texStereoNoLod(tex_fb_unfiltered, vec2(u.x -w_h_height.x, u.y -w_h_height.y)).xyz,
		texStereoNoLod(tex_fb_unfiltered, vec2(u.x              , u.y -w_h_height.y)).xyz,
		texStereoNoLod(tex_fb_unfiltered, vec2(u.x +w_h_height.x, u.y -w_h_height.y)).xyz,

		texStereoNoLod(tex_fb_unfiltered, vec2(u.x -w_h_height.x, u.y)).xyz,
		mid,
		texStereoNoLod(tex_fb_unfiltered, vec2(u.x +w_h_height.x, u.y)).xyz,

		texStereoNoLod(tex_fb_unfiltered, vec2(u.x -w_h_height.x, u.y +w_h_height.y)).xyz,
		texStereoNoLod(tex_fb_unfiltered, vec2(u.x              , u.y +w_h_height.y)).xyz,
		texStereoNoLod(tex_fb_unfiltered, vec2(u.x +w_h_height.x, u.y +w_h_height.y)).xyz
	ARRAY_END();
	
	// Bilateral Blur (crippled)
	vec3 final_colour = vec3(0.,0.,0.);
	float Z = 0.0;
	UNROLL for (int j=-2; j <= 2; ++j) // 2 = kernelradius
	{
		int y = j < 0 ? -3 : 0;
		y = j > 0 ? 3 : y;

		UNROLL for (int i=-2; i <= 2; ++i)
		{
			int x = i < 0 ? -1 : 0;
			x = i > 0 ? 1 : x;

			vec3 cc;
			if((abs(j) == 2 || j == 0) && (abs(i) == 2 || i == 0)) // integer offset pixels, no filtering
				cc = e[4 + x + y];
			else
			{
				if(abs(j) == 1 && abs(i) == 1)
					cc = ((e[4] + e[4 + x]) + (e[4 + y] + e[4 + x + y]))*0.25;
				else
				{
					if(abs(j) == 2)
						cc = e[4 + y] + e[4 + x + y];
					else
						if (abs(i) == 2)
							cc = e[4 + x] + e[4 + x + y];
						else
							if (abs(j) == 1)
								cc = e[4] + e[4 + y];
							else
								cc = e[4] + e[4 + x];
					cc *= 0.5;
				}
			}

			const float factor = normpdf(cc-e[4], 0.25); // 0.25 = BSIGMA
			Z += factor;
			final_colour += factor*cc;
		}
	}

	// CAS (without Better Diagonals)
	const float b = LI(e[1]);
	const float d = LI(e[3]);
	const float f = LI(e[5]);
	const float h = LI(e[7]);
	const float e1 = LI(e[4]);

	const float mnRGB = min(min(min(d, e1), min(f, b)), h);
	const float mxRGB = max(max(max(d, e1), max(f, b)), h);

	// Smooth minimum distance to signal limit divided by smooth max.
	const float rcpMRGB = rcp(mxRGB);
	float ampRGB = saturate(min(mnRGB, 1.0-mxRGB) * rcpMRGB);

	vec3 sharpen = (e[4]-final_colour/Z) * sharpness;

	const float gs_sharpen = (sharpen.x+sharpen.y+sharpen.z) * 0.333333333333;
	sharpen = lerp(vec3_splat(gs_sharpen), sharpen, 0.5);

	ampRGB *= saturate(sharpness);
	ampRGB  = lerp(ampRGB, 1.0-ampRGB, balance);

	gl_FragColor = vec4(e[4] + sharpen*ampRGB, 1.0);
}
