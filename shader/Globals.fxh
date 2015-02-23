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

sampler2D texSampler1 : TEXUNIT1 = sampler_state // environment and texB for lights //!! split up
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

sampler2D texSampler6 : TEXUNIT2 = sampler_state // color grade LUT
{
	Texture	  = (Texture4);
    MIPFILTER = NONE;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

#include "Material.fxh"

bool color_grade;


float3 FBColorGrade(float3 color)
{
   if(!color_grade)
       return color;

   color.xy = color.xy*(15.0/16.0) + 1.0/32.0; // assumes 16x16x16 resolution flattened to 256x16 texture
   color.z *= 15.0;

   float x = (color.x + floor(color.z))/16.0;
   float3 lut1 = tex2Dlod(texSampler6, float4(x,          color.y, 0.,0.)).xyz; // two lookups to blend/lerp between blue 2D regions
   float3 lut2 = tex2Dlod(texSampler6, float4(x+1.0/16.0, color.y, 0.,0.)).xyz;
   return lerp(lut1,lut2, frac(color.z));
}
