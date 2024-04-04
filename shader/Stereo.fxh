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
// Retinal rivalry happens when something is seen by one eye and not the other. In turn, this happens when a color passes through 
// only one of the eye filter (for example full saturated red, 100% passing through left and 0% through right). The trick is to 
// identify this situation by applying the filters to incoming colors and if needed adjust the colors to allow them to pass the 2 
// filters instead of 1 (at least a little to satisfy the viewer brain and allow it to merge the 2 images, solving the retinal 
// rivalry). There are two ways to do this:
// - desaturation of the incoming colors (turn to gray), accounting for the loss of perceived luminance (Helmholtz�Kohlrausch effect)
// - hue shifting which allows to keep saturated colors, but not the right ones (tested but without generally satisfying results).
UNIFORM float4 Stereo_LeftLuminance_Gamma;
UNIFORM float4 Stereo_RightLuminance_DynDesat;

/*float3 HueShift(const float3 color, const float dhue)
{
    float s = sin(dhue);
    float c = cos(dhue);
    return (color * c) + (color * s) * mat3(
        float3(0.167444, 0.329213, -0.496657),
        float3(-0.327948, 0.035669, 0.292279),
        float3(1.250268, -1.047561, -0.202707)
    ) + dot(float3(0.299, 0.587, 0.114), color) * (1.0 - c);
}*/

// lCol/rCol are expected to be in linear color space
void DynamicDesatAnaglyph(const float3 lCol, const float3 rCol, out float3 lDesatCol, out float3 rDesatCol)
{
    const float left2LeftLum = dot(lCol, Stereo_LeftLuminance_Gamma.xyz);
    const float left2RightLum = dot(lCol, Stereo_RightLuminance_DynDesat.xyz);
    const float right2LeftLum = dot(rCol, Stereo_LeftLuminance_Gamma.xyz);
    const float right2RightLum = dot(rCol, Stereo_RightLuminance_DynDesat.xyz);
    const float leftLum = left2LeftLum + left2RightLum;
    const float rightLum = right2LeftLum + right2RightLum;
    const float leftDesat = Stereo_RightLuminance_DynDesat.w * abs((left2LeftLum - left2RightLum) / (leftLum + 0.0001));
    const float rightDesat = Stereo_RightLuminance_DynDesat.w * abs((right2LeftLum - right2RightLum) / (rightLum + 0.0001));
#ifdef GLSL
    lDesatCol = lerp(lCol, float3(leftLum), leftDesat);
    rDesatCol = lerp(rCol, float3(rightLum), rightDesat);
#else
    lDesatCol = lerp(lCol, leftLum, leftDesat);
    rDesatCol = lerp(rCol, rightLum, rightDesat);
#endif
    // Hue shift experiments for which no function that would look good with any table was found
    //lDesatCol = HueShift(lDesatCol, -leftDesat  * PI * 120. / 180.);
    //rDesatCol = HueShift(rDesatCol, -rightDesat * PI * 120. / 180.);
    //lDesatCol = HueShift(lCol, -leftDesat  * PI * 120. / 180.);
    //rDesatCol = HueShift(rCol, -rightDesat * PI * 120. / 180.);
}


// Compose anaglyph linearly from stereo colors, applying in linear RGB space
// see naive filters (full channel filter)
// see https://www.site.uottawa.ca/~edubois/anaglyph/
// see https://www.visus.uni-stuttgart.de/en/research/computer-graphics/anaglyph-stereo/anaglyph-stereo-without-ghosting/
// lCol/rCol are expected to be in linear color space
float3 LinearAnaglyph(const float3 lCol, const float3 rCol)
{
    const float3 color = (mul(float4(lCol, 1.), Stereo_LeftMat) + mul(float4(rCol, 1.), Stereo_RightMat)).rgb;
    // Uncomment to vizualize parts that should exhibit ghosting (clamping always result in ghosting)
    //if (color.r < 0.0 || color.g < 0.0 || color.b < 0.0 || color.r > 1.0 || color.g > 1.0 || color.b > 1.0)
    //    color = vec3(0.0);
    return color;
}


// Compose anaglyph by applying John Einselen's contrast and deghosting method
// see http://iaian7.com/quartz/AnaglyphCompositing & vectorform.com
// lCol/rCol are expected to be in sRGB color space
UNIFORM float4 Stereo_DeghostGamma; // Gamma adjustment (depending on glasses)
UNIFORM float4x4 Stereo_DeghostFilter; // Cross eye deghosting filter (depending on glasses)
float3 DeghostAnaglyph(const float3 lCol, const float3 rCol)
{
    const float3 color = LinearAnaglyph(lCol, rCol);
    return mul(pow(color, Stereo_DeghostGamma.rgb), to_float3x3(Stereo_DeghostFilter));
}
