//
// Lights
//

#define NUM_LIGHTS 2

struct CLight 
{ 
   float3 vPos; 
   float3 vEmission;
};

#define iLightPointNum NUM_LIGHTS
#define iLightPointBallsNum (NUM_LIGHTS+NUM_BALL_LIGHTS)

#if iLightPointBallsNum == iLightPointNum // basic shader
float4 packedLights[3]; //!! 4x3 = NUM_LIGHTSx6
static CLight lights[iLightPointBallsNum] = (CLight[iLightPointBallsNum])packedLights;
#else                                     // ball shader
float4 packedLights[15]; //!! 4x15 = (NUM_LIGHTS+NUM_BALL_LIGHTS)x6
static CLight lights[iLightPointBallsNum] = (CLight[iLightPointBallsNum])packedLights;
#endif

float4 cAmbient_LightRange = float4(0.0,0.0,0.0, 0.0); //!! remove completely, just rely on envmap/IBL?

float2 fenvEmissionScale_TexWidth;

float2 fDisableLighting_top_below = float2(0.,0.);

//
// Material Params
//

float4 cBase_Alpha = float4(0.5,0.5,0.5, 1.0); //!! 0.04-0.95 in RGB

float4 Roughness_WrapL_Edge_Thickness = float4(4.0, 0.5, 1.0, 0.05); // wrap in [0..1] for rim/wrap lighting

//
// Material Helper Functions
//

float GeometricOpacity(const float NdotV, const float alpha, const float blending, const float t)
{
    //old version without thickness
    //return lerp(alpha, 1.0, blending*pow(1.0-abs(NdotV),5)); // fresnel for falloff towards silhouette

    //new version (COD/IW, t = thickness), t = 0.05 roughly corresponds to above version
    const float x = abs(NdotV); // flip normal in case of wrong orientation (backside lighting)
    const float g = blending - blending * ( x / (x * (1.0 - t) + t) ); // Smith-Schlick G
    return lerp(alpha, 1.0, g); // fake opacity lerp to ‘shadowed’
}

float3 FresnelSchlick(const float3 spec, const float LdotH, const float edge)
{
    return spec + (float3(edge,edge,edge) - spec) * pow(1.0 - LdotH, 5); // UE4: float3(edge,edge,edge) = saturate(50.0*spec.g)
}

//

float3 DoPointLight(const float3 pos, const float3 N, const float3 V, const float3 diffuse, const float3 glossy, const float edge, const float glossyPower, const int i, const bool is_metal) 
{ 
   // early out here or maybe we can add more material elements without lighting later?
   if(fDisableLighting_top_below.x == 1.0)
      return diffuse;

   const float3 lightDir = mul_w1(lights[i].vPos, matView) - pos; //!! do in vertex shader?! or completely before?!
   const float3 L = normalize(lightDir);
   const float NdotL = dot(N, L);
   float3 Out = float3(0.0,0.0,0.0);
   
   // compute diffuse color (lambert with optional rim/wrap component)
   if(!is_metal && (NdotL + Roughness_WrapL_Edge_Thickness.y > 0.0))
      Out = diffuse * ((NdotL + Roughness_WrapL_Edge_Thickness.y) / sqr(1.0+Roughness_WrapL_Edge_Thickness.y));
    
   // add glossy component (modified ashikhmin/blinn bastard), not fully energy conserving, but good enough
   [branch] if(NdotL > 0.0)
   {
	 const float3 H = normalize(L + V); // half vector
	 const float NdotH = dot(N, H);
	 const float LdotH = dot(L, H);
	 const float VdotH = dot(V, H);
	 if((NdotH > 0.0) && (LdotH > 0.0) && (VdotH > 0.0))
		Out += FresnelSchlick(glossy, LdotH, edge) * (((glossyPower + 1.0) / (8.0*VdotH)) * pow(NdotH, glossyPower));
   }
 
   //float fAtten = saturate( 1.0 - dot(lightDir/cAmbient_LightRange.w, lightDir/cAmbient_LightRange.w) );
   //float fAtten = 1.0/dot(lightDir,lightDir); // original/correct falloff
   
   const float sqrl_lightDir = dot(lightDir,lightDir); // tweaked falloff to have ranged lightsources
   float fAtten = saturate(1.0 - sqrl_lightDir*sqrl_lightDir/(cAmbient_LightRange.w*cAmbient_LightRange.w*cAmbient_LightRange.w*cAmbient_LightRange.w)); //!! pre-mult/invert cAmbient_LightRange.w?
   fAtten = fAtten*fAtten/(sqrl_lightDir + 1.0);

   float3 ambient = glossy;
   if(!is_metal)
       ambient += diffuse;

   const float3 result = Out * lights[i].vEmission * fAtten + ambient * cAmbient_LightRange.xyz;
   if(fDisableLighting_top_below.x != 0.0)
       return lerp(result,diffuse,fDisableLighting_top_below.x);
   else
       return result;
}

// does /PI-corrected lookup/final color already
float3 DoEnvmapDiffuse(const float3 N, const float3 diffuse)
{
   const float2 uv = float2( // remap to 2D envmap coords
		0.5 + atan2_approx_div2PI(N.y, N.x),
	    acos_approx_divPI(N.z));

   float3 env = tex2Dlod(texSampler2, float4(uv, 0.,0.)).xyz;
   if(!hdrEnvTextures)
       env = InvGamma(env);
   return diffuse * env*fenvEmissionScale_TexWidth.x;
}

//!! PI?
// very very crude approximation by abusing miplevels
float3 DoEnvmapGlossy(const float3 N, const float3 V, const float2 Ruv, const float3 glossy, const float glossyPower)
{
   const float mip = log2(fenvEmissionScale_TexWidth.y * sqrt(3.0)) - 0.5*log2(glossyPower + 1.0);
   float3 env = tex2Dlod(texSampler1, float4(Ruv, 0., mip)).xyz;
   if(!hdrEnvTextures)
       env = InvGamma(env);
   return glossy * env*fenvEmissionScale_TexWidth.x;
}

//!! PI?
float3 DoEnvmap2ndLayer(const float3 color1stLayer, const float3 pos, const float3 N, const float3 V, const float NdotV, const float2 Ruv, const float3 specular)
{
   const float3 w = FresnelSchlick(specular, NdotV, Roughness_WrapL_Edge_Thickness.z); //!! ?
   float3 env = tex2Dlod(texSampler1, float4(Ruv, 0., 0.)).xyz;
   if(!hdrEnvTextures)
       env = InvGamma(env);
   return lerp(color1stLayer, env*fenvEmissionScale_TexWidth.x, w); // weight (optional) lower diffuse/glossy layer with clearcoat/specular
}

float3 lightLoop(const float3 pos, float3 N, const float3 V, float3 diffuse, float3 glossy, const float3 specular, const float edge, const bool fix_normal_orientation, const bool is_metal) // input vectors (N,V) are normalized for BRDF evals
{
   // normalize BRDF layer inputs //!! use diffuse = (1-glossy)*diffuse instead?
   const float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));
   const float glossyMax = max(glossy.x,max(glossy.y,glossy.z));
   const float specularMax = max(specular.x,max(specular.y,specular.z)); //!! not needed as 2nd layer only so far
   const float sum = diffuseMax + glossyMax /*+ specularMax*/;
   if(sum > 1.0)
   {
      const float invsum = 1.0/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      //specular *= invsum;
   }

   float NdotV = dot(N,V);
   if(fix_normal_orientation && (NdotV < 0.0)) // flip normal in case of wrong orientation? (backside lighting), currently disabled if normal mapping active, for that case we should actually clamp the normal with respect to V instead (see f.e. 'view-dependant shading normal adaptation')
   {
      N = -N;
	  NdotV = -NdotV;
   }

   float3 color = float3(0.0, 0.0, 0.0);

   // 1st Layer
   [branch] if((!is_metal && (diffuseMax > 0.0)) || (glossyMax > 0.0))
   {
      for(int i = 0; i < iLightPointNum; i++)
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal); // no clearcoat needed as only pointlights so far
   }

   [branch] if(!is_metal && (diffuseMax > 0.0))
      color += DoEnvmapDiffuse(normalize(mul(matView, N).xyz), diffuse); // trafo back to world for lookup into world space envmap // actually: mul(float4(N,0.0), matViewInverseInverseTranspose), but optimized to save one matrix

   [branch] if((glossyMax > 0.0) || (specularMax > 0.0))
   {
	   float3 R = (2.0*NdotV)*N - V; // reflect(-V,n);
	   R = normalize(mul(matView, R).xyz); // trafo back to world for lookup into world space envmap // actually: mul(float4(R,0.0), matViewInverseInverseTranspose), but optimized to save one matrix

	   const float2 Ruv = float2( // remap to 2D envmap coords
			0.5 + atan2_approx_div2PI(R.y, R.x),
			acos_approx_divPI(R.z));

	   if(glossyMax > 0.0)
		  color += DoEnvmapGlossy(N, V, Ruv, glossy, Roughness_WrapL_Edge_Thickness.x);

	   // 2nd Layer
	   if(specularMax > 0.0)
		  color = DoEnvmap2ndLayer(color, pos, N, V, NdotV, Ruv, specular);
   }

   return /*Gamma(ToneMap(*/color/*))*/;
}
