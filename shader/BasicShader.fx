#define NUM_LIGHTS 2

#define PI 3.1415926535897932384626433832795f
 
float3 vDiffuseColor = float3(192.f/255.f, 128.f/255.f, 96.f/255.f);
float  fDiffuseWrap = 0.5f; //!! pass from material, w in [0..1] for rim/wrap lighting

float3 vGlossyColor = float3(1.0f, 1.0f, 1.0f); //!! pass from material/texture
float  fGlossyPower = 16.f; //!! pass from material(/texture?)

float3 vSpecularColor = float3(1.0f, 1.0f, 1.0f); //!! pass from material/texture

float  fmaterialAlpha = 1.0f; //!! allow for texture?

bool   bDiffuse  = true;  //!! remove, steer from diffuse?  (performance?)
bool   bGlossy   = false; //!! remove, steer from glossy?   (performance?)
bool   bSpecular = false; //!! remove, steer from specular? (performance?)
      
float3 vAmbient = float3(0.0f,0.0f,0.0f);

float  flightRange = 3000.0f;

struct CLight 
{ 
   float3 vPos; 
   float3 vEmission;
}; 
 
int iLightPointNum=NUM_LIGHTS; 
CLight lights[NUM_LIGHTS] = {          //NUM_LIGHTS == 2
   { 
      float3(0.0f, 0.0f, 0.0f),        //position 
      float3(0.0f, 0.0f, 0.0f)         //emission 
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

//float4 camera;

texture Texture0;

sampler2D texSampler0 : TEXUNIT0 = sampler_state
{
	Texture	  = (Texture0);
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

 
float3 FresnelSchlick(float3 spec, float LdotH)
{
    return spec + (1.0f - spec) * pow(1.0f - LdotH, 5);
}

// assumes all light emission is premultiplied by PI
float3 DoPointLight(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, int i) 
{ 
   float3 lightDir = mul(float4(lights[i].vPos,1.0f), matWorldView).xyz - pos; //!! do in vertex shader?!
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
	 float VdotH = dot(V ,H);
	 if((NdotH > 0.0f) && (LdotH > 0.0f) && (VdotH > 0.0f))
		Out += glossy * FresnelSchlick(glossy, LdotH) * (((fGlossyPower + 1.0f) / (8.0f*VdotH)) * pow(NdotH, fGlossyPower));
   }
 
   float fAtten = saturate( 1.0f - dot(lightDir/flightRange, lightDir/flightRange) ); //!!
   //float fAtten = flightRange/dot(lightDir,lightDir);

   Out *= lights[i].vEmission * fAtten;
   
   return Out; 
}

//!! PI?
float3 DoEnvmap(float3 pos, float3 N, float3 V, float3 diffuse, float3 specular)
{
   return float3(0,0,0); //!!
}

float4 lightLoop(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, float3 specular)
{
   // normalize input vectors for BRDF evals
   N = normalize(N);
   V = normalize(V);
   
   // normalize BRDF layer inputs
   float diffuseMax = bDiffuse ? max(diffuse.x,max(diffuse.y,diffuse.z)) : 0.0f;
   float glossyMax = bGlossy ? max(glossy.x,max(glossy.y,glossy.z)) : 0.0f;
   float specularMax = bSpecular ? max(specular.x,max(specular.y,specular.z)) : 0.0f;
   float sum = diffuseMax + glossyMax + specularMax;
   if(sum > 1.0f)
   {
      float invsum = 1.0f/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      specular *= invsum;
   }

   //if(dot(N,V) < 0.0f) //!! flip normal in case of wrong orientation? (backside lighting)
   //   N = -N;

   float3 color = float3(0.0f, 0.0f, 0.0f);
      
   if((bDiffuse && diffuseMax > 0.0f) || (bGlossy && glossyMax > 0.0f))
      for(int i = 0; i < iLightPointNum; i++)  
         color += DoPointLight(pos, N, V, diffuse, glossy, i); // no specular needed as only pointlights so far
         
   if((bDiffuse && diffuseMax > 0.0f) || (bSpecular && specularMax > 0.0f))
      color += DoEnvmap(pos, N, V, diffuse, specular); // no glossy, as it's the most hacky one //!! -> use mipmap-hacks for glossy?
  
   return float4(saturate(vAmbient + color), fmaterialAlpha); //!! in case of HDR out later on, remove saturate
}


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
   
   return lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular);
}

float4 ps_main_texture( in VS_OUTPUT IN) : COLOR
{
   float3 diffuse  = vDiffuseColor * tex2D(texSampler0, IN.tex0);
   float3 glossy   = vGlossyColor;
   float3 specular = vSpecularColor;
   
   return lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular);
}

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
