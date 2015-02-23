//!! have switch to choose if texture is weighted by diffuse/glossy or is just used raw?

#define NUM_BALL_LIGHTS 0 // just to avoid having to much constant mem allocated

#include "Globals.fxh"

float3 cGlossy = float3(0.5, 0.5, 0.5);
float3 cClearcoat = float3(0.5, 0.5, 0.5);
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance:  0.5-1.0

bool bPerformAlphaTest;
bool bAdd_Blend;
float3 staticColor = float3(1.,1.,1.);
float  fAlphaTestValue = 128.0/255.0;

float blend_modulate_vs_add;

struct VS_OUTPUT 
{ 
   float4 pos           : POSITION; 
   float2 tex0          : TEXCOORD0; 
   float3 worldPos      : TEXCOORD1; 
   float3 normal        : TEXCOORD2;
};

//------------------------------------

//
// Standard Materials
//

VS_OUTPUT vs_main (float4 vPosition  : POSITION0,  
                   float3 vNormal    : NORMAL0,  
                   float2 tc         : TEXCOORD0) 
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
   //return float4((IN.normal+1.0f)*0.5f,1.0f); // visualize normals
   
   float3 diffuse  = cBase_Alpha.xyz;
   float3 glossy   = bIsMetal ? cBase_Alpha.xyz : cGlossy*0.08f;
   float3 specular = cClearcoat*0.08f;
   float edge = bIsMetal ? 1.0f : Roughness_WrapL_Edge.z;
   
   float4 result;
   result.xyz = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
   result.a = cBase_Alpha.a;
   return result;
}

float4 ps_main_texture(in VS_OUTPUT IN) : COLOR
{
   //return float4((IN.normal+1.0f)*0.5f,1.0f); // visualize normals
   
   float4 pixel = tex2D(texSampler0, IN.tex0);

   if (bPerformAlphaTest && pixel.a<=fAlphaTestValue )
    clip(-1);           //stop the pixel shader if alpha test should reject pixel

   pixel.a *= cBase_Alpha.a;
   float3 t = InvGamma(pixel.xyz);

   // early out if no normal set (e.g. decal vertices)
   if(IN.normal.x == 0.0f && IN.normal.y == 0.0f && IN.normal.z == 0.0f)
      return float4(InvToneMap(t*staticColor),pixel.a);
      
   float3 diffuse  = t*cBase_Alpha.xyz;
   float3 glossy   = bIsMetal ? diffuse : t*cGlossy*0.08f; //!! use AO for glossy? specular?
   float3 specular = cClearcoat*0.08f;
   float edge = bIsMetal ? 1.0f : Roughness_WrapL_Edge.z;

   float4 result;
   result.xyz = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge);
   result.a = pixel.a;
   return result;
}

//
// Flasher
//

float  fAlpha=1.0f;
float  fFilterAmount=1.0f;
bool bMultiply;
bool bAdditive;
bool bOverlay;
bool bScreen;

struct VS_SIMPLE_OUTPUT 
{ 
   float4 pos           : POSITION; 
   float2 tex0          : TEXCOORD0; 
};

VS_SIMPLE_OUTPUT vs_simple_main (float4 vPosition  : POSITION0,  
                                 float2 tc         : TEXCOORD0)
{ 
   VS_SIMPLE_OUTPUT Out;

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;
   
   return Out; 
}

float4 ps_main_textureOne_noLight( in VS_SIMPLE_OUTPUT IN) : COLOR
{
   float4 pixel = tex2D(texSampler0, IN.tex0);

   if (bPerformAlphaTest && pixel.a<=fAlphaTestValue )
    clip(-1);           //stop the pixel shader if alpha test should reject pixel

   float4 result;
   result.xyz = staticColor*InvGamma(pixel.xyz);
   result.a = pixel.a*fAlpha;

   if(!bAdd_Blend)
      return result;

   result.xyz = result.xyz*(-blend_modulate_vs_add*result.a); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
   result.a = 1.0f/blend_modulate_vs_add - 1.0f;
   return result;
}

float4 ps_main_textureAB_noLight( in VS_SIMPLE_OUTPUT IN) : COLOR
{
   float4 pixel1 = tex2D(texSampler0, IN.tex0);
   float4 pixel2 = tex2D(texSampler1, IN.tex0);
   if (bPerformAlphaTest && (pixel1.a<=fAlphaTestValue || pixel2.a<=fAlphaTestValue))
    clip(-1);           //stop the pixel shader if alpha test should reject pixel

   pixel1.xyz = InvGamma(pixel1.xyz);
   pixel2.xyz = InvGamma(pixel2.xyz);

   float4 result;
   result.xyz = staticColor;
   result.a = fAlpha;
   if ( bOverlay )
      result *= Overlay(pixel1,pixel2);
   else if ( bMultiply )
      result *= Multiply(pixel1,pixel2, fFilterAmount);
   else if ( bAdditive )
      result *= Additive(pixel1,pixel2, fFilterAmount);
   else if ( bScreen )
      result *= Screen(pixel1,pixel2);

   if(!bAdd_Blend)
      return result;

   result.xyz = result.xyz*(-blend_modulate_vs_add*result.a); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
   result.a = 1.0f/blend_modulate_vs_add - 1.0f;
   return result;
}

float4 ps_main_noLight( in VS_SIMPLE_OUTPUT IN) : COLOR
{
   if(!bAdd_Blend)
      return float4(staticColor,fAlpha);

	float4 result;
	result.xyz = staticColor*(-blend_modulate_vs_add*fAlpha); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
	result.a = 1.0f/blend_modulate_vs_add - 1.0f;
	return result;
}


//------------------------------------------
// Light (Bulb/Shapes)

float3   lightColor = float3(1.f,1.f,1.f);
float3   lightColor2 = float3(1.f,1.f,1.f);
float3   lightCenter;
float    maxRange;
float    intensity = 1.0f;
float    falloff_power = 2.0f;
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

VS_LIGHT_OUTPUT vs_light_main (float4 vPosition  : POSITION0,  
                         float3 vNormal    : NORMAL0,  
                         float2 tc         : TEXCOORD0) 
{ 
   VS_LIGHT_OUTPUT Out;

   // trafo all into worldview space (as most of the weird trafos happen in view, world is identity so far)
   float3 P = mul(vPosition, matWorldView).xyz;
   float3 N = normalize(mul(float4(vNormal,0.0f), matWorldViewInverseTranspose).xyz);

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;
   Out.tablePos = vPosition.xyz;
   Out.worldPos = P;
   Out.normal = N;
   
   return Out; 
}


float4 PS_LightWithTexel(in VS_LIGHT_OUTPUT IN ) : COLOR
{	
    float4 pixel = tex2D(texSampler0, IN.tex0); //!! IN.tex0 abused in backglass mode  
    pixel.xyz = InvGamma(pixel.xyz);

    float4 color;
	// no lighting if HUD vertices or passthrough mode
    if(imageMode || backglassMode)
        color = pixel;
    else
	{
	    float3 diffuse = pixel.xyz*cBase_Alpha.xyz;
        float3 glossy = bIsMetal ? diffuse : pixel.xyz*cGlossy*0.08f; //!! use AO for glossy? specular?
        float3 specular = cClearcoat*0.08f;
        float edge = bIsMetal ? 1.0f : Roughness_WrapL_Edge.z;

	    color.xyz = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
		color.a = pixel.a;
    }
    color.a *= cBase_Alpha.a;

    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    if ( intensity!=0.0f )
    {
        float len = length(lightCenter - (!backglassMode ? IN.tablePos : float3(IN.tex0,0.0f))) / max(maxRange, 0.1f);
        float atten = pow(1.0f - saturate(len), falloff_power);
        float3 lcolor = lerp(lightColor2, lightColor, sqrt(len));
        result.xyz = lcolor*(atten*intensity);
        result.a = saturate(atten*intensity);
        color += result;
        color = Overlay(pixel, color);
        color = Screen(pixel, color);
    }

    return color;
}

float4 PS_LightWithoutTexel(in VS_LIGHT_OUTPUT IN ) : COLOR
{
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (intensity != 0.0f)
    {
        float len = length(lightCenter - (!backglassMode ? IN.tablePos : float3(IN.tex0,0.0f))) / max(maxRange, 0.1f);
        float atten = pow(1.0f - saturate(len), falloff_power);
        float3 lcolor = lerp(lightColor2, lightColor, sqrt(len));
        result.xyz = lcolor*(atten*intensity);
        result.a = saturate(atten*intensity);
    }

	float4 color;
	// no lighting if HUD vertices or passthrough mode
    if(imageMode || backglassMode)
        color.xyz = lightColor;
    else
	{
	    float3 diffuse  = lightColor*cBase_Alpha.xyz;
        float3 glossy   = bIsMetal ? diffuse : lightColor*cGlossy*0.08f;
        float3 specular = cClearcoat*0.08f;
	    float edge = bIsMetal ? 1.0f : Roughness_WrapL_Edge.z;

	    color.xyz = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
	}
    color.a = cBase_Alpha.a;
    
    return color+result;
}

float4 PS_BulbLight( in VS_LIGHT_OUTPUT IN ) : COLOR
{
	float len = length(lightCenter - IN.tablePos) / max(maxRange,0.1f);
    float atten = pow(1.0f - saturate(len), falloff_power);
	float3 lcolor = lerp(lightColor2, lightColor, sqrt(len));
	float4 result;
	result.xyz = lcolor*(-blend_modulate_vs_add*atten*intensity); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
	result.a = 1.0f/blend_modulate_vs_add - 1.0f; //saturate(atten*intensity);
	return result;
}


//------------------------------------------
// Kicker boolean vertex shader

VS_OUTPUT vs_kicker (float4 vPosition  : POSITION0,  
                     float3 vNormal    : NORMAL0,  
                     float2 tc         : TEXCOORD0) 
{ 
    VS_OUTPUT Out;
    float3 P = mul(vPosition, matWorldView).xyz;
    float4 P2 = vPosition;
    float3 N = normalize(mul(float4(vNormal,0.0f), matWorldViewInverseTranspose).xyz);

    Out.pos = mul(vPosition, matWorldViewProj);
    P2.z -= 100.0f;
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
// Flasher
//

technique basic_with_textureOne_noLight
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_simple_main(); 
	  PixelShader = compile ps_3_0 ps_main_textureOne_noLight();
   } 
}

technique basic_with_textureAB_noLight
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_simple_main(); 
	  PixelShader = compile ps_3_0 ps_main_textureAB_noLight();
   } 
}

technique basic_with_noLight
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_simple_main(); 
	  PixelShader = compile ps_3_0 ps_main_noLight();
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
