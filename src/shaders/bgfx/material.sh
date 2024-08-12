// license:GPLv3+

//
// Lights
//

#define NUM_LIGHTS 2

#define iLightPointNum NUM_LIGHTS
#define iLightPointBallsNum (NUM_LIGHTS+NUM_BALL_LIGHTS)

#if iLightPointBallsNum == iLightPointNum // basic shader
uniform vec4 basicLightPos[NUM_LIGHTS + NUM_BALL_LIGHTS];
uniform vec4 basicLightEmission[NUM_LIGHTS + NUM_BALL_LIGHTS];
#define lightPos basicLightPos
#define lightEmission basicLightEmission
#else
uniform vec4 ballLightPos[NUM_LIGHTS + NUM_BALL_LIGHTS];
uniform vec4 ballLightEmission[NUM_LIGHTS + NUM_BALL_LIGHTS];
#define lightPos ballLightPos
#define lightEmission ballLightEmission
#endif

uniform vec4 cAmbient_LightRange; //!! remove completely, just rely on envmap/IBL?

uniform vec4 fenvEmissionScale_TexWidth; // vec2 extended to vec4 for BGFX

uniform vec4 fDisableLighting_top_below; // vec2 extended to vec4 for BGFX

uniform vec4 fSceneScale; // float extended to vec4 for BGFX FIXME float uniforms are not supported. group with something else

//
// Material Params
//

uniform vec4 cBase_Alpha; //!! 0.04-0.95 in RGB

uniform vec4 Roughness_WrapL_Edge_Thickness; // wrap in [0..1] for rim/wrap lighting

//
// Material Helper Functions
//

// Lower alpha on border of geometry
// - blending: amount of effect (0=no darkening, 1=full darkening)
// - thickness: thickness (0=no darkening, 1=darkening linear with N.V dot product)
float GeometricOpacity(const float NdotV, const float alpha, const float blending, const float thickness)
{
    //old version without thickness
    //return mix(alpha, 1.0, blending*pow(1.0-abs(NdotV),5)); // fresnel for falloff towards silhouette

    //new version (COD/IW, t = thickness), thickness = 0.05 roughly corresponds to above version
    const float x = abs(NdotV); // flip normal in case of wrong orientation (backside lighting)
    const float g = blending * (1.0 - (x / (x * (1.0 - thickness) + thickness))); // Smith-Schlick G
    return mix(alpha, 1.0, g); // fake opacity lerp to 'shadowed'
}

vec3 FresnelSchlick(const vec3 spec, const float LdotH, const float edge)
{
    return spec + (vec3(edge,edge,edge) - spec) * pow(1.0 - LdotH, 5.); // UE4: vec3(edge,edge,edge) = saturate(50.0*spec.g)
}

//

vec3 DoPointLight(const vec3 pos, const vec3 N, const vec3 V, const vec3 diffuse, const vec3 glossy, const float edge, const float glossyPower, const int i, const bool is_metal) 
{ 
   //!! do in vertex shader?! or completely before?!
   //const vec3 lightDir = mul_w1(lightPos[i].xyz, matView) - pos;
   const vec3 lightDir = ( mul(matView, vec4(lightPos[i].xyz, 1.0)) ).xyz - pos;
   const vec3 L = normalize(lightDir);
   const float NdotL = dot(N, L);
   vec3 Out = vec3(0.0,0.0,0.0);

   // compute diffuse color (lambert with optional rim/wrap component)
   if (!is_metal && (NdotL + Roughness_WrapL_Edge_Thickness.y > 0.0))
      Out = diffuse * ((NdotL + Roughness_WrapL_Edge_Thickness.y) / sqr(1.0+Roughness_WrapL_Edge_Thickness.y));

   // add glossy component (modified ashikhmin/blinn bastard), not fully energy conserving, but good enough
   BRANCH if (NdotL > 0.0)
   {
	 const vec3 H = normalize(L + V); // half vector
	 const float NdotH = dot(N, H);
	 const float LdotH = dot(L, H);
	 const float VdotH = dot(V, H);
	 if ((NdotH > 0.0) && (LdotH > 0.0) && (VdotH > 0.0))
		Out += FresnelSchlick(glossy, LdotH, edge) * (((glossyPower + 1.0) / (8.0*VdotH)) * pow(NdotH, glossyPower));
   }

   //float fAtten = saturate( 1.0 - dot(lightDir/cAmbient_LightRange.w, lightDir/cAmbient_LightRange.w) );
   //float fAtten = 1.0/dot(lightDir,lightDir); // original/correct falloff

   const float sqrl_lightDir = dot(lightDir,lightDir); // tweaked falloff to have ranged lightsources
   float fAtten = saturate(1.0 - sqrl_lightDir*sqrl_lightDir/(cAmbient_LightRange.w*cAmbient_LightRange.w*cAmbient_LightRange.w*cAmbient_LightRange.w)); //!! pre-mult/invert cAmbient_LightRange.w?
   fAtten = fAtten*fAtten/(sqrl_lightDir + 1.0);

   vec3 ambient = glossy;
   if (!is_metal)
       ambient += diffuse;

   const vec3 result = Out * lightEmission[i].xyz * fAtten + ambient * cAmbient_LightRange.xyz;
   if (fDisableLighting_top_below.x != 0.0)
       return mix(result,diffuse,fDisableLighting_top_below.x);
   else
       return result;
}

// does /PI-corrected lookup/final color already
vec3 DoEnvmapDiffuse(const vec3 N, const vec3 diffuse)
{
   const vec3 env = texture2DLod(tex_diffuse_env, ray_to_equirectangular_uv(N), 0.0).xyz;
   return diffuse * env*fenvEmissionScale_TexWidth.x;
}

//!! PI?
// very very crude approximation by abusing miplevels
vec3 DoEnvmapGlossy(const vec3 N, const vec3 V, const vec2 Ruv, const vec3 glossy, const float glossyPower)
{
   const float mip = min(log2(fenvEmissionScale_TexWidth.y * sqrt(3.0)) - 0.5*log2(glossyPower + 1.0), log2(fenvEmissionScale_TexWidth.y)-1.); //!! do diffuse lookup instead of this limit/min, if too low?? and blend?
   const vec3 env = texture2DLod(tex_env, Ruv, mip).xyz;
   return glossy * env*fenvEmissionScale_TexWidth.x;
}

//!! PI?
vec3 DoEnvmap2ndLayer(const vec3 color1stLayer, const vec3 pos, const vec3 N, const vec3 V, const float NdotV, const vec2 Ruv, const vec3 specular)
{
   const vec3 w = FresnelSchlick(specular, NdotV, Roughness_WrapL_Edge_Thickness.z); //!! ?
   const vec3 env = texture2DLod(tex_env, Ruv, 0.0).xyz;
   return mix(color1stLayer, env*fenvEmissionScale_TexWidth.x, w); // weight (optional) lower diffuse/glossy layer with clearcoat/specular
}

// ////////////////////////////////////////////////////////////////////////////
// Apply lighting from the environment and the 2 scene lights
// - Lighting is kinda PBR since glossy and diffuse are normalized, and the 2nd layer (specular/clearcoat) is 'blended' via Fresnel
// - Apply the 2 points lights to the diffuse (Lambert with optional rim/wrap) and glossy (Ashikhmin/Blinn) components, clearcoat (a.k.a. specular) is not applied (as point lights).
//   Light energy can be tweaked in order to have ranged lights instead of the physical 1/d² energy
//   This lighting can be 'disabled', in fact replacing it by 2 times the diffuse color (backwards compatibility bug)
// - Apply the environment lighting with diffuse, glossy and a specular/clearcoat layer
//   Diffuse is applied via a precomputed/'filtered' version of the envmap.
//   Glossy is performed with a very crude approximation of the BRDF and the roughness parameter (simple mip-mapping of the envmap).
//   Specular just does a plain lookup in the envmap.
// - Backside (normal pointing away) is lighted as well as front facing (needed since quite a lot of tables feature wrong normals, or for transparents with 2 pass rendering)
vec3 lightLoop(const vec3 pos, vec3 N, const vec3 V, vec3 diffuse, vec3 glossy, const vec3 specular, const float edge, const bool is_metal) // input vectors (N,V) are normalized for BRDF evals
{
   vec3 color = vec3_splat(0.0);

   float NdotV = dot(N,V);
   if (NdotV < 0.0)
   {
       // Flip normal in case for backside lighting.
       // Before 10.8, this was disabled when using normal mapping, but since tables are not all well behaving, this caused artefacts (for example Cirqus Voltaire vs Monster Bash), so this is now always performed.
       N = -N;
       NdotV = -NdotV;
   }
   NdotV = min(NdotV, 1.0); // For some reason I don't get, N (which is normalized) dot V (which is also normalized) may lead to value exceeding 1.0 then causing invalid results (in FresnelSchlick, negative pow)
   // normalize BRDF layer inputs //!! use diffuse = (1-glossy)*diffuse instead?
   const float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));
   const float glossyMax = max(glossy.x,max(glossy.y,glossy.z));
   const float specularMax = max(specular.x,max(specular.y,specular.z)); //!! not needed as 2nd layer only so far
   const float sum = diffuseMax + glossyMax /*+ specularMax*/;
   // energy conservation:
   if (sum > 1.0 && fDisableLighting_top_below.x < 1.0) // but allow overly bright contribution if lighting is disabled
   {
      const float invsum = 1.0/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      //specular *= invsum;
   }

   // Scene point lights
   BRANCH if ((!is_metal && (diffuseMax > 0.0)) || (glossyMax > 0.0))
   {
      BRANCH if (fDisableLighting_top_below.x == 1.0)
         color += float(iLightPointNum) * diffuse; // Old bug kept for backward compatibility: when lighting is disabled, it results to applying it twice
      else for (int i = 0; i < NUM_LIGHTS; i++)
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal); // no clearcoat needed as only pointlights so far
   }

   // Environment IBL
   BRANCH if (!is_metal && (diffuseMax > 0.0))
      // trafo back to world for lookup into world space envmap // actually: mul(float4(N,0.0), matViewInverseInverseTranspose), but optimized to save one matrix
      // matView is always an orthonormal matrix, so no need to normalize after transform
      color += DoEnvmapDiffuse(/*normalize*/(( mul(vec4(N,0.0), matView) ).xyz), diffuse); // trafo back to world for lookup into world space envmap // actually: mul(vec4(N,0.0), matViewInverseInverseTranspose), but optimized to save one matrix
   BRANCH if ((glossyMax > 0.0) || (specularMax > 0.0))
   {
	   vec3 R = (2.0*NdotV)*N - V; // reflect(-V,N);
	   // trafo back to world for lookup into world space envmap // actually: mul(float4(R,0.0), matViewInverseInverseTranspose), but optimized to save one matrix
	   // matView is always an orthonormal matrix, so no need to normalize after transform
	   R = /*normalize*/(( mul(vec4(R,0.0), matView) ).xyz); // trafo back to world for lookup into world space envmap // actually: mul(vec4(R,0.0), matViewInverseInverseTranspose), but optimized to save one matrix
	   const vec2 Ruv = ray_to_equirectangular_uv(R);
	   if (glossyMax > 0.0)
		  color += DoEnvmapGlossy(N, V, Ruv, glossy, Roughness_WrapL_Edge_Thickness.x);
	   if (specularMax > 0.0)
		  color = DoEnvmap2ndLayer(color, pos, N, V, NdotV, Ruv, specular);
   }

   return /*Gamma(ToneMap(*/color/*))*/;
}
