//!! add reflection direction occlusion, so that that will be used for blocking reflection/envmap?
//!! opt.?

////FRAGMENT

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

float3 get_nonunit_normal(const float depth0, const float2 u, const float eye) // use neighboring pixels // quite some tex access by this
{
	const float depth1 = texStereoNoLod(tex_depth, float2(u.x, u.y - w_h_height.y)).x;
	const float depth2 = texStereoNoLod(tex_depth, float2(u.x + w_h_height.x, u.y)).x;
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

/*float4 ps_main_normals(const in VS_OUTPUT_2D IN) : COLOR // separate pass to generate normals (should actually reduce bandwidth needed in AO pass, but overall close to no performance difference or even much worse perf, depending on gfxboard)
{
	const float2 u = IN.tex0;

	const float depth0 = texStereoNoLod(tex_depth, u).x;
	BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) //!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
		return float4(0.0, 0.,0.,0.);

	const float3 normal = normalize(get_nonunit_normal(depth0, u, eye)) *0.5+0.5;
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

/*
// Pack into hemisphere octahedron
// Assume normalized input on +Z hemisphere. Output [-1, 1].
void EncodeHemiOctaNormal( const float3 v, inout float2 encV )
{
	// Project the hemisphere onto the hemi-octahedron, and then into the xy plane
	const float rcp_denom = 1.0 / ( abs( v[0] ) + abs( v[1] ) + v[2] );
	const float tx = v[0] * rcp_denom;
	const float ty = v[1] * rcp_denom;
	encV[0] = tx + ty;
	encV[1] = tx - ty;
} 

void DecodeHemiOctaNormal( const float2 encV, inout float3 v ) 
{
	//	Rotate and scale the unit square back to the center diamond
	v[0] = ( encV[0] + encV[1] ) * 0.5;
	v[1] = ( encV[0] - encV[1] ) * 0.5;
	v[2] = 1.0 - abs( v[0] ) - abs( v[1] );
}
*/

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

////ps_main_ao

void main()
{
	FS_LAYER_TO_GLOBAL;
	const float2 u = tex0;
	const float2 uv0 = u - w_h_height.xy * 0.5 + w_h_height.xy; // half pixel shift in x & y for filter
	const float2 uv1 = u - w_h_height.xy * 0.5; // dto.

	const float depth0 = texStereoNoLod(tex_depth, u).x;
	BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) //!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
	{
		color = float4(1.0, 0.,0.,0.);
		return;
	}

	const float3 ushift = /*hash(uv1) + w_h_height.zw*/ // jitter samples via hash of position on screen and then jitter samples by time //!! see below for non-shifted variant
	                      texNoLod(tex_ao_dither, uv1/(64.0*w_h_height.xy) + w_h_height.zw).xyz; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
	//const float base = 0.0;
	const float area = 0.06; //!!
	const float falloff = 0.0002; //!!
	const int samples = 8/*9*/; //4,8,9,13,16,21,25,32 korobov,fibonacci
	const float samples_float = float(samples);
	const float radius = 0.001+/*frac*/(ushift.z)*0.009; // sample radius
	const float depth_threshold_normal = 0.005;
	const float total_strength = AO_scale_timeblur.x * (/*1.0 for uniform*/0.5 / samples_float);
	const float3 normal = normalize(get_nonunit_normal(depth0, u, eye));
	//const float3 normal = texNoLod(tex_normals, u).xyz *2.0-1.0; // use 8bitRGB pregenerated normals
	const float radius_depth = radius/depth0;

	float occlusion = 0.0;
	UNROLL for(int i=0; i < samples; ++i) {
		const float i_float = float(i);
		const float2 r = float2(i_float* (1.0 / samples_float), i_float* (5.0/*2.0*/ / samples_float)); //1,5,2,8,4,13,7,7 korobov,fibonacci //!! could also use progressive/extensible lattice via rad_inv(i)*(1501825329, 359975893) (check precision though as this should be done in double or uint64)
		//const float3 ray = sphere_sample(frac(r+ushift.xy)); // shift lattice // uniform variant
		const float2 ray = rotate_to_vector_upper(cos_hemisphere_sample(frac(r+ushift.xy)), normal).xy; // shift lattice
		//!! maybe a bit worse distribution: const float2 ray = cos_hemisphere_sample(normal,frac(r+ushift.xy)).xy; // shift lattice
		//const float rdotn = dot(ray,normal);
		const float2 hemi_ray = u + (radius_depth /** sign(rdotn) for uniform*/) * ray.xy;
		const float occ_depth = texStereoNoLod(tex_depth, hemi_ray).x;
		const float3 occ_normal = get_nonunit_normal(occ_depth, hemi_ray, eye);
		//const float3 occ_normal = texNoLod(tex_normals, hemi_ray).xyz *2.0-1.0;  // use 8bitRGB pregenerated normals, can also omit normalization below then
		const float diff_depth = depth0 - occ_depth;
		const float diff_norm = dot(occ_normal,normal);
		occlusion += step(falloff, diff_depth) * /*abs(rdotn)* for uniform*/ (diff_depth < depth_threshold_normal ? (1.0-diff_norm*diff_norm/dot(occ_normal,occ_normal)) : 1.0) * (1.0-smoothstep(falloff, area, diff_depth));
	}
	// weight with result(s) from previous frames
	const float ao = 1.0 - total_strength * occlusion;
	color= float4((texStereoNoLod(tex_fb_filtered, uv0).x //abuse bilerp for filtering (by using half texel/pixel shift)
				  +texStereoNoLod(tex_fb_filtered, uv1).x
				  +texStereoNoLod(tex_fb_filtered, float2(uv0.x,uv1.y)).x
				  +texStereoNoLod(tex_fb_filtered, float2(uv1.x,uv0.y)).x)
		*(0.25*(1.0-AO_scale_timeblur.y))+saturate(ao /*+base*/)*AO_scale_timeblur.y, 0.,0.,0.);
}

////ps_main_nfaa

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
float2 findContrastByLuminance(const float2 XYCoord, const float filterSpread, const float eye)
{
	const float2 upOffset    = float2(0.0, w_h_height.y * filterSpread);
	const float2 rightOffset = float2(w_h_height.x * filterSpread, 0.0);

	const float topHeight         = GetLuminance(texStereoNoLod(tex_fb_filtered, XYCoord +               upOffset).rgb);
	const float bottomHeight      = GetLuminance(texStereoNoLod(tex_fb_filtered, XYCoord -               upOffset).rgb);
	const float rightHeight       = GetLuminance(texStereoNoLod(tex_fb_filtered, XYCoord + rightOffset           ).rgb);
	const float leftHeight        = GetLuminance(texStereoNoLod(tex_fb_filtered, XYCoord - rightOffset           ).rgb);
	const float leftTopHeight     = GetLuminance(texStereoNoLod(tex_fb_filtered, XYCoord - rightOffset + upOffset).rgb);
	const float leftBottomHeight  = GetLuminance(texStereoNoLod(tex_fb_filtered, XYCoord - rightOffset - upOffset).rgb);
	const float rightBottomHeight = GetLuminance(texStereoNoLod(tex_fb_filtered, XYCoord + rightOffset + upOffset).rgb);
	const float rightTopHeight    = GetLuminance(texStereoNoLod(tex_fb_filtered, XYCoord + rightOffset - upOffset).rgb);

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

float2 findContrastByColor(const float2 XYCoord, const float filterSpread, const float eye)
{
	const float2 upOffset    = float2(0.0, w_h_height.y * filterSpread);
	const float2 rightOffset = float2(w_h_height.x * filterSpread, 0.0);

	const float3 topHeight         = texStereoNoLod(tex_fb_filtered, XYCoord +               upOffset).rgb;
	const float3 bottomHeight      = texStereoNoLod(tex_fb_filtered, XYCoord -               upOffset).rgb;
	const float3 rightHeight       = texStereoNoLod(tex_fb_filtered, XYCoord + rightOffset           ).rgb;
	const float3 leftHeight        = texStereoNoLod(tex_fb_filtered, XYCoord - rightOffset           ).rgb;
	const float3 leftTopHeight     = texStereoNoLod(tex_fb_filtered, XYCoord - rightOffset + upOffset).rgb;
	const float3 leftBottomHeight  = texStereoNoLod(tex_fb_filtered, XYCoord - rightOffset - upOffset).rgb;
	const float3 rightBottomHeight = texStereoNoLod(tex_fb_filtered, XYCoord + rightOffset + upOffset).rgb;
	const float3 rightTopHeight    = texStereoNoLod(tex_fb_filtered, XYCoord + rightOffset - upOffset).rgb;

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

void main()
{
	FS_LAYER_TO_GLOBAL;
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

	const float2 u = tex0;

	const float3 Scene0 = texStereoNoLod(tex_fb_filtered, u).rgb;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			color = float4(Scene0, 1.0);
			return;
		}
	}

#ifdef NFAA_USE_COLOR // edges from color
	float2 Vectors = findContrastByColor(u, filterSpread, eye);
#else
	float2 Vectors = findContrastByLuminance(u, filterSpread, eye);
#endif

#ifndef NFAA_VARIANT2
	const float filterStrength2 = filterStrength + filterSpread*0.5;
	const float filterClamp = filterStrength2 / filterSpread;

	Vectors = clamp(Vectors * filterStrength2, -float2(filterClamp, filterClamp), float2(filterClamp, filterClamp));
#else
	Vectors *= filterSpread;
#endif

	const float2 Normal = Vectors * (w_h_height.xy /* * 2.0*/);

	const float3 Scene1 = texStereoNoLod(tex_fb_filtered, u + Normal).rgb;
	const float3 Scene2 = texStereoNoLod(tex_fb_filtered, u - Normal).rgb;
#if defined(NFAA_VARIANT) || defined(NFAA_VARIANT2)
	const float3 Scene3 = texStereoNoLod(tex_fb_filtered, u + float2(Normal.x, -Normal.y)*0.5).rgb;
	const float3 Scene4 = texStereoNoLod(tex_fb_filtered, u - float2(Normal.x, -Normal.y)*0.5).rgb;
#else
	const float3 Scene3 = texStereoNoLod(tex_fb_filtered, u + float2(Normal.x, -Normal.y)).rgb;
	const float3 Scene4 = texStereoNoLod(tex_fb_filtered, u - float2(Normal.x, -Normal.y)).rgb;
#endif

#ifdef NFAA_TEST_MODE // debug
	const float3 o_Color = normalize(float3(Vectors * 0.5 + 0.5, 1.0));
#else
	const float3 o_Color = (Scene0 + Scene1 + Scene2 + Scene3 + Scene4) * 0.2;
#endif

	color = float4(o_Color, 1.0);
}

// DLAA approximation

////FRAGMENT

float3 sampleOffset(const float2 u, const float2 pixelOffset, const float eye)
{
   return texStereoNoLod(tex_fb_filtered, u + pixelOffset * w_h_height.xy).xyz;
}

float4 sampleOffseta(const float2 u, const float2 pixelOffset, const float eye)
{
   return texStereoNoLod(tex_fb_filtered, u + pixelOffset * w_h_height.xy);
}

float avg(const float3 l)
{
   //return dot(l, float3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
   return (l.x+l.y+l.z) * (1.0 / 3.0);
}

////ps_main_dlaa_edge

void main()
{
   FS_LAYER_TO_GLOBAL;
   const float2 u = tex0;

   const float3 sCenter    = sampleOffset(u, float2( 0.0,  0.0), eye);
   const float3 sUpLeft    = sampleOffset(u, float2(-0.5, -0.5), eye);
   const float3 sUpRight   = sampleOffset(u, float2( 0.5, -0.5), eye);
   const float3 sDownLeft  = sampleOffset(u, float2(-0.5,  0.5), eye);
   const float3 sDownRight = sampleOffset(u, float2( 0.5,  0.5), eye);

   const float3 diff       = abs( (sUpLeft + sUpRight) + (sDownLeft + sDownRight) - sCenter * 4.0 );
   const float  edgeMask   = avg(diff) * 4.0; //!! magic

   color = /*test: float4(edgeMask,edgeMask,edgeMask,1.0);*/ float4(sCenter, edgeMask);
}

////ps_main_dlaa

void main()
{
   FS_LAYER_TO_GLOBAL;
   const float2 u = tex0;

   const float4 sampleCenter = sampleOffseta(u, float2(0.0, 0.0), eye);
   BRANCH if(w_h_height.w == 1.0 /*&& sampleCenter.a == 0.0*/) // depth buffer available? /*AND no edge here? -> ignored because of performance*/
   {
      const float depth0 = texStereoNoLod(tex_depth, u).x;
      BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
      {
         color = float4(sampleCenter.xyz, 1.0);
         return;
      }
   }

   // short edges
   const float4 sampleHorizNeg0  = sampleOffseta(u, float2(-1.5,  0.0), eye);
   const float4 sampleHorizPos0  = sampleOffseta(u, float2( 1.5,  0.0), eye);
   const float4 sampleVertNeg0   = sampleOffseta(u, float2( 0.0, -1.5), eye);
   const float4 sampleVertPos0   = sampleOffseta(u, float2( 0.0,  1.5), eye);

   const float3 sumHoriz         = sampleHorizNeg0.xyz + sampleHorizPos0.xyz;
   const float3 sumVert          = sampleVertNeg0.xyz  + sampleVertPos0.xyz;

   const float3 sampleLeft       = sampleOffset(u, float2(-1.0,  0.0), eye);
   const float3 sampleRight      = sampleOffset(u, float2( 1.0,  0.0), eye);
   const float3 sampleTop        = sampleOffset(u, float2( 0.0, -1.0), eye);
   const float3 sampleDown       = sampleOffset(u, float2( 0.0,  1.0), eye);

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
   const float4 sampleVertNeg1   = sampleOffseta(u, float2(0.0, -3.5), eye);
   const float4 sampleVertNeg15  = sampleOffseta(u, float2(0.0, -5.5), eye);
   const float4 sampleVertNeg2   = sampleOffseta(u, float2(0.0, -7.5), eye);
   const float4 sampleVertPos1   = sampleOffseta(u, float2(0.0,  3.5), eye);
   const float4 sampleVertPos15  = sampleOffseta(u, float2(0.0,  5.5), eye);
   const float4 sampleVertPos2   = sampleOffseta(u, float2(0.0,  7.5), eye);

   const float4 sampleHorizNeg1  = sampleOffseta(u, float2(-3.5, 0.0), eye);
   const float4 sampleHorizNeg15 = sampleOffseta(u, float2(-5.5, 0.0), eye);
   const float4 sampleHorizNeg2  = sampleOffseta(u, float2(-7.5, 0.0), eye);
   const float4 sampleHorizPos1  = sampleOffseta(u, float2( 3.5, 0.0), eye);
   const float4 sampleHorizPos15 = sampleOffseta(u, float2( 5.5, 0.0), eye);
   const float4 sampleHorizPos2  = sampleOffseta(u, float2( 7.5, 0.0), eye);

   const float pass1EdgeAvgHoriz = saturate(( sampleHorizNeg2.a + sampleHorizNeg1.a + sampleHorizNeg15.a + sampleHorizNeg0.a + sampleHorizPos0.a + sampleHorizPos1.a + sampleHorizPos15.a + sampleHorizPos2.a ) * (2.0 / 8.0) - 1.0);
   const float pass1EdgeAvgVert  = saturate(( sampleVertNeg2.a  + sampleVertNeg1.a + sampleVertNeg15.a  + sampleVertNeg0.a  + sampleVertPos0.a + sampleVertPos1.a + sampleVertPos15.a  + sampleVertPos2.a  ) * (2.0 / 8.0) - 1.0);

   BRANCH if(abs(pass1EdgeAvgHoriz - pass1EdgeAvgVert) > 0.2) //!! magic
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

   color = /*test: float4(sampleCenter.a,sampleCenter.a,sampleCenter.a,1.0);*/ float4(aaResult, 1.0);
}

////FRAGMENT

// FXAA

float luma(const float3 l)
{
    return dot(l, float3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
    //return 0.299*l.x + 0.587*l.y + 0.114*l.z;
    //return 0.2126*l.x + 0.7152*l.y + 0.0722*l.z; // photometric
    //return sqrt(0.299 * l.x*l.x + 0.587 * l.y*l.y + 0.114 * l.z*l.z); // hsp
}

////ps_main_fxaa1

// Approximation of FXAA

void main()
{
	FS_LAYER_TO_GLOBAL;
	const float2 u = tex0;

	const float3 rMc = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			color = float4(rMc, 1.0);
			return;
		}
	}

	const float2 offs = w_h_height.xy;
	const float rNW = luma(texStereoNoLod(tex_fb_unfiltered, u - offs).xyz);
	const float rN = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(0.0,offs.y)).xyz);
	const float rNE = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(-offs.x,offs.y)).xyz);
	const float rW = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(offs.x,0.0)).xyz);
	const float rM = luma(rMc);
	const float rE = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(offs.x,0.0)).xyz);
	const float rSW = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(-offs.x,offs.y)).xyz);
	const float rS = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(0.0,offs.y)).xyz);
	const float rSE = luma(texStereoNoLod(tex_fb_unfiltered, u + offs).xyz);
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
	dir = clamp(dir*temp, float2(-8.0), float2(8.0)) * offs; //!! tweak?
	const float3 rgbA = 0.5 * (texStereoNoLod(tex_fb_filtered, u-dir*(0.5/3.0)).xyz + texStereoNoLod(tex_fb_filtered, u+dir*(0.5/3.0)).xyz);
	const float3 rgbB = 0.5 * rgbA + 0.25 * (texStereoNoLod(tex_fb_filtered, u-dir*0.5).xyz + texStereoNoLod(tex_fb_filtered, u+dir*0.5).xyz);
	const float lumaB = luma(rgbB);
	color = float4(((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB, 1.0);
}

////ps_main_fxaa2

#define FXAA_QUALITY_P0 1.5
#define FXAA_QUALITY_P1 2.0
#define FXAA_QUALITY_P2 8.0

// Full mid-quality PC FXAA 3.11
void main()
{
	FS_LAYER_TO_GLOBAL;
	const float2 u = tex0;

	const float3 rgbyM = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			color = float4(rgbyM, 1.0);
			return;
		}
	}

	const float2 offs = w_h_height.xy;
	const float lumaNW = luma(texStereoNoLod(tex_fb_unfiltered, u - offs).xyz);
	float lumaN = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(0.0,offs.y)).xyz);
	const float lumaNE = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(-offs.x,offs.y)).xyz);
	const float lumaW = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(offs.x,0.0)).xyz);
	const float lumaM = luma(rgbyM);
	const float lumaE = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(offs.x,0.0)).xyz);
	const float lumaSW = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(-offs.x,offs.y)).xyz);
	float lumaS = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(0.0,offs.y)).xyz);
	const float lumaSE = luma(texStereoNoLod(tex_fb_unfiltered, u + offs).xyz);
	const float maxSM = max(lumaS, lumaM);
	const float minSM = min(lumaS, lumaM);
	const float maxESM = max(lumaE, maxSM);
	const float minESM = min(lumaE, minSM);
	const float maxWN = max(lumaN, lumaW);
	const float minWN = min(lumaN, lumaW);
	const float rangeMax = max(maxWN, maxESM);
	const float rangeMin = min(minWN, minESM);
	const float rangeMaxScaled = rangeMax * 0.125; //0.333 (faster) .. 0.063 (slower) // reshade: 0.125, fxaa : 0.166
	const float range = rangeMax - rangeMin;
	const float rangeMaxClamped = max(0.0833, rangeMaxScaled); //0.0625 (high quality/faster) .. 0.0312 (visible limit/slower) // reshade: 0.0, fxaa : 0.0833
	const bool earlyExit = range < rangeMaxClamped;
	BRANCH if(earlyExit)
	{
		color = float4(rgbyM, 1.0);
		return;
	}
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
	float2 posN = float2(posB.x - offNP.x * FXAA_QUALITY_P0, posB.y - offNP.y * FXAA_QUALITY_P0);
	float2 posP = float2(posB.x + offNP.x * FXAA_QUALITY_P0, posB.y + offNP.y * FXAA_QUALITY_P0);
	const float subpixD = -2.0 * subpixC + 3.0;
	float lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
	const float subpixE = subpixC * subpixC;
	float lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
	if(!pairN) lumaNN = lumaSS;
	const float gradientScaled = gradient * (1.0/4.0);
	const float lumaMM = lumaM - lumaNN * 0.5;
	const float subpixF = subpixD * subpixE;
	const bool lumaMLTZero = (lumaMM < 0.0);
	lumaEndN -= lumaNN * 0.5;
	lumaEndP -= lumaNN * 0.5;
	bool doneN = (abs(lumaEndN) >= gradientScaled);
	bool doneP = (abs(lumaEndP) >= gradientScaled);
	if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P1;
	if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P1;
	bool doneNP = ((!doneN) || (!doneP));
	if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P1;
	if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P1;
	if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P2;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P2;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P2;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P2;
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
	const float subpixH = subpixG * 0.5; //1.00 (upper limit/softer) .. 0.50 (lower limit/sharper) .. 0.00 (completely off) // reshade : 0.25, fxaa : 0.75
	const float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
	const float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
	float2 un = u;
	const float pl = pixelOffsetSubpix * lengthSign;
	if(horzSpan) un.y += pl;
	else un.x += pl;
	color = float4(texStereoNoLod(tex_fb_filtered, un).xyz, 1.0);
}

#undef FXAA_QUALITY_P0
#undef FXAA_QUALITY_P1
#undef FXAA_QUALITY_P2

////ps_main_fxaa3

#define FXAA_QUALITY_P0 1.0
#define FXAA_QUALITY_P1 1.0
#define FXAA_QUALITY_P2 1.0
#define FXAA_QUALITY_P3 1.0
#define FXAA_QUALITY_P4 1.0
#define FXAA_QUALITY_P5 1.5
#define FXAA_QUALITY_P6 2.0
#define FXAA_QUALITY_P7 2.0
#define FXAA_QUALITY_P8 2.0
#define FXAA_QUALITY_P9 2.0
#define FXAA_QUALITY_P10 4.0
#define FXAA_QUALITY_P11 8.0

// Full extreme-quality PC FXAA 3.11
void main()
{
	FS_LAYER_TO_GLOBAL;
	const float2 u = tex0;

	const float3 rgbyM = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			color = float4(rgbyM, 1.0);
			return;
		}
	}

	const float2 offs = w_h_height.xy;
	const float lumaNW = luma(texStereoNoLod(tex_fb_unfiltered, u - offs).xyz);
	float lumaN = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(0.0,offs.y)).xyz);
	const float lumaNE = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(-offs.x,offs.y)).xyz);
	const float lumaW = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(offs.x,0.0)).xyz);
	const float lumaM = luma(rgbyM);
	const float lumaE = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(offs.x,0.0)).xyz);
	const float lumaSW = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(-offs.x,offs.y)).xyz);
	float lumaS = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(0.0,offs.y)).xyz);
	const float lumaSE = luma(texStereoNoLod(tex_fb_unfiltered, u + offs).xyz);
	const float maxSM = max(lumaS, lumaM);
	const float minSM = min(lumaS, lumaM);
	const float maxESM = max(lumaE, maxSM);
	const float minESM = min(lumaE, minSM);
	const float maxWN = max(lumaN, lumaW);
	const float minWN = min(lumaN, lumaW);
	const float rangeMax = max(maxWN, maxESM);
	const float rangeMin = min(minWN, minESM);
	const float rangeMaxScaled = rangeMax * 0.125; //0.333 (faster) .. 0.063 (slower) // reshade: 0.125, fxaa : 0.166
	const float range = rangeMax - rangeMin;
	const float rangeMaxClamped = max(0.0833, rangeMaxScaled); //0.0625 (high quality/faster) .. 0.0312 (visible limit/slower) // reshade: 0.0, fxaa : 0.0833
	const bool earlyExit = range < rangeMaxClamped;
	BRANCH if(earlyExit)
	{
		color = float4(rgbyM, 1.0);
		return;
	}
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
	float2 posN = float2(posB.x - offNP.x * FXAA_QUALITY_P0, posB.y - offNP.y * FXAA_QUALITY_P0);
	float2 posP = float2(posB.x + offNP.x * FXAA_QUALITY_P0, posB.y + offNP.y * FXAA_QUALITY_P0);
	const float subpixD = -2.0 * subpixC + 3.0;
	float lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
	const float subpixE = subpixC * subpixC;
	float lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
	if(!pairN) lumaNN = lumaSS;
	const float gradientScaled = gradient * (1.0/4.0);
	const float lumaMM = lumaM - lumaNN * 0.5;
	const float subpixF = subpixD * subpixE;
	const bool lumaMLTZero = (lumaMM < 0.0);
	lumaEndN -= lumaNN * 0.5;
	lumaEndP -= lumaNN * 0.5;
	bool doneN = (abs(lumaEndN) >= gradientScaled);
	bool doneP = (abs(lumaEndP) >= gradientScaled);
	if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P1;
	if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P1;
	bool doneNP = ((!doneN) || (!doneP));
	if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P1;
	if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P1;
	BRANCH if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P2;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P2;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P2;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P2;

		//

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P3;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P3;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P3;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P3;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P4;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P4;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P4;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P4;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P5;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P5;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P5;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P5;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P6;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P6;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P6;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P6;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P7;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P7;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P7;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P7;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P8;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P8;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P8;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P8;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P9;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P9;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P9;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P9;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P10;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P10;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P10;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P10;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P11;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P11;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P11;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P11;
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
	const float subpixH = subpixG * 0.5; //1.00 (upper limit/softer) .. 0.50 (lower limit/sharper) .. 0.00 (completely off) // reshade : 0.25, fxaa : 0.75
	const float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
	const float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
	float2 un = u;
	const float pl = pixelOffsetSubpix * lengthSign;
	if(horzSpan) un.y += pl;
	else un.x += pl;
	color = float4(texStereoNoLod(tex_fb_filtered, un).xyz, 1.0);
}

////ps_main_CAS

void main()
{
	FS_LAYER_TO_GLOBAL;
	// variant with better diagonals

	const float Contrast   = 0.0; // 0..1, Adjusts the range the shader adapts to high contrast (0 is not all the way off).  Higher values = more high contrast sharpening.
	const float Sharpening = 1.0; // 0..1, Adjusts sharpening intensity by averaging the original pixels to the sharpened result.  1.0 is the unmodified default.

	const float2 u = tex0;

	const float3 e = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			color = float4(e, 1.0);
			return;
		}
	}

	// fetch a 3x3 neighborhood around the pixel 'e',
	//  a b c
	//  d(e)f
	//  g h i
	const float2 um1 = u - w_h_height.xy;
	const float2 up1 = u + w_h_height.xy;

	const float3 a = texStereoNoLod(tex_fb_unfiltered,        um1          ).xyz;
	const float3 b = texStereoNoLod(tex_fb_unfiltered, float2(u.x,   um1.y)).xyz;
	const float3 c = texStereoNoLod(tex_fb_unfiltered, float2(up1.x, um1.y)).xyz;
	const float3 d = texStereoNoLod(tex_fb_unfiltered, float2(um1.x, u.y  )).xyz;
	const float3 g = texStereoNoLod(tex_fb_unfiltered, float2(um1.x, up1.y)).xyz; 
	const float3 f = texStereoNoLod(tex_fb_unfiltered, float2(up1.x, u.y  )).xyz;
	const float3 h = texStereoNoLod(tex_fb_unfiltered, float2(u.x,   up1.y)).xyz;
	const float3 i = texStereoNoLod(tex_fb_unfiltered,        up1          ).xyz;

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

	const float3 rcpWeightRGB = float3(1.0) / (4.0 * wRGB + float3(1.0));

	//                          0 w 0
	//  Filter shape:           w 1 w
	//                          0 w 0  
	const float3 window = (b + d) + (f + h);
	const float3 outColor = saturate((window * wRGB + e) * rcpWeightRGB);

	color = float4(lerp(e, outColor, Sharpening), 1.);
}


////ps_main_BilateralSharp_CAS

float normpdf(const float3 v, const float sigma)
{
	return exp(dot(v,v)*(-0.5/(sigma*sigma)))*(0.39894228040143/sigma);
}

float LI(const float3 l)
{
	return dot(l, float3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
	//return dot(l, float3(0.2126, 0.7152, 0.0722));
}

void main()
{
	FS_LAYER_TO_GLOBAL;
	const float sharpness = 0.625*3.1; // ~0.1..~2 *3.1
	const float balance = 0.0;         // [0..1]

	const float2 u = tex0;

	const float3 mid = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			color = float4(mid, 1.0);
			return;
		}
	}

	const float3 e[9] = float3[](
		texStereoNoLod(tex_fb_unfiltered, float2(u.x -w_h_height.x, u.y -w_h_height.y)).xyz,
		texStereoNoLod(tex_fb_unfiltered, float2(u.x              , u.y -w_h_height.y)).xyz,
		texStereoNoLod(tex_fb_unfiltered, float2(u.x +w_h_height.x, u.y -w_h_height.y)).xyz,

		texStereoNoLod(tex_fb_unfiltered, float2(u.x -w_h_height.x, u.y)).xyz,
		mid,
		texStereoNoLod(tex_fb_unfiltered, float2(u.x +w_h_height.x, u.y)).xyz,

		texStereoNoLod(tex_fb_unfiltered, float2(u.x -w_h_height.x, u.y +w_h_height.y)).xyz,
		texStereoNoLod(tex_fb_unfiltered, float2(u.x              , u.y +w_h_height.y)).xyz,
		texStereoNoLod(tex_fb_unfiltered, float2(u.x +w_h_height.x, u.y +w_h_height.y)).xyz);

	// Bilateral Blur (crippled)
	float3 final_colour = float3(0.,0.,0.);
	float Z = 0.0;
	UNROLL for (int j=-2; j <= 2; ++j) // 2 = kernelradius
	{
		int y = j < 0 ? -3 : 0;
		y = j > 0 ? 3 : y;

		UNROLL for (int i=-2; i <= 2; ++i)
		{
			int x = i < 0 ? -1 : 0;
			x = i > 0 ? 1 : x;

			float3 cc;
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

	float3 sharpen = (e[4]-final_colour/Z) * sharpness;

	const float gs_sharpen = (sharpen.x+sharpen.y+sharpen.z) * 0.333333333333;
	sharpen = lerp(float3(gs_sharpen), sharpen, 0.5);

	ampRGB *= saturate(sharpness);
	ampRGB  = lerp(ampRGB, 1.0-ampRGB, balance);

	color = float4(e[4] + sharpen*ampRGB, 1.0);
}
