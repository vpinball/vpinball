//
// Lights
//

// disable warning 'pow(f, e) will not work for negative f, use abs(f) or conditionally handle negative values if you expect them' as it is fairly common and not actionable.
#pragma warning(disable : 3571)

#define NUM_LIGHTS 2

#define iLightPointNum NUM_LIGHTS
#define iLightPointBallsNum (NUM_LIGHTS+NUM_BALL_LIGHTS)

#ifdef GLSL
#if iLightPointBallsNum == iLightPointNum // basic shader
uniform vec4 basicLightPos[iLightPointNum];
uniform vec4 basicLightEmission[iLightPointNum];
#define lightPos basicLightPos
#define lightEmission basicLightEmission
#else // ball shader
uniform vec4 ballLightPos[iLightPointBallsNum];
uniform vec4 ballLightEmission[iLightPointBallsNum];
#define lightPos ballLightPos
#define lightEmission ballLightEmission
#endif

#else // HLSL
struct CLight
{
   float3 vPos;
   float3 vEmission;
};
#if iLightPointBallsNum == iLightPointNum // basic shader
const float4 basicPackedLights[3]; //!! 4x3 = NUM_LIGHTSx6
static const CLight lights[iLightPointBallsNum] = (CLight[iLightPointBallsNum])basicPackedLights;
#define packedLights basicPackedLights
#else // ball shader
const float4 ballPackedLights[15]; //!! 4x15 = (NUM_LIGHTS+NUM_BALL_LIGHTS)x6
static const CLight lights[iLightPointBallsNum] = (CLight[iLightPointBallsNum])ballPackedLights;
#define packedLights ballPackedLights
#endif
#endif

const float4 cAmbient_LightRange; //!! remove completely, just rely on envmap/IBL?

const float2 fenvEmissionScale_TexWidth;

const float2 fDisableLighting_top_below;

texture Texture1; // envmap
texture Texture2; // envmap radiance

sampler2D tex_env : TEXUNIT1 = sampler_state // environment
{
   Texture = (Texture1);
   MIPFILTER = LINEAR; //!! ?
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
   ADDRESSU = Wrap;
   ADDRESSV = Clamp;
   SRGBTexture = true;
};

sampler2D tex_diffuse_env : TEXUNIT2 = sampler_state // diffuse environment contribution/radiance
{
   Texture = (Texture2);
   MIPFILTER = NONE;
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
   ADDRESSU = Wrap;
   ADDRESSV = Clamp;
   SRGBTexture = true;
};

//
// Material Params
//

const float4 cBase_Alpha; //!! 0.04-0.95 in RGB

const float4 Roughness_WrapL_Edge_Thickness; // wrap in [0..1] for rim/wrap lighting

//
// Material Helper Functions
//

// Lower alpha on border of geometry
// - blending: amount of effect (0=no darkening, 1=full darkening)
// - thickness: thickness (0=no darkening, 1=darkening linear with N.V dot product)
float GeometricOpacity(const float NdotV, const float alpha, const float blending, const float thickness)
{
    //old version without thickness
    //return lerp(alpha, 1.0, blending*pow(1.0-abs(NdotV),5)); // fresnel for falloff towards silhouette

    //new version (COD/IW, t = thickness), thickness = 0.05 roughly corresponds to above version
    const float x = abs(NdotV); // flip normal in case of wrong orientation (backside lighting)
    const float g = blending * (1.0 - (x / (x * (1.0 - thickness) + thickness))); // Smith-Schlick G
    return lerp(alpha, 1.0, g); // fake opacity lerp to 'shadowed'
}

float3 FresnelSchlick(const float3 spec, const float LdotH, const float edge)
{
    return spec + (float3(edge,edge,edge) - spec) * pow(1.0 - LdotH, 5.); // UE4: float3(edge,edge,edge) = saturate(50.0*spec.g)
}

//

float3 DoPointLight(const float3 pos, const float3 N, const float3 V, const float3 diffuse, const float3 glossy, const float edge, const float glossyPower, const int i, const bool is_metal) 
{ 
   //!! do in vertex shader?! or completely before?!
   const float3 lightDir = mul_w1(lights[i].vPos, matView) - pos;
   const float3 L = normalize(lightDir);
   const float NdotL = dot(N, L);
   float3 Out = float3(0.0,0.0,0.0);

   // compute diffuse color (lambert with optional rim/wrap component)
   if (!is_metal && (NdotL + Roughness_WrapL_Edge_Thickness.y > 0.0))
      Out = diffuse * ((NdotL + Roughness_WrapL_Edge_Thickness.y) / sqr(1.0+Roughness_WrapL_Edge_Thickness.y));

   // add glossy component (modified ashikhmin/blinn bastard), not fully energy conserving, but good enough
   BRANCH if (NdotL > 0.0)
   {
	 const float3 H = normalize(L + V); // half vector
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

   float3 ambient = glossy;
   if (!is_metal)
       ambient += diffuse;

   const float3 result = Out * lights[i].vEmission * fAtten + ambient * cAmbient_LightRange.xyz;
   if (fDisableLighting_top_below.x != 0.0)
       return lerp(result,diffuse,fDisableLighting_top_below.x);
   else
       return result;
}

// does /PI-corrected lookup/final color already
float3 DoEnvmapDiffuse(const float3 N, const float3 diffuse)
{
   const float3 env = texNoLod(tex_diffuse_env, ray_to_equirectangular_uv(N)).xyz;
   return diffuse * env * fenvEmissionScale_TexWidth.x;
}

//!! PI?
// very very crude approximation by abusing miplevels
float3 DoEnvmapGlossy(const float3 N, const float3 V, const float2 Ruv, const float3 glossy, const float glossyPower)
{
   const float mip = min(log2(fenvEmissionScale_TexWidth.y * sqrt(3.0)) - 0.5*log2(glossyPower + 1.0), log2(fenvEmissionScale_TexWidth.y)-1.); //!! do diffuse lookup instead of this limit/min, if too low?? and blend?
   const float3 env = tex2Dlod(tex_env, float4(Ruv, 0., mip)).xyz;
   return glossy * env*fenvEmissionScale_TexWidth.x;
}

//!! PI?
float3 DoEnvmap2ndLayer(const float3 color1stLayer, const float3 pos, const float3 N, const float3 V, const float NdotV, const float2 Ruv, const float3 specular)
{
   const float3 w = FresnelSchlick(specular, NdotV, Roughness_WrapL_Edge_Thickness.z); //!! ?
   const float3 env = texNoLod(tex_env, Ruv).xyz;
   return lerp(color1stLayer, env*fenvEmissionScale_TexWidth.x, w); // weight (optional) lower diffuse/glossy layer with clearcoat/specular
}

// ////////////////////////////////////////////////////////////////////////////
// Apply lighting from the environment and the 2 scene lights
// - Lighting is not fully PBR since the specualr, glossy and diffuse are added without limiting the energy between layers to the one not reflected by the previous layer.
// - Apply the 2 points lights to the diffuse (Lambert with rim/wrap) and glossy (Ashikhmin/Blinn) components, clearcoat (a.k.a. specular) is not applied.
//   Light energy is tweaked in order to have ranged lights instead of the physical 1/d� energy
//   These lighting can be 'disabled' in fact replacing it by the 2 times the diffuse color (backward compatibility bug)
// - Apply the environment lighting with diffuse, glossy and cleacoat (named specular) layers.
//   Glossy is performed with a somewhat crude approximation for the BRDF and for the roughness parameter.
// - Backside (normal pointing away) is lighted as well as front facing (needed since quite a lot of tables have wrong normal, or for transparents with 2 pass rendering)
float3 lightLoop(const float3 pos, float3 N, const float3 V, float3 diffuse, float3 glossy, const float3 specular, const float edge, const bool is_metal) // input vectors (N,V) are normalized for BRDF evals
{
   float3 color = float3(0.0, 0.0, 0.0);
   
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
      else for (int i = 0; i < iLightPointNum; i++)
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal); // no clearcoat needed as only pointlights so far
   }

   // Environment IBL
   BRANCH if (!is_metal && (diffuseMax > 0.0))
	  // trafo back to world for lookup into world space envmap // actually: mul(float4(N,0.0), matViewInverseInverseTranspose), but optimized to save one matrix
	  // matView is always an orthonormal matrix, so no need to normalize after transform
      color += DoEnvmapDiffuse(/*normalize*/(mul(matView, N).xyz), diffuse); // actually: mul(float4(N,0.0), matViewInverseInverseTranspose), but optimized to save one matrix
   BRANCH if ((glossyMax > 0.0) || (specularMax > 0.0))
   {
	   float3 R = (2.0*NdotV)*N - V; // reflect(-V,N);
	   // trafo back to world for lookup into world space envmap // actually: mul(float4(R,0.0), matViewInverseInverseTranspose), but optimized to save one matrix
	   // matView is always an orthonormal matrix, so no need to normalize after transform
	   R = /*normalize*/(mul(matView, R).xyz); // actually: mul(float4(R,0.0), matViewInverseInverseTranspose), but optimized to save one matrix
	   const float2 Ruv = ray_to_equirectangular_uv(R);
#if !ENABLE_VR
	   if (glossyMax > 0.0)
		  color += DoEnvmapGlossy(N, V, Ruv, glossy, Roughness_WrapL_Edge_Thickness.x);
	   if (specularMax > 0.0)
		  color = DoEnvmap2ndLayer(color, pos, N, V, NdotV, Ruv, specular);
#else
      // Abuse mipmaps to reduce shimmering in VR
      float4 colorMip;
      if (is_metal)
      {
         // Use low-res mipmap for metallic objects to reduce shimmering in VR
         // Closer objects we query the lod and add 2 to make it a bit blurrier but always at least 6.0
         // Far away objects we get smallest lod and divide by 1.6 which is a good trade-off between "metallic enough" and "low shimmer"
         float mipLevel = min(textureQueryLod(tex_env, Ruv).y+2.0, textureQueryLevels(tex_env)/1.6);
         if (mipLevel < 6.0)
            mipLevel = 6.0;
         colorMip = textureLod(tex_env, Ruv, mipLevel);
      }
      else
      {
         // For non-metallic objects we use different values
         //colorMip = texture(tex_env, Ruv);
         float mipLevel = min(textureQueryLod(tex_env, Ruv).y, textureQueryLevels(tex_env)/2);
         if (mipLevel < 4.0)
            mipLevel = 4.0;
         colorMip = textureLod(tex_env, Ruv, mipLevel);
      }

      const float3 envTex = colorMip.rgb;

      // EnvmapGlossy
      if(glossyMax > 0.0)
        color += glossy * envTex * fenvEmissionScale_TexWidth.x;

      // Envmap2ndLayer
      if(specularMax > 0.0)
      {
        const float3 w = FresnelSchlick(specular, NdotV, Roughness_WrapL_Edge_Thickness.z);
        color = mix(color, envTex * fenvEmissionScale_TexWidth.x, w);
      }
#endif
   }

   return /*Gamma(ToneMap(*/color/*))*/;
}
