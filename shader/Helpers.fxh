#define PI 3.1415926535897932384626433832795

//
// Gamma & ToneMapping
//

#define BURN_HIGHLIGHTS 0.25

float3 InvGamma(const float3 color) //!! use hardware support? D3DSAMP_SRGBTEXTURE
{
	return /*color * (color * (color * 0.305306011 + 0.682171111) + 0.012522878);/*/ pow(color, 2.2); // pow does not matter anymore on current GPUs
}

float3 InvToneMap(const float3 color)
{
    const float inv_2bh = 0.5/BURN_HIGHLIGHTS;
    const float bh = 4.0*BURN_HIGHLIGHTS - 2.0;
	return (color - 1.0 + sqrt(color*(color + bh) + 1.0))*inv_2bh;
}

float3 FBGamma(const float3 color) //!! use hardware support? D3DRS_SRGBWRITEENABLE
{
	return pow(color, 1.0/2.2); // pow does not matter anymore on current GPUs

	/*const float3 t0 = sqrt(color);
	const float3 t1 = sqrt(t0);
	const float3 t2 = sqrt(t1);
	return 0.662002687 * t0 + 0.684122060 * t1 - 0.323583601 * t2 - 0.0225411470 * color;*/
}

float3 FBToneMap(const float3 color)
{
    const float l = color.x*0.176204 + color.y*0.812985 + color.z*0.0108109;
    return color * ((l*BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}

float3 FilmicToneMap(const float3 hdr, const float whitepoint) //!! test/experimental
{
    const float4 vh = float4(hdr,whitepoint);
    const float4 va = 1.425*vh + 0.05;
    const float4 vf = (vh*va + 0.004)/(vh*(va+0.55) + 0.0491) - 0.0821;
    return vf.rgb/vf.aaa;
}

//
// Blends
//

float4 Additive(const float4 cBase, const float4 cBlend, const float percent)
{
   return cBase + cBlend*percent;
}

float3 Screen (const float3 cBase, const float3 cBlend)
{
	return 1.0 - (1.0 - cBase) * (1.0 - cBlend);
}

float4 Screen (const float4 cBase, const float4 cBlend)
{
	return 1.0 - (1.0 - cBase) * (1.0 - cBlend);
}

float3 ScreenHDR (const float3 cBase, const float3 cBlend)
{
	return max(1.0 - (1.0 - cBase) * (1.0 - cBlend), float3(0.,0.,0.));
}

float4 ScreenHDR (const float4 cBase, const float4 cBlend)
{
	return max(1.0 - (1.0 - cBase) * (1.0 - cBlend), float4(0.,0.,0.,0.));
}

float4 Multiply (const float4 cBase, const float4 cBlend, const float percent)
{
	return cBase * cBlend*percent;
}

float4 Overlay (const float4 cBase, const float4 cBlend)
{
	// overlay has two output possbilities
	// which is taken is decided if pixel value
	// is below half or not

	float4 cNew = step(0.5, cBase);
	
	// we pick either solution
	// depending on pixel
	
	// first is case of < 0.5
	// second is case for >= 0.5
	
	// interpolate between the two, 
	// using color as influence value
	cNew = lerp(cBase*cBlend*2.0, 1.0-2.0*(1.0-cBase)*(1.0-cBlend), cNew);

	//cNew.a = 1.0f;
	return cNew;
}

float4 OverlayHDR (const float4 cBase, const float4 cBlend)
{
	// overlay has two output possbilities
	// which is taken is decided if pixel value
	// is below half or not

	float4 cNew = step(0.5, cBase);
	
	// we pick either solution
	// depending on pixel
	
	// first is case of < 0.5
	// second is case for >= 0.5
	
	// interpolate between the two, 
	// using color as influence value
	cNew = lerp(cBase*cBlend*2.0, max(1.0-2.0*(1.0-cBase)*(1.0-cBlend), float4(0.,0.,0.,0.)), cNew);

	//cNew.a = 1.0f;
	return cNew;
}
