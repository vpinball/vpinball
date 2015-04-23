//!! have switch to choose if texture is weighted by diffuse/glossy or is just used raw?

#define NUM_BALL_LIGHTS 0 // just to avoid having too much constant mem allocated

#include "Helpers.fxh"

// transformation matrices
float4x4 matWorldViewProj : WORLDVIEWPROJ;
float4x4 matWorldView     : WORLDVIEW;
float4x4 matWorldViewInverseTranspose;
float4x4 matView;
float4x4 matViewInverse;

texture Texture0; // base texture
texture Texture1; // envmap
texture Texture2; // envmap radiance
texture Texture3; // bulb light buffer
 
sampler2D texSampler0 : TEXUNIT0 = sampler_state // base texture
{
	Texture	  = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
	//ADDRESSU  = Wrap; //!! ?
	//ADDRESSV  = Wrap;
	//!! SRGBTexture = true;
};

sampler2D texSampler1 : TEXUNIT1 = sampler_state // environment
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

sampler2D texSamplerBL : TEXUNIT3 = sampler_state // bulb light/transmission buffer texture
{
	Texture	  = (Texture3);
    MIPFILTER = NONE; //!! ??
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};


#include "Material.fxh"

float4 cClearcoat_EdgeAlpha;
float3 cGlossy;
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance:  0.5-1.0

float fAlphaTestValue;

struct VS_OUTPUT 
{ 
   float4 pos      : POSITION; 
   float2 tex0     : TEXCOORD0; 
   float2 tex1     : TEXCOORD1;
   float3 worldPos : TEXCOORD2; 
   float3 normal   : TEXCOORD3;
};

struct VS_NOTEX_OUTPUT 
{ 
   float4 pos      : POSITION; 
   float2 tex1     : TEXCOORD0;
   float3 worldPos : TEXCOORD1; 
   float3 normal   : TEXCOORD2;
};

//------------------------------------
//
// Standard Materials
//

VS_OUTPUT vs_main (float4 vPosition : POSITION0,  
                   float3 vNormal   : NORMAL0,  
                   float2 tc        : TEXCOORD0) 
{ 
   VS_OUTPUT Out;

   // trafo all into worldview space (as most of the weird trafos happen in view, world is identity so far)
   const float3 P = mul(vPosition, matWorldView).xyz;
   const float3 N = normalize(mul(float4(vNormal,0.0), matWorldViewInverseTranspose).xyz);

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;
   //if(cBase_Alpha.a < 1.0)
      Out.tex1 = Out.pos.xy/Out.pos.w;
   Out.worldPos = P;
   Out.normal = N;
   
   return Out; 
}

VS_NOTEX_OUTPUT vs_notex_main (float4 vPosition : POSITION0,  
                               float3 vNormal   : NORMAL0,  
                               float2 tc        : TEXCOORD0) 
{ 
   VS_NOTEX_OUTPUT Out;

   // trafo all into worldview space (as most of the weird trafos happen in view, world is identity so far)
   const float3 P = mul(vPosition, matWorldView).xyz;
   const float3 N = normalize(mul(float4(vNormal,0.0), matWorldViewInverseTranspose).xyz);

   Out.pos = mul(vPosition, matWorldViewProj);
   //if(cBase_Alpha.a < 1.0)
      Out.tex1 = Out.pos.xy/Out.pos.w;
   Out.worldPos = P;
   Out.normal = N;
   
   return Out; 
}

float4 ps_main(in VS_NOTEX_OUTPUT IN) : COLOR
{
   //return float4((IN.normal+1.0)*0.5,1.0); // visualize normals
   
   const float3 diffuse  = cBase_Alpha.xyz;
   const float3 glossy   = (Roughness_WrapL_Edge_IsMetal.w != 0.0) ? cBase_Alpha.xyz : cGlossy*0.08;
   const float3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
   const float  edge     = (Roughness_WrapL_Edge_IsMetal.w != 0.0) ? 1.0 : Roughness_WrapL_Edge_IsMetal.z;
   
   const float3 N = normalize(IN.normal);
   const float3 V = normalize(/*camera=0,0,0,1*/-IN.worldPos);

   float4 result;
   result.xyz = lightLoop(IN.worldPos, N, V, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
   result.a = cBase_Alpha.a;

   if(cBase_Alpha.a < 1.0) {
      result.a = lerp(result.a, 1.0, cClearcoat_EdgeAlpha.w*pow(1.0-abs(dot(N,V)),5)); // fresnel for falloff towards silhouette, flip normal in case of wrong orientation (backside lighting)

      // add light from "below" from user-flagged bulb lights, pre-rendered/blurred in previous renderpass //!! sqrt = magic
      result.xyz += sqrt(diffuse)*tex2Dlod(texSamplerBL, float4(float2(0.5*IN.tex1.x,-0.5*IN.tex1.y)+0.5, 0.,0.)).xyz*result.a; //!! depend on normal of light (unknown though) vs geom normal, too?
   }

   return result;
}

float4 ps_main_texture(in VS_OUTPUT IN) : COLOR
{
   //return float4((IN.normal+1.0)*0.5,1.0); // visualize normals
   
   float4 pixel = tex2D(texSampler0, IN.tex0);

   if (pixel.a<=fAlphaTestValue)
      clip(-1);           //stop the pixel shader if alpha test should reject pixel

   pixel.a *= cBase_Alpha.a;
   const float3 t = InvGamma(pixel.xyz);

   // early out if no normal set (e.g. decal vertices)
   if(!any(IN.normal))
      return float4(InvToneMap(t*cBase_Alpha.xyz),pixel.a);
      
   const float3 diffuse  = t*cBase_Alpha.xyz;
   const float3 glossy   = (Roughness_WrapL_Edge_IsMetal.w != 0.0) ? diffuse : t*cGlossy*0.08; //!! use AO for glossy? specular?
   const float3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
   const float  edge     = (Roughness_WrapL_Edge_IsMetal.w != 0.0) ? 1.0 : Roughness_WrapL_Edge_IsMetal.z;

   const float3 N = normalize(IN.normal);
   const float3 V = normalize(/*camera=0,0,0,1*/-IN.worldPos);   

   float4 result;
   result.xyz = lightLoop(IN.worldPos, N, V, diffuse, glossy, specular, edge);
   result.a = pixel.a;

   if(cBase_Alpha.a < 1.0 && result.a < 1.0) {
      result.a = lerp(result.a, 1.0, cClearcoat_EdgeAlpha.w*pow(1.0-abs(dot(N,V)),5)); // fresnel for falloff towards silhouette, flip normal in case of wrong orientation (backside lighting)

      // add light from "below" from user-flagged bulb lights, pre-rendered/blurred in previous renderpass //!! sqrt = magic
      result.xyz += sqrt(diffuse)*tex2Dlod(texSamplerBL, float4(float2(0.5*IN.tex1.x,-0.5*IN.tex1.y)+0.5, 0.,0.)).xyz*result.a; //!! depend on normal of light (unknown though) vs geom normal, too?
   }

   return result;
}

//------------------------------------------
// Kicker boolean vertex shader

float fKickerScale = 1.;

VS_NOTEX_OUTPUT vs_kicker (float4 vPosition : POSITION0,  
                           float3 vNormal   : NORMAL0,  
                           float2 tc        : TEXCOORD0) 
{ 
    VS_NOTEX_OUTPUT Out;
    const float3 P = mul(vPosition, matWorldView).xyz;
    float4 P2 = vPosition;
    const float3 N = normalize(mul(float4(vNormal,0.0), matWorldViewInverseTranspose).xyz);

    Out.pos = mul(vPosition, matWorldViewProj);
    P2.z -= 70.0f*fKickerScale;
    P2 = mul(P2, matWorldViewProj);
    Out.pos.z = P2.z;
    //if(cBase_Alpha.a < 1.0)
        Out.tex1 = Out.pos.xy/Out.pos.w; //!! not necessary
    Out.worldPos = P;
    Out.normal = N;
   
    return Out; 
}

//------------------------------------
// Techniques
//

//
// Standard Materials
//

technique basic_without_texture
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_notex_main(); 
	  PixelShader = compile ps_3_0 ps_main();
   } 
}

technique basic_with_texture
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main_texture();
   } 
}

//
// Kicker
//

technique kickerBoolean
{ 
   pass P0 
   { 
      //ZWriteEnable=TRUE;
      VertexShader = compile vs_3_0 vs_kicker(); 
	  PixelShader = compile ps_3_0 ps_main();
   } 
}

#ifndef SEPARATE_CLASSICLIGHTSHADER
 #include "ClassicLightShader.fx"
#endif
