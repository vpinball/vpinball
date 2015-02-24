//!! have switch to choose if texture is weighted by diffuse/glossy or is just used raw?

#define NUM_BALL_LIGHTS 0 // just to avoid having to much constant mem allocated

#include "Globals.fxh"

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

#include "Material.fxh"

float3 cGlossy;
float3 cClearcoat;
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance:  0.5-1.0

float fAlphaTestValue;

struct VS_OUTPUT 
{ 
   float4 pos      : POSITION; 
   float2 tex0     : TEXCOORD0; 
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
   Out.worldPos = P;
   Out.normal = N;
   
   return Out; 
}

float4 ps_main(in VS_OUTPUT IN) : COLOR
{
   //return float4((IN.normal+1.0)*0.5,1.0); // visualize normals
   
   const float3 diffuse  = cBase_Alpha.xyz;
   const float3 glossy   = bIsMetal ? cBase_Alpha.xyz : cGlossy*0.08;
   const float3 specular = cClearcoat*0.08;
   const float edge = bIsMetal ? 1.0 : Roughness_WrapL_Edge.z;
   
   float4 result;
   result.xyz = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
   result.a = cBase_Alpha.a;
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
   if(IN.normal.x == 0.0 && IN.normal.y == 0.0 && IN.normal.z == 0.0)
      return float4(InvToneMap(t*cBase_Alpha.xyz),pixel.a);
      
   const float3 diffuse  = t*cBase_Alpha.xyz;
   const float3 glossy   = bIsMetal ? diffuse : t*cGlossy*0.08; //!! use AO for glossy? specular?
   const float3 specular = cClearcoat*0.08;
   const float edge = bIsMetal ? 1.0 : Roughness_WrapL_Edge.z;

   float4 result;
   result.xyz = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge);
   result.a = pixel.a;
   return result;
}

//------------------------------------------
// Light (Bulb/Shapes)

float4   lightColor_intensity;
float4   lightColor2_falloff_power;
float4   lightCenter_maxRange;
float    blend_modulate_vs_add;
bool imageMode;
bool backglassMode;

struct VS_LIGHT_OUTPUT 
{ 
   float4 pos           : POSITION; 
   float2 tex0          : TEXCOORD0; 
   float3 worldPos      : TEXCOORD1;
   float3 tablePos      : TEXCOORD2; 
   float3 normal        : TEXCOORD3;
};

VS_LIGHT_OUTPUT vs_light_main (float4 vPosition : POSITION0,  
                               float3 vNormal   : NORMAL0,  
                               float2 tc        : TEXCOORD0) 
{ 
   VS_LIGHT_OUTPUT Out;

   // trafo all into worldview space (as most of the weird trafos happen in view, world is identity so far)
   const float3 P = mul(vPosition, matWorldView).xyz;
   const float3 N = normalize(mul(float4(vNormal,0.0), matWorldViewInverseTranspose).xyz);

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;
   Out.tablePos = vPosition.xyz;
   Out.worldPos = P;
   Out.normal = N;
   
   return Out; 
}

float4 PS_LightWithTexel(in VS_LIGHT_OUTPUT IN) : COLOR
{	
    float4 pixel = tex2D(texSampler0, IN.tex0); //!! IN.tex0 abused in backglass mode  
    pixel.xyz = InvGamma(pixel.xyz);

    float4 color;
	// no lighting if HUD vertices or passthrough mode
    if(imageMode || backglassMode)
        color = pixel;
    else
	{
	    const float3 diffuse = pixel.xyz*cBase_Alpha.xyz;
        const float3 glossy = bIsMetal ? diffuse : pixel.xyz*cGlossy*0.08; //!! use AO for glossy? specular?
        const float3 specular = cClearcoat*0.08;
        const float edge = bIsMetal ? 1.0 : Roughness_WrapL_Edge.z;

	    color.xyz = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
		color.a = pixel.a;
    }
    color.a *= cBase_Alpha.a;

    if ( lightColor_intensity.w!=0.0 )
    {
        const float len = length(lightCenter_maxRange.xyz - (!backglassMode ? IN.tablePos : float3(IN.tex0,0.0))) * lightCenter_maxRange.w;
        const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
        const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
        color += float4(lcolor*(atten*lightColor_intensity.w),
		                saturate(atten*lightColor_intensity.w));
        color = Overlay(pixel, color);
        color = Screen(pixel, color);
    }

    return color;
}

float4 PS_LightWithoutTexel(in VS_LIGHT_OUTPUT IN) : COLOR
{
    float4 result = float4(0.0, 0.0, 0.0, 0.0);
    if (lightColor_intensity.w != 0.0)
    {
        const float len = length(lightCenter_maxRange.xyz - (!backglassMode ? IN.tablePos : float3(IN.tex0,0.0))) * lightCenter_maxRange.w;
        const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
        const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
        result.xyz = lcolor*(atten*lightColor_intensity.w);
        result.a = saturate(atten*lightColor_intensity.w);
    }

	float4 color;
	// no lighting if HUD vertices or passthrough mode
    if(imageMode || backglassMode)
        color.xyz = lightColor_intensity.xyz;
    else
	{
	    const float3 diffuse  = lightColor_intensity.xyz*cBase_Alpha.xyz;
        const float3 glossy   = bIsMetal ? diffuse : lightColor_intensity.xyz*cGlossy*0.08;
        const float3 specular = cClearcoat*0.08;
	    const float edge = bIsMetal ? 1.0 : Roughness_WrapL_Edge.z;

	    color.xyz = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
	}
    color.a = cBase_Alpha.a;
    
    return color+result;
}

float4 PS_BulbLight(in VS_LIGHT_OUTPUT IN) : COLOR
{
	const float len = length(lightCenter_maxRange.xyz - IN.tablePos) * lightCenter_maxRange.w;
    const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
	const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
	float4 result;
	result.xyz = lcolor*(-blend_modulate_vs_add*atten*lightColor_intensity.w); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
	result.a = 1.0/blend_modulate_vs_add - 1.0; //saturate(atten*lightColor_intensity.w);
	return result;
}


//------------------------------------------
// Kicker boolean vertex shader

VS_OUTPUT vs_kicker (float4 vPosition : POSITION0,  
                     float3 vNormal   : NORMAL0,  
                     float2 tc        : TEXCOORD0) 
{ 
    VS_OUTPUT Out;
    const float3 P = mul(vPosition, matWorldView).xyz;
    float4 P2 = vPosition;
    const float3 N = normalize(mul(float4(vNormal,0.0), matWorldViewInverseTranspose).xyz);

    Out.pos = mul(vPosition, matWorldViewProj);
    P2.z -= 100.0;
    P2 = mul(P2, matWorldViewProj);
    Out.pos.z = P2.z;
    Out.tex0 = tc;
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
      VertexShader = compile vs_3_0 vs_main(); 
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
// Light (Bulb/Shapes)
//

technique light_with_texture
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_light_main(); 
	  PixelShader = compile ps_3_0 PS_LightWithTexel();
   } 
}

technique light_without_texture
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_light_main(); 
	  PixelShader = compile ps_3_0 PS_LightWithoutTexel();
   } 
}

technique bulb_light
{ 
   pass P0 
   { 
		vertexshader = compile vs_3_0 vs_light_main();
		pixelshader  = compile ps_3_0 PS_BulbLight();
		SrcBlend=SRCALPHA;     // add the lightcontribution
		DestBlend=INVSRCCOLOR; // but also modulate the light first with the underlying elements by (1+lightcontribution, e.g. a very crude approximation of real lighting)
		AlphaTestEnable=false;
		AlphaBlendEnable=true;
		BlendOp=RevSubtract;   // see above
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
