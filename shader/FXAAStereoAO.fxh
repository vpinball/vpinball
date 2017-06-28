//!! add reflection direction occlusion, so that that will be used for blocking reflection/envmap?
//!! opt.?

#if 0
float2 hash(const float2 gridcell)
{
	/*const float3 o = float3(26.0, 161.0, 26.0);
	const float d = 71.0;
	const float lf = 1.0/951.135664;
	float3 P = float3(gridcell.x,gridcell.y,gridcell.x+1.0); // gridcell is assumed to be an integer coordinate (scaled by width&height basically)
	P = P - floor(P * ( 1.0 / d )) * d;
	P += o;
	P *= P;
	return frac( P.xz * P.yy * lf );*/

	return frac(float2(sin(dot(gridcell, float2(12.9898, 78.233) * 2.0)),
	                   sin(dot(gridcell, float2(12.9898, 78.233)      ))) * 43758.5453);
}
#endif

float3 get_nonunit_normal(const float depth0, const float2 u) // use neighboring pixels // quite some tex access by this
{
	const float depth1 = tex2Dlod(texSamplerDepth, float4(u.x, u.y+w_h_height.y, 0.,0.)).x;
	const float depth2 = tex2Dlod(texSamplerDepth, float4(u.x+w_h_height.x, u.y, 0.,0.)).x;
	return float3(w_h_height.y * (depth2 - depth0), (depth1 - depth0) * w_h_height.x, w_h_height.y * w_h_height.x); //!!
}

//float3 sphere_sample(const float2 t)
//{
//const float phi = t.y * (2.0*3.1415926535897932384626433832795);
//const float z = 1.0 - t.x*2.0;
//const float r = sqrt(1.0 - z*z);
//float sp,cp;
//sincos(phi,sp,cp);
//return float3(cp*r, z, sp*r);
//}

float3 cos_hemisphere_sample(const float2 t) // u,v in [0..1), returns y-up
{
	const float phi = t.y * (2.0*3.1415926535897932384626433832795);
	const float cosTheta = sqrt(1.0 - t.x);
	const float sinTheta = sqrt(t.x);
	float sp,cp;
	sincos(phi,sp,cp);
	return float3(cp * sinTheta, cosTheta, sp * sinTheta);
}

float3 rotate_to_vector_upper(const float3 vec, const float3 normal)
{
	if(normal.y > -0.99999)
	{
		const float h = 1.0/(1.0+normal.y);
		const float hz = h*normal.z;
		const float hzx = hz*normal.x;
		return float3(
			vec.x * (normal.y+hz*normal.z) + vec.y * normal.x - vec.z * hzx,
			vec.y * normal.y - vec.x * normal.x - vec.z * normal.z,
			vec.y * normal.z - vec.x * hzx + vec.z * (normal.y+h*normal.x*normal.x));
	}
	else return -vec;
}

/*float3 cos_hemisphere_sample(const float3 normal, float2 uv)
{
	const float theta = (2.0*3.1415926535897932384626433832795) * uv.x;
	uv.y = 2.0 * uv.y - 1.0;
	float st,ct;
	sincos(theta,st,ct);
	const float3 spherePoint = float3(sqrt(1.0 - uv.y * uv.y) * float2(ct, st), uv.y);
	return normalize(normal + spherePoint);
}*/

/*float3 cos_hemisphere_nonunit_sample(const float3 normal, float2 uv)
{
	const float theta = (2.0*3.1415926535897932384626433832795) * uv.x;
	uv.y = 2.0 * uv.y - 1.0;
	float st,ct;
	sincos(theta,st,ct);
	const float3 spherePoint = float3(sqrt(1.0 - uv.y * uv.y) * float2(ct, st), uv.y);
	return normal + spherePoint;
}*/

/*float4 ps_main_normals(in VS_OUTPUT_2D IN) : COLOR // separate pass to generate normals (should actually reduce bandwidth needed in AO pass, but overall close to no performance difference or even much worse perf, depending on gfxboard)
{
	const float2 u = IN.tex0 + w_h_height.xy*0.5;

	const float depth0 = tex2Dlod(texSamplerDepth, float4(u, 0.,0.)).x;
	[branch] if((depth0 == 1.0) || (depth0 == 0.0)) //!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
		return float4(0.0, 0.,0.,0.);

	const float3 normal = normalize(get_nonunit_normal(depth0, u)) *0.5+0.5;
	return float4(normal,0.);
}*/

#if 0
float2 UnitVectorToOctahedron( float3 N )
{
	N.xy /= dot( 1., abs(N) );
	if( N.z <= 0. )
		N.xy = ( 1. - abs(N.yx) ) * ( N.xy >= 0. ? float2(1.,1.) : float2(-1.,-1.) );
	return N.xy;
}

float3 OctahedronToUnitVector( const float2 Oct )
{
	float3 N = float3( Oct, 1. - dot( 1., abs(Oct) ) );
	if( N.z < 0. )
		N.xy = ( 1. - abs(N.yx) ) * ( N.xy >= 0. ? float2(1.,1.) : float2(-1.,-1.) );
	return normalize(N);
}

float3 Pack1212To888( const float2 x )
{
	// Pack 12:12 to 8:8:8
	const float2 x1212 = floor( x * 4095. );
	const float2 High = floor( x1212 / 256. );	// x1212 >> 8
	const float2 Low = x1212 - High * 256.;	// x1212 & 255
	const float3 x888 = float3( Low, High.x + High.y * 16. );
	return saturate( x888 / 255. );
}

float2 Pack888To1212( const float3 x )
{
	// Pack 8:8:8 to 12:12
	const float3 x888 = floor( x * 255. );
	const float High = floor( x888.z / 16. );	// x888.z >> 4
	const float Low = x888.z - High * 16.;		// x888.z & 15
	const float2 x1212 = x888.xy + float2( Low, High ) * 256.;
	return saturate( x1212 / 4095. );
}

float3 EncodeNormal( const float3 N )
{
	return Pack1212To888( UnitVectorToOctahedron( N ) * 0.5 + 0.5 );
}

float3 DecodeNormal( const float3 N )
{
	return OctahedronToUnitVector( Pack888To1212( N ) * 2. - 1. );
}
float2 compress_normal(const float3 n)
{
    const float p = sqrt(n.z*8.+8.);
    return n.xy/p + 0.5;
}

float3 decompress_normal(const float2 c)
{
    const float2 fc = c*4.-2.;
    const float f = dot(fc,fc);
    return float3(fc*sqrt(1.-f*0.25), 1.-f*0.5);
}
#endif

float4 ps_main_ao(in VS_OUTPUT_2D IN) : COLOR
{
	const float2 u = IN.tex0 + w_h_height.xy*0.5;

	const float depth0 = tex2Dlod(texSamplerDepth, float4(u, 0.,0.)).x;
	[branch] if((depth0 == 1.0) || (depth0 == 0.0)) //!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
		return float4(1.0, 0.,0.,0.);

	const float3 ushift = /*hash(IN.tex0) + w_h_height.zw*/ // jitter samples via hash of position on screen and then jitter samples by time //!! see below for non-shifted variant
	                      tex2Dlod(texSamplerAOdither, float4(IN.tex0/(64.0*w_h_height.xy) + w_h_height.zw, 0.,0.)).xyz; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
	//const float base = 0.0;
	const float area = 0.06; //!!
	const float falloff = 0.0002; //!!
	const int samples = 9; //4,8,9,13,21,25 korobov,fibonacci
	const float radius = 0.001+frac(ushift.z+w_h_height.z*(float)(samples-1))*0.009; // sample radius //!! w_h_height.z reused, but should not be that bad
	const float depth_threshold_normal = 0.005;
	const float total_strength = AO_scale_timeblur.x * (/*1.0 for uniform*/0.5 / samples);
	const float3 normal = normalize(get_nonunit_normal(depth0, u));
	//const float3 normal = tex2Dlod(texSamplerNormals, float4(u, 0.,0.)).xyz *2.0-1.0; // use 8bitRGB pregenerated normals
	const float radius_depth = radius/depth0;

	float occlusion = 0.0;
	[unroll] for(int i=0; i < samples; ++i) {
		const float2 r = float2(i*(1.0 / samples), i*(2.0 / samples)); //1,5,2,8,13,7 korobov,fibonacci //!! could also use progressive/extensible lattice via rad_inv(i)*(1501825329, 359975893) (check precision though as this should be done in double or uint64)
		//const float3 ray = sphere_sample(frac(r+ushift.xy)); // shift lattice // uniform variant
		const float2 ray = rotate_to_vector_upper(cos_hemisphere_sample(frac(r+ushift.xy)), normal).xy; // shift lattice
		//!! maybe a bit worse distribution: const float2 ray = cos_hemisphere_sample(normal,frac(r+ushift.xy)).xy; // shift lattice
		//const float rdotn = dot(ray,normal);
		const float2 hemi_ray = u + (radius_depth /** sign(rdotn) for uniform*/) * ray.xy;
		const float occ_depth = tex2Dlod(texSamplerDepth, float4(hemi_ray, 0.,0.)).x;
		const float3 occ_normal = get_nonunit_normal(occ_depth, hemi_ray);
		//const float3 occ_normal = tex2Dlod(texSamplerNormals, float4(hemi_ray, 0.,0.)).xyz *2.0-1.0;  // use 8bitRGB pregenerated normals, can also omit normalization below then
		const float diff_depth = depth0 - occ_depth;
		const float diff_norm = dot(occ_normal,normal);
		occlusion += step(falloff, diff_depth) * /*abs(rdotn)* for uniform*/ (diff_depth < depth_threshold_normal ? (1.0-diff_norm*diff_norm/dot(occ_normal,occ_normal)) : 1.0) * (1.0-smoothstep(falloff, area, diff_depth));
	}
	// weight with result(s) from previous frames
	const float ao = 1.0 - total_strength * occlusion;
	return float4( (tex2Dlod(texSampler5, float4(u+w_h_height.xy*0.5, 0.,0.)).x //abuse bilerp for filtering (by using half texel/pixel shift)
				   +tex2Dlod(texSampler5, float4(u-w_h_height.xy*0.5, 0.,0.)).x
				   +tex2Dlod(texSampler5, float4(u+float2(w_h_height.x,-w_h_height.y)*0.5, 0.,0.)).x
				   +tex2Dlod(texSampler5, float4(u-float2(w_h_height.x,-w_h_height.y)*0.5, 0.,0.)).x)
		*(0.25*(1.0-AO_scale_timeblur.y))+saturate(ao /*+base*/)*AO_scale_timeblur.y, 0.,0.,0.);
}

// stereo

//!! opt.?
float4 ps_main_stereo(in VS_OUTPUT_2D IN) : COLOR
{
	float2 u = IN.tex0 + w_h_height.xy*0.5;
	const float MaxSeparation = ms_zpd_ya_td.x;
	const float ZPD = ms_zpd_ya_td.y;
	const bool yaxis = (ms_zpd_ya_td.z != 0.0); //!! uniform
	const bool topdown = (ms_zpd_ya_td.w == 1.0); //!! uniform
	const bool sidebyside = (ms_zpd_ya_td.w == 2.0); //!! uniform
	const int y = w_h_height.z*u.y;
	const bool l = sidebyside ? (u.x < 0.5) : topdown ? (u.y < 0.5) : ((y+1)/2 == y/2); //last check actually means (y&1) //!! %2 //!! float diff = frac(dot(tex,(screen_size / 2.0))+0.25); if(diff < 0.5)... //returns 0.25 and 0.75
	if(topdown) { u.y *= 2.0; if(!l) u.y -= 1.0; }  //!! !topdown: (u.y+w_h_height.y) ?
	else if(sidebyside) { u.x *= 2.0; if(!l) u.x -= 1.0; }
	const float su = l ? MaxSeparation : -MaxSeparation;
	float minDepth = min(min(tex2Dlod(texSamplerDepth, float4(u + (yaxis ? float2(0.0,0.5*su) : float2(0.5*su,0.0)), 0.,0.)).x, tex2Dlod(texSamplerDepth, float4(u + (yaxis ? float2(0.0,0.666*su) : float2(0.666*su,0.0)), 0.,0.)).x), tex2Dlod(texSamplerDepth, float4(u + (yaxis ? float2(0.0,su) : float2(su,0.0)), 0.,0.)).x);
	float parallax = (w_h_height.w+MaxSeparation) - min(MaxSeparation/(0.5+minDepth*(1.0/ZPD-0.5)), (w_h_height.w+MaxSeparation));
	if(!l)
		parallax = -parallax;
	if(yaxis)
		parallax = -parallax;
	const float3 col = tex2Dlod(texSampler5, float4(u + (yaxis ? float2(0.0,parallax) : float2(parallax,0.0)), 0.,0.)).xyz;
	//if(!aa)
	//	return float4(col, 1.0); // otherwise blend with 'missing' scanline
	const float2 aaoffs = sidebyside ? float2(w_h_height.x,0.0) : float2(0.0,w_h_height.y);
	minDepth = min(min(tex2Dlod(texSamplerDepth, float4(u + (yaxis ? float2(0.0,0.5*su) : float2(0.5*su,0.0)) + aaoffs, 0.,0.)).x, tex2Dlod(texSamplerDepth, float4(u + (yaxis ? float2(0.0,0.666*su) : float2(0.666*su,0.0)) + aaoffs, 0.,0.)).x), tex2Dlod(texSamplerDepth, float4(u + (yaxis ? float2(0.0,su) : float2(su,0.0)) + aaoffs, 0.,0.)).x);
	parallax = (w_h_height.w+MaxSeparation) - min(MaxSeparation/(0.5+minDepth*(1.0/ZPD-0.5)), (w_h_height.w+MaxSeparation));
	if(!l)
		parallax = -parallax;
	if(yaxis)
		parallax = -parallax;
	return float4((col + tex2Dlod(texSampler5, float4(u + (yaxis ? float2(0.0,parallax) : float2(parallax,0.0)) + aaoffs, 0.,0.)).xyz)*0.5, 1.0);
}

// NFAA

//#define NFAA_EDGE_DETECTION_VARIANT // different edge detection (sums for finite differences differ)
//#define NFAA_USE_COLOR // use color instead of luminance
//#define NFAA_TEST_MODE // debug output
// undef both of the following for variant 0
#define NFAA_VARIANT // variant 1
//#define NFAA_VARIANT2 // variant 2

float GetLuminance(const float3 l)
{
	return dot(l, float3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
	//return 0.299*l.x + 0.587*l.y + 0.114*l.z;
	//return 0.2126*l.x + 0.7152*l.y + 0.0722*l.z; // photometric
	//return sqrt(0.299 * l.x*l.x + 0.587 * l.y*l.y + 0.114 * l.z*l.z); // hsp
}

#ifndef NFAA_USE_COLOR
float2 findContrastByLuminance(const float2 XYCoord, const float filterSpread)
{
	const float2 upOffset    = float2(0.0, w_h_height.y * filterSpread);
	const float2 rightOffset = float2(w_h_height.x * filterSpread, 0.0);

	const float topHeight         = GetLuminance(tex2Dlod(texSampler5, float4(XYCoord +               upOffset, 0.,0.)).rgb);
	const float bottomHeight      = GetLuminance(tex2Dlod(texSampler5, float4(XYCoord -               upOffset, 0.,0.)).rgb);
	const float rightHeight       = GetLuminance(tex2Dlod(texSampler5, float4(XYCoord + rightOffset           , 0.,0.)).rgb);
	const float leftHeight        = GetLuminance(tex2Dlod(texSampler5, float4(XYCoord - rightOffset           , 0.,0.)).rgb);
	const float leftTopHeight     = GetLuminance(tex2Dlod(texSampler5, float4(XYCoord - rightOffset + upOffset, 0.,0.)).rgb);
	const float leftBottomHeight  = GetLuminance(tex2Dlod(texSampler5, float4(XYCoord - rightOffset - upOffset, 0.,0.)).rgb);
	const float rightBottomHeight = GetLuminance(tex2Dlod(texSampler5, float4(XYCoord + rightOffset + upOffset, 0.,0.)).rgb);
	const float rightTopHeight    = GetLuminance(tex2Dlod(texSampler5, float4(XYCoord + rightOffset - upOffset, 0.,0.)).rgb);

#ifdef NFAA_EDGE_DETECTION_VARIANT
	const float sum0 = rightTopHeight    + bottomHeight + leftTopHeight;
	const float sum1 = leftBottomHeight  + topHeight    + rightBottomHeight;
	const float sum2 = leftTopHeight     + rightHeight  + leftBottomHeight;
	const float sum3 = rightBottomHeight + leftHeight   + rightTopHeight;
#else
	const float sum0 = rightTopHeight + topHeight + rightBottomHeight;
	const float sum1 = leftTopHeight + bottomHeight + leftBottomHeight;
	const float sum2 = leftTopHeight + leftHeight + rightTopHeight;
	const float sum3 = leftBottomHeight + rightHeight + rightBottomHeight;
#endif

	// finite differences for final vectors
	return float2( sum1 - sum0, sum2 - sum3 );
}

#else

float2 findContrastByColor(const float2 XYCoord, const float filterSpread)
{
	const float2 upOffset    = float2(0.0, w_h_height.y * filterSpread);
	const float2 rightOffset = float2(w_h_height.x * filterSpread, 0.0);

	const float3 topHeight         = tex2Dlod(texSampler5, float4(XYCoord +               upOffset, 0.,0.)).rgb;
	const float3 bottomHeight      = tex2Dlod(texSampler5, float4(XYCoord -               upOffset, 0.,0.)).rgb;
	const float3 rightHeight       = tex2Dlod(texSampler5, float4(XYCoord + rightOffset           , 0.,0.)).rgb;
	const float3 leftHeight        = tex2Dlod(texSampler5, float4(XYCoord - rightOffset           , 0.,0.)).rgb;
	const float3 leftTopHeight     = tex2Dlod(texSampler5, float4(XYCoord - rightOffset + upOffset, 0.,0.)).rgb;
	const float3 leftBottomHeight  = tex2Dlod(texSampler5, float4(XYCoord - rightOffset - upOffset, 0.,0.)).rgb;
	const float3 rightBottomHeight = tex2Dlod(texSampler5, float4(XYCoord + rightOffset + upOffset, 0.,0.)).rgb;
	const float3 rightTopHeight    = tex2Dlod(texSampler5, float4(XYCoord + rightOffset - upOffset, 0.,0.)).rgb;

#ifdef NFAA_EDGE_DETECTION_VARIANT
	const float sum0 = rightTopHeight    + bottomHeight + leftTopHeight;
	const float sum1 = leftBottomHeight  + topHeight    + rightBottomHeight;
	const float sum2 = leftTopHeight     + rightHeight  + leftBottomHeight;
	const float sum3 = rightBottomHeight + leftHeight   + rightTopHeight;
#else
	const float sum0 = rightTopHeight + topHeight + rightBottomHeight;
	const float sum1 = leftTopHeight + bottomHeight + leftBottomHeight;
	const float sum2 = leftTopHeight + leftHeight + rightTopHeight;
	const float sum3 = leftBottomHeight + rightHeight + rightBottomHeight;
#endif

	// finite differences for final vectors
	return float2( length(sum1 - sum0), length(sum2 - sum3) );
}
#endif

float4 ps_main_nfaa(in VS_OUTPUT_2D IN) : COLOR
{
#ifndef NFAA_VARIANT2
 #ifdef NFAA_VARIANT
	const float filterStrength = 1.0;
 #else
	const float filterStrength = 0.5;
 #endif
	const float filterSpread = 4.0; //!! or original 3? or larger 5?
#else
	const float filterSpread = 1.0;
#endif

	const float2 u = IN.tex0 + w_h_height.xy*0.5;

	const float3 Scene0 = tex2Dlod(texSampler5, float4(u, 0.,0.)).rgb;
	[branch] if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = tex2Dlod(texSamplerDepth, float4(u, 0.,0.)).x;
		[branch] if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
			return float4(Scene0, 1.0);
	}

#ifdef NFAA_USE_COLOR // edges from color
	float2 Vectors = findContrastByColor(u, filterSpread);
#else
	float2 Vectors = findContrastByLuminance(u, filterSpread);
#endif

#ifndef NFAA_VARIANT2
	const float filterStrength2 = filterStrength + filterSpread*0.5;
	const float filterClamp = filterStrength2 / filterSpread;

	Vectors = clamp(Vectors * filterStrength2, -float2(filterClamp, filterClamp), float2(filterClamp, filterClamp));
#else
	Vectors *= filterSpread;
#endif

	const float2 Normal = Vectors * (w_h_height.xy /* * 2.0*/);

	const float3 Scene1 = tex2Dlod(texSampler5, float4(u + Normal, 0.,0.)).rgb;
	const float3 Scene2 = tex2Dlod(texSampler5, float4(u - Normal, 0.,0.)).rgb;
#if defined(NFAA_VARIANT) || defined(NFAA_VARIANT2)
	const float3 Scene3 = tex2Dlod(texSampler5, float4(u + float2(Normal.x, -Normal.y)*0.5, 0.,0.)).rgb;
	const float3 Scene4 = tex2Dlod(texSampler5, float4(u - float2(Normal.x, -Normal.y)*0.5, 0.,0.)).rgb;
#else
	const float3 Scene3 = tex2Dlod(texSampler5, float4(u + float2(Normal.x, -Normal.y), 0.,0.)).rgb;
	const float3 Scene4 = tex2Dlod(texSampler5, float4(u - float2(Normal.x, -Normal.y), 0.,0.)).rgb;
#endif

#ifdef NFAA_TEST_MODE // debug
	const float3 o_Color = normalize(float3(Vectors * 0.5 + 0.5, 1.0));
#else
	const float3 o_Color = (Scene0 + Scene1 + Scene2 + Scene3 + Scene4) * 0.2;
#endif

	return float4(o_Color, 1.0);
}

// DLAA approximation

float3 sampleOffset(const float2 u, const float2 pixelOffset )
{
   return tex2Dlod(texSampler5, float4(u + pixelOffset * w_h_height.xy, 0.,0.)).xyz;
}

float4 sampleOffseta(const float2 u, const float2 pixelOffset )
{
   return tex2Dlod(texSampler5, float4(u + pixelOffset * w_h_height.xy, 0.,0.));
}

float avg(const float3 l)
{
   //return dot(l, float3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
   return (l.x+l.y+l.z) * (1.0 / 3.0);
}

float4 ps_main_dlaa_edge(in VS_OUTPUT_2D IN) : COLOR
{
   const float2 u = IN.tex0 + w_h_height.xy*0.5;

   const float3 sCenter    = sampleOffset(u, float2( 0.0,  0.0) );
   const float3 sUpLeft    = sampleOffset(u, float2(-0.5, -0.5) );
   const float3 sUpRight   = sampleOffset(u, float2( 0.5, -0.5) );
   const float3 sDownLeft  = sampleOffset(u, float2(-0.5,  0.5) );
   const float3 sDownRight = sampleOffset(u, float2( 0.5,  0.5) );

   const float3 diff       = abs( (sUpLeft + sUpRight) + (sDownLeft + sDownRight) - sCenter * 4.0 );
   const float  edgeMask   = avg(diff) * 4.0; //!! magic

   return /*test: float4(edgeMask,edgeMask,edgeMask,1.0);*/ float4(sCenter, edgeMask);
}


float4 ps_main_dlaa(in VS_OUTPUT_2D IN) : COLOR
{
   const float2 u = IN.tex0 + w_h_height.xy*0.5;

   const float4 sampleCenter = sampleOffseta(u, float2( 0.0,  0.0) );
   [branch] if(w_h_height.w == 1.0 /*&& sampleCenter.a == 0.0*/) // depth buffer available? /*AND no edge here? -> ignored because of performance*/
   {
      const float depth0 = tex2Dlod(texSamplerDepth, float4(u, 0.,0.)).x;
      [branch] if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
         return float4(sampleCenter.xyz, 1.0);
   }

   // short edges
   const float4 sampleHorizNeg0  = sampleOffseta(u, float2(-1.5,  0.0) );
   const float4 sampleHorizPos0  = sampleOffseta(u, float2( 1.5,  0.0) ); 
   const float4 sampleVertNeg0   = sampleOffseta(u, float2( 0.0, -1.5) ); 
   const float4 sampleVertPos0   = sampleOffseta(u, float2( 0.0,  1.5) );

   const float3 sumHoriz         = sampleHorizNeg0.xyz + sampleHorizPos0.xyz;
   const float3 sumVert          = sampleVertNeg0.xyz  + sampleVertPos0.xyz;

   const float3 sampleLeft       = sampleOffset(u, float2(-1.0,  0.0) );
   const float3 sampleRight      = sampleOffset(u, float2( 1.0,  0.0) );
   const float3 sampleTop        = sampleOffset(u, float2( 0.0, -1.0) );
   const float3 sampleDown       = sampleOffset(u, float2( 0.0,  1.0) );

   const float3 diffToCenterHoriz= abs((sampleLeft+sampleRight) * 0.5 - sampleCenter.xyz); //!! was sumHoriz instead of l&r
   const float3 diffToCenterVert = abs((sampleTop+sampleDown) * 0.5 - sampleCenter.xyz); //!! was sumVert instead of t&d

   const float valueEdgeHoriz    = avg(diffToCenterHoriz);
   const float valueEdgeVert     = avg(diffToCenterVert);

   const float edgeDetectHoriz   = 3.0 * valueEdgeHoriz - 0.1; //!! magic params
   const float edgeDetectVert    = 3.0 * valueEdgeVert  - 0.1;

   const float3 avgHoriz         = (sumHoriz + sampleCenter.xyz) * (1.0/3.0);
   const float3 avgVert          = (sumVert  + sampleCenter.xyz) * (1.0/3.0);

   const float valueHoriz        = avg(avgHoriz);
   const float valueVert         = avg(avgVert);

   const float blurAmountHoriz   = saturate(edgeDetectHoriz / valueHoriz);
   const float blurAmountVert    = saturate(edgeDetectVert  / valueVert);

   float3 aaResult               = lerp( sampleCenter.xyz, avgHoriz, blurAmountVert * 0.5); //!! magic sharpen
   aaResult                      = lerp( aaResult,         avgVert,  blurAmountHoriz * 0.5); //!! magic sharpen

   // long edges
   const float4 sampleVertNeg1   = sampleOffseta(u, float2(0.0, -3.5) );
   const float4 sampleVertNeg15  = sampleOffseta(u, float2(0.0, -5.5) );
   const float4 sampleVertNeg2   = sampleOffseta(u, float2(0.0, -7.5) );
   const float4 sampleVertPos1   = sampleOffseta(u, float2(0.0,  3.5) ); 
   const float4 sampleVertPos15  = sampleOffseta(u, float2(0.0,  5.5) ); 
   const float4 sampleVertPos2   = sampleOffseta(u, float2(0.0,  7.5) ); 

   const float4 sampleHorizNeg1  = sampleOffseta(u, float2(-3.5, 0.0) ); 
   const float4 sampleHorizNeg15 = sampleOffseta(u, float2(-5.5, 0.0) ); 
   const float4 sampleHorizNeg2  = sampleOffseta(u, float2(-7.5, 0.0) );
   const float4 sampleHorizPos1  = sampleOffseta(u, float2( 3.5, 0.0) ); 
   const float4 sampleHorizPos15 = sampleOffseta(u, float2( 5.5, 0.0) ); 
   const float4 sampleHorizPos2  = sampleOffseta(u, float2( 7.5, 0.0) ); 

   const float pass1EdgeAvgHoriz = saturate(( sampleHorizNeg2.a + sampleHorizNeg1.a + sampleHorizNeg15.a + sampleHorizNeg0.a + sampleHorizPos0.a + sampleHorizPos1.a + sampleHorizPos15.a + sampleHorizPos2.a ) * (2.0 / 8.0) - 1.0);
   const float pass1EdgeAvgVert  = saturate(( sampleVertNeg2.a  + sampleVertNeg1.a + sampleVertNeg15.a  + sampleVertNeg0.a  + sampleVertPos0.a + sampleVertPos1.a + sampleVertPos15.a  + sampleVertPos2.a  ) * (2.0 / 8.0) - 1.0);

   [branch] if(abs(pass1EdgeAvgHoriz - pass1EdgeAvgVert) > 0.2) //!! magic
   {
        const float valueHorizLong = avg(sampleHorizNeg2.xyz + sampleHorizNeg1.xyz + sampleHorizNeg15.xyz + sampleHorizNeg0.xyz + sampleHorizPos0.xyz + sampleHorizPos1.xyz + sampleHorizPos15.xyz + sampleHorizPos2.xyz) * (1.0/8.0);
        const float valueVertLong  = avg(sampleVertNeg2.xyz  + sampleVertNeg1.xyz + sampleVertNeg15.xyz + sampleVertNeg0.xyz  + sampleVertPos0.xyz + sampleVertPos1.xyz + sampleVertPos15.xyz + sampleVertPos2.xyz) * (1.0/8.0);

        const float valueCenter    = avg(sampleCenter.xyz);
        const float valueLeft      = avg(sampleLeft);
        const float valueRight     = avg(sampleRight);
        const float valueTop       = avg(sampleTop);
        const float valueBottom    = avg(sampleDown);

        const float vx = (valueCenter == valueLeft)   ? 0. : saturate(      ( valueVertLong  - valueLeft   ) / (valueCenter - valueLeft));
        const float hx = (valueCenter == valueTop)    ? 0. : saturate(      ( valueHorizLong - valueTop    ) / (valueCenter - valueTop));
        const float vy = (valueCenter == valueRight)  ? 0. : saturate(1.0 + ( valueVertLong  - valueCenter ) / (valueCenter - valueRight));
        const float hy = (valueCenter == valueBottom) ? 0. : saturate(1.0 + ( valueHorizLong - valueCenter ) / (valueCenter - valueBottom));

        const float3 longBlurVert  = lerp( sampleRight,
                                           lerp( sampleLeft,  sampleCenter.xyz, vx ),
                                           vy );
        const float3 longBlurHoriz = lerp( sampleDown,
                                           lerp( sampleTop,   sampleCenter.xyz, hx ),
                                           hy );

        aaResult                   = lerp( aaResult, longBlurVert, pass1EdgeAvgVert * 0.5); //!! magic
        aaResult                   = lerp( aaResult, longBlurHoriz, pass1EdgeAvgHoriz * 0.5); //!! magic

        //test: return float4(aaResult,1.);
   }
   //test: return float4(0,0,0,1);

   return /*test: float4(sampleCenter.a,sampleCenter.a,sampleCenter.a,1.0);*/ float4(aaResult, 1.0);
}


// FXAA

float luma(const float3 l)
{
    return dot(l, float3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
    //return 0.299*l.x + 0.587*l.y + 0.114*l.z;
    //return 0.2126*l.x + 0.7152*l.y + 0.0722*l.z; // photometric
    //return sqrt(0.299 * l.x*l.x + 0.587 * l.y*l.y + 0.114 * l.z*l.z); // hsp
}

// Approximation of FXAA
float4 ps_main_fxaa1(in VS_OUTPUT_2D IN) : COLOR
{
	const float2 u = IN.tex0 + w_h_height.xy*0.5;

	const float3 rMc = tex2Dlod(texSampler5, float4(u, 0.,0.)).xyz;
	[branch] if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = tex2Dlod(texSamplerDepth, float4(u, 0.,0.)).x;
		[branch] if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
			return float4(rMc, 1.0);
	}

	const float2 offs = w_h_height.xy;
	const float rNW = luma(tex2Dlod(texSampler5, float4(u - offs, 0.,0.)).xyz);
	const float rN = luma(tex2Dlod(texSampler5, float4(u - float2(0.0,offs.y), 0.,0.)).xyz);
	const float rNE = luma(tex2Dlod(texSampler5, float4(u - float2(-offs.x,offs.y), 0.,0.)).xyz);
	const float rW = luma(tex2Dlod(texSampler5, float4(u - float2(offs.x,0.0), 0.,0.)).xyz);
	const float rM = luma(rMc);
	const float rE = luma(tex2Dlod(texSampler5, float4(u + float2(offs.x,0.0), 0.,0.)).xyz);
	const float rSW = luma(tex2Dlod(texSampler5, float4(u + float2(-offs.x,offs.y), 0.,0.)).xyz);
	const float rS = luma(tex2Dlod(texSampler5, float4(u + float2(0.0,offs.y), 0.,0.)).xyz);
	const float rSE = luma(tex2Dlod(texSampler5, float4(u + offs, 0.,0.)).xyz);
	const float rMrN = rM+rN;
	const float lumaNW = rMrN+rNW+rW;
	const float lumaNE = rMrN+rNE+rE;
	const float rMrS = rM+rS;
	const float lumaSW = rMrS+rSW+rW;
	const float lumaSE = rMrS+rSE+rE;
	const bool g0 = (lumaSW > lumaSE);
	const float tempMax = g0 ? lumaSW : lumaSE;
	const float tempMin = g0 ? lumaSE : lumaSW;
	const bool g1 = (lumaNW > lumaNE);
	const float tempMax2 = g1 ? lumaNW : lumaNE;
	const float tempMin2 = g1 ? lumaNE : lumaNW;
	const float lumaMin = min(rM, min(tempMin, tempMin2));
	const float lumaMax = max(rM, max(tempMax, tempMax2));
	const float SWSE = lumaSW + lumaSE;
	const float NWNE = lumaNW + lumaNE;
	float2 dir = float2(SWSE - NWNE, (lumaNW + lumaSW) - (lumaNE + lumaSE));
	const float temp = 1.0/(min(abs(dir.x), abs(dir.y)) + max((NWNE + SWSE)*0.03125, 0.0078125)); //!! tweak?
	dir = min(8.0, max(-8.0, dir*temp)) * offs; //!! tweak?
	const float3 rgbA = 0.5 * (tex2Dlod(texSampler5, float4(u-dir*(0.5/3.0), 0.,0.)).xyz + tex2Dlod(texSampler5, float4(u+dir*(0.5/3.0), 0.,0.)).xyz);
	const float3 rgbB = 0.5 * rgbA + 0.25 * (tex2Dlod(texSampler5, float4(u-dir*0.5, 0.,0.)).xyz + tex2Dlod(texSampler5, float4(u+dir*0.5, 0.,0.)).xyz);
	const float lumaB = luma(rgbB);
	return float4(((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB, 1.0);
}

#define FXAA_QUALITY__P0 1.5
#define FXAA_QUALITY__P1 2.0
#define FXAA_QUALITY__P2 8.0

// Full mid-quality PC FXAA 3.11
float4 ps_main_fxaa2(in VS_OUTPUT_2D IN) : COLOR
{
	const float2 u = IN.tex0 + w_h_height.xy*0.5;

	const float3 rgbyM = tex2Dlod(texSampler5, float4(u, 0.,0.)).xyz;
	[branch] if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = tex2Dlod(texSamplerDepth, float4(u, 0.,0.)).x;
		[branch] if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
			return float4(rgbyM, 1.0);
	}

	const float2 offs = w_h_height.xy;
	const float lumaNW = luma(tex2Dlod(texSampler5, float4(u - offs, 0.f,0.f)).xyz);
	float lumaN = luma(tex2Dlod(texSampler5, float4(u - float2(0.0,offs.y), 0.f,0.f)).xyz);
	const float lumaNE = luma(tex2Dlod(texSampler5, float4(u - float2(-offs.x,offs.y), 0.f,0.f)).xyz);
	const float lumaW = luma(tex2Dlod(texSampler5, float4(u - float2(offs.x,0.0), 0.f,0.f)).xyz);
	const float lumaM = luma(rgbyM);
	const float lumaE = luma(tex2Dlod(texSampler5, float4(u + float2(offs.x,0.0), 0.f,0.f)).xyz);
	const float lumaSW = luma(tex2Dlod(texSampler5, float4(u + float2(-offs.x,offs.y), 0.f,0.f)).xyz);
	float lumaS = luma(tex2Dlod(texSampler5, float4(u + float2(0.0,offs.y), 0.f,0.f)).xyz);
	const float lumaSE = luma(tex2Dlod(texSampler5, float4(u + offs, 0.f,0.f)).xyz);
	const float maxSM = max(lumaS, lumaM);
	const float minSM = min(lumaS, lumaM);
	const float maxESM = max(lumaE, maxSM);
	const float minESM = min(lumaE, minSM);
	const float maxWN = max(lumaN, lumaW);
	const float minWN = min(lumaN, lumaW);
	const float rangeMax = max(maxWN, maxESM);
	const float rangeMin = min(minWN, minESM);
	const float rangeMaxScaled = rangeMax * 0.166; //0.333 (faster) .. 0.063 (slower)
	const float range = rangeMax - rangeMin;
	const float rangeMaxClamped = max(0.0833, rangeMaxScaled); //0.0625 (high quality/faster) .. 0.0312 (visible limit/slower)
	const bool earlyExit = range < rangeMaxClamped;
	[branch] if(earlyExit)
		return float4(rgbyM, 1.0);
	const float lumaNS = lumaN + lumaS;
	const float lumaWE = lumaW + lumaE;
	const float subpixRcpRange = 1.0/range;
	const float subpixNSWE = lumaNS + lumaWE;
	const float edgeHorz1 = -2.0 * lumaM + lumaNS;
	const float edgeVert1 = -2.0 * lumaM + lumaWE;
	const float lumaNESE = lumaNE + lumaSE;
	const float lumaNWNE = lumaNW + lumaNE;
	const float edgeHorz2 = -2.0 * lumaE + lumaNESE;
	const float edgeVert2 = -2.0 * lumaN + lumaNWNE;
	const float lumaNWSW = lumaNW + lumaSW;
	const float lumaSWSE = lumaSW + lumaSE;
	const float edgeHorz4 = abs(edgeHorz1) * 2.0 + abs(edgeHorz2);
	const float edgeVert4 = abs(edgeVert1) * 2.0 + abs(edgeVert2);
	const float edgeHorz3 = -2.0 * lumaW + lumaNWSW;
	const float edgeVert3 = -2.0 * lumaS + lumaSWSE;
	const float edgeHorz = abs(edgeHorz3) + edgeHorz4;
	const float edgeVert = abs(edgeVert3) + edgeVert4;
	const float subpixNWSWNESE = lumaNWSW + lumaNESE;
	float lengthSign = offs.x;
	const bool horzSpan = edgeHorz >= edgeVert;
	const float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
	if(!horzSpan) lumaN = lumaW;
	if(!horzSpan) lumaS = lumaE;
	if(horzSpan) lengthSign = offs.y;
	const float subpixB = subpixA * (1.0/12.0) - lumaM;
	const float gradientN = lumaN - lumaM;
	const float gradientS = lumaS - lumaM;
	float lumaNN = lumaN + lumaM;
	const float lumaSS = lumaS + lumaM;
	const bool pairN = (abs(gradientN) >= abs(gradientS));
	const float gradient = max(abs(gradientN), abs(gradientS));
	if(pairN) lengthSign = -lengthSign;
	const float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);
	const float2 offNP = float2(!horzSpan ? 0.0 : offs.x, horzSpan ? 0.0 : offs.y);
	float2 posB = u;
	const float l05 = lengthSign * 0.5;
	if(horzSpan) posB.y += l05;
	else posB.x += l05;
	float2 posN = float2(posB.x - offNP.x * FXAA_QUALITY__P0, posB.y - offNP.y * FXAA_QUALITY__P0);
	float2 posP = float2(posB.x + offNP.x * FXAA_QUALITY__P0, posB.y + offNP.y * FXAA_QUALITY__P0);
	const float subpixD = -2.0 * subpixC + 3.0;
	float lumaEndN = luma(tex2Dlod(texSampler5, float4(posN, 0.f,0.f)).xyz);
	const float subpixE = subpixC * subpixC;
	float lumaEndP = luma(tex2Dlod(texSampler5, float4(posP, 0.f,0.f)).xyz);
	if(!pairN) lumaNN = lumaSS;
	const float gradientScaled = gradient * (1.0/4.0);
	const float lumaMM = lumaM - lumaNN * 0.5;
	const float subpixF = subpixD * subpixE;
	const bool lumaMLTZero = (lumaMM < 0.0);
	lumaEndN -= lumaNN * 0.5;
	lumaEndP -= lumaNN * 0.5;
	bool doneN = (abs(lumaEndN) >= gradientScaled);
	bool doneP = (abs(lumaEndP) >= gradientScaled);
	if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P1;
	if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P1;
	bool doneNP = ((!doneN) || (!doneP));
	if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P1;
	if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P1;
	if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P2;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P2;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P2;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P2;
	}
	const float dstN = horzSpan ? (u.x - posN.x) : (u.y - posN.y);
	const float dstP = horzSpan ? (posP.x - u.x) : (posP.y - u.y);
	const bool goodSpanN = ((lumaEndN < 0.0) != lumaMLTZero);
	const float spanLength = dstP + dstN;
	const bool goodSpanP = ((lumaEndP < 0.0) != lumaMLTZero);
	const float spanLengthRcp = 1.0/spanLength;
	const bool directionN = (dstN < dstP);
	const float dst = min(dstN, dstP);
	const bool goodSpan = directionN ? goodSpanN : goodSpanP;
	const float subpixG = subpixF * subpixF;
	const float pixelOffset = 0.5 - dst * spanLengthRcp;
	const float subpixH = subpixG * 0.75; //1.00 (upper limit/softer) .. 0.50 (lower limit/sharper) .. 0.00 (completely off)
	const float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
	const float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
	float2 un = u;
	const float pl = pixelOffsetSubpix * lengthSign;
	if(horzSpan) un.y += pl;
	else un.x += pl;
	return float4(tex2Dlod(texSampler5, float4(un, 0.f,0.f)).xyz, 1.0f);
}

#undef FXAA_QUALITY__P0
#undef FXAA_QUALITY__P1
#undef FXAA_QUALITY__P2

#define FXAA_QUALITY__P0 1.0
#define FXAA_QUALITY__P1 1.0
#define FXAA_QUALITY__P2 1.0
#define FXAA_QUALITY__P3 1.0
#define FXAA_QUALITY__P4 1.0
#define FXAA_QUALITY__P5 1.5
#define FXAA_QUALITY__P6 2.0
#define FXAA_QUALITY__P7 2.0
#define FXAA_QUALITY__P8 2.0
#define FXAA_QUALITY__P9 2.0
#define FXAA_QUALITY__P10 4.0
#define FXAA_QUALITY__P11 8.0

// Full extreme-quality PC FXAA 3.11
float4 ps_main_fxaa3(in VS_OUTPUT_2D IN) : COLOR
{
	const float2 u = IN.tex0 + w_h_height.xy*0.5;

	const float3 rgbyM = tex2Dlod(texSampler5, float4(u, 0.,0.)).xyz;
	[branch] if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = tex2Dlod(texSamplerDepth, float4(u, 0.,0.)).x;
		[branch] if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
			return float4(rgbyM, 1.0);
	}

	const float2 offs = w_h_height.xy;
	const float lumaNW = luma(tex2Dlod(texSampler5, float4(u - offs, 0.f,0.f)).xyz);
	float lumaN = luma(tex2Dlod(texSampler5, float4(u - float2(0.0,offs.y), 0.f,0.f)).xyz);
	const float lumaNE = luma(tex2Dlod(texSampler5, float4(u - float2(-offs.x,offs.y), 0.f,0.f)).xyz);
	const float lumaW = luma(tex2Dlod(texSampler5, float4(u - float2(offs.x,0.0), 0.f,0.f)).xyz);
	const float lumaM = luma(rgbyM);
	const float lumaE = luma(tex2Dlod(texSampler5, float4(u + float2(offs.x,0.0), 0.f,0.f)).xyz);
	const float lumaSW = luma(tex2Dlod(texSampler5, float4(u + float2(-offs.x,offs.y), 0.f,0.f)).xyz);
	float lumaS = luma(tex2Dlod(texSampler5, float4(u + float2(0.0,offs.y), 0.f,0.f)).xyz);
	const float lumaSE = luma(tex2Dlod(texSampler5, float4(u + offs, 0.f,0.f)).xyz);
	const float maxSM = max(lumaS, lumaM);
	const float minSM = min(lumaS, lumaM);
	const float maxESM = max(lumaE, maxSM);
	const float minESM = min(lumaE, minSM);
	const float maxWN = max(lumaN, lumaW);
	const float minWN = min(lumaN, lumaW);
	const float rangeMax = max(maxWN, maxESM);
	const float rangeMin = min(minWN, minESM);
	const float rangeMaxScaled = rangeMax * 0.166; //0.333 (faster) .. 0.063 (slower)
	const float range = rangeMax - rangeMin;
	const float rangeMaxClamped = max(0.0833, rangeMaxScaled); //0.0625 (high quality/faster) .. 0.0312 (visible limit/slower)
	const bool earlyExit = range < rangeMaxClamped;
	[branch] if(earlyExit)
		return float4(rgbyM, 1.0f);
	const float lumaNS = lumaN + lumaS;
	const float lumaWE = lumaW + lumaE;
	const float subpixRcpRange = 1.0/range;
	const float subpixNSWE = lumaNS + lumaWE;
	const float edgeHorz1 = -2.0 * lumaM + lumaNS;
	const float edgeVert1 = -2.0 * lumaM + lumaWE;
	const float lumaNESE = lumaNE + lumaSE;
	const float lumaNWNE = lumaNW + lumaNE;
	const float edgeHorz2 = -2.0 * lumaE + lumaNESE;
	const float edgeVert2 = -2.0 * lumaN + lumaNWNE;
	const float lumaNWSW = lumaNW + lumaSW;
	const float lumaSWSE = lumaSW + lumaSE;
	const float edgeHorz4 = abs(edgeHorz1) * 2.0 + abs(edgeHorz2);
	const float edgeVert4 = abs(edgeVert1) * 2.0 + abs(edgeVert2);
	const float edgeHorz3 = -2.0 * lumaW + lumaNWSW;
	const float edgeVert3 = -2.0 * lumaS + lumaSWSE;
	const float edgeHorz = abs(edgeHorz3) + edgeHorz4;
	const float edgeVert = abs(edgeVert3) + edgeVert4;
	const float subpixNWSWNESE = lumaNWSW + lumaNESE;
	float lengthSign = offs.x;
	const bool horzSpan = edgeHorz >= edgeVert;
	const float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
	if(!horzSpan) lumaN = lumaW;
	if(!horzSpan) lumaS = lumaE;
	if(horzSpan) lengthSign = offs.y;
	const float subpixB = subpixA * (1.0/12.0) - lumaM;
	const float gradientN = lumaN - lumaM;
	const float gradientS = lumaS - lumaM;
	float lumaNN = lumaN + lumaM;
	const float lumaSS = lumaS + lumaM;
	const bool pairN = (abs(gradientN) >= abs(gradientS));
	const float gradient = max(abs(gradientN), abs(gradientS));
	if(pairN) lengthSign = -lengthSign;
	const float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);
	const float2 offNP = float2(!horzSpan ? 0.0 : offs.x, horzSpan ? 0.0 : offs.y);
	float2 posB = u;
	const float l05 = lengthSign * 0.5;
	if(horzSpan) posB.y += l05;
	else posB.x += l05;
	float2 posN = float2(posB.x - offNP.x * FXAA_QUALITY__P0, posB.y - offNP.y * FXAA_QUALITY__P0);
	float2 posP = float2(posB.x + offNP.x * FXAA_QUALITY__P0, posB.y + offNP.y * FXAA_QUALITY__P0);
	const float subpixD = -2.0 * subpixC + 3.0;
	float lumaEndN = luma(tex2Dlod(texSampler5, float4(posN, 0.f,0.f)).xyz);
	const float subpixE = subpixC * subpixC;
	float lumaEndP = luma(tex2Dlod(texSampler5, float4(posP, 0.f,0.f)).xyz);
	if(!pairN) lumaNN = lumaSS;
	const float gradientScaled = gradient * (1.0/4.0);
	const float lumaMM = lumaM - lumaNN * 0.5;
	const float subpixF = subpixD * subpixE;
	const bool lumaMLTZero = (lumaMM < 0.0);
	lumaEndN -= lumaNN * 0.5;
	lumaEndP -= lumaNN * 0.5;
	bool doneN = (abs(lumaEndN) >= gradientScaled);
	bool doneP = (abs(lumaEndP) >= gradientScaled);
	if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P1;
	if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P1;
	bool doneNP = ((!doneN) || (!doneP));
	if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P1;
	if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P1;
	[branch] if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P2;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P2;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P2;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P2;

		//

		if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P3;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P3;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P3;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P3;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P4;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P4;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P4;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P4;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P5;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P5;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P5;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P5;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P6;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P6;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P6;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P6;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P7;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P7;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P7;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P7;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P8;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P8;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P8;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P8;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P9;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P9;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P9;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P9;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P10;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P10;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P10;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P10;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(tex2Dlod(texSampler5, float4(posN.xy, 0.f,0.f)).xyz);
		if(!doneP) lumaEndP = luma(tex2Dlod(texSampler5, float4(posP.xy, 0.f,0.f)).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P11;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P11;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P11;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P11;
		}
		}
		}
		}
		}
		}
		}
		}
		}
	}
	const float dstN = horzSpan ? (u.x - posN.x) : (u.y - posN.y);
	const float dstP = horzSpan ? (posP.x - u.x) : (posP.y - u.y);
	const bool goodSpanN = ((lumaEndN < 0.0) != lumaMLTZero);
	const float spanLength = dstP + dstN;
	const bool goodSpanP = ((lumaEndP < 0.0) != lumaMLTZero);
	const float spanLengthRcp = 1.0/spanLength;
	const bool directionN = (dstN < dstP);
	const float dst = min(dstN, dstP);
	const bool goodSpan = directionN ? goodSpanN : goodSpanP;
	const float subpixG = subpixF * subpixF;
	const float pixelOffset = 0.5 - dst * spanLengthRcp;
	const float subpixH = subpixG * 0.75; //1.00 (upper limit/softer) .. 0.50 (lower limit/sharper) .. 0.00 (completely off)
	const float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
	const float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
	float2 un = u;
	const float pl = pixelOffsetSubpix * lengthSign;
	if(horzSpan) un.y += pl;
	else un.x += pl;
	return float4(tex2Dlod(texSampler5, float4(un, 0.f,0.f)).xyz, 1.0f);
}
