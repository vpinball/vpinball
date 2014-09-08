//!! change to PB model, 2layer? (for clearcoat)
//!! change all to float3 pipeline

#define NUM_LIGHTS 2

#define PI 3.1415926535897932384626433832795f
 
float3 vDiffuseColor = float3(192.f/255.f, 128.f/255.f, 96.f/255.f);
float3 vSpecularColor = float3(1.0f, 1.0f, 1.0f); //!! pass from material & texture
float  fMaterialPower = 16.f; //!! allow for texture
float  materialAlpha = 1.0f; //!! allow for texture
bool   bSpecular = false; 
float  fWrap = 0.5f; //!! pass from material, w in [0..1] for wrap lighting
      
float3 vAmbient = float3(0.0f,0.0f,0.0f);
float  lightRange = 3000.0f;

struct CLight 
{ 
   float3 vPos; 
   float3 vDiffuse; //!! only have one emission
   float3 vSpecular; 
}; 
 
int iLightPointNum=NUM_LIGHTS; 
CLight lights[NUM_LIGHTS] = {          //NUM_LIGHTS == 2
   { 
      float3(0.0f, 0.0f, 0.0f),        //position 
      float3(0.0f, 0.0f, 0.0f),        //diffuse 
      float3(0.0f, 0.0f, 0.0f),        //specular 
   }, 
   { 
      float3(0.0f, 0.0f, 0.0f),        //position 
      float3(0.0f, 0.0f, 0.0f),        //diffuse 
      float3(0.0f, 0.0f, 0.0f),        //specular 
   } 
}; 
 
//transformation matrices 
float4x4 matWorldViewProj  : WORLDVIEWPROJ; 
float4x4 matWorldView      : WORLDVIEW; 
float4x4 matWorld          : WORLD; 

float4 camera;

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
   float4 Pos           : POSITION; 
   float2 Tex0          : TEXCOORD0; 
   float3 worldPos      : TEXCOORD1; 
   float3 normal        : TEXCOORD2;
   float3 viewPos       : TEXCOORD3;
}; 
 
float3 FresnelSchlick(float3 spec, float3 E, float3 H)
{
    return spec + (1.0f - spec) * pow(1.0f - saturate(dot(E, H)), 5);
}

// assumes all light input premultiplied by PI
float3 DoPointLight(float3 vPosition, float3 N, float3 V, float3 diffuse, float3 specular, int i) 
{ 
   float3 pos = (float3)mul(matWorld,vPosition);
   float3 light = lights[i].vPos;
   float3 lightDir = light-pos;
   float3 L = normalize(lightDir); 
   float NdotL = dot(N, L); 
   float3 Out = float3(0.0f,0.0f,0.0f);
   
   if(NdotL >= 0.f) 
   { 
      //compute diffuse color 
      Out = /*(NdotL * lights[i].vDiffuse * diffuse); /*/ lights[i].vDiffuse * diffuse * (NdotL + fWrap)/((1.0f+fWrap) * (1.0f+fWrap));
 
      //add specular component 
      if(bSpecular) 
      { 
		 float3 H = normalize(L + V);   //half vector 
         Out += FresnelSchlick(lights[i].vSpecular * specular, L, H) * ((fMaterialPower + 2.0f) / 8.0f ) * pow(saturate(dot(N, H)), fMaterialPower) * NdotL * lights[i].vSpecular * specular;
         //Out += pow(max(0.f, dot(H,N)), fMaterialPower) * lights[i].vSpecular * specular; 
      } 
 
      float fAtten = saturate( 1.0f - dot(lightDir/lightRange, lightDir/lightRange) ); //!!
  	  //float fAtten = lightRange*lightRange/dot(lightDir,lightDir);
      Out *= fAtten; 
   } 
   
   return Out; 
} 

 
//----------------------------------------------------------------------------- 
// Name: vs_main() 
// Desc: The vertex shader 
//----------------------------------------------------------------------------- 
VS_OUTPUT vs_main (float4 vPosition  : POSITION0,  
                   float3 vNormal    : NORMAL0,  
                   float2 tc         : TEXCOORD0, 
                   float2 tc2        : TEXCOORD1) 
{ 
   VS_OUTPUT Out = (VS_OUTPUT) 0; 
 
   vNormal = normalize(vNormal); 
   Out.Pos = mul(vPosition, matWorldViewProj); 
 
   float3 P = mul(matWorldView,vPosition).xyz;           //position in view space 
   float4 nn = float4(vNormal,0.0f);
   float3 N = normalize(mul(matWorld,nn).xyz);
   float3 C = mul(matWorldView,camera).xyz;
   float3 V = normalize(P-C);                          //viewer
 
   Out.Tex0 = tc; 
   Out.worldPos = vPosition.xyz;
   Out.normal = N;
   Out.viewPos = V;
   
   return Out; 
} 

float4 ps_main( in VS_OUTPUT IN) : COLOR
{	
   float3 color = float3(0.0f, 0.0f, 0.0f);
   
   for(int i = 0; i < iLightPointNum; i++)  
   { 
      color += DoPointLight(IN.worldPos, IN.normal, IN.viewPos, vDiffuseColor, vSpecularColor, i); 
   } 
  
   return float4(saturate(vAmbient + color), materialAlpha);
}

float4 ps_main_texture( in VS_OUTPUT IN) : COLOR
{
   float3 color = float3(0.0f, 0.0f, 0.0f);
   
   for(int i = 0; i < iLightPointNum; i++)  
   { 
      color += DoPointLight(IN.worldPos, IN.normal, IN.viewPos, vDiffuseColor*tex2D(texSampler0,IN.Tex0), vSpecularColor, i); 
   } 
 
   return float4(saturate(vAmbient + color), materialAlpha);
}

// Techniques 
technique basic_without_texture
{ 
   pass P0 
   { 
      SPECULARENABLE = (bSpecular); 
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main();
   } 
}

technique basic_with_texture
{ 
   pass P0 
   { 
      SPECULARENABLE = (bSpecular); 
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main_texture();
   } 
} 
