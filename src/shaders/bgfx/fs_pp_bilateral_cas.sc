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

SAMPLER2DSTEREO(tex_fb_unfiltered,  0); // Framebuffer (unfiltered)
SAMPLER2DSTEREO(tex_depth,          4); // DepthBuffer



float normpdf(const vec3 v, const float sigma)
{
	return exp(dot(v,v)*(-0.5/(sigma*sigma)))*(0.39894228040143/sigma);
}

float LI(const vec3 l)
{
	return (l.x*0.5 + l.y) + l.z*0.5; // experimental, red and blue should not suffer too much //!! scaled by 2, so that we save a mul, factors out below, except for one constant! (see below)

	//!! if using weights that sum up to 1, change constant in CAS section from 2.0 to 1.0 again!
	//return dot(l, vec3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
	//return dot(l, vec3(0.2126, 0.7152, 0.0722));
}

void main()
{
	const float sharpness = 0.625*3.1; // ~0.1..~2 *3.1
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


#if 0
	// RCAS (without Better Diagonals)

	// RCAS also supports a define to enable a more expensive path to avoid some sharpening of noise.
	// Better to apply film grain after RCAS sharpening instead of using this define.
	#define RCAS_DENOISE 0 // 1 if there is noise, such as film grain // seems to be not worth it in our variant here, barely noticeable

	const float RCASLimit = -0.25; // should be only 0.0 .. 0.1875 though?! but then too sharp single pixel highlights! // Limits how much pixels can be sharpened. Lower values reduce artifacts, but reduce sharpening. It's recommended to lower this value when using a very high (> 1.2) sharpness value

	#if RCAS_DENOISE == 1 // Noise detection.
		const float bL = LI(e[1]);
		const float dL = LI(e[3]);
		const float eL = LI(e[4]);
		const float fL = LI(e[5]);
		const float hL = LI(e[7]);

		float nz = ((bL + dL) + (fL + hL))*-0.25 + eL;
		const float range = max(max(max(dL, eL), max(fL, bL)), hL) - min(min(min(dL, eL), min(fL, bL)), hL);
		nz = 1.0 - 0.5*saturate(abs(nz) * rcp(range));
	#endif

	const vec3 mnRGB = min(min(e[1], e[3]), min(e[5], e[7]));
	const vec3 mxRGB = max(max(e[1], e[3]), max(e[5], e[7]));

	const vec3 hitMin = mnRGB * rcp(mxRGB);
	vec3 hitMax = rcp(mnRGB - 1.0);
	hitMax = mxRGB * hitMax - hitMax;
	const vec3 lobeRGB = min(hitMin, hitMax);
	float lobe = min(RCASLimit*4.0, max(min(lobeRGB.r, min(lobeRGB.g, lobeRGB.b)), 0.0)) * (saturate(sharpness) * -0.25); // sharpness could be larger than 0..1 according to the reshade variant (up to 1.3), but doesn't look great

	#if RCAS_DENOISE == 1 // Apply noise removal.
		lobe *= nz;
	#endif

	// Resolve, which needs medium precision rcp approximation to avoid visible tonality changes.
	vec3 ampRGB = saturate((((e[1] + e[3]) + (e[5] + e[7]))*lobe + e[4]) * rcp(4.0*lobe + 1.0));

#else
	// CAS (without Better Diagonals)

	const float b = LI(e[1]);
	const float d = LI(e[3]);
	const float f = LI(e[5]);
	const float h = LI(e[7]);
	const float e4 = LI(e[4]);

	const float mnRGB = min(min(min(d, e4), min(f, b)), h);
	const float mxRGB = max(max(max(d, e4), max(f, b)), h);

	// Smooth minimum distance to signal limit divided by smooth max.
	const float rcpMRGB = rcp(mxRGB);
	float ampRGB = saturate(min(mnRGB, 2.0-mxRGB) * rcpMRGB); //!! 2.0 instead of 1.0, as LI() is scaled by x2!
	ampRGB *= saturate(sharpness);

#endif

	ampRGB  = mix(ampRGB, 1.0-ampRGB, balance);

	vec3 sharpen = (e[4]-final_colour/Z) * sharpness;

	const float gs_sharpen = (sharpen.x+sharpen.y+sharpen.z) * 0.333333333333;
	sharpen = mix(vec3_splat(gs_sharpen), sharpen, 0.5);

	gl_FragColor = vec4(e[4] + sharpen*ampRGB, 1.0);
}
