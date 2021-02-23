// uses texSamplerDepth & texSampler4,texSampler5 & texSamplerAOdither & w_h_height.xy

const float4 SSR_bumpHeight_fresnelRefl_scale_FS;

float3 approx_bump_normal(const float2 coords, const float2 offs, const float scale, const float sharpness)
{
    const float3 lumw = float3(0.212655,0.715158,0.072187);

    const float lpx = dot(tex2Dlod(texSampler5, float4(coords.x+offs.x,coords.y, 0.,0.)).xyz, lumw);
    const float lmx = dot(tex2Dlod(texSampler5, float4(coords.x-offs.x,coords.y, 0.,0.)).xyz, lumw);
    const float lpy = dot(tex2Dlod(texSampler5, float4(coords.x,coords.y+offs.y, 0.,0.)).xyz, lumw);
    const float lmy = dot(tex2Dlod(texSampler5, float4(coords.x,coords.y-offs.y, 0.,0.)).xyz, lumw);

    const float dpx = tex2Dlod(texSamplerDepth, float4(coords.x+offs.x,coords.y, 0.,0.)).x;
    const float dmx = tex2Dlod(texSamplerDepth, float4(coords.x-offs.x,coords.y, 0.,0.)).x;
    const float dpy = tex2Dlod(texSamplerDepth, float4(coords.x,coords.y+offs.y, 0.,0.)).x;
    const float dmy = tex2Dlod(texSamplerDepth, float4(coords.x,coords.y-offs.y, 0.,0.)).x;

    const float2 xymult = max(1.0 - float2(abs(dmx - dpx), abs(dmy - dpy)) * sharpness, 0.0);

    return normalize(float3(float2(lmx - lpx,lmy - lpy)*xymult/offs, scale));
}

float normal_fade_factor(const float3 n)
{
    return min(sqr(1.0-n.z)*0.5 + max(SSR_bumpHeight_fresnelRefl_scale_FS.w == 0.0 ? n.y : n.x,0.0) + abs(SSR_bumpHeight_fresnelRefl_scale_FS.w == 0.0 ? n.x : n.y)*0.5,1.0); // dot(n,float3(0,0,1))  dot(n,float3(0,1,0))  dot(n,float3(1,0,0)) -> penalty for z-axis/up (geometry like playfield), bonus for y-axis (like backwall) and x-axis (like sidewalls)
}

float4 ps_main_fb_ss_refl(in VS_OUTPUT_2D IN) : COLOR
{
	const float2 u = IN.tex0 + w_h_height.xy*0.5;

	const float3 color0 = tex2Dlod(texSampler4, float4(u, 0.,0.)).xyz; // original pixel

	const float depth0 = tex2Dlod(texSamplerDepth, float4(u, 0.,0.)).x;
	[branch] if((depth0 == 1.0) || (depth0 == 0.0)) //!!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
		return float4(color0, 1.0);

	const float3 normal = normalize(get_nonunit_normal(depth0,u));
	float3 normal_b = approx_bump_normal(u, 0.01 * w_h_height.xy / depth0, depth0 / (0.05*depth0 + 0.0001), 1000.0); //!! magic
	       normal_b = normalize(float3(normal.xy*normal_b.z + normal_b.xy*normal.z, normal.z*normal_b.z));
	       normal_b = normalize(lerp(normal,normal_b, SSR_bumpHeight_fresnelRefl_scale_FS.x * normal_fade_factor(normal))); // have less impact of fake bump normals on playfield, etc

	const float3 V = normalize(float3(0.5-u, -0.5)); // WTF?! cam is in 0,0,0 but why z=-0.5?

	const float fresnel = (SSR_bumpHeight_fresnelRefl_scale_FS.y + (1.0-SSR_bumpHeight_fresnelRefl_scale_FS.y) * pow(1.0-saturate(dot(V,normal_b)),5)) // fresnel for falloff towards silhouette
	                     * SSR_bumpHeight_fresnelRefl_scale_FS.z // user scale
	                     * sqr(normal_fade_factor(normal_b/*normal*/)); // avoid reflections on playfield, etc

#if 0 // test code
    return float4(0.,sqr(normal_fade_factor(normal_b/*normal*/)),0., 1.0);
#endif

	[branch] if(fresnel < 0.01) //!! early out if contribution too low
		return float4(color0, 1.0);

	const int samples = 32; //!! reduce to ~24? would save ~1-4% overall frame time, depending on table
	const float ReflBlurWidth = 2.2; //!! magic, small enough to not collect too much, and large enough to have cool reflection effects

	const float ushift = /*hash(IN.tex0) + w_h_height.zw*/ // jitter samples via hash of position on screen and then jitter samples by time //!! see below for non-shifted variant
	                     /*frac(*/tex2Dlod(texSamplerAOdither, float4(IN.tex0/(64.0*w_h_height.xy), 0.,0.)).z /*+ w_h_height.z)*/; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
	const float2 offsMul = normal_b.xy * (/*w_h_height.xy*/ float2(1.0/1920.0,1.0/1080.0) * ReflBlurWidth * (32./(float)samples)); //!! makes it more resolution independent?? test with 4xSSAA

	// loop in screen space, simply collect all pixels in the normal direction (not even a depth check done!)
	float3 refl = float3(0.,0.,0.);
	float color0w = 0.;
	[unroll] for(int i=1; i</*=*/samples; i++) //!! due to jitter
	{
		const float2 offs = u + ((float)i+ushift)*offsMul; //!! jitter per pixel (uses blue noise tex)
		const float3 color = tex2Dlod(texSampler5, float4(offs, 0.,0.)).xyz;
		
		[branch] if(i==1) // necessary special case as compiler warns/'optimizes' sqrt below into rqsrt?!
		{
		refl += color;
		}
		else
		{
		const float w = sqrt((float)(i-1)/(float)samples); //!! fake falloff for samples more far away
		refl += color*(1.0-w); //!! dampen large color values in addition?
		color0w += w;
		}
	}

	refl += color0*color0w;
	refl *= 1.0/(float)(samples-1); //!! -1 due to jitter

	return float4(lerp(color0,refl, min(fresnel,1.0)), 1.0);
}
