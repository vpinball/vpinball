$input v_worldPos, v_normal, v_texcoord0

#include "bgfx_shader.sh"
#include "common.sh"

#define NUM_BALL_LIGHTS 8

uniform mat4 matWorldViewProj;
uniform mat4 matWorldView;
uniform mat4 matWorldViewInverse;
// uniform mat4 matWorldViewInverseTranspose; // matWorldViewInverse used instead and multiplied from other side
uniform mat4 matView;
// uniform mat4 matViewInverseInverseTranspose; // matView used instead and multiplied from other side

SAMPLER2D(tex_ball_color, 0); // base texture
SAMPLER2D(tex_env, 1); // envmap
SAMPLER2D(tex_diffuse_env, 2); // envmap radiance
SAMPLER2D(tex_ball_decal, 3); // ball decal
SAMPLER2D(tex_ball_playfield, 4); // playfield

uniform vec4 u_basic_shade_mode;
#define doMetal       		(u_basic_shade_mode.x)
#define doNormalMapping     (u_basic_shade_mode.y)
#define doReflections       (u_basic_shade_mode.z)

#include "material.sh"

uniform vec4  invTableRes_playfield_height_reflection;
// uniform vec4  reflection_ball_playfield; // actually float but extended to vec4 for BGFX
uniform vec4  disableLighting; // float extended to vec4 for BGFX 

vec3 ballLightLoop(const vec3 pos, vec3 N, vec3 V, vec3 diffuse, vec3 glossy, const vec3 specular, const float edge, const bool is_metal)
{
   // normalize input vectors for BRDF evals
   N = normalize(N);
   V = normalize(V);

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

   //if(dot(N,V) < 0.0) //!! flip normal in case of wrong orientation? (backside lighting)
   //   N = -N;

   vec3 color = vec3(0.0, 0.0, 0.0);

   BRANCH if((!is_metal && (diffuseMax > 0.0)) || (glossyMax > 0.0))
   {
      for(int i = 0; i < NUM_LIGHTS + NUM_BALL_LIGHTS; i++)  
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal); // no clearcoat needed as only pointlights so far
   }

   BRANCH if(!is_metal && (diffuseMax > 0.0))
      color += DoEnvmapDiffuse(normalize(mul(vec4(N, 0.0), matView).xyz), diffuse); // trafo back to world for lookup into world space envmap // actually: mul(vec4(N, 0.0), matViewInverseInverseTranspose)

   if(specularMax > 0.0)
      color += specular; //!! blend? //!! Fresnel with 1st layer?

   return color;
}

vec3 PFDoPointLight(const vec3 pos, const vec3 N, const vec3 diffuse, const int i) 
{
   //!! do in vertex shader?! or completely before?!
#if enable_VR
   const vec3 lightDir = (mul(matView, vec4(lightPos[i].xyz, 1.0)).xyz - pos) / fSceneScale; // In VR we need to scale to the overall scene scaling
#else
   const vec3 lightDir = mul(matView, vec4(lightPos[i].xyz, 1.0)).xyz - pos;
#endif
   const vec3 L = normalize(lightDir);
   const float NdotL = dot(N, L);
   // compute diffuse color (lambert)
   const vec3 Out = (NdotL > 0.0) ? diffuse * NdotL : vec3(0.0,0.0,0.0);

   const float sqrl_lightDir = dot(lightDir,lightDir); // tweaked falloff to have ranged lightsources
   float fAtten = saturate(1.0 - sqrl_lightDir*sqrl_lightDir/(cAmbient_LightRange.w*cAmbient_LightRange.w*cAmbient_LightRange.w*cAmbient_LightRange.w)); //!! pre-mult/invert cAmbient_LightRange.w?
   fAtten = fAtten*fAtten/(sqrl_lightDir + 1.0);

   return Out * lightEmission[i].xyz * fAtten;
}

vec3 PFlightLoop(const vec3 pos, const vec3 N, const vec3 diffuse)
{
   const float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));

   vec3 color = vec3(0.0,0.0,0.0);

   BRANCH if (diffuseMax > 0.0)
   {
      for (int i = 0; i < NUM_LIGHTS + NUM_BALL_LIGHTS; i++)
         color += PFDoPointLight(pos, N, diffuse, i);

      color += DoEnvmapDiffuse(vec3(0.,0.,1.), diffuse); // directly wire world space playfield normal
   }

   return color;
}

void main()
{
    const vec3 v = normalize(/*camera=0,0,0,1*/-v_worldPos.xyz);
    const vec3 r = reflect(v, normalize(v_normal.xyz));
    // calculate the intermediate value for the final texture coords. found here http://www.ozone3d.net/tutorials/glsl_texturing_p04.php
    const float  m = (r.z + 1.0 > 0.) ? 0.3535533905932737622 * inversesqrt(r.z + 1.0) : 0.; // 0.353...=0.5/sqrt(2)
    const float edge = dot(v, r);
    const float lod = (edge > 0.6) ? // edge falloff to reduce aliasing on edges (picks smaller mipmap -> more blur)
		edge*(6.0*1.0/0.4)-(6.0*0.6/0.4) :
		0.0;

	 #ifdef CAB
    const vec2 uv0 = vec2(r.y*-m + 0.5, r.x*-m + 0.5);
    #else
    const vec2 uv0 = vec2(r.x*-m + 0.5, r.y*m + 0.5);
	 #endif
    vec3 ballImageColor = texture2DLod(tex_ball_color, uv0, lod).xyz;

    const vec4 decalColorT = texture2D(tex_ball_decal, v_texcoord0);
    vec3 decalColor = decalColorT.xyz;
	 #ifdef DECAL
       ballImageColor = ScreenHDR(ballImageColor, decalColor);
    #else
       // decal texture is an alpha scratch texture and must be added to the ball texture
       // the strength of the scratches totally rely on the alpha values.
       decalColor *= decalColorT.a;
       ballImageColor += decalColor;
	 #endif

    BRANCH if (disableLighting.x != 0.0)
    {
       gl_FragColor = vec4(ballImageColor,cBase_Alpha.a);
       return;
    }

	#ifdef DECAL
       ballImageColor *= 0.5*fenvEmissionScale_TexWidth.x; //!! 0.5=magic
    #else
       ballImageColor *= fenvEmissionScale_TexWidth.x;
	#endif

    const vec3 playfield_normal = normalize(mul(vec4(0.,0.,1.,0.), matWorldViewInverse).xyz); //!! normalize necessary? // actually: mul(vec4(0.,0.,1.,0.), matWorldViewInverseTranspose), but optimized to save one matrix
    const float NdotR = dot(playfield_normal,r);

    vec3 playfieldColor;
    BRANCH if(/*(reflection_ball_playfield.x > 0.0) && */ (NdotR > 0.0))
    {
       const vec3 playfield_p0 = mul(matWorldView, vec4(/*playfield_pos=*/0.,0.,invTableRes_playfield_height_reflection.z, 1.0)).xyz;
       const float t = dot(playfield_normal, v_worldPos.xyz - playfield_p0) / NdotR;
       const vec3 playfield_hit = v_worldPos.xyz - t*r;

       const vec2 uv = mul(matWorldViewInverse, vec4(playfield_hit, 1.0)).xy * invTableRes_playfield_height_reflection.xy;
       playfieldColor = (t < 0.) ? vec3(0.,0.,0.) // happens for example when inside kicker
                                 : texture2DLod(tex_ball_playfield, uv, 0.0).xyz * invTableRes_playfield_height_reflection.w; //!! rather use screen space sample from previous frame??

       //!! hack to get some lighting on reflection sample, but only diffuse, the rest is not setup correctly anyhow
       playfieldColor = PFlightLoop(playfield_hit, playfield_normal, playfieldColor);
       // previous VPVR only shading to be removed after tests
       // vec3 lightLoop_normal = mul(matWorldView, vec4(0.,0.,1.,0.)).xyz;
       //playfieldColor = PFlightLoop(playfield_hit, lightLoop_normal, playfieldColor);

       //!! magic falloff & weight the rest in from the ballImage
       const float weight = NdotR*NdotR;
       playfieldColor = mix(ballImageColor,playfieldColor,weight);
    }
    else
       playfieldColor = ballImageColor;

    vec3 diffuse = cBase_Alpha.xyz*0.075;

	#ifndef DECAL
	diffuse *= decalColor; // scratches make the material more rough
	#endif

    const vec3 glossy = max(diffuse*2.0, vec3(0.1,0.1,0.1)); //!! meh
    vec3 specular = playfieldColor*cBase_Alpha.xyz; //!! meh, too, as only added in ballLightLoop anyhow

	#ifndef DECAL
    specular *= vec3(1.,1.,1.)-decalColor; // see above
	#endif

    gl_FragColor.xyz = ballLightLoop(v_worldPos, v_normal, /*camera=0,0,0,1*/-v_worldPos, diffuse, glossy, specular, 1.0, false);
    gl_FragColor.a = cBase_Alpha.a;
}
