// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"

uniform vec4 Stereo_Saturation;
#define STEREO_SATURATION (Stereo_Saturation.x)

uniform vec4 Stereo_MS_ZPD_YAxis;
#define STEREO_MAX_SEPARATION (Stereo_MS_ZPD_YAxis.x)
#define STEREO_ZERO_POINT_DEPTH (Stereo_MS_ZPD_YAxis.y)
#define STEREO_YAXIS (Stereo_MS_ZPD_YAxis.z != 0.)

SAMPLER2DSTEREO(tex_stereo_fb,    0); // Render buffer with either a single render for fake stereo by depth reconstruction or 2 renders in a layered texture
SAMPLER2DSTEREO(tex_stereo_depth, 1); // Depth, only used for fake stereo by depth reconstruction



// ////////////////////////////////////////////////////////////////////////////
// Gather stereo image, either:
// - From per eye renders stored in a layered texture,
// - By reconstructing 2 fake stereo images from a centered render and its depth buffer.
// The functions return colors in the sRGB color space

// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel.
// Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

#ifndef STEREO
vec3 gatherEyeColor(vec2 u, bool isLeft)
{
    const bool yaxis = STEREO_YAXIS;
    const float MaxSeparation = STEREO_MAX_SEPARATION;
    BRANCH if (isLeft)
    {
        const float lminDepth = min(min(texNoLod(tex_stereo_depth, u + (yaxis ? vec2(0.0, 0.500 * MaxSeparation) : vec2(0.500 * MaxSeparation, 0.0))).x, 
                                        texNoLod(tex_stereo_depth, u + (yaxis ? vec2(0.0, 0.666 * MaxSeparation) : vec2(0.666 * MaxSeparation, 0.0))).x), 
                                        texNoLod(tex_stereo_depth, u + (yaxis ? vec2(0.0,         MaxSeparation) : vec2(        MaxSeparation, 0.0))).x);
        float lparallax = max(0.0, w_h_height.w + MaxSeparation * (1.0 - 1.0 / (0.5 + lminDepth * (1.0 / STEREO_ZERO_POINT_DEPTH - 0.5))));
        return texNoLod(tex_stereo_fb, u + (yaxis ? vec2(0.0, -lparallax) : vec2(lparallax, 0.0))).xyz;
    }
    else
    {
        const float rminDepth = min(min(texNoLod(tex_stereo_depth, u - (yaxis ? vec2(0.0, 0.500 * MaxSeparation) : vec2(0.500 * MaxSeparation, 0.0))).x, 
                                        texNoLod(tex_stereo_depth, u - (yaxis ? vec2(0.0, 0.666 * MaxSeparation) : vec2(0.666 * MaxSeparation, 0.0))).x), 
                                        texNoLod(tex_stereo_depth, u - (yaxis ? vec2(0.0,         MaxSeparation) : vec2(        MaxSeparation, 0.0))).x);
        float rparallax = max(0.0, w_h_height.w + MaxSeparation * (1.0 - 1.0 / (0.5 + rminDepth * (1.0 / STEREO_ZERO_POINT_DEPTH - 0.5))));
        return texNoLod(tex_stereo_fb, u - (yaxis ? vec2(0.0, -rparallax) : vec2(rparallax, 0.0))).xyz;
    }
}
void gatherLeftRightColors(vec2 u, out vec3 lcol, out vec3 rcol)
{
    const bool yaxis = STEREO_YAXIS;
    const float MaxSeparation = STEREO_MAX_SEPARATION;

    const float lminDepth = min(min(texNoLod(tex_stereo_depth, u + (yaxis ? vec2(0.0, 0.500 * MaxSeparation) : vec2(0.500 * MaxSeparation, 0.0))).x, 
                                    texNoLod(tex_stereo_depth, u + (yaxis ? vec2(0.0, 0.666 * MaxSeparation) : vec2(0.666 * MaxSeparation, 0.0))).x), 
                                    texNoLod(tex_stereo_depth, u + (yaxis ? vec2(0.0,         MaxSeparation) : vec2(        MaxSeparation, 0.0))).x);
    float lparallax = max(0.0, w_h_height.w + MaxSeparation * (1.0 - 1.0 / (0.5 + lminDepth * (1.0 / STEREO_ZERO_POINT_DEPTH - 0.5))));
    lcol = texNoLod(tex_stereo_fb, u + (yaxis ? vec2(0.0, -lparallax) : vec2(lparallax, 0.0))).xyz;

    const float rminDepth = min(min(texNoLod(tex_stereo_depth, u - (yaxis ? vec2(0.0, 0.500 * MaxSeparation) : vec2(0.500 * MaxSeparation, 0.0))).x, 
                                    texNoLod(tex_stereo_depth, u - (yaxis ? vec2(0.0, 0.666 * MaxSeparation) : vec2(0.666 * MaxSeparation, 0.0))).x), 
                                    texNoLod(tex_stereo_depth, u - (yaxis ? vec2(0.0,         MaxSeparation) : vec2(        MaxSeparation, 0.0))).x);
    float rparallax = max(0.0, w_h_height.w + MaxSeparation * (1.0 - 1.0 / (0.5 + rminDepth * (1.0 / STEREO_ZERO_POINT_DEPTH - 0.5))));
    rcol = texNoLod(tex_stereo_fb, u - (yaxis ? vec2(0.0, -rparallax) : vec2(rparallax, 0.0))).xyz;
}

#else
// TODO use image gather instead of texture sampling. It is faster and more clear to the intent
vec3 gatherEyeColor(vec2 u, bool isLeft)
{
    return texture2DArrayLod(tex_stereo_fb, vec3(u.x, u.y, isLeft ? 0.0 : 1.0), 0.).rgb;
}
void gatherLeftRightColors(vec2 u, out vec3 lcol, out vec3 rcol)
{
    lcol = texture2DArrayLod(tex_stereo_fb, vec3(u.x, u.y, 0.), 0.).xyz;
    rcol = texture2DArrayLod(tex_stereo_fb, vec3(u.x, u.y, 1.), 0.).xyz;
}
#endif


// ////////////////////////////////////////////////////////////////////////////
// Anaglyph image composition

uniform mat4 Stereo_LeftMat;
uniform mat4 Stereo_RightMat;

// Perform dynamic desaturation based on filter colors for limited retinal rivalry
// Retinal rivalry happens when something is seen by one eye and not the other. In turn, this happens when a color passes through 
// only one of the eye filter (for example full saturated red, 100% passing through left and 0% through right). The trick is to 
// identify this situation by applying the filters to incoming colors and if needed adjust the colors to allow them to pass the 2 
// filters instead of 1 (at least a little to satisfy the viewer brain and allow it to merge the 2 images, solving the retinal 
// rivalry). There are two ways to do this:
// - desaturation of the incoming colors (turn to gray), accounting for the loss of perceived luminance (Helmholtz-Kohlrausch effect)
// - hue shifting which allows to keep saturated colors, but not the right ones (tested but without generally satisfying results).
uniform vec4 Stereo_LeftLuminance_Gamma;
uniform vec4 Stereo_RightLuminance_DynDesat;

/*vec3 HueShift(const vec3 color, const float dhue)
{
    float s = sin(dhue);
    float c = cos(dhue);
    return (color * c) + (color * s) * mat3(
        vec3(0.167444, 0.329213, -0.496657),
        vec3(-0.327948, 0.035669, 0.292279),
        vec3(1.250268, -1.047561, -0.202707)
    ) + dot(vec3(0.299, 0.587, 0.114), color) * (1.0 - c);
}*/

// lCol/rCol are expected to be in linear color space
void DynamicDesatAnaglyph(const vec3 lCol, const vec3 rCol, out vec3 lDesatCol, out vec3 rDesatCol)
{
    const float left2LeftLum = dot(lCol, Stereo_LeftLuminance_Gamma.xyz);
    const float left2RightLum = dot(lCol, Stereo_RightLuminance_DynDesat.xyz);
    const float right2LeftLum = dot(rCol, Stereo_LeftLuminance_Gamma.xyz);
    const float right2RightLum = dot(rCol, Stereo_RightLuminance_DynDesat.xyz);
    const float leftLum = left2LeftLum + left2RightLum;
    const float rightLum = right2LeftLum + right2RightLum;
    const float leftDesat = Stereo_RightLuminance_DynDesat.w * abs((left2LeftLum - left2RightLum) / (leftLum + 0.0001));
    const float rightDesat = Stereo_RightLuminance_DynDesat.w * abs((right2LeftLum - right2RightLum) / (rightLum + 0.0001));
    lDesatCol = mix(lCol, vec3_splat(leftLum), leftDesat);
    rDesatCol = mix(rCol, vec3_splat(rightLum), rightDesat);
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
vec3 LinearAnaglyph(const vec3 lCol, const vec3 rCol)
{
    const vec3 color = (mul(vec4(lCol, 1.), Stereo_LeftMat) + mul(vec4(rCol, 1.), Stereo_RightMat)).rgb;
    // Uncomment to vizualize parts that should exhibit ghosting (clamping always result in ghosting)
    //if (color.r < 0.0 || color.g < 0.0 || color.b < 0.0 || color.r > 1.0 || color.g > 1.0 || color.b > 1.0)
    //    color = vec3(0.0);
    return color;
}


// Compose anaglyph by applying John Einselen's contrast and deghosting method
// see http://iaian7.com/quartz/AnaglyphCompositing & vectorform.com
// lCol/rCol are expected to be in sRGB color space
uniform vec4 Stereo_DeghostGamma; // Gamma adjustment (depending on glasses)
uniform mat4 Stereo_DeghostFilter; // Cross eye deghosting filter (depending on glasses)
vec3 DeghostAnaglyph(const vec3 lCol, const vec3 rCol)
{
    const vec3 color = LinearAnaglyph(lCol, rCol);
    //return mul(pow(color, Stereo_DeghostGamma.rgb), to_vec3x3(Stereo_DeghostFilter));
    return mul(pow(color, Stereo_DeghostGamma.rgb), mat3(Stereo_DeghostFilter[0].xyz, Stereo_DeghostFilter[1].xyz, Stereo_DeghostFilter[2].xyz));
}




void main()
{
   #if defined(TB)
      gl_FragColor = vec4(gatherEyeColor(vec2(v_texcoord0.x, v_texcoord0.y * 2.0), v_texcoord0.y < 0.5), 1.0);
   #elif defined(SBS)
      gl_FragColor = vec4(gatherEyeColor(vec2(v_texcoord0.x * 2.0, v_texcoord0.y), v_texcoord0.x < 0.5), 1.0);
   #elif defined(INT)
      gl_FragColor = vec4(gatherEyeColor(v_texcoord0, fract(gl_FragCoord.y*0.5) < 0.5), 1.0);
   #elif defined(FLIPPED_INT)
      gl_FragColor = vec4(gatherEyeColor(v_texcoord0, fract(gl_FragCoord.y*0.5) >= 0.5), 1.0);
   #elif defined(ANAGLYPH)
      vec3 lCol, rCol;
      gatherLeftRightColors(v_texcoord0, lCol, rCol);
      #if defined(DEGHOST)
         gl_FragColor = vec4(DeghostAnaglyph(lCol, rCol), 1.0);
      #else
         #ifdef SRGB
            lCol = InvGamma(lCol);
            rCol = InvGamma(rCol);
         #elif defined(GAMMA)
            lCol = pow(lCol, vec3(Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w));
            rCol = pow(rCol, vec3(Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w));
         #endif
         #ifdef DYNDESAT
            vec3 lColDesat, rColDesat;
            DynamicDesatAnaglyph(lCol, rCol, lColDesat, rColDesat);
            lCol = lColDesat;
            rCol = rColDesat;
         #endif
         vec3 color = LinearAnaglyph(lCol, rCol);
         #ifdef SRGB
            gl_FragColor = vec4(FBGamma(color), 1.0);
         #elif defined(GAMMA)
            gl_FragColor = vec4(pow(color, vec3(1./Stereo_LeftLuminance_Gamma.w, 1./Stereo_LeftLuminance_Gamma.w, 1./Stereo_LeftLuminance_Gamma.w)), 1.0);
         #endif
      #endif
   #endif
}
