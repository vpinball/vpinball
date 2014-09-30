#include "Globals.fxh"

float3 vGlossyColor = float3(0.04f, 0.04f, 0.04f); //!! pass from material/texture
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance:  0.5-1.0
float3 vSpecularColor = float3(0.04f, 0.04f, 0.04f); //!! pass from material/texture

bool   bPerformAlphaTest = false;
float  fAlphaTestValue = 128.0f/255.0f;

//function output structures 
struct VS_OUTPUT 
{ 
   float4 pos           : POSITION; 
   float2 tex0          : TEXCOORD0; 
   float3 worldPos      : TEXCOORD1; 
   float3 normal        : TEXCOORD2;
};

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
   
   float4 result=lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
   result.a *= fmaterialAlpha;
   return result;
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
   float4 result = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular);
   result.a *= fmaterialAlpha;
   return result;
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
