//!! change to PB model, 2layer? (for clearcoat)

#define NUM_LIGHTS 2

#define PI 3.1415926535897932384626433832795f
 
float3 vDiffuseColor = float3(192.f/255.f, 128.f/255.f, 96.f/255.f);
float3 vSpecularColor = float3(1.0f, 1.0f, 1.0f); //!! pass from material & texture
float  fMaterialPower = 16.f; //!! pass from material
float  fmaterialAlpha = 1.0f; //!! allow for texture
bool   bSpecular = false; //!! remove, steer from specular
float  fWrap = 0.5f; //!! pass from material, w in [0..1] for wrap lighting
      
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
 
//transformation matrices 
float4x4 matWorldViewProj  : WORLDVIEWPROJ; 
//float4x4 matWorldView      : WORLDVIEW; 
float4x4 matWorld          : WORLD; 
float4x4 matWorldInverseTranspose; 

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
   float4 pos           : POSITION; 
   float2 tex0          : TEXCOORD0; 
   float3 worldPos      : TEXCOORD1; 
   float3 normal        : TEXCOORD2;
   float3 viewDir       : TEXCOORD3;
}; 

 
float3 FresnelSchlick(float3 spec, float LdotH)
{
    return spec + (1.0f - spec) * pow(1.0f - LdotH, 5);
}

// assumes all light input premultiplied by PI
float3 DoPointLight(float3 pos, float3 N, float3 V, float3 diffuse, float3 specular, int i) 
{ 
   float3 lightDir = lights[i].vPos - pos;
   float3 L = normalize(lightDir);
   float NdotL = dot(N, L);
   float3 Out = float3(0.0f,0.0f,0.0f);
   
   if(NdotL + fWrap > 0.0f)
   { 
      //compute diffuse color 
      Out = diffuse * ((NdotL + fWrap) / ((1.0f+fWrap) * (1.0f+fWrap)));
 
      //add specular component 
      if(bSpecular && (NdotL > 0.0f)) 
      { 
		 float3 H = normalize(L + V);   //half vector 
		 float NdotH = dot(N, H);
		 float LdotH = dot(L, H);
		 if((NdotH > 0.0f) && (LdotH > 0.0f))
			Out += specular * FresnelSchlick(specular, LdotH) * (((fMaterialPower + 1.0f) / (8.0f*dot(H,V))) * pow(NdotH, fMaterialPower));
      }
 
      float fAtten = saturate( 1.0f - dot(lightDir/flightRange, lightDir/flightRange) ); //!!
  	  //float fAtten = flightRange/dot(lightDir,lightDir);
      Out *= lights[i].vEmission * fAtten; 
   } 
   
   return Out; 
} 

float4 lightLoop(float3 pos, float3 N, float3 V, float3 diffuse, float3 specular)
{
   //!! normalize spec diffuse

   N = normalize(N);
   V = normalize(V);

   //if(dot(N,V) < 0.0f) //!! ?
   //    N = -N;

   float3 color = float3(0.0f, 0.0f, 0.0f);
   
   for(int i = 0; i < iLightPointNum; i++)  
   { 
      color += DoPointLight(pos, N, V, diffuse, specular, i); 
   } 
  
   return float4(saturate(vAmbient + color), fmaterialAlpha);
}


VS_OUTPUT vs_main (float4 vPosition  : POSITION0,  
                   float3 vNormal    : NORMAL0,  
                   float2 tc         : TEXCOORD0, 
                   float2 tc2        : TEXCOORD1) 
{ 
   VS_OUTPUT Out = (VS_OUTPUT)0;

   //trafo all into world space
   float3 P = mul(vPosition,matWorld).xyz;
   float3 C = mul(camera,matWorld).xyz;
   float3 N = normalize(mul(float4(vNormal,0.0f),matWorldInverseTranspose).xyz);
   float3 V = normalize(C-P); //view direction

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;
   Out.worldPos = P;
   Out.normal = N;
   Out.viewDir = V;
   
   return Out; 
} 

float4 ps_main( in VS_OUTPUT IN) : COLOR
{
   float3 diffuse = vDiffuseColor;
   float3 specular = vSpecularColor;
   
   return lightLoop(IN.worldPos, IN.normal, IN.viewDir, diffuse, specular);
}

float4 ps_main_texture( in VS_OUTPUT IN) : COLOR
{
   float3 diffuse = vDiffuseColor * tex2D(texSampler0, IN.tex0);
   float3 specular = vSpecularColor;
   
   return lightLoop(IN.worldPos, IN.normal, IN.viewDir, diffuse, specular);
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
