#define PI 3.1415926535897932384626433832795f
#define NUM_LIGHTS 2

bool color_grade;

float3 cBase = float3(0.5f, 0.5f, 0.5f); //!! 0.04-0.95 in RGB

float  fEdge = 1.0f;
float  fWrapLighting = 0.5f; // w in [0..1] for rim/wrap lighting
float  fRoughness = 4.0f;

bool   bIsMetal = false;

float3 vAmbient = float3(0.0f,0.0f,0.0f); //!! remove completely, just rely on envmap/IBL?

float fenvEmissionScale;
float flightRange;

float fmaterialAlpha = 1.0f; //!! allow for texture? -> use from diffuse? and/or glossy,etc?

float fenvTexWidth;

// transformation matrices 
float4x4 matWorldViewProj : WORLDVIEWPROJ;
float4x4 matWorldView     : WORLDVIEW;
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

texture Texture0; // base texture
texture Texture1; // envmap
texture Texture2; // envmap radiance
texture Texture3; // AO tex
texture Texture4; // color grade
 
sampler2D texSampler0 : TEXUNIT0 = sampler_state // base texture
{
	Texture	  = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
	//ADDRESSU  = Wrap; //!! ?
	//ADDRESSV  = Wrap;
};

sampler2D texSampler1 : TEXUNIT1 = sampler_state // environment and texB for lights
{
	Texture	  = (Texture1);
    MIPFILTER = LINEAR; //!! ?
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Wrap;
	ADDRESSV  = Wrap;
};

sampler2D texSampler2 : TEXUNIT2 = sampler_state // diffuse environment contribution/radiance
{
	Texture	  = (Texture2);
    MIPFILTER = NONE;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Wrap;
	ADDRESSV  = Wrap;
};

sampler2D texSampler6 : TEXUNIT1 = sampler_state // color grade LUT
{
	Texture	  = (Texture4);
    MIPFILTER = NONE;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};


float3 FresnelSchlick(float3 spec, float LdotH, float edge)
{
    return spec + (float3(edge,edge,edge) - spec) * pow(1.0f - LdotH, 5);
}

float3 InvGamma(float3 color) //!! use hardware support? D3DSAMP_SRGBTEXTURE
{
	return /*color * (color * (color * 0.305306011f + 0.682171111f) + 0.012522878f);/*/ pow(color,2.2f); // pow does not matter anymore on current GPUs
}

float3 FBGamma(float3 color) //!! use hardware support? D3DRS_SRGBWRITEENABLE
{
	return pow(color,1.0/2.2); // pow does not matter anymore on current GPUs

	/*const float3 t0 = sqrt(color);
	const float3 t1 = sqrt(t0);
	const float3 t2 = sqrt(t1);
	return 0.662002687f * t0 + 0.684122060f * t1 - 0.323583601f * t2 - 0.0225411470f * color;*/
}

float3 FBToneMap(float3 color)
{
    float burnhighlights = 0.25f;
    
    float l = color.x*0.176204f + color.y*0.812985f + color.z*0.0108109f;
    return saturate(color * ((l*burnhighlights + 1.0f) / (l + 1.0f))); //!! bloom instead?
}

float3 FBColorGrade(float3 color)
{
   if(!color_grade)
       return color;

   color.xy = color.xy*(15.0f/16.0f) + 1.0f/32.0f; // assumes 16x16x16 resolution flattened to 256x16 texture
   color.z *= 15.0f;

   float x = (color.x + floor(color.z))/16.0f;
   float3 lut1 = tex2Dlod(texSampler6, float4(x,            color.y, 0.f,0.f)).xyz; // two lookups to blend/lerp between blue 2D regions
   float3 lut2 = tex2Dlod(texSampler6, float4(x+1.0f/16.0f, color.y, 0.f,0.f)).xyz;
   return lerp(lut1,lut2, frac(color.z));
}

float3 DoPointLight(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, float edge, float glossyPower, int i) 
{ 
   float3 lightDir = mul(float4(lights[i].vPos,1.0f), matView).xyz - pos; //!! do in vertex shader?! or completely before?!
   float3 L = normalize(lightDir);
   float NdotL = dot(N, L);
   float3 Out = float3(0.0f,0.0f,0.0f);
   
   // compute diffuse color (lambert with optional rim/wrap component)
   if(!bIsMetal && (NdotL + fWrapLighting > 0.0f))
      Out = diffuse * ((NdotL + fWrapLighting) / ((1.0f+fWrapLighting) * (1.0f+fWrapLighting)));
 
   // add glossy component (modified ashikhmin/blinn bastard), not fully energy conserving, but good enough
   if(NdotL > 0.0f)
   {
	 float3 H = normalize(L + V); // half vector
	 float NdotH = dot(N, H);
	 float LdotH = dot(L, H);
	 float VdotH = dot(V, H);
	 if((NdotH > 0.0f) && (LdotH > 0.0f) && (VdotH > 0.0f))
		Out += FresnelSchlick(glossy, LdotH, edge) * (((glossyPower + 1.0f) / (8.0f*VdotH)) * pow(NdotH, glossyPower));
   }
 
   //float fAtten = saturate( 1.0f - dot(lightDir/flightRange, lightDir/flightRange) );
   //float fAtten = 1.0f/dot(lightDir,lightDir); // original/correct falloff
   
   float sqrl_lightDir = dot(lightDir,lightDir); // tweaked falloff to have ranged lightsources
   float fAtten = saturate(1.0f - sqrl_lightDir*sqrl_lightDir/(flightRange*flightRange*flightRange*flightRange)); //!! pre-mult/invert flightRange?
   fAtten = fAtten*fAtten/(sqrl_lightDir + 1.0f);

   Out *= lights[i].vEmission * fAtten;
   
   return Out; 
}

// does /PI-corrected lookup/final color already
float3 DoEnvmapDiffuse(float3 N, float3 diffuse)
{
   float2 uv = float2( // remap to 2D envmap coords
		atan2(N.y, N.x) * (0.5f/PI) + 0.5f,
	    acos(N.z) * (1.0f/PI));

   return diffuse * InvGamma(tex2Dlod(texSampler2, float4(uv, 0.f,0.f)).xyz)*fenvEmissionScale; //!! replace by real HDR instead? -> remove invgamma then
}

//!! PI?
// very very crude approximation by abusing miplevels
float3 DoEnvmapGlossy(float3 N, float3 V, float3 glossy, float glossyPower)
{
   float3 r = reflect(-V,N);
   r = normalize(mul(float4(r,0.0f), matViewInverse).xyz); // trafo back to world

   float mip = log2(fenvTexWidth * sqrt(3.0f)) - 0.5f*log2(glossyPower + 1.0f);

   float2 uv = float2( // remap to 2D envmap coords
		atan2(r.y, r.x) * (0.5f/PI) + 0.5f,
	    acos(r.z) * (1.0f/PI));

   return glossy * InvGamma(tex2Dlod(texSampler1, float4(uv, 0, mip)).xyz)*fenvEmissionScale; //!! replace by real HDR instead? -> remove invgamma then
}

//!! PI?
float3 DoEnvmap2ndLayer(float3 color1stLayer, float3 pos, float3 N, float3 V, float3 specular)
{
   float3 r = reflect(-V,N);
   r = normalize(mul(float4(r,0.0f), matViewInverse).xyz); // trafo back to world

   float2 uv = float2( // remap to 2D envmap coords
		atan2(r.y, r.x) * (0.5f/PI) + 0.5f,
	    acos(r.z) * (1.0f/PI));
	    
   float3 w = FresnelSchlick(specular, dot(V, N), fEdge); //!! ?
   return lerp(color1stLayer, InvGamma(tex2Dlod(texSampler1, float4(uv, 0, 0)).xyz)*fenvEmissionScale, w); // weight (optional) lower diffuse/glossy layer with clearcoat/specular //!! replace by real HDR instead? -> remove invgamma then
}

float4 lightLoop(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, float3 specular, float edge)
{
   // normalize input vectors for BRDF evals
   N = normalize(N);
   V = normalize(V);
   
   // normalize BRDF layer inputs //!! use diffuse = (1-glossy)*diffuse instead?
   float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));
   float glossyMax = max(glossy.x,max(glossy.y,glossy.z));
   float specularMax = max(specular.x,max(specular.y,specular.z)); //!! not needed as 2nd layer only so far
   float sum = diffuseMax + glossyMax /*+ specularMax*/;
   if(sum > 1.0f)
   {
      float invsum = 1.0f/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      //specular *= invsum;
   }

   if(dot(N,V) < 0.0f) // flip normal in case of wrong orientation? (backside lighting)
      N = -N;

   float3 color = float3(0.0f, 0.0f, 0.0f);
      
   // 1st Layer
   if((!bIsMetal && (diffuseMax > 0.0f)) || (glossyMax > 0.0f))
   {
      for(int i = 0; i < iLightPointNum; i++)
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, fRoughness, i); // no clearcoat needed as only pointlights so far
   }
         
   if(!bIsMetal && (diffuseMax > 0.0f))
      color += DoEnvmapDiffuse(N, diffuse);

   if(glossyMax > 0.0f)
      color += DoEnvmapGlossy(N, V, glossy, fRoughness);

   // 2nd Layer
   if(specularMax > 0.0f)
      color = DoEnvmap2ndLayer(color, pos, N, V, specular);
  
   return float4(/*Gamma(ToneMap(*/vAmbient + color/*))*/, 1.0f);
}

float4 Additive(float4 cBase, float4 cBlend, float percent)
{
   return cBase + cBlend*percent;
}

float4 Screen (float4 cBase, float4 cBlend, float percent)
{
	return 1.0f - (1.0f - cBase) * (1.0f - cBlend);
}

float4 Multiply (float4 cBase, float4 cBlend, float percent)
{
	return cBase * (cBlend*percent);
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
	cNew= lerp(cBase*blend*2.0f, 1.0f-2.0f*(1.0f-cBase)*(1.0f-blend), cNew);

	//cNew.a = 1.0f;
	return cNew;
}
