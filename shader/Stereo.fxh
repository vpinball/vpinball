// Shared shader file used for stereo rendering (OpenGL and DX9)

UNIFORM float4 Stereo_Saturation;
#define STEREO_SATURATION (Stereo_Saturation.x)

UNIFORM float4 Stereo_MS_ZPD_YAxis;
#define STEREO_MAX_SEPARATION (Stereo_MS_ZPD_YAxis.x)
#define STEREO_ZERO_POINT_DEPTH (Stereo_MS_ZPD_YAxis.y)
#define STEREO_YAXIS (Stereo_MS_ZPD_YAxis.z != 0.)


// ////////////////////////////////////////////////////////////////////////////
// Gather stereo image, either:
// - From per eye renders stored in a layered texture,
// - By reconstructing 2 fake stereo images from a centered render and its depth buffer.
// The functions return colors in the sRGB color space

// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel.
// Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
UNIFORM float4 w_h_height;

#if (N_EYES == 1)
float3 gatherEyeColor(float2 u, bool isLeft)
{
    const bool yaxis = STEREO_YAXIS;
    const float MaxSeparation = STEREO_MAX_SEPARATION;
    BRANCH if (isLeft)
    {
        const float lminDepth = min(min(texNoLod(tex_stereo_depth, u + (yaxis ? float2(0.0, 0.500 * MaxSeparation) : float2(0.500 * MaxSeparation, 0.0))).x, 
                                        texNoLod(tex_stereo_depth, u + (yaxis ? float2(0.0, 0.666 * MaxSeparation) : float2(0.666 * MaxSeparation, 0.0))).x), 
                                        texNoLod(tex_stereo_depth, u + (yaxis ? float2(0.0,         MaxSeparation) : float2(        MaxSeparation, 0.0))).x);
        float lparallax = max(0.0, w_h_height.w + MaxSeparation * (1.0 - 1.0 / (0.5 + lminDepth * (1.0 / STEREO_ZERO_POINT_DEPTH - 0.5))));
        return texNoLod(tex_stereo_fb, u + (yaxis ? float2(0.0, -lparallax) : float2(lparallax, 0.0))).xyz;
    }
    else
    {
        const float rminDepth = min(min(texNoLod(tex_stereo_depth, u - (yaxis ? float2(0.0, 0.500 * MaxSeparation) : float2(0.500 * MaxSeparation, 0.0))).x, 
                                        texNoLod(tex_stereo_depth, u - (yaxis ? float2(0.0, 0.666 * MaxSeparation) : float2(0.666 * MaxSeparation, 0.0))).x), 
                                        texNoLod(tex_stereo_depth, u - (yaxis ? float2(0.0,         MaxSeparation) : float2(        MaxSeparation, 0.0))).x);
        float rparallax = max(0.0, w_h_height.w + MaxSeparation * (1.0 - 1.0 / (0.5 + rminDepth * (1.0 / STEREO_ZERO_POINT_DEPTH - 0.5))));
        return texNoLod(tex_stereo_fb, u - (yaxis ? float2(0.0, -rparallax) : float2(rparallax, 0.0))).xyz;
    }
}
void gatherLeftRightColors(float2 u, out float3 lcol, out float3 rcol)
{
    const bool yaxis = STEREO_YAXIS;
    const float MaxSeparation = STEREO_MAX_SEPARATION;

    const float lminDepth = min(min(texNoLod(tex_stereo_depth, u + (yaxis ? float2(0.0, 0.500 * MaxSeparation) : float2(0.500 * MaxSeparation, 0.0))).x, 
                                    texNoLod(tex_stereo_depth, u + (yaxis ? float2(0.0, 0.666 * MaxSeparation) : float2(0.666 * MaxSeparation, 0.0))).x), 
                                    texNoLod(tex_stereo_depth, u + (yaxis ? float2(0.0,         MaxSeparation) : float2(        MaxSeparation, 0.0))).x);
    float lparallax = max(0.0, w_h_height.w + MaxSeparation * (1.0 - 1.0 / (0.5 + lminDepth * (1.0 / STEREO_ZERO_POINT_DEPTH - 0.5))));
    lcol = texNoLod(tex_stereo_fb, u + (yaxis ? float2(0.0, -lparallax) : float2(lparallax, 0.0))).xyz;

    const float rminDepth = min(min(texNoLod(tex_stereo_depth, u - (yaxis ? float2(0.0, 0.500 * MaxSeparation) : float2(0.500 * MaxSeparation, 0.0))).x, 
                                    texNoLod(tex_stereo_depth, u - (yaxis ? float2(0.0, 0.666 * MaxSeparation) : float2(0.666 * MaxSeparation, 0.0))).x), 
                                    texNoLod(tex_stereo_depth, u - (yaxis ? float2(0.0,         MaxSeparation) : float2(        MaxSeparation, 0.0))).x);
    float rparallax = max(0.0, w_h_height.w + MaxSeparation * (1.0 - 1.0 / (0.5 + rminDepth * (1.0 / STEREO_ZERO_POINT_DEPTH - 0.5))));
    rcol = texNoLod(tex_stereo_fb, u - (yaxis ? float2(0.0, -rparallax) : float2(rparallax, 0.0))).xyz;
}

#else
// TODO use image gather instead of texture sampling. It is faster and more clear to the intent
float3 gatherEyeColor(float2 u, bool isLeft)
{
    return textureLod(tex_stereo_fb, vec3(u.x, u.y, isLeft ? 0.0 : 1.0), 0.).rgb;
}
void gatherLeftRightColors(float2 u, out float3 lcol, out float3 rcol)
{
    lcol = textureLod(tex_stereo_fb, float3(u.x, u.y, 0.), 0.).xyz;
    rcol = textureLod(tex_stereo_fb, float3(u.x, u.y, 1.), 0.).xyz;
}
#endif


// ////////////////////////////////////////////////////////////////////////////
// Anaglyph image composition

UNIFORM float4x4 Stereo_LeftMat;
UNIFORM float4x4 Stereo_RightMat;

// Perform dynamic desaturation based on filter colors for limited retinal rivalry
// I did find quite a lot of papers regarding reducing ghosting but nearly nothing regarding limiting retinal rivalry in real time.
// So I designed this little trick: retinal rivalry happens when something is seen by one eye and not the other. In turn,
// this happens when a color passes through only one of the eye filter (for example full saturated red, 100% passing through left 
// and 0% through right). The trick is to desaturate this color in order to have it pass through both filters (at least a little to 
// satisfy the viewer brain and allow it to merge the 2 images, solving the retinal rivalry). This is done dynamically based on the
// perceived luminance difference (obtained through the filter calibration) to limit the desaturation color lost.
// lCol/rCol are expected to be in sRGB color space
UNIFORM float4 Stereo_LeftLuminance;
UNIFORM float4 Stereo_RightLuminance;
void DynamicDesatAnaglyph(const float3 lCol, const float3 rCol, out float3 lDesatCol, out float3 rDesatCol)
{
	float leftLum  = dot(lCol, Stereo_LeftLuminance.xyz);
	float rightLum = dot(rCol, Stereo_RightLuminance.xyz);
    // float desat = pow(abs(leftLum - rightLum), 0.5); // less retinal rivalry and ghosting at the price of less colors
    float desat = abs(leftLum - rightLum);
    #ifdef GLSL
	lDesatCol = lerp(lCol, float3(Luminance(lCol)), desat);
	rDesatCol = lerp(rCol, float3(Luminance(rCol)), desat);
    #else
    lDesatCol = lerp(lCol,        Luminance(lCol) , desat);
    rDesatCol = lerp(rCol,        Luminance(rCol) , desat);
    #endif
}

// Compose anaglyph linearly from stereo colors
// see naive filters (full channel filter)
// see https://www.site.uottawa.ca/~edubois/anaglyph/
// see https://www.visus.uni-stuttgart.de/en/research/computer-graphics/anaglyph-stereo/anaglyph-stereo-without-ghosting/
// lCol/rCol are expected to be in sRGB color space
float3 LinearAnaglyph(const float3 lCol, const float3 rCol)
{
    const float3 color = (mul(float4(lCol, 1.), Stereo_LeftMat) + mul(float4(rCol, 1.), Stereo_RightMat)).rgb;
    return saturate(color);
}


// Compose anaglyph by applying John Einselen's contrast and deghosting method
// see http://iaian7.com/quartz/AnaglyphCompositing & vectorform.com
// lCol/rCol are expected to be in sRGB color space
UNIFORM float4 Stereo_DeghostGamma; // Gamma adjustment (depending on glasses)
UNIFORM float4x4 Stereo_DeghostFilter; // Cross eye deghosting filter (depending on glasses)
float3 DeghostAnaglyph(const float3 lCol, const float3 rCol)
{
    const float3 color = (mul(float4(lCol, 1.), Stereo_LeftMat) + mul(float4(rCol, 1.), Stereo_RightMat)).rgb;
    return saturate(mul(pow(color, Stereo_DeghostGamma.rgb), to_float3x3(Stereo_DeghostFilter)));
}
