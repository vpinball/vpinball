// license:GPLv3+

$input v_worldPos, v_normal, v_texcoord0
#ifdef STEREO
	$input v_eye
#endif
#ifdef CLIP
	$input v_clipDistance
#endif


#include "common.sh"

#define NUM_BALL_LIGHTS 8

uniform mat4 matView;
uniform mat4 matWorldView;
uniform mat4 matWorldViewInverse;
#ifdef STEREO
	uniform mat4 matWorldViewProj[2];
	uniform mat4 matProj[2];
	// FIXME v_eye needs to be flat interpolated, but if declared as such in varying.def.sc, DX11 will fail (OpenGL/Vulkan are good)
	#define mProj matProj[int(round(v_eye))]
#else
	uniform mat4 matWorldViewProj;
	uniform mat4 matProj;
	#define mProj matProj
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

vec3 ballLightLoop(const vec3 pos, vec3 N, vec3 V, vec3 diffuse, vec3 glossy, const vec3 specular, const float edge, const bool is_metal)
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
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal); // no clearcoat needed as only pointlights so far

   BRANCH if(!is_metal && (diffuseMax > 0.0))
      color += DoEnvmapDiffuse(normalize(mul(matView, vec4(N, 0.0)).xyz), diffuse); // trafo back to world for lookup into world space envmap // actually: mul(vec4(N, 0.0), matViewInverseInverseTranspose)

   if(specularMax > 0.0)
      color += specular; //!! blend? //!! Fresnel with 1st layer?

   return color;
}

#ifndef CLIP
EARLY_DEPTH_STENCIL
#endif
void main()
{
    #ifdef CLIP
    if (v_clipDistance < 0.0)
       discard;
    #endif

    const vec3 V = normalize(/*camera=0,0,0,1*/-v_worldPos.xyz);
    const vec3 N = normalize(v_normal.xyz);
    const vec3 R = reflect(V, N);

    vec3 ballImageColor;
    const float edge = dot(V, R);
    // edge falloff to reduce aliasing on edges (picks smaller mipmap -> more blur)
    const float lod = (edge > 0.6) ? edge*(6.0*1.0/0.4)-(6.0*0.6/0.4) : 0.0;
    #ifdef EQUIRECTANGULAR
        // Equirectangular Map Reflections
        // trafo back to world for lookup into world space envmap
        // matView is always an orthonormal matrix, so no need to normalize after transform
        const vec3 rv = /*normalize*/(mul(vec4(-R,0.0), matView).xyz);
        const vec2 uv = ray_to_equirectangular_uv(rv);
        ballImageColor = texture2DLod(tex_ball_color, uv, lod).rgb;
    #else
        // Spherical Map Reflections
        // calculate the intermediate value for the final texture coords. found here http://www.ozone3d.net/tutorials/glsl_texturing_p04.php
        const float m = (1.0 - R.z > 0.) ? 0.3535533905932737622 * inversesqrt(1.0 - R.z) : 0.; // 0.353...=0.5/sqrt(2)
        const vec2 uv = vec2(0.5 - m * R.x, 0.5 - m * R.y);
        ballImageColor = texture2DLod(tex_ball_color, uv, lod).rgb;
    #endif

    const vec4 decalColorT = texture2D(tex_ball_decal, v_texcoord0);
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
       gl_FragColor = vec4(ballImageColor,cBase_Alpha.a);
       return;
    }

    #ifndef DECAL
        ballImageColor *= fenvEmissionScale_TexWidth.x;
    #else
        ballImageColor *= 0.5*fenvEmissionScale_TexWidth.x; //!! 0.5=magic
    #endif

    // No need to normalize here since the matWorldView matrix is normal (world is identity and view is always orthonormal)
    // No need to use a dedicated 'normal' matrix since the matWorldView is orthonormal (world is identity and view is always orthonormal)
    //const vec3 playfield_normal = normalize(mul(vec4(0.,0.,1.,0.), matWorldViewInverse).xyz); //!! normalize necessary? // actually: mul(vec4(0.,0.,1.,0.), matWorldViewInverseTranspose), but optimized to save one matrix
    const vec3 playfield_normal = mul(matWorldView, vec4(0.,0.,1.,0.)).xyz;
    //const vec3 playfield_normal = matWorldView[2].xyz;
    const float NdotR = dot(playfield_normal, R);

    const vec3 playfield_p0 = mul(matWorldView, vec4(/*playfield_pos=*/0.,0.,0.,1.0)).xyz;
    //const vec3 playfield_p0 = matWorldView[3].xyz;
    const float t = dot(playfield_normal, v_worldPos.xyz - playfield_p0) / NdotR;
    const vec3 playfield_hit = v_worldPos.xyz - t * R;

    // New implementation: use previous frame as a reflection probe instead of computing a simplified render (this is faster and more accurate, support playfield mesh, lighting,... but there can be artefacts, with self reflection,...)
    // TODO use previous frame projection instead of the one of the current frame to limit reflection distortion (still this is minimal)
    const vec4 proj = mul(mProj, vec4(playfield_hit, 1.0));
    #if BGFX_SHADER_LANGUAGE_GLSL
		// OpenGL and OpenGL ES have reversed render targets
		const vec2 uvp = vec2(0.5, 0.5) + vec2(proj.x, proj.y) * (0.5 / proj.w);
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
    // const vec2 uv = mul(matWorldViewInverse, vec4(playfield_hit, 1.0)).xy * invTableRes_reflection.xy;
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

    gl_FragColor.rgb = ballLightLoop(v_worldPos.xyz, N, V, diffuse, glossy, specular, 1.0, false);
    gl_FragColor.a = cBase_Alpha.a;
}
