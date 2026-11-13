// license:GPLv3+

$input v_rayOriginView, v_rayDirView, v_rayOriginObj, v_rayDirObj // camera ray in view space and in unit sphere space (same ray parameter)
#ifdef STEREO
    $input v_eye
#endif


#include "common.sh"

#define NUM_BALL_LIGHTS 8

uniform mat4 orientation; // unit sphere object space to world space (rot * scale * translate)
#ifdef STEREO
    uniform mat4 matView[2];
    uniform mat4 matWorldView[2];
    uniform mat4 matWorldViewInverse[2];
    uniform mat4 matPrevViewProj[2];
    uniform mat4 matProj[2];
    uniform mat4 ballViewToObj[2];
    // FIXME v_eye needs to be flat interpolated, but if declared as such in varying.def.sc, DX11 will fail (OpenGL/Vulkan are good)
    #define mView             matView[int(round(v_eye))]
    #define mWorldView        matWorldView[int(round(v_eye))]
    #define mWorldViewInverse matWorldViewInverse[int(round(v_eye))]
    #define mPrevViewProj     matPrevViewProj[int(round(v_eye))]
    #define mProj             matProj[int(round(v_eye))]
    #define mBallViewToObj    ballViewToObj[int(round(v_eye))]
#else
    uniform mat4 matView;
    uniform mat4 matWorldView;
    uniform mat4 matWorldViewInverse;
    uniform mat4 matPrevViewProj;
    uniform mat4 matProj;
    uniform mat4 ballViewToObj;
    #define mView             matView
    #define mWorldView        matWorldView
    #define mWorldViewInverse matWorldViewInverse
    #define mPrevViewProj     matPrevViewProj
    #define mProj             matProj
    #define mBallViewToObj    ballViewToObj
#endif
#ifdef CLIP
    uniform vec4 clip_plane;
#endif

SAMPLER2D      (tex_ball_color, 0);     // base texture (used as a reflection map)
SAMPLER2D      (tex_env, 1);            // envmap
SAMPLER2D      (tex_diffuse_env, 2);    // envmap radiance
SAMPLER2D      (tex_ball_decal, 3);     // ball decal
SAMPLER2DSTEREO(tex_ball_playfield, 4); // playfield probe, mixed with based texture

uniform vec4 u_basic_shade_mode;
#define doMetal             (u_basic_shade_mode.x)
#define doNormalMapping     (u_basic_shade_mode.y)
#define doRefractions       (u_basic_shade_mode.z != 0.0)

#include "material.sh"

uniform vec4  invTableRes_reflection;
uniform vec4  w_h_disableLighting; 
#define disableLighting (w_h_disableLighting.z != 0.)

#ifdef STEREO
vec3 ballLightLoop(const vec3 pos, vec3 N, vec3 V, vec3 diffuse, vec3 glossy, const vec3 specular, const float edge, const bool is_metal, const float v_eye)
#else
vec3 ballLightLoop(const vec3 pos, vec3 N, vec3 V, vec3 diffuse, vec3 glossy, const vec3 specular, const float edge, const bool is_metal)
#endif
{
   // N and V must be already normalized by the caller

   // N = normalize(N);
   // V = normalize(V);

   // normalize BRDF layer inputs //!! use diffuse = (1-glossy)*diffuse instead?
   const float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));
   const float glossyMax = max(glossy.x,max(glossy.y,glossy.z));
   const float specularMax = max(specular.x,max(specular.y,specular.z)); //!! not needed as 2nd layer only so far
   const float sum = diffuseMax + glossyMax; //+ specularMax
   if(sum > 1.0)
   {
      const float invsum = 1.0/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      //specular *= invsum;
   }

   vec3 color = vec3(0.0, 0.0, 0.0);

   BRANCH if((!is_metal && (diffuseMax > 0.0)) || (glossyMax > 0.0))
      for(int i = 0; i < NUM_LIGHTS + NUM_BALL_LIGHTS; i++)  
          #ifdef STEREO
             color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal, v_eye); // no clearcoat needed as only pointlights so far
          #else
             color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal); // no clearcoat needed as only pointlights so far
          #endif

   BRANCH if(!is_metal && (diffuseMax > 0.0))
      color += DoEnvmapDiffuse(normalize(mul(vec4(N, 0.0), mView).xyz), diffuse); // trafo back to world for lookup into world space envmap // actually: mul(vec4(N, 0.0), matViewInverseInverseTranspose)

   if(specularMax > 0.0)
      color += specular; //!! blend? //!! Fresnel with 1st layer?

   return color;
}

void main()
{
    // Analytic raycast of the unit sphere, in its own space (so the anti stretch ellipsoid comes for free): exact silhouette, surface point and normal
    const vec3 o_o = v_rayOriginObj;
    const vec3 d_o = v_rayDirObj;
    // Numerically stable form, for balls small relative to their distance: with a normalized direction, the squared
    // distance of the ray to the center is computed directly instead of as a difference of large terms
    const float invLenD = inversesqrt(dot(d_o, d_o));
    const vec3 dir = d_o * invLenD;
    const float b = dot(o_o, dir);
    const vec3 h = o_o - b * dir; // ray point closest to the center
    const float discr = 1.0 - dot(h, h);
    // Analytic silhouette antialiasing: fade out the coverage over ~1 pixel around the outline (discr is 0 there)
    const float coverage = saturate(discr / max(fwidth(discr), 1e-7) + 0.5);
    // Evaluated before the discards (for a ray missing the sphere, at its closest point): the decal gradients need every
    // pixel of each 2x2 quad
    const float t_dir = -b - sqrt(max(discr, 0.0)); // along the normalized direction
    const vec3 n_obj = normalize(o_o + t_dir * dir); // hit point on the unit sphere = exact normal
    const float t_hit = t_dir * invLenD; // ray parameter, shared with view space

    // Decal UV, reproducing the ball mesh UVs, with explicit gradients: implicit ones jump where the longitude wraps
    // (at +-pi, and u at 1), selecting the smallest mip along a meridian. The longitude gradient is derived from the
    // normal instead, d(atan2(y, x)) = (x dy - y dx) / (x^2 + y^2), which has no wrap (clamped to pi like a wrapped
    // difference, as it grows unbounded at the poles)
    const vec2 decalUV = vec2(fract(0.5935 + atan2(n_obj.y, n_obj.x) * 0.1339), 0.5 + asin(clamp(n_obj.z, -1., 1.)) * 0.2678);
    const vec2 nxyDx = dFdx(n_obj.xy);
    const vec2 nxyDy = dFdy(n_obj.xy);
    const float invLen2 = 1.0 / max(dot(n_obj.xy, n_obj.xy), 1e-6);
    const float dLonX = clamp((n_obj.x * nxyDx.y - n_obj.y * nxyDx.x) * invLen2, -3.14159265, 3.14159265);
    const float dLonY = clamp((n_obj.x * nxyDy.y - n_obj.y * nxyDy.x) * invLen2, -3.14159265, 3.14159265);
    const vec2 decalDx = vec2(dLonX * 0.1339, dFdx(decalUV.y));
    const vec2 decalDy = vec2(dLonY * 0.1339, dFdy(decalUV.y));

    if (coverage <= 0.0)
        discard;
    if (t_hit <= 0.0) // Before the ray origin, that is in front of the near plane: clipped like the rest of the frame
        discard;
    #ifdef CLIP
    // Test the clip plane at the analytic surface point (not at the impostor quad depth)
    if (dot(clip_plane, vec4(mul(orientation, vec4(n_obj, 1.0)).xyz, 1.0)) < 0.0)
       discard;
    #endif
    const vec3 hit_view = v_rayOriginView + t_hit * v_rayDirView;

    // Exact per pixel depth of the analytic hit
    const vec4 hit_clip = mul(mProj, vec4(hit_view, 1.0));
    #if BGFX_SHADER_LANGUAGE_GLSL || BGFX_SHADER_LANGUAGE_ESSL
        gl_FragDepth = 0.5 + 0.5 * (hit_clip.z / hit_clip.w);
    #else
        gl_FragDepth = hit_clip.z / hit_clip.w;
    #endif

    // Normals transform by the transpose of the inverse transform (normalized as it is not orthonormal with anti stretch)
    const vec3 N = normalize(mul(vec4(n_obj, 0.0), mBallViewToObj).xyz);
    const vec3 V = normalize(/*camera=0,0,0,1*/-hit_view);
    const vec3 R = reflect(V, N);

    vec3 ballImageColor;
    const float edge = dot(V, R);
    // edge falloff to reduce aliasing on edges (picks smaller mipmap -> more blur)
    const float lod = (edge > 0.6) ? edge*(6.0*1.0/0.4)-(6.0*0.6/0.4) : 0.0;
    #ifdef EQUIRECTANGULAR
        // Equirectangular Map Reflections
        // trafo back to world for lookup into world space envmap
        // mView is always an orthonormal matrix, so no need to normalize after transform
        const vec3 rv = /*normalize*/(mul(vec4(-R,0.0), mView).xyz);
        const vec2 uv = ray_to_equirectangular_uv(rv);
        ballImageColor = texture2DLod(tex_ball_color, uv, lod).rgb;
    #else
        // Spherical Map Reflections
        // calculate the intermediate value for the final texture coords. found here http://www.ozone3d.net/tutorials/glsl_texturing_p04.php
        const float m = (1.0 - R.z > 0.) ? 0.3535533905932737622 * inversesqrt(1.0 - R.z) : 0.; // 0.353...=0.5/sqrt(2)
        const vec2 uv = vec2(0.5 - m * R.x, 0.5 - m * R.y);
        ballImageColor = texture2DLod(tex_ball_color, uv, lod).rgb;
    #endif

    const vec4 decalColorT = texture2DGrad(tex_ball_decal, decalUV, decalDx, decalDy);
    vec3 decalColor = decalColorT.rgb;
    #ifndef DECAL
        // decal texture is an alpha scratch texture and must be added to the ball texture
        // the strength of the scratches totally rely on the alpha values.
        decalColor *= decalColorT.a;
        ballImageColor += decalColor;
    #else
        ballImageColor = ScreenHDR(ballImageColor, decalColor);

    #endif

    BRANCH if (disableLighting)
    {
       gl_FragColor = vec4(ballImageColor,cBase_Alpha.a * coverage);
       return;
    }

    #ifndef DECAL
        ballImageColor *= fenvEmissionScale_TexWidth.x;
    #else
        ballImageColor *= 0.5*fenvEmissionScale_TexWidth.x; //!! 0.5=magic
    #endif

    // No need to normalize here since the matWorldView matrix is normal (world is identity and view is always orthonormal)
    // No need to use a dedicated 'normal' matrix since the matWorldView is orthonormal (world is identity and view is always orthonormal)
    //const vec3 playfield_normal = normalize(mul(vec4(0.,0.,1.,0.), mWorldViewInverse).xyz); //!! normalize necessary? // actually: mul(vec4(0.,0.,1.,0.), mWorldViewInverseTranspose), but optimized to save one matrix
    const vec3 playfield_normal = mul(mWorldView, vec4(0.,0.,1.,0.)).xyz;
    //const vec3 playfield_normal = mWorldView[2].xyz;
    const float NdotR = dot(playfield_normal, R);

    const vec3 playfield_p0 = mul(mWorldView, vec4(/*playfield_pos=*/0.,0.,0.,1.0)).xyz;
    //const vec3 playfield_p0 = mWorldView[3].xyz;
    const float t = dot(playfield_normal, hit_view - playfield_p0) / NdotR;
    const vec3 playfield_hit = hit_view - t * R;

    // New implementation: use previous frame as a reflection probe instead of computing a simplified render (this is faster and more accurate, support playfield mesh, lighting,... but there can be artefacts, with self reflection,...)
    // Move the hit point back to world space, then reproject it with the view-projection the previous frame was rendered with
    const vec4 playfield_hit_world = mul(mWorldViewInverse, vec4(playfield_hit, 1.0));
    const vec4 proj = mul(mPrevViewProj, playfield_hit_world);
    #if TEX_V_IS_UP
        // OpenGL and OpenGL ES have reversed render targets
        const vec2 uvp = vec2(0.5, 0.5) + proj.xy               * (0.5 / proj.w);
    #else
        const vec2 uvp = vec2(0.5, 0.5) + vec2(proj.x, -proj.y) * (0.5 / proj.w);
    #endif
    const vec3 playfieldColor = 0.25 * (
          texStereo(tex_ball_playfield, uvp + vec2(w_h_disableLighting.x, 0.)).rgb
        + texStereo(tex_ball_playfield, uvp - vec2(w_h_disableLighting.x, 0.)).rgb
        + texStereo(tex_ball_playfield, uvp + vec2(0., w_h_disableLighting.y)).rgb
        + texStereo(tex_ball_playfield, uvp - vec2(0., w_h_disableLighting.y)).rgb
    ); // a bit of supersampling, not strictly needed, but a bit better and not that costly

    // we don't clamp sampling outside the playfield (costly and no real visual impact)
    // const vec2 uv = mul(mWorldViewInverse, vec4(playfield_hit, 1.0)).xy * invTableRes_reflection.xy;
    // && !(uv.x < 0.1 && uv.y < 0.1 && uv.x > 0.9 && uv.y > 0.9)
    BRANCH if (!(uvp.x < 0. || uvp.x > 1. || uvp.y < 0. || uvp.y > 1.) // outside of previous render => discard (we could use sampling techniques to optimize a bit)
            && !(t <= 0.)) // t < 0.0 may happen in some situation where ball intersects the playfield and the reflected point is inside the ball (like in kicker)
    {
        // NdotR allows to fade between playfield (down) and environment (up)
        ballImageColor = mix(ballImageColor, playfieldColor, smoothstep(0.0, 0.15, NdotR) * invTableRes_reflection.z);
    }

    // We can face infinite reflections (ball->playfield->ball->playfield->...) which would overflow, or very bright dots that would cause lighting artefacts, so we saturate to an arbitrary value
    ballImageColor = min(ballImageColor, vec3(15., 15., 15.));

    vec3 diffuse = cBase_Alpha.rgb*0.075;
    #ifndef DECAL
        diffuse *= decalColor; // scratches make the material more rough
    #endif

    const vec3 glossy = max(diffuse*2.0, vec3(0.1,0.1,0.1)); //!! meh

    vec3 specular = ballImageColor * cBase_Alpha.rgb; //!! meh, too, as only added in ballLightLoop anyhow
    #ifndef DECAL
       specular *= vec3(1.,1.,1.)-decalColor; // see above
    #endif

    #ifdef STEREO
       gl_FragColor.rgb = ballLightLoop(hit_view, N, V, diffuse, glossy, specular, 1.0, false, v_eye);
    #else
       gl_FragColor.rgb = ballLightLoop(hit_view, N, V, diffuse, glossy, specular, 1.0, false);
    #endif
    gl_FragColor.a = cBase_Alpha.a * coverage;
}
