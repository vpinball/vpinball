//!! have switch to choose if texture is weighted by diffuse/glossy or is just used raw?

#include "Globals.fxh"

float3 cGlossy = float3(0.5f, 0.5f, 0.5f);
float3 cClearcoat = float3(0.5f, 0.5f, 0.5f);
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance:  0.5-1.0

bool   bPerformAlphaTest = false;
float4 staticColor = float4(1,1,1,1);
float  fAlphaTestValue = 128.0f/255.0f;

float2 fb_inv_resolution_05;

sampler2D texSampler3 : TEXUNIT2 = sampler_state // AO
{
	Texture	  = (Texture3);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

sampler2D texSampler4 : TEXUNIT0 = sampler_state // Framebuffer tex (unfiltered)
{
	Texture	  = (Texture0);
    MIPFILTER = NONE; //!! ??
    MAGFILTER = POINT;
    MINFILTER = POINT;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

sampler2D texSampler5 : TEXUNIT0 = sampler_state // Framebuffer tex (filtered)
{
	Texture	  = (Texture0);
    MIPFILTER = NONE; //!! ??
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

//

//function output structures 
struct VS_OUTPUT 
{ 
   float4 pos           : POSITION; 
   float2 tex0          : TEXCOORD0; 
   float3 worldPos      : TEXCOORD1; 
   float3 normal        : TEXCOORD2;
};

#include "FXAAStereoAO.fxh"

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

VS_OUTPUT vs_main_no_trafo (float4 vPosition  : POSITION0,  
                            float2 tc         : TEXCOORD0) 
{ 
   VS_OUTPUT Out;

   Out.pos = vPosition;
   Out.tex0 = tc;
   Out.worldPos = float3(0,0,0);
   Out.normal = float3(0,0,0);
   
   return Out; 
}

float4 ps_main( in VS_OUTPUT IN) : COLOR
{
   //return float4((IN.normal+1.0f)*0.5f,1.0f); // visualize normals
   
   float3 diffuse  = cBase;
   float3 glossy   = bIsMetal ? cBase : cGlossy*0.08f;
   float3 specular = cClearcoat*0.08f;
   float edge = bIsMetal ? 1.0f : fEdge;
   
   float4 result=lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
   result.a *= fmaterialAlpha;
   return result;
}

float4 ps_main_texture(in VS_OUTPUT IN) : COLOR
{
   //return float4((IN.normal+1.0f)*0.5f,1.0f); // visualize normals
   
   float4 pixel = tex2D(texSampler0, IN.tex0);
   pixel.a *= fmaterialAlpha;

   if (bPerformAlphaTest && pixel.a<=fAlphaTestValue )
    clip(-1);           //stop the pixel shader if alpha test should reject pixel

   // early out if no normal set (e.g. HUD vertices)
   if(IN.normal.x == 0.0f && IN.normal.y == 0.0f && IN.normal.z == 0.0f)
    return pixel*staticColor;

   float3 t = InvGamma(pixel.xyz);
   float3 diffuse  = t*cBase;
   float3 glossy   = bIsMetal ? diffuse : t*cGlossy*0.08f; //!! use AO for glossy? specular?
   float3 specular = cClearcoat*0.08f;
   float edge = bIsMetal ? 1.0f : fEdge;

   float4 result = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge);
   result.a = pixel.a;
   return result;
}

// ****** simple shader for rendering without lighting *******
float  fAlpha=1.0f;
float  fFilterAmount=1.0f;
bool   bMultiply=false;
bool   bAdditive=false;
bool   bOverlay=false;
bool   bScreen=false;
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

   float4 result = staticColor*pixel;
   result.a *= fAlpha;
   return result;
}

float4 ps_main_textureAB_noLight( in VS_SIMPLE_OUTPUT IN) : COLOR
{
   float4 pixel1 = tex2D(texSampler0, IN.tex0);
   float4 pixel2 = tex2D(texSampler1, IN.tex0);
   if (bPerformAlphaTest && pixel1.a<=fAlphaTestValue )
    clip(-1);           //stop the pixel shader if alpha test should reject pixel
   if (bPerformAlphaTest && pixel2.a<=fAlphaTestValue )
    clip(-1);           //stop the pixel shader if alpha test should reject pixel

   float4 result = staticColor;
   if ( bOverlay )
      result *= Overlay(pixel1,pixel2, fFilterAmount);
   else if ( bMultiply )
      result *= Multiply(pixel1,pixel2, fFilterAmount);
   else if ( bAdditive )
      result *= Additive(pixel1,pixel2, fFilterAmount);
   else if ( bScreen )
      result *= Screen(pixel1,pixel2, fFilterAmount);
      
   result.a *= fAlpha;
   return result;
}

float4 ps_main_noLight( in VS_SIMPLE_OUTPUT IN) : COLOR
{
   return staticColor;
}

//####### Light shader ##################
float3   lightColor = float3(1,1,1);
float4   lightCenter;
float    maxRange;
float    intensity=1.0f;

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
                         float2 tc         : TEXCOORD0, 
                         float2 tc2        : TEXCOORD1) 
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
    float4 pixel = tex2D(texSampler0, IN.tex0);  
    float3 t = InvGamma(pixel.xyz);
    float3 diffuse = t*cBase;
    float3 glossy = bIsMetal ? diffuse : t*cGlossy*0.08f; //!! use AO for glossy? specular?
    float3 specular = cClearcoat*0.08f;
    float edge = bIsMetal ? 1.0f : fEdge;
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 color;

	// early out if no normal set (e.g. HUD vertices)
    if(IN.normal.x == 0.0f && IN.normal.y == 0.0f && IN.normal.z == 0.0f)
     color = float4(t,1.0f); //!! misses tonemapping and gamma here
    else
	 color = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable

    if ( intensity!=0.0f )
    {
        float len = length(lightCenter.xyz - IN.tablePos.xyz) / max(maxRange, 0.1f);
        float atten = 1.0f - saturate(len);
        atten *= atten;
        float3 lcolor = lerp(float3(1.0f, 1.0f, 1.0f), lightColor, sqrt(len));
        result.xyz = lcolor*(atten*intensity);
        result.a = saturate(atten*intensity);
        color.a *= fmaterialAlpha;
        color += result;
        color = Overlay(pixel, color, 1.0f);
        color = Screen(pixel, color, 1.0f);
    }
    else
    {
        color.a = pixel.a*fmaterialAlpha;
    }
	
    return color;
}

float4 PS_LightWithoutTexel(in VS_LIGHT_OUTPUT IN ) : COLOR
{	
    float3 diffuse  = cBase;
    float3 glossy   = bIsMetal ? cBase : cGlossy*0.08f;
    float3 specular = cClearcoat*0.08f;
	float edge = bIsMetal ? 1.0f : fEdge;
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

    if (intensity != 0.0f)
    {
        float len = length(lightCenter.xyz - IN.tablePos.xyz) / max(maxRange, 0.1f);
        float atten = 1.0f - saturate(len);
        atten *= atten;
        float3 lcolor = lerp(float3(1.0f, 1.0f, 1.0f), lightColor, sqrt(len));
        result.xyz = lcolor*(atten*intensity);
        result.a = saturate(atten*intensity);
    }
    
	float4 color;
	// early out if no normal set (e.g. HUD vertices)
    if(IN.normal.x == 0.0f && IN.normal.y == 0.0f && IN.normal.z == 0.0f)
     color = float4(0,0,0,1); //!! misses tonemapping and gamma here
    else
	 color = lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable

    color.a *= fmaterialAlpha;
    color += result;
    return color;
}

float4 PS_BulbLight( in VS_LIGHT_OUTPUT IN ) : COLOR
{
	float len = length(lightCenter.xyz-IN.tablePos.xyz)/max(maxRange,0.1f);
    float atten = 1.0f-saturate(len);
    atten*=atten;
	float3 lcolor = lerp(float3(1.0f,1.0f,1.0f), lightColor, sqrt(len));
	float4 result;
	result.xyz = lcolor*(atten*intensity);
	result.a = saturate(atten*intensity);	
	return result;
}


//------------------------------------------
// Kicker boolean vertex shader
VS_OUTPUT vs_kicker (float4 vPosition  : POSITION0,  
                     float3 vNormal    : NORMAL0,  
                     float2 tc         : TEXCOORD0, 
                     float2 tc2        : TEXCOORD1) 
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

//

float4 ps_main_fb_tonemap( in VS_OUTPUT IN) : COLOR
{
   return float4(FBColorGrade(FBGamma(FBToneMap(tex2D(texSampler5, IN.tex0).xyz))), 1.0f);
}

float4 ps_main_fb_tonemap_AO( in VS_OUTPUT IN) : COLOR
{
	return float4(FBColorGrade(FBGamma(FBToneMap(tex2D(texSampler5, IN.tex0).xyz*(
           tex2D(texSampler3, IN.tex0-fb_inv_resolution_05).x /*+ //Blur:

           tex2D(texSampler3, IN.tex0+float2(0.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0-float2(0.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0+float2(fb_inv_resolution_05.x*2.0f,0.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0-float2(fb_inv_resolution_05.x*2.0f,0.0f)-fb_inv_resolution_05).x+

           tex2D(texSampler3, IN.tex0+float2(fb_inv_resolution_05.x*2.0f,-fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0+float2(-fb_inv_resolution_05.x*2.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0+float2(fb_inv_resolution_05.x*2.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0-float2(fb_inv_resolution_05.x*2.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x)/9.0f*/) ))), 1.0f);
}

float4 ps_main_fb_tonemap_no_filter( in VS_OUTPUT IN) : COLOR
{
   return float4(FBColorGrade(FBGamma(FBToneMap(tex2D(texSampler4, IN.tex0).xyz))), 1.0f);
}

float4 ps_main_fb_tonemap_AO_no_filter( in VS_OUTPUT IN) : COLOR
{
	return float4(FBColorGrade(FBGamma(FBToneMap(tex2D(texSampler4, IN.tex0).xyz*(
           tex2D(texSampler3, IN.tex0-fb_inv_resolution_05).x /*+ //Blur:

           tex2D(texSampler3, IN.tex0+float2(0.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0-float2(0.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0+float2(fb_inv_resolution_05.x*2.0f,0.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0-float2(fb_inv_resolution_05.x*2.0f,0.0f)-fb_inv_resolution_05).x+

           tex2D(texSampler3, IN.tex0+float2(fb_inv_resolution_05.x*2.0f,-fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0+float2(-fb_inv_resolution_05.x*2.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0+float2(fb_inv_resolution_05.x*2.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x+
		   tex2D(texSampler3, IN.tex0-float2(fb_inv_resolution_05.x*2.0f,fb_inv_resolution_05.y*2.0f)-fb_inv_resolution_05).x)/9.0f*/) ))), 1.0f);
}

//------------------------------------
// Techniques

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
		SrcBlend=ONE;
		DestBlend=ONE;
		AlphaTestEnable=true;
		AlphaBlendEnable=true;
		BlendOp=Add;
   } 
}

technique kickerBoolean
{ 
   pass P0 
   { 
     //ZWriteEnable=TRUE;
     VertexShader = compile vs_3_0 vs_kicker(); 
	  PixelShader = compile ps_3_0 ps_main();
   } 
}

//

technique AO
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_ao();
   } 
}

technique stereo
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_stereo();
   } 
}

technique FXAA1
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fxaa1();
   } 
}

technique FXAA2
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fxaa2();
   } 
}

technique fb_tonemap
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_tonemap();
   } 
}

technique fb_tonemap_AO
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_tonemap_AO();
   } 
}

technique fb_tonemap_no_filter
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_tonemap_no_filter();
   } 
}

technique fb_tonemap_AO_no_filter
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_tonemap_AO_no_filter();
   } 
}
