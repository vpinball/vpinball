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

float3 get_nonunit_normal(const float depth0, const float2 u) // use neighboring pixels // quite some tex access by this
{
	const float depth1 = textureLod(tex_depth, float2(u.x, u.y+w_h_height.y), 0).x;
	const float depth2 = textureLod(tex_depth, float2(u.x+w_h_height.x, u.y), 0).x;
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

in float2 tex0;

void main()
{
	const float2 u = tex0 + w_h_height.xy*0.5;
	
	const float2 uv0 = tex0 + w_h_height.xy; // half pixel shift in x & y for filter
	const float2 uv1 = tex0;                 // dto.

	const float depth0 = textureLod(tex_depth, u, 0).x;
	if((depth0 == 1.0) || (depth0 == 0.0)) //!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
	{
		color = float4(1.0, 0.,0.,0.);
		return;
	}

	const float3 ushift = /*hash(tex0) + w_h_height.zw*/ // jitter samples via hash of position on screen and then jitter samples by time //!! see below for non-shifted variant
						  textureLod(tex_ao_dither, tex0/(64.0*w_h_height.xy) + w_h_height.zw, 0).xyz; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
	//const float base = 0.0;
	const float area = 0.06; //!!
	const float falloff = 0.0002; //!!
	const int samples = 8/*9*/; //4,8,9,13,16,21,25,32 korobov,fibonacci
	const float radius = 0.001+/*frac*/(ushift.z)*0.009; // sample radius
	const float depth_threshold_normal = 0.005;
	const float total_strength = AO_scale_timeblur.x * (/*1.0 for uniform*/0.5 / samples);
	const float3 normal = normalize(get_nonunit_normal(depth0, u));
	//const float3 normal = tex2Dlod(tex_normals, float4(u, 0.,0.)).xyz *2.0-1.0; // use 8bitRGB pregenerated normals
	const float radius_depth = radius/depth0;

	float occlusion = 0.0;
	for(int i=0; i < samples; ++i) {
		const float2 r = float2(i*(1.0 / samples), i*(5.0/*2.0*/ / samples)); //1,5,2,8,4,13,7,7 korobov,fibonacci //!! could also use progressive/extensible lattice via rad_inv(i)*(1501825329, 359975893) (check precision though as this should be done in double or uint64)
		//const float3 ray = sphere_sample(frac(r+ushift.xy)); // shift lattice // uniform variant
		const float2 ray = rotate_to_vector_upper(cos_hemisphere_sample(frac(r+ushift.xy)), normal).xy; // shift lattice
		//!! maybe a bit worse distribution: const float2 ray = cos_hemisphere_sample(normal,frac(r+ushift.xy)).xy; // shift lattice
		//const float rdotn = dot(ray,normal);
		const float2 hemi_ray = u + (radius_depth /** sign(rdotn) for uniform*/) * ray.xy;
		const float occ_depth = textureLod(tex_depth, hemi_ray, 0).x;
		const float3 occ_normal = get_nonunit_normal(occ_depth, hemi_ray);
		//const float3 occ_normal = tex2Dlod(tex_normals, float4(hemi_ray, 0.,0.)).xyz *2.0-1.0;  // use 8bitRGB pregenerated normals, can also omit normalization below then
		const float diff_depth = depth0 - occ_depth;
		const float diff_norm = dot(occ_normal,normal);
		occlusion += step(falloff, diff_depth) * /*abs(rdotn)* for uniform*/ (diff_depth < depth_threshold_normal ? (1.0-diff_norm*diff_norm/dot(occ_normal,occ_normal)) : 1.0) * (1.0-smoothstep(falloff, area, diff_depth));
	}
	// weight with result(s) from previous frames
	const float ao = 1.0 - total_strength * occlusion;
	color = float4( (textureLod(tex_fb_filtered, uv0, 0).x //abuse bilerp for filtering (by using half texel/pixel shift)
			  +textureLod(tex_fb_filtered, uv1, 0).x
			  +textureLod(tex_fb_filtered, float2(uv0.x, uv1.y), 0).x
			  +textureLod(tex_fb_filtered, float2(uv1.x, uv0.y), 0).x)
		*(0.25*(1.0-AO_scale_timeblur.y))+saturate(ao /*+base*/)*AO_scale_timeblur.y, 0.,0.,0.);
}


// stereo

//!! opt.?
////ps_main_stereo

in float2 tex0;

void main()
{
	float2 u = tex0 + w_h_height.xy*0.5;
	float MaxSeparation = ms_zpd_ya_td.x;
	float ZPD = ms_zpd_ya_td.y;
	bool yaxis = (ms_zpd_ya_td.z != 0.0); //!! uniform
	bool topdown = (ms_zpd_ya_td.w == 1.0); //!! uniform
	bool sidebyside = (ms_zpd_ya_td.w == 2.0); //!! uniform
	int y = w_h_height.z*u.y;
	bool l = sidebyside ? (u.x < 0.5) : topdown ? (u.y < 0.5) : ((y+1)/2 == y/2); //last check actually means (y&1) //!! %2 //!! float diff = frac(dot(tex,(screen_size / 2.0))+0.25); if(diff < 0.5)... //returns 0.25 and 0.75
	if(topdown) { u.y *= 2.0; if(!l) u.y -= 1.0; }  //!! !topdown: (u.y+w_h_height.y) ?
	else if(sidebyside) { u.x *= 2.0; if(!l) u.x -= 1.0; }
	float su = l ? MaxSeparation : -MaxSeparation;
	float minDepth = min(min(textureLod(tex_depth, u + (yaxis ? float2(0.0,0.5*su) : float2(0.5*su,0.0)), 0).x, textureLod(tex_depth, u + (yaxis ? float2(0.0,0.666*su) : float2(0.666*su,0.0)), 0).x), textureLod(tex_depth, u + (yaxis ? float2(0.0,su) : float2(su,0.0)), 0).x);
	float parallax = (w_h_height.w+MaxSeparation) - min(MaxSeparation/(0.5+minDepth*(1.0/ZPD-0.5)), (w_h_height.w+MaxSeparation));
	if(!l)
		parallax = -parallax;
	if(yaxis)
		parallax = -parallax;
	float3 col = textureLod(tex_fb_filtered, u + (yaxis ? float2(0.0,parallax) : float2(parallax,0.0)), 0).xyz;
	//if(!aa)
	//	return float4(col, 1.0); // otherwise blend with 'missing' scanline
	float2 aaoffs = sidebyside ? float2(w_h_height.x,0.0) : float2(0.0,w_h_height.y);
	minDepth = min(min(textureLod(tex_depth, u + (yaxis ? float2(0.0,0.5*su) : float2(0.5*su,0.0)) + aaoffs, 0).x, textureLod(tex_depth, u + (yaxis ? float2(0.0,0.666*su) : float2(0.666*su,0.0)) + aaoffs, 0).x), textureLod(tex_depth, u + (yaxis ? float2(0.0,su) : float2(su,0.0)) + aaoffs, 0).x);
	parallax = (w_h_height.w+MaxSeparation) - min(MaxSeparation/(0.5+minDepth*(1.0/ZPD-0.5)), (w_h_height.w+MaxSeparation));
	if(!l)
		parallax = -parallax;
	if(yaxis)
		parallax = -parallax;
	color = float4((col + textureLod(tex_fb_filtered, u + (yaxis ? float2(0.0,parallax) : float2(parallax,0.0)) + aaoffs, 0).xyz)*0.5, 1.0);
}

////ps_main_nfaa

// NFAA

//#define NFAA_EDGE_DETECTION_VARIANT // different edge detection (sums for finite differences differ)
//#define NFAA_USE_COLOR // use color instead of luminance
//#define NFAA_TEST_MODE // debug output
// undef both of the following for variant 0
#define NFAA_VARIANT // variant 1
//#define NFAA_VARIANT2 // variant 2

float GetLuminance(float3 l)
{
	return dot(l, float3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
	//return 0.299*l.x + 0.587*l.y + 0.114*l.z;
	//return 0.2126*l.x + 0.7152*l.y + 0.0722*l.z; // photometric
	//return sqrt(0.299 * l.x*l.x + 0.587 * l.y*l.y + 0.114 * l.z*l.z); // hsp
}

#ifndef NFAA_USE_COLOR
float2 findContrastByLuminance(float2 XYCoord, float filterSpread)
{
	float2 upOffset    = float2(0.0, w_h_height.y * filterSpread);
	float2 rightOffset = float2(w_h_height.x * filterSpread, 0.0);

	float topHeight         = GetLuminance(textureLod(tex_fb_filtered, XYCoord +               upOffset, 0).rgb);
	float bottomHeight      = GetLuminance(textureLod(tex_fb_filtered, XYCoord -               upOffset, 0).rgb);
	float rightHeight       = GetLuminance(textureLod(tex_fb_filtered, XYCoord + rightOffset           , 0).rgb);
	float leftHeight        = GetLuminance(textureLod(tex_fb_filtered, XYCoord - rightOffset           , 0).rgb);
	float leftTopHeight     = GetLuminance(textureLod(tex_fb_filtered, XYCoord - rightOffset + upOffset, 0).rgb);
	float leftBottomHeight  = GetLuminance(textureLod(tex_fb_filtered, XYCoord - rightOffset - upOffset, 0).rgb);
	float rightBottomHeight = GetLuminance(textureLod(tex_fb_filtered, XYCoord + rightOffset + upOffset, 0).rgb);
	float rightTopHeight    = GetLuminance(textureLod(tex_fb_filtered, XYCoord + rightOffset - upOffset, 0).rgb);

#ifdef NFAA_EDGE_DETECTION_VARIANT
	float sum0 = rightTopHeight    + bottomHeight + leftTopHeight;
	float sum1 = leftBottomHeight  + topHeight    + rightBottomHeight;
	float sum2 = leftTopHeight     + rightHeight  + leftBottomHeight;
	float sum3 = rightBottomHeight + leftHeight   + rightTopHeight;
#else
	float sum0 = rightTopHeight + topHeight + rightBottomHeight;
	float sum1 = leftTopHeight + bottomHeight + leftBottomHeight;
	float sum2 = leftTopHeight + leftHeight + rightTopHeight;
	float sum3 = leftBottomHeight + rightHeight + rightBottomHeight;
#endif

	// finite differences for final vectors
	return float2( sum1 - sum0, sum2 - sum3 );
}

#else

float2 findContrastByColor(float2 XYCoord, float filterSpread)
{
	float2 upOffset    = float2(0.0, w_h_height.y * filterSpread);
	float2 rightOffset = float2(w_h_height.x * filterSpread, 0.0);

	float3 topHeight         = textureLod(tex_fb_filtered, XYCoord +               upOffset, 0).rgb;
	float3 bottomHeight      = textureLod(tex_fb_filtered, XYCoord -               upOffset, 0).rgb;
	float3 rightHeight       = textureLod(tex_fb_filtered, XYCoord + rightOffset           , 0).rgb;
	float3 leftHeight        = textureLod(tex_fb_filtered, XYCoord - rightOffset           , 0).rgb;
	float3 leftTopHeight     = textureLod(tex_fb_filtered, XYCoord - rightOffset + upOffset, 0).rgb;
	float3 leftBottomHeight  = textureLod(tex_fb_filtered, XYCoord - rightOffset - upOffset, 0).rgb;
	float3 rightBottomHeight = textureLod(tex_fb_filtered, XYCoord + rightOffset + upOffset, 0).rgb;
	float3 rightTopHeight    = textureLod(tex_fb_filtered, XYCoord + rightOffset - upOffset, 0).rgb;

#ifdef NFAA_EDGE_DETECTION_VARIANT
	float sum0 = rightTopHeight    + bottomHeight + leftTopHeight;
	float sum1 = leftBottomHeight  + topHeight    + rightBottomHeight;
	float sum2 = leftTopHeight     + rightHeight  + leftBottomHeight;
	float sum3 = rightBottomHeight + leftHeight   + rightTopHeight;
#else
	float sum0 = rightTopHeight + topHeight + rightBottomHeight;
	float sum1 = leftTopHeight + bottomHeight + leftBottomHeight;
	float sum2 = leftTopHeight + leftHeight + rightTopHeight;
	float sum3 = leftBottomHeight + rightHeight + rightBottomHeight;
#endif

	// finite differences for final vectors
	return float2( length(sum1 - sum0), length(sum2 - sum3) );
}
#endif

in float2 tex0;

void main()
{
#ifndef NFAA_VARIANT2
 #ifdef NFAA_VARIANT
	float filterStrength = 1.0;
 #else
	float filterStrength = 0.5;
 #endif
	float filterSpread = 4.0; //!! or original 3? or larger 5?
#else
	float filterSpread = 1.0;
#endif

	float2 u = tex0 + w_h_height.xy*0.5;

	float3 Scene0 = textureLod(tex_fb_filtered, u, 0).rgb;
	float depth0 = textureLod(tex_depth, u, 0).x;
	if ((w_h_height.w == 1.0) && ((depth0 == 1.0) || (depth0 == 0.0))) // early out if depth too large (=BG) or too small (=DMD,etc)
			color = float4(Scene0, 1.0);
	else {
#ifdef NFAA_USE_COLOR // edges from color
		float2 Vectors = findContrastByColor(u, filterSpread);
#else
		float2 Vectors = findContrastByLuminance(u, filterSpread);
#endif

#ifndef NFAA_VARIANT2
		float filterStrength2 = filterStrength + filterSpread*0.5;
		float filterClamp = filterStrength2 / filterSpread;

		Vectors = clamp(Vectors * filterStrength2, -float2(filterClamp, filterClamp), float2(filterClamp, filterClamp));
#else
		Vectors *= filterSpread;
#endif

		float2 Normal = Vectors * (w_h_height.xy /* * 2.0*/);

		float3 Scene1 = textureLod(tex_fb_filtered, u + Normal, 0).rgb;
		float3 Scene2 = textureLod(tex_fb_filtered, u - Normal, 0).rgb;
#if defined(NFAA_VARIANT) || defined(NFAA_VARIANT2)
		float3 Scene3 = textureLod(tex_fb_filtered, u + float2(Normal.x, -Normal.y)*0.5, 0).rgb;
		float3 Scene4 = textureLod(tex_fb_filtered, u - float2(Normal.x, -Normal.y)*0.5, 0).rgb;
#else
		float3 Scene3 = textureLod(tex_fb_filtered, u + float2(Normal.x, -Normal.y), 0).rgb;
		float3 Scene4 = textureLod(tex_fb_filtered, u - float2(Normal.x, -Normal.y), 0).rgb;
#endif

#ifdef NFAA_TEST_MODE // debug
		float3 o_Color = normalize(float3(Vectors * 0.5 + 0.5, 1.0));
#else
		float3 o_Color = (Scene0 + Scene1 + Scene2 + Scene3 + Scene4) * 0.2;
#endif

		color = float4(o_Color, 1.0);
	}
}

////FRAGMENT

float3 sampleOffset(float2 u, float2 pixelOffset )
{
   return textureLod(tex_fb_filtered, u + pixelOffset * w_h_height.xy, 0).xyz;
}

float4 sampleOffseta(float2 u, float2 pixelOffset )
{
   return textureLod(tex_fb_filtered, u + pixelOffset * w_h_height.xy, 0);
}

float avg(float3 l)
{
   //return dot(l, float3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
   return (l.x+l.y+l.z) * (1.0 / 3.0);
}

////ps_main_dlaa_edge

// DLAA approximation


in float2 tex0;

void main()
{
   float2 u = tex0 + w_h_height.xy*0.5;

   float3 sCenter    = sampleOffset(u, float2( 0.0,  0.0) );
   float3 sUpLeft    = sampleOffset(u, float2(-0.5, -0.5) );
   float3 sUpRight   = sampleOffset(u, float2( 0.5, -0.5) );
   float3 sDownLeft  = sampleOffset(u, float2(-0.5,  0.5) );
   float3 sDownRight = sampleOffset(u, float2( 0.5,  0.5) );

   float3 diff       = abs( (sUpLeft + sUpRight) + (sDownLeft + sDownRight) - sCenter * 4.0 );
   float  edgeMask   = avg(diff) * 4.0; //!! magic

   color = /*test: float4(edgeMask,edgeMask,edgeMask,1.0);*/ float4(sCenter, edgeMask);
}

////ps_main_dlaa

in float2 tex0;

void main()
{
   float2 u = tex0 + w_h_height.xy*0.5;

   float4 sampleCenter = sampleOffseta(u, float2( 0.0,  0.0) );
   
	float depth0 = textureLod(tex_depth, u, 0).x;
	if ((w_h_height.w == 1.0) && ((depth0 == 1.0) || (depth0 == 0.0))) // early out if depth too large (=BG) or too small (=DMD,etc)
			color = float4(sampleCenter.xyz, 1.0);
	else {
	   // short edges
	   float4 sampleHorizNeg0  = sampleOffseta(u, float2(-1.5,  0.0) );
	   float4 sampleHorizPos0  = sampleOffseta(u, float2( 1.5,  0.0) ); 
	   float4 sampleVertNeg0   = sampleOffseta(u, float2( 0.0, -1.5) ); 
	   float4 sampleVertPos0   = sampleOffseta(u, float2( 0.0,  1.5) );

	   float3 sumHoriz         = sampleHorizNeg0.xyz + sampleHorizPos0.xyz;
	   float3 sumVert          = sampleVertNeg0.xyz  + sampleVertPos0.xyz;

	   float3 sampleLeft       = sampleOffset(u, float2(-1.0,  0.0) );
	   float3 sampleRight      = sampleOffset(u, float2( 1.0,  0.0) );
	   float3 sampleTop        = sampleOffset(u, float2( 0.0, -1.0) );
	   float3 sampleDown       = sampleOffset(u, float2( 0.0,  1.0) );

	   float3 diffToCenterHoriz= abs((sampleLeft+sampleRight) * 0.5 - sampleCenter.xyz); //!! was sumHoriz instead of l&r
	   float3 diffToCenterVert = abs((sampleTop+sampleDown) * 0.5 - sampleCenter.xyz); //!! was sumVert instead of t&d

	   float valueEdgeHoriz    = avg(diffToCenterHoriz);
	   float valueEdgeVert     = avg(diffToCenterVert);

	   float edgeDetectHoriz   = 3.0 * valueEdgeHoriz - 0.1; //!! magic params
	   float edgeDetectVert    = 3.0 * valueEdgeVert  - 0.1;

	   float3 avgHoriz         = (sumHoriz + sampleCenter.xyz) * (1.0/3.0);
	   float3 avgVert          = (sumVert  + sampleCenter.xyz) * (1.0/3.0);

	   float valueHoriz        = avg(avgHoriz);
	   float valueVert         = avg(avgVert);

	   float blurAmountHoriz   = saturate(edgeDetectHoriz / valueHoriz);
	   float blurAmountVert    = saturate(edgeDetectVert  / valueVert);

	   float3 aaResult               = lerp( sampleCenter.xyz, avgHoriz, blurAmountVert * 0.5); //!! magic sharpen
	   aaResult                      = lerp( aaResult,         avgVert,  blurAmountHoriz * 0.5); //!! magic sharpen

	   // long edges
	   float4 sampleVertNeg1   = sampleOffseta(u, float2(0.0, -3.5) );
	   float4 sampleVertNeg15  = sampleOffseta(u, float2(0.0, -5.5) );
	   float4 sampleVertNeg2   = sampleOffseta(u, float2(0.0, -7.5) );
	   float4 sampleVertPos1   = sampleOffseta(u, float2(0.0,  3.5) ); 
	   float4 sampleVertPos15  = sampleOffseta(u, float2(0.0,  5.5) ); 
	   float4 sampleVertPos2   = sampleOffseta(u, float2(0.0,  7.5) ); 

	   float4 sampleHorizNeg1  = sampleOffseta(u, float2(-3.5, 0.0) ); 
	   float4 sampleHorizNeg15 = sampleOffseta(u, float2(-5.5, 0.0) ); 
	   float4 sampleHorizNeg2  = sampleOffseta(u, float2(-7.5, 0.0) );
	   float4 sampleHorizPos1  = sampleOffseta(u, float2( 3.5, 0.0) ); 
	   float4 sampleHorizPos15 = sampleOffseta(u, float2( 5.5, 0.0) ); 
	   float4 sampleHorizPos2  = sampleOffseta(u, float2( 7.5, 0.0) ); 

	   float pass1EdgeAvgHoriz = saturate(( sampleHorizNeg2.a + sampleHorizNeg1.a + sampleHorizNeg15.a + sampleHorizNeg0.a + sampleHorizPos0.a + sampleHorizPos1.a + sampleHorizPos15.a + sampleHorizPos2.a ) * (2.0 / 8.0) - 1.0);
	   float pass1EdgeAvgVert  = saturate(( sampleVertNeg2.a  + sampleVertNeg1.a + sampleVertNeg15.a  + sampleVertNeg0.a  + sampleVertPos0.a + sampleVertPos1.a + sampleVertPos15.a  + sampleVertPos2.a  ) * (2.0 / 8.0) - 1.0);

	   if(abs(pass1EdgeAvgHoriz - pass1EdgeAvgVert) > 0.2) //!! magic
	   {
			float valueHorizLong = avg(sampleHorizNeg2.xyz + sampleHorizNeg1.xyz + sampleHorizNeg15.xyz + sampleHorizNeg0.xyz + sampleHorizPos0.xyz + sampleHorizPos1.xyz + sampleHorizPos15.xyz + sampleHorizPos2.xyz) * (1.0/8.0);
			float valueVertLong  = avg(sampleVertNeg2.xyz  + sampleVertNeg1.xyz + sampleVertNeg15.xyz + sampleVertNeg0.xyz  + sampleVertPos0.xyz + sampleVertPos1.xyz + sampleVertPos15.xyz + sampleVertPos2.xyz) * (1.0/8.0);

			float valueCenter    = avg(sampleCenter.xyz);
			float valueLeft      = avg(sampleLeft);
			float valueRight     = avg(sampleRight);
			float valueTop       = avg(sampleTop);
			float valueBottom    = avg(sampleDown);

			float vx = (valueCenter == valueLeft)   ? 0. : saturate(      ( valueVertLong  - valueLeft   ) / (valueCenter - valueLeft));
			float hx = (valueCenter == valueTop)    ? 0. : saturate(      ( valueHorizLong - valueTop    ) / (valueCenter - valueTop));
			float vy = (valueCenter == valueRight)  ? 0. : saturate(1.0 + ( valueVertLong  - valueCenter ) / (valueCenter - valueRight));
			float hy = (valueCenter == valueBottom) ? 0. : saturate(1.0 + ( valueHorizLong - valueCenter ) / (valueCenter - valueBottom));

			float3 longBlurVert  = lerp( sampleRight,
											   lerp( sampleLeft,  sampleCenter.xyz, vx ),
											   vy );
			float3 longBlurHoriz = lerp( sampleDown,
											   lerp( sampleTop,   sampleCenter.xyz, hx ),
											   hy );

			aaResult                   = lerp( aaResult, longBlurVert, pass1EdgeAvgVert * 0.5); //!! magic
			aaResult                   = lerp( aaResult, longBlurHoriz, pass1EdgeAvgHoriz * 0.5); //!! magic

			//test: return float4(aaResult,1.);
	   }
	   //test: return float4(0,0,0,1);

	   color =  /*test: float4(sampleCenter.a,sampleCenter.a,sampleCenter.a,1.0);*/ float4(aaResult, 1.0);
   }
}

////FRAGMENT

float luma(float3 l)
{
    return dot(l, float3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
    //return 0.299*l.x + 0.587*l.y + 0.114*l.z;
    //return 0.2126*l.x + 0.7152*l.y + 0.0722*l.z; // photometric
    //return sqrt(0.299 * l.x*l.x + 0.587 * l.y*l.y + 0.114 * l.z*l.z); // hsp
}

////ps_main_fxaa1

// FXAA

// Approximation of FXAA


in float2 tex0;

void main()
{
	float2 u = tex0 + w_h_height.xy*0.5;

	float3 rMc = textureLod(tex_fb_unfiltered, u, 0).xyz;
	float depth0 = textureLod(tex_depth, u, 0).x;
	if ((w_h_height.w == 1.0) && ((depth0 == 1.0) || (depth0 == 0.0))) // early out if depth too large (=BG) or too small (=DMD,etc)
			color = float4(rMc, 1.0);
	else {
		float2 offs = w_h_height.xy;
		float rNW = luma(textureLod(tex_fb_unfiltered, u - offs, 0).xyz);
		float rN = luma(textureLod(tex_fb_unfiltered, u - float2(0.0,offs.y), 0).xyz);
		float rNE = luma(textureLod(tex_fb_unfiltered, u - float2(-offs.x,offs.y), 0).xyz);
		float rW = luma(textureLod(tex_fb_unfiltered, u - float2(offs.x,0.0), 0).xyz);
		float rM = luma(rMc);
		float rE = luma(textureLod(tex_fb_unfiltered, u + float2(offs.x,0.0), 0).xyz);
		float rSW = luma(textureLod(tex_fb_unfiltered, u + float2(-offs.x,offs.y), 0).xyz);
		float rS = luma(textureLod(tex_fb_unfiltered, u + float2(0.0,offs.y), 0).xyz);
		float rSE = luma(textureLod(tex_fb_unfiltered, u + offs, 0).xyz);
		float rMrN = rM+rN;
		float lumaNW = rMrN+rNW+rW;
		float lumaNE = rMrN+rNE+rE;
		float rMrS = rM+rS;
		float lumaSW = rMrS+rSW+rW;
		float lumaSE = rMrS+rSE+rE;
		bool g0 = (lumaSW > lumaSE);
		float tempMax = g0 ? lumaSW : lumaSE;
		float tempMin = g0 ? lumaSE : lumaSW;
		bool g1 = (lumaNW > lumaNE);
		float tempMax2 = g1 ? lumaNW : lumaNE;
		float tempMin2 = g1 ? lumaNE : lumaNW;
		float lumaMin = min(rM, min(tempMin, tempMin2));
		float lumaMax = max(rM, max(tempMax, tempMax2));
		float SWSE = lumaSW + lumaSE;
		float NWNE = lumaNW + lumaNE;
		float2 dir = float2(SWSE - NWNE, (lumaNW + lumaSW) - (lumaNE + lumaSE));
		float temp = 1.0/(min(abs(dir.x), abs(dir.y)) + max((NWNE + SWSE)*0.03125, 0.0078125)); //!! tweak?
		dir = clamp(dir*temp, float2(-8.0), float2(8.0)) * offs; //!! tweak?
		float3 rgbA = 0.5 * (textureLod(tex_fb_filtered, u-dir*(0.5/3.0), 0).xyz + textureLod(tex_fb_filtered, u+dir*(0.5/3.0), 0).xyz);
		float3 rgbB = 0.5 * rgbA + 0.25 * (textureLod(tex_fb_filtered, u-dir*0.5, 0).xyz + textureLod(tex_fb_filtered, u+dir*0.5, 0).xyz);
		float lumaB = luma(rgbB);
		color = float4(((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB, 1.0);
	}
}

////ps_main_fxaa2

#define FXAA_QUALITY__P0 1.5
#define FXAA_QUALITY__P1 2.0
#define FXAA_QUALITY__P2 8.0

// Full mid-quality PC FXAA 3.11

in float2 tex0;

void main()
{
	float2 u = tex0 + w_h_height.xy*0.5;

	float3 rgbyM = textureLod(tex_fb_unfiltered, u, 0).xyz;
	float depth0 = textureLod(tex_depth, u, 0).x;
	if ((w_h_height.w == 1.0) && ((depth0 == 1.0) || (depth0 == 0.0))) // early out if depth too large (=BG) or too small (=DMD,etc)
			color = float4(rgbyM, 1.0);
	else {
		float2 offs = w_h_height.xy;
		float lumaNW = luma(textureLod(tex_fb_unfiltered, u - offs, 0).xyz);
		float lumaN = luma(textureLod(tex_fb_unfiltered, u - float2(0.0,offs.y), 0).xyz);
		float lumaNE = luma(textureLod(tex_fb_unfiltered, u - float2(-offs.x,offs.y), 0).xyz);
		float lumaW = luma(textureLod(tex_fb_unfiltered, u - float2(offs.x,0.0), 0).xyz);
		float lumaM = luma(rgbyM);
		float lumaE = luma(textureLod(tex_fb_unfiltered, u + float2(offs.x,0.0), 0).xyz);
		float lumaSW = luma(textureLod(tex_fb_unfiltered, u + float2(-offs.x,offs.y), 0).xyz);
		float lumaS = luma(textureLod(tex_fb_unfiltered, u + float2(0.0,offs.y), 0).xyz);
		float lumaSE = luma(textureLod(tex_fb_unfiltered, u + offs, 0).xyz);
		float maxSM = max(lumaS, lumaM);
		float minSM = min(lumaS, lumaM);
		float maxESM = max(lumaE, maxSM);
		float minESM = min(lumaE, minSM);
		float maxWN = max(lumaN, lumaW);
		float minWN = min(lumaN, lumaW);
		float rangeMax = max(maxWN, maxESM);
		float rangeMin = min(minWN, minESM);
		float rangeMaxScaled = rangeMax * 0.125; //0.333 (faster) .. 0.063 (slower) // reshade: 0.125, fxaa : 0.166
		float range = rangeMax - rangeMin;
		float rangeMaxClamped = max(0.0833, rangeMaxScaled); //0.0625 (high quality/faster) .. 0.0312 (visible limit/slower) // reshade: 0.0, fxaa : 0.0833
		bool earlyExit = range < rangeMaxClamped;
		if (earlyExit)
			color = float4(rgbyM, 1.0);
		else {
			float lumaNS = lumaN + lumaS;
			float lumaWE = lumaW + lumaE;
			float subpixRcpRange = 1.0/range;
			float subpixNSWE = lumaNS + lumaWE;
			float edgeHorz1 = -2.0 * lumaM + lumaNS;
			float edgeVert1 = -2.0 * lumaM + lumaWE;
			float lumaNESE = lumaNE + lumaSE;
			float lumaNWNE = lumaNW + lumaNE;
			float edgeHorz2 = -2.0 * lumaE + lumaNESE;
			float edgeVert2 = -2.0 * lumaN + lumaNWNE;
			float lumaNWSW = lumaNW + lumaSW;
			float lumaSWSE = lumaSW + lumaSE;
			float edgeHorz4 = abs(edgeHorz1) * 2.0 + abs(edgeHorz2);
			float edgeVert4 = abs(edgeVert1) * 2.0 + abs(edgeVert2);
			float edgeHorz3 = -2.0 * lumaW + lumaNWSW;
			float edgeVert3 = -2.0 * lumaS + lumaSWSE;
			float edgeHorz = abs(edgeHorz3) + edgeHorz4;
			float edgeVert = abs(edgeVert3) + edgeVert4;
			float subpixNWSWNESE = lumaNWSW + lumaNESE;
			float lengthSign = offs.x;
			bool horzSpan = edgeHorz >= edgeVert;
			float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
			if(!horzSpan) lumaN = lumaW;
			if(!horzSpan) lumaS = lumaE;
			if(horzSpan) lengthSign = offs.y;
			float subpixB = subpixA * (1.0/12.0) - lumaM;
			float gradientN = lumaN - lumaM;
			float gradientS = lumaS - lumaM;
			float lumaNN = lumaN + lumaM;
			float lumaSS = lumaS + lumaM;
			bool pairN = (abs(gradientN) >= abs(gradientS));
			float gradient = max(abs(gradientN), abs(gradientS));
			if(pairN) lengthSign = -lengthSign;
			float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);
			float2 offNP = float2(!horzSpan ? 0.0 : offs.x, horzSpan ? 0.0 : offs.y);
			float2 posB = u;
			float l05 = lengthSign * 0.5;
			if(horzSpan) posB.y += l05;
			else posB.x += l05;
			float2 posN = float2(posB.x - offNP.x * FXAA_QUALITY__P0, posB.y - offNP.y * FXAA_QUALITY__P0);
			float2 posP = float2(posB.x + offNP.x * FXAA_QUALITY__P0, posB.y + offNP.y * FXAA_QUALITY__P0);
			float subpixD = -2.0 * subpixC + 3.0;
			float lumaEndN = luma(textureLod(tex_fb_filtered, posN, 0).xyz);
			float subpixE = subpixC * subpixC;
			float lumaEndP = luma(textureLod(tex_fb_filtered, posP, 0).xyz);
			if(!pairN) lumaNN = lumaSS;
			float gradientScaled = gradient * (1.0/4.0);
			float lumaMM = lumaM - lumaNN * 0.5;
			float subpixF = subpixD * subpixE;
			bool lumaMLTZero = (lumaMM < 0.0);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
			float dstN = horzSpan ? (u.x - posN.x) : (u.y - posN.y);
			float dstP = horzSpan ? (posP.x - u.x) : (posP.y - u.y);
			bool goodSpanN = ((lumaEndN < 0.0) != lumaMLTZero);
			float spanLength = dstP + dstN;
			bool goodSpanP = ((lumaEndP < 0.0) != lumaMLTZero);
			float spanLengthRcp = 1.0/spanLength;
			bool directionN = (dstN < dstP);
			float dst = min(dstN, dstP);
			bool goodSpan = directionN ? goodSpanN : goodSpanP;
			float subpixG = subpixF * subpixF;
			float pixelOffset = 0.5 - dst * spanLengthRcp;
			float subpixH = subpixG * 0.5; //1.00 (upper limit/softer) .. 0.50 (lower limit/sharper) .. 0.00 (completely off) // reshade : 0.25, fxaa : 0.75
			float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
			float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
			float2 un = u;
			float pl = pixelOffsetSubpix * lengthSign;
			if(horzSpan) un.y += pl;
			else un.x += pl;
			color = float4(textureLod(tex_fb_filtered, un, 0).xyz, 1.0);
		}
	}
}

#undef FXAA_QUALITY__P0
#undef FXAA_QUALITY__P1
#undef FXAA_QUALITY__P2

////ps_main_fxaa3

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

in float2 tex0;

void main()
{
	float2 u = tex0 + w_h_height.xy*0.5;

	float3 rgbyM = textureLod(tex_fb_unfiltered, u, 0).xyz;
	float depth0 = textureLod(tex_depth, u, 0).x;
	if ((w_h_height.w == 1.0) && ((depth0 == 1.0) || (depth0 == 0.0))) // early out if depth too large (=BG) or too small (=DMD,etc)
			color = float4(rgbyM, 1.0);
	else {
		float2 offs = w_h_height.xy;
		float lumaNW = luma(textureLod(tex_fb_unfiltered, u - offs, 0).xyz);
		float lumaN = luma(textureLod(tex_fb_unfiltered, u - float2(0.0,offs.y), 0).xyz);
		float lumaNE = luma(textureLod(tex_fb_unfiltered, u - float2(-offs.x,offs.y), 0).xyz);
		float lumaW = luma(textureLod(tex_fb_unfiltered, u - float2(offs.x,0.0), 0).xyz);
		float lumaM = luma(rgbyM);
		float lumaE = luma(textureLod(tex_fb_unfiltered, u + float2(offs.x,0.0), 0).xyz);
		float lumaSW = luma(textureLod(tex_fb_unfiltered, u + float2(-offs.x,offs.y), 0).xyz);
		float lumaS = luma(textureLod(tex_fb_unfiltered, u + float2(0.0,offs.y), 0).xyz);
		float lumaSE = luma(textureLod(tex_fb_unfiltered, u + offs, 0).xyz);
		float maxSM = max(lumaS, lumaM);
		float minSM = min(lumaS, lumaM);
		float maxESM = max(lumaE, maxSM);
		float minESM = min(lumaE, minSM);
		float maxWN = max(lumaN, lumaW);
		float minWN = min(lumaN, lumaW);
		float rangeMax = max(maxWN, maxESM);
		float rangeMin = min(minWN, minESM);
		float rangeMaxScaled = rangeMax * 0.125; //0.333 (faster) .. 0.063 (slower) // reshade: 0.125, fxaa : 0.166
		float range = rangeMax - rangeMin;
		float rangeMaxClamped = max(0.0833, rangeMaxScaled); //0.0625 (high quality/faster) .. 0.0312 (visible limit/slower) // reshade: 0.0, fxaa : 0.0833
		bool earlyExit = range < rangeMaxClamped;
		if(earlyExit)
			color = float4(rgbyM, 1.0);
		else {
			float lumaNS = lumaN + lumaS;
			float lumaWE = lumaW + lumaE;
			float subpixRcpRange = 1.0/range;
			float subpixNSWE = lumaNS + lumaWE;
			float edgeHorz1 = -2.0 * lumaM + lumaNS;
			float edgeVert1 = -2.0 * lumaM + lumaWE;
			float lumaNESE = lumaNE + lumaSE;
			float lumaNWNE = lumaNW + lumaNE;
			float edgeHorz2 = -2.0 * lumaE + lumaNESE;
			float edgeVert2 = -2.0 * lumaN + lumaNWNE;
			float lumaNWSW = lumaNW + lumaSW;
			float lumaSWSE = lumaSW + lumaSE;
			float edgeHorz4 = abs(edgeHorz1) * 2.0 + abs(edgeHorz2);
			float edgeVert4 = abs(edgeVert1) * 2.0 + abs(edgeVert2);
			float edgeHorz3 = -2.0 * lumaW + lumaNWSW;
			float edgeVert3 = -2.0 * lumaS + lumaSWSE;
			float edgeHorz = abs(edgeHorz3) + edgeHorz4;
			float edgeVert = abs(edgeVert3) + edgeVert4;
			float subpixNWSWNESE = lumaNWSW + lumaNESE;
			float lengthSign = offs.x;
			bool horzSpan = edgeHorz >= edgeVert;
			float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
			if(!horzSpan) lumaN = lumaW;
			if(!horzSpan) lumaS = lumaE;
			if(horzSpan) lengthSign = offs.y;
			float subpixB = subpixA * (1.0/12.0) - lumaM;
			float gradientN = lumaN - lumaM;
			float gradientS = lumaS - lumaM;
			float lumaNN = lumaN + lumaM;
			float lumaSS = lumaS + lumaM;
			bool pairN = (abs(gradientN) >= abs(gradientS));
			float gradient = max(abs(gradientN), abs(gradientS));
			if(pairN) lengthSign = -lengthSign;
			float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);
			float2 offNP = float2(!horzSpan ? 0.0 : offs.x, horzSpan ? 0.0 : offs.y);
			float2 posB = u;
			float l05 = lengthSign * 0.5;
			if(horzSpan) posB.y += l05;
			else posB.x += l05;
			float2 posN = float2(posB.x - offNP.x * FXAA_QUALITY__P0, posB.y - offNP.y * FXAA_QUALITY__P0);
			float2 posP = float2(posB.x + offNP.x * FXAA_QUALITY__P0, posB.y + offNP.y * FXAA_QUALITY__P0);
			float subpixD = -2.0 * subpixC + 3.0;
			float lumaEndN = luma(textureLod(tex_fb_filtered, posN, 0).xyz);
			float subpixE = subpixC * subpixC;
			float lumaEndP = luma(textureLod(tex_fb_filtered, posP, 0).xyz);
			if(!pairN) lumaNN = lumaSS;
			float gradientScaled = gradient * (1.0/4.0);
			float lumaMM = lumaM - lumaNN * 0.5;
			float subpixF = subpixD * subpixE;
			bool lumaMLTZero = (lumaMM < 0.0);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
				if(!doneN) lumaEndN = luma(textureLod(tex_fb_filtered, posN.xy, 0).xyz);
				if(!doneP) lumaEndP = luma(textureLod(tex_fb_filtered, posP.xy, 0).xyz);
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
			float dstN = horzSpan ? (u.x - posN.x) : (u.y - posN.y);
			float dstP = horzSpan ? (posP.x - u.x) : (posP.y - u.y);
			bool goodSpanN = ((lumaEndN < 0.0) != lumaMLTZero);
			float spanLength = dstP + dstN;
			bool goodSpanP = ((lumaEndP < 0.0) != lumaMLTZero);
			float spanLengthRcp = 1.0/spanLength;
			bool directionN = (dstN < dstP);
			float dst = min(dstN, dstP);
			bool goodSpan = directionN ? goodSpanN : goodSpanP;
			float subpixG = subpixF * subpixF;
			float pixelOffset = 0.5 - dst * spanLengthRcp;
			float subpixH = subpixG * 0.5; //1.00 (upper limit/softer) .. 0.50 (lower limit/sharper) .. 0.00 (completely off) // reshade : 0.25, fxaa : 0.75
			float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
			float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
			float2 un = u;
			float pl = pixelOffsetSubpix * lengthSign;
			if(horzSpan) un.y += pl;
			else un.x += pl;
			color = float4(textureLod(tex_fb_filtered, un, 0).xyz, 1.0);
		}
	}
}

////ps_main_CAS

in float2 tex0;

void main()
{
	// variant with better diagonals

	const float Contrast   = 0.0; // 0..1, Adjusts the range the shader adapts to high contrast (0 is not all the way off).  Higher values = more high contrast sharpening.
	const float Sharpening = 1.0; // 0..1, Adjusts sharpening intensity by averaging the original pixels to the sharpened result.  1.0 is the unmodified default.

	const float2 u = tex0 + w_h_height.xy*0.5;

	const float3 e = tex2Dlod(tex_fb_unfiltered, float4(u, 0.,0.)).xyz;
	if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = tex2Dlod(tex_depth, float4(u, 0.,0.)).x;
		if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
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

	const float3 a = tex2Dlod(tex_fb_unfiltered, float4(um1,          0.,0.)).xyz;
	const float3 b = tex2Dlod(tex_fb_unfiltered, float4(u.x,   um1.y, 0.,0.)).xyz;
	const float3 c = tex2Dlod(tex_fb_unfiltered, float4(up1.x, um1.y, 0.,0.)).xyz;
	const float3 d = tex2Dlod(tex_fb_unfiltered, float4(um1.x, u.y,   0.,0.)).xyz;
	const float3 g = tex2Dlod(tex_fb_unfiltered, float4(um1.x, up1.y, 0.,0.)).xyz; 
	const float3 f = tex2Dlod(tex_fb_unfiltered, float4(up1.x, u.y,   0.,0.)).xyz;
	const float3 h = tex2Dlod(tex_fb_unfiltered, float4(u.x,   up1.y, 0.,0.)).xyz;
	const float3 i = tex2Dlod(tex_fb_unfiltered, float4(up1,          0.,0.)).xyz;

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
	const float3 rcpMRGB = float3(1.0) / (mxRGB);
	float3 ampRGB = saturate(min(mnRGB, 2.0 - mxRGB) * rcpMRGB);

	// Shaping amount of sharpening.
	ampRGB = rsqrt(ampRGB);

	const float peak = -3.0 * Contrast + 8.0;
	const float3 wRGB = -float3(1.0) / (ampRGB * peak);

	const float3 rcpWeightRGB = float3(1.0) / (4.0 * wRGB + float3(1.0));

	//                          0 w 0
	//  Filter shape:           w 1 w
	//                          0 w 0  
	const float3 window = (b + d) + (f + h);
	const float3 outColor = saturate((window * wRGB + e) * rcpWeightRGB);

	color = float4(lerp(e, outColor, Sharpening), 1.);
	
	color = vec4(outColor, 1.0);
}


////ps_main_BilateralSharp_CAS

in float2 tex0;

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
	const float sharpness = 0.625*3.1;

	const float2 u = tex0 + w_h_height.xy*0.5;

	const float3 e = tex2Dlod(tex_fb_unfiltered, float4(u, 0.,0.)).xyz;
	if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = tex2Dlod(tex_depth, float4(u, 0.,0.)).x;
		if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
		{
			color = float4(e, 1.0);
			return;
		}
	}

	// Bilateral Blur (crippled)
	float3 final_colour = float3(0.,0.,0.);
	float Z = 0.0;
	for (int j=-2; j <= 2; ++j) // 2 = kernelradius
		for (int i=-2; i <= 2; ++i)
		{
			const float3 cc = tex2Dlod(tex_fb_unfiltered, float4(u.x + i*(w_h_height.x*0.5), u.y + j*(w_h_height.y*0.5), 0.,0.)).xyz; // *0.5 = 1/kernelradius
			const float factor = normpdf(cc-e, 0.25); // 0.25 = BSIGMA
			Z += factor;
			final_colour += factor*cc;
		}

	// CAS (without Better Diagonals)
	const float2 um1 = u - w_h_height.xy;
	const float2 up1 = u + w_h_height.xy;

	const float b = LI(tex2Dlod(tex_fb_unfiltered, float4(u.x, um1.y, 0.,0.)).xyz);
	const float d = LI(tex2Dlod(tex_fb_unfiltered, float4(um1.x, u.y, 0.,0.)).xyz);
	const float f = LI(tex2Dlod(tex_fb_unfiltered, float4(up1.x, u.y, 0.,0.)).xyz);
	const float h = LI(tex2Dlod(tex_fb_unfiltered, float4(u.x, up1.y, 0.,0.)).xyz);
	const float e1 = LI(e);

	const float mnRGB = min(min(min(d, e1), min(f, b)), h);
	const float mxRGB = max(max(max(d, e1), max(f, b)), h);

	// Smooth minimum distance to signal limit divided by smooth max.
	const float rcpMRGB = 1.0 / mxRGB;
	const float ampRGB = saturate(min(mnRGB, 1.0 - mxRGB) * rcpMRGB);

	float3 sharpen = (e-final_colour/Z) * sharpness;

	const float gs_sharpen = dot(sharpen, float3(0.333333333333));
	sharpen = lerp(float3(gs_sharpen), sharpen, 0.5);

	color = float4(lerp(e, sharpen+e, ampRGB*saturate(sharpness)), 1.0);
}
