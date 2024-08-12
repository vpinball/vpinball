// license:GPLv3+

$input v_worldPos, v_tablePos, v_normal, v_texcoord0
#ifdef STEREO
	$input v_eye
#endif
#ifdef CLIP
	$input v_clipDistance
#endif

#include "common.sh"

#define NUM_BALL_LIGHTS 0 // just to avoid having too much constant mem allocated

#ifdef TEX
SAMPLER2D(tex_light_color, 0); // base color
#endif
SAMPLER2D(tex_env, 1); // envmap
SAMPLER2D(tex_diffuse_env, 2); // envmap radiance
SAMPLER2D(tex_base_transmission, 3); // bulb light buffer
SAMPLER2D(tex_base_normalmap, 4); // normal map
SAMPLER2D(tex_reflection, 5); // reflections
SAMPLER2D(tex_refraction, 6); // refractions
SAMPLER2D(tex_probe_depth, 7); // depth probe

uniform vec4 cGlossy_ImageLerp; // actually doesn't feature image lerp / BGFX converted to vec4 instead of vec3
uniform vec4 cClearcoat_EdgeAlpha; // actually doesn't feature edge-alpha / BGFX converted to vec4 instead of vec3
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance:  0.5-1.0

uniform vec4 lightColor_intensity;
uniform vec4 lightColor2_falloff_power;
uniform vec4 lightCenter_maxRange;
uniform vec4 lightingOff; // single float, passed as vec4 (BGFX does not support float/int/bool uniforms)

uniform mat4 matView;

#include "material.sh"
#include "ball_shadows.sh"

uniform vec4 u_basic_shade_mode;
#define doMetal       	    (u_basic_shade_mode.x != 0.0)
#define doNormalMapping     (u_basic_shade_mode.y != 0.0)
#define doRefractions       (u_basic_shade_mode.z != 0.0)

// Light don't write to depth buffer, so they can have EARLY_DEPTH_STENCIL even when using discard for clip plane
EARLY_DEPTH_STENCIL void main()
{
    #ifdef CLIP
    if (v_clipDistance < 0.0)
        discard;
    #endif
    #ifdef TEX
        // FIXME check tex_light_color declaration
        vec4 pixel = texNoLod(tex_light_color, v_texcoord0); //!! IN.tex0 abused in backglass mode
    #else
        vec4 pixel = vec4(lightColor_intensity.rgb, 1.0);
    #endif

    vec4 color;
    // no lighting if HUD vertices or passthrough mode
    BRANCH if (lightingOff.x != 0.0)
        color = pixel;
    else
    {
        #ifdef TEX
            pixel.rgb = saturate(pixel.rgb); // could be HDR
        #endif
        const vec3 diffuse  = pixel.rgb*cBase_Alpha.rgb;
        const vec3 glossy   = doMetal ? diffuse : pixel.rgb*cGlossy_ImageLerp.rgb*0.08; //!! use AO for glossy? specular?
        const vec3 specular = cClearcoat_EdgeAlpha.rgb*0.08;
        const float  edge   = doMetal ? 1.0 : Roughness_WrapL_Edge_Thickness.z;

        color.rgb = lightLoop(v_worldPos, normalize(v_normal), normalize(/*camera=0,0,0,1*/-v_worldPos), diffuse, glossy, specular, edge, doMetal); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
        color.a = pixel.a;
    }
    color.a *= cBase_Alpha.a;

    BRANCH if (lightColor_intensity.w != 0.0)
    {
        const float len = length(lightCenter_maxRange.xyz - v_tablePos) * lightCenter_maxRange.w;
        const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
        const vec3 lcolor = mix(lightColor2_falloff_power.rgb, lightColor_intensity.rgb, sqrt(len));
        color += vec4(lcolor*(atten*lightColor_intensity.w), saturate(atten*lightColor_intensity.w));
        #ifdef TEX
            color = OverlayHDR(pixel, color); // could be HDR //!! have mode to choose: if simple mode picked and surface images match then can skip lighting texel above and JUST alpha blend with this here
            color = ScreenHDR(pixel, color);
        #endif
    }

    #ifdef BALLSHADOW
       const vec3 light_dir = v_tablePos - lightCenter_maxRange.xyz;
       const float light_dist = length(light_dir);
       color.rgb *= get_light_ball_shadow(lightCenter_maxRange.xyz, light_dir, light_dist);
    #endif

    gl_FragColor = color;
}
