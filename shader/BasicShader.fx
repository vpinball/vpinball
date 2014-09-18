#define NUM_LIGHTS 2

#define PI 3.1415926535897932384626433832795f
 
float3 vDiffuseColor = float3(0.5f, 0.5f, 0.5f); //!! 0.04-0.95 in RGB
float  fDiffuseWrap = 0.5f; //!! pass from material, w in [0..1] for rim/wrap lighting

float3 vGlossyColor = float3(0.04f, 0.04f, 0.04f); //!! pass from material/texture
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance:  0.5-1.0
float  fGlossyPower = 0.1f; //!! pass from material(/texture?)

float3 vSpecularColor = float3(0.04f, 0.04f, 0.04f); //!! pass from material/texture

float  fmaterialAlpha = 1.0f; //!! allow for texture? -> use from diffuse? and/or glossy,etc?

bool   bDiffuse  = true;  //!! remove, steer from diffuse?  (performance?)
bool   bGlossy   = false; //!! remove, steer from glossy?   (performance?)
bool   bSpecular = false; //!! remove, steer from specular? (performance?)

bool   bPerformAlphaTest = false;
float  fAlphaTestValue = 128.0f/255.0f;
      
float3 vAmbient = float3(0.0f,0.0f,0.0f); //!! remove completely, just rely on envmap/IBL?

float EnvEmissionScale = 10.0f; //!! also have envmap of fixed size?
float LightEmissionScale = 1000000.0f; //!! remove! put into emission below

float  flightRange = 3000.0f; //!! ??

struct CLight 
{ 
   float3 vPos; 
   float3 vEmission;
}; 
 
int iLightPointNum=NUM_LIGHTS; 
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
 
// transformation matrices 
float4x4 matWorldViewProj  : WORLDVIEWPROJ;
float4x4 matWorldView      : WORLDVIEW;
//float4x4 matWorld          : WORLD;
float4x4 matWorldViewInverseTranspose;
float4x4 matView;
float4x4 matViewInverse;

//float4 camera;

texture Texture0; // diffuse
//texture Texture1; // glossy //!!
//texture Texture2; // specular //!!
texture Texture3; // envmap

sampler2D texSampler0 : TEXUNIT0 = sampler_state
{
	Texture	  = (Texture0);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
};

/*sampler2D texSampler1 : TEXUNIT1 = sampler_state
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
};*/

sampler2D texSampler3 : TEXUNIT3 = sampler_state
{
	Texture	  = (Texture3);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
};
 
//function output structures 
struct VS_OUTPUT 
{ 
   float4 pos           : POSITION; 
   float2 tex0          : TEXCOORD0; 
   float3 worldPos      : TEXCOORD1; 
   float3 normal        : TEXCOORD2;
};

//------------------------------------
 
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

// assumes all light emission is premultiplied by PI
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

//!! diffuse missing so far -> needs LUT with normal(->2D mapped) input
//!! PI?
float3 DoEnvmapDiffuse(float3 N, float3 diffuse)
{
   return float3(0,0,0); //!!
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
   return lerp(color1stLayer, InvGamma(tex2D(texSampler3, uv).xyz)*EnvEmissionScale, w); // weight (optional) lower diffuse/glossy layer with clearcoat/specular //!! replace by real HDR instead? -> remove invgamma then
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
  
   return float4(Gamma(ToneMap(vAmbient + color)), fmaterialAlpha); //!! in case of HDR out later on, remove tonemap and gamma //!! also problematic for alpha blends
}

//------------------------------------

VS_OUTPUT vs_main (float4 vPosition  : POSITION0,  
                   float3 vNormal    : NORMAL0,  
                   float2 tc         : TEXCOORD0, 
                   float2 tc2        : TEXCOORD1) 
{ 
   VS_OUTPUT Out;

   // trafo all into worldview space (as most of the weird trafos happen in view, world is identity so far)
   float3 P = mul(vPosition, matWorldView).xyz;
   float3 N = normalize(mul(float4(vNormal,0.0f), matWorldViewInverseTranspose).xyz);

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;
   Out.worldPos = P;
   Out.normal = N;
   
   return Out; 
}

float4 ps_main( in VS_OUTPUT IN) : COLOR
{
   float3 diffuse  = vDiffuseColor;
   float3 glossy   = vGlossyColor;
   float3 specular = vSpecularColor;
   
   return lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
}

float4 ps_main_texture( in VS_OUTPUT IN) : COLOR
{
   float4 pixel = tex2D(texSampler0, IN.tex0);
   
   if (bPerformAlphaTest && pixel.a<=fAlphaTestValue )
    clip(-1);           //stop the pixel shader if alpha test should reject pixel
    
   float3 t = InvGamma(pixel.xyz);
   float3 diffuse  = vDiffuseColor*t;
   float3 glossy   = vGlossyColor*t;
   float3 specular = vSpecularColor; //!! texture?
   
   return lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular);
}

float4 ps_main_texture_no_lighting( in VS_OUTPUT IN) : COLOR
{
   return tex2D(texSampler0, IN.tex0);
}

//------------------------------------
// Techniques

technique basic_without_texture
{ 
   pass P0 
   { 
      //SPECULARENABLE = (bSpecular);
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main();
   } 
}

technique basic_with_texture
{ 
   pass P0 
   { 
      //SPECULARENABLE = (bSpecular);
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main_texture();
   } 
}


technique basic_with_texture_no_lighting
{ 
   pass P0 
   { 
      //SPECULARENABLE = (bSpecular);
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main_texture_no_lighting();
   } 
}
