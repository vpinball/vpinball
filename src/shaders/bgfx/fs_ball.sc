// license:GPLv3+

$input v_tablePos // view space point on the camera ray (unnormalized direction)
#ifdef STEREO
    $input v_eye
#endif


#include "common.sh"

#define NUM_BALL_LIGHTS 8

uniform mat4 orientation;    // unit sphere object space to world space (rot * scale * translate)
uniform mat4 orientationInv; // world space to unit sphere object space
#ifdef STEREO
    uniform mat4 matView[2];
    uniform mat4 matWorldView[2];
    uniform mat4 matWorldViewInverse[2];
    uniform mat4 matRotViewProj[2];
    uniform vec4 cameraPosWorld[2];
    uniform mat4 matPrevViewProj[2];
    // FIXME v_eye needs to be flat interpolated, but if declared as such in varying.def.sc, DX11 will fail (OpenGL/Vulkan are good)
    #define mView             matView[int(round(v_eye))]
    #define mWorldView        matWorldView[int(round(v_eye))]
    #define mWorldViewInverse matWorldViewInverse[int(round(v_eye))]
    #define mRotViewProj      matRotViewProj[int(round(v_eye))]
    #define cCameraPosWorld   cameraPosWorld[int(round(v_eye))].xyz
    #define mPrevViewProj     matPrevViewProj[int(round(v_eye))]
#else
    uniform mat4 matView;
    uniform mat4 matWorldView;
    uniform mat4 matWorldViewInverse;
    uniform mat4 matRotViewProj;
    uniform vec4 cameraPosWorld;
    uniform mat4 matPrevViewProj;
    #define mView             matView
    #define mWorldView        matWorldView
    #define mWorldViewInverse matWorldViewInverse
    #define mRotViewProj      matRotViewProj
    #define cCameraPosWorld   cameraPosWorld.xyz
    #define mPrevViewProj     matPrevViewProj
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
    // Analytic raycast of the unit sphere: v_tablePos carries a view space point on the camera ray
    // (the impostor vertex shader unprojects the quad corner through the inverse projection). The
    // ray is transformed to unit sphere space by the inverse view and inverse orientation, then
    // intersected to get the exact silhouette, surface point and normal. Since the ray is moved
    // by orientationInv, this supports the anti stretch ellipsoid for free.
    const vec3 o_w = mul(mWorldViewInverse, vec4(0., 0., 0., 1.)).xyz;
    const vec3 d_w = mul(mWorldViewInverse, vec4(v_tablePos, 0.)).xyz;
    const vec3 o_o = mul(orientationInv, vec4(o_w, 1.)).xyz;
    const vec3 d_o = mul(orientationInv, vec4(d_w, 0.)).xyz;
    const float a = dot(d_o, d_o);
    const float b = dot(o_o, d_o);
    const float c = dot(o_o, o_o) - 1.0;
    const float discr = b * b - a * c;
    // Analytic silhouette antialiasing: fade out the coverage over ~1 pixel around the outline
    // (discr/a is the squared projection of the hit on the ray, = 0 at the sphere outline)
    const float coverage = saturate(discr / (a * max(fwidth(discr / a), 1e-7)) + 0.5);
    if (coverage <= 0.0)
        discard;
    const float t_hit = (-b - sqrt(max(discr, 0.0))) / a;
    if (t_hit <= 0.0)
        discard;
    const vec3 n_obj = normalize(o_o + t_hit * d_o); // hit point on the unit sphere = exact normal
    const vec3 hit_world = mul(orientation, vec4(n_obj, 1.0)).xyz;
    const vec3 hit_view = mul(mWorldView, vec4(hit_world, 1.0)).xyz;

    #ifdef CLIP
    // Test the clip plane at the analytic surface point (not at the impostor quad depth)
    if (dot(clip_plane, vec4(hit_world, 1.0)) < 0.0)
       discard;
    #endif

    // Exact per pixel depth of the analytic hit (uses the same camera relative transform as the rest of the frame)
    const vec4 hit_clip = mul(mRotViewProj, vec4(hit_world - cCameraPosWorld, 1.0));
    #if BGFX_SHADER_LANGUAGE_GLSL || BGFX_SHADER_LANGUAGE_ESSL
        gl_FragDepth = 0.5 + 0.5 * (hit_clip.z / hit_clip.w);
    #else
        gl_FragDepth = hit_clip.z / hit_clip.w;
    #endif

    // Decal UV, reproducing the mapping baked in the ball mesh UVs
    const vec2 decalUV = vec2(fract(0.5935 + atan2(n_obj.y, n_obj.x) * 0.1339),
                              0.5 + asin(clamp(n_obj.z, -1., 1.)) * 0.2678);

    const vec3 nspin = mul(orientation, vec4(n_obj, 0.0)).xyz;
    // Needs to use a 'normal' matrix, and to normalize since we allow non uniform stretching, therefore mWorldView is not orthonormal
    const vec3 N = normalize(mul(vec4(nspin, 0.0), mWorldViewInverse).xyz); // actually: mul(vec4(nspin,0.), mWorldViewInverseTranspose), but optimized to save one matrix
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

    const vec4 decalColorT = texture2D(tex_ball_decal, decalUV);
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
