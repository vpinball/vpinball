#define PI 3.1415926535897932384626433832795f
#define NUM_LIGHTS 2

bool   bDiffuse  = true;  //!! remove, steer from diffuse?  (performance?)
bool   bGlossy   = false; //!! remove, steer from glossy?   (performance?)
bool   bSpecular = false; //!! remove, steer from specular? (performance?)

float3 vDiffuseColor = float3(0.5f, 0.5f, 0.5f); //!! 0.04-0.95 in RGB

float  fDiffuseWrap = 0.5f; //!! pass from material, w in [0..1] for rim/wrap lighting
float  fGlossyPower = 0.1f; //!! pass from material(/texture?)
float3 vAmbient = float3(0.0f,0.0f,0.0f); //!! remove completely, just rely on envmap/IBL?

float EnvEmissionScale = 10.0f; //!! also have envmap of fixed size?
float LightEmissionScale = 1000000.0f; //!! remove! put into emission below
float  flightRange = 3000.0f;

float  fmaterialAlpha = 1.0f; //!! allow for texture? -> use from diffuse? and/or glossy,etc?

// transformation matrices 
float4x4 matWorldViewProj  : WORLDVIEWPROJ;
float4x4 matWorldView      : WORLDVIEW;
float4x4 matWorldViewInverseTranspose;
float4x4 matView;
float4x4 matViewInverse;

struct CLight 
{ 
   float3 vPos; 
   float3 vEmission;
}; 
 
int iLightPointNum = NUM_LIGHTS;
CLight lights[NUM_LIGHTS] = {          //NUM_LIGHTS == 2
   { 
      float3(0.0f, 0.0f, 0.0f),        //position 
      float3(0.0f, 0.0f, 0.0f)         //emission //!! have emission > 1.0f
   }, 
   { 
      float3(0.0f, 0.0f, 0.0f),        //position 
      float3(0.0f, 0.0f, 0.0f)         //emission
   }
}; 

texture Texture0; // diffuse
texture Texture1; // envmap
texture Texture2; // envmap radiance

sampler2D texSampler0 : TEXUNIT0 = sampler_state
{
	Texture	  = (Texture0);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
};

sampler2D texSampler1 : TEXUNIT1 = sampler_state
{
	Texture	  = (Texture1);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
};

sampler2D texSampler2 : TEXUNIT2 = sampler_state
{
	Texture	  = (Texture2);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
};


float3 FresnelSchlick(float3 spec, float LdotH)
{
    return spec + (float3(1.0f,1.0f,1.0f) - spec) * pow(1.0f - LdotH, 5);
}

float3 InvGamma(float3 color) //!! use hardware support? D3DSAMP_SRGBTEXTURE,etc
{
	return pow(color,2.2f);
}

float3 Gamma(float3 color) //!! use hardware support? D3DSAMP_SRGBTEXTURE,etc
{
	return pow(color,1.0f/2.2f);
}

float3 ToneMap(float3 color)
{
    float burnhighlights = 0.2f;
    
    float l = color.x*0.176204f + color.y*0.812985f + color.z*0.0108109f;
    return color * ((l*burnhighlights + 1.0f) / (l + 1.0f));
    
    //return saturate(color);
}

float3 DoPointLight(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, float glossyPower, int i) 
{ 
   float3 lightDir = mul(float4(lights[i].vPos,1.0f), matView).xyz - pos; //!! do in vertex shader?! or completely before?!
   float3 L = normalize(lightDir);
   float NdotL = dot(N, L);
   float3 Out = float3(0.0f,0.0f,0.0f);
   
   // compute diffuse color (lambert with optional rim/wrap component)
   if(bDiffuse && (NdotL + fDiffuseWrap > 0.0f))
      Out = diffuse * ((NdotL + fDiffuseWrap) / ((1.0f+fDiffuseWrap) * (1.0f+fDiffuseWrap)));
 
   // add glossy component (modified ashikhmin/blinn bastard), not fully energy conserving, but good enough
   if(bGlossy && (NdotL > 0.0f))
   { 
	 float3 H = normalize(L + V); // half vector
	 float NdotH = dot(N, H);
	 float LdotH = dot(L, H);
	 float VdotH = dot(V, H);
	 if((NdotH > 0.0f) && (LdotH > 0.0f) && (VdotH > 0.0f))
		Out += FresnelSchlick(glossy, LdotH) * (((glossyPower + 1.0f) / (8.0f*VdotH)) * pow(NdotH, glossyPower));
   }
 
   //float fAtten = saturate( 1.0f - dot(lightDir/flightRange, lightDir/flightRange) );
   //float fAtten = 1.0f/dot(lightDir,lightDir); // original/correct falloff
   
   float sqrl_lightDir = dot(lightDir,lightDir); // tweaked falloff to have ranged lightsources
   float fAtten = saturate(1.0f - sqrl_lightDir*sqrl_lightDir/(flightRange*flightRange*flightRange*flightRange)); //!! pre-mult/invert flightRange?
   fAtten = fAtten*fAtten/(sqrl_lightDir + 1.0f);

   Out *= lights[i].vEmission * LightEmissionScale * fAtten;
   
   return Out; 
}

// does /PI-corrected lookup/final color already
float3 DoEnvmapDiffuse(float3 N, float3 diffuse)
{
   float2 uv = float2( // remap to 2D envmap coords
		atan2(N.y, N.x) * (0.5f/PI) + 0.5f,
	    acos(N.z) * (1.0f/PI));

   return diffuse * InvGamma(tex2D(texSampler2, uv).xyz)*EnvEmissionScale; //!! replace by real HDR instead? -> remove invgamma then
}

//!! PI?
float3 DoEnvmap2ndLayer(float3 color1stLayer, float3 pos, float3 N, float3 V, float3 specular)
{
   float3 r = reflect(-V,N);
   r = normalize(mul(float4(r,0.0f), matViewInverse).xyz); // trafo back to world
   float2 uv = float2( // remap to 2D envmap coords
		atan2(r.y, r.x) * (0.5f/PI) + 0.5f,
	    acos(r.z) * (1.0f/PI));
	    
   float3 w = FresnelSchlick(specular, dot(V, N)); //!! ?
   return lerp(color1stLayer, InvGamma(tex2D(texSampler1, uv).xyz)*EnvEmissionScale, w); // weight (optional) lower diffuse/glossy layer with clearcoat/specular //!! replace by real HDR instead? -> remove invgamma then
}

float4 lightLoop(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, float3 specular)
{
   // normalize input vectors for BRDF evals
   N = normalize(N);
   V = normalize(V);
   
   // normalize BRDF layer inputs //!! use diffuse = (1-glossy)*diffuse instead?
   float diffuseMax = bDiffuse ? max(diffuse.x,max(diffuse.y,diffuse.z)) : 0.0f;
   float glossyMax = bGlossy ? max(glossy.x,max(glossy.y,glossy.z)) : 0.0f;
   //float specularMax = bSpecular ? max(specular.x,max(specular.y,specular.z)) : 0.0f; //!! not needed as 2nd layer only so far
   float sum = diffuseMax + glossyMax /*+ specularMax*/;
   if(sum > 1.0f)
   {
      float invsum = 1.0f/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      //specular *= invsum;
   }

   //if(dot(N,V) < 0.0f) //!! flip normal in case of wrong orientation? (backside lighting)
   //   N = -N;

   float3 color = float3(0.0f, 0.0f, 0.0f);
      
   // 1st Layer
   if((bDiffuse && diffuseMax > 0.0f) || (bGlossy && glossyMax > 0.0f))
   {
      float glossyPower = exp2(10.0f * fGlossyPower + 1.0f); // map from 0..1 to 2..2048 //!! precalc?
      for(int i = 0; i < iLightPointNum; i++)
         color += DoPointLight(pos, N, V, diffuse, glossy, glossyPower, i); // no specular needed as only pointlights so far
   }
         
   if(bDiffuse && diffuseMax > 0.0f)
      color += DoEnvmapDiffuse(N, diffuse); // no glossy, as it's the most hacky one //!! -> use mipmap-hacks for glossy?
         
   // 2nd Layer
   if(bSpecular /*&& specularMax > 0.0f*/)
      color = DoEnvmap2ndLayer(color, pos, N, V, specular);
  
   return float4(Gamma(ToneMap(vAmbient + color)), 1.0f); //!! in case of HDR out later on, remove tonemap and gamma //!! also problematic for alpha blends
}

float4 Additive(float4 cBase, float4 cBlend, float percent)
{
   return (cBase+cBlend*percent);
}

float4 Screen (float4 cBase, float4 cBlend, float percent)
{
	return (1.0f - (1.0f - cBase) * (1.0f - cBlend));
}
float4 Multiply (float4 cBase, float4 cBlend, float percent)
{
	return (cBase * (cBlend*percent));
}
float4 Overlay (float4 cBase, float4 cBlend, float percent)
{
	// Vectorized (easier for compiler)
	float4 cNew;
	
	// overlay has two output possbilities
	// which is taken is decided if pixel value
	// is below half or not

	cNew = step(0.5f,cBase);
	
	// we pick either solution
	// depending on pixel
	
	// first is case of < 0.5
	// second is case for >= 0.5
	
	// interpolate between the two, 
	// using color as influence value
	float4 blend = cBlend;
	cNew= lerp((cBase*blend*2.0f),(1.0f-(2.0f*(1.0f-cBase)*(1.0f-blend))),cNew);

	cNew.a = 1.0f;
	return cNew;
}
