#include "Globals.fxh"

float4x4 matWorld           : WORLD;
float4   diffuseMaterial    = float4(1,1,1,1);
float4   lightCenter;
float    maxRange;
float    intensity=1.0f;
float4   camera;
float4 ambient = float4( 0.1f, 0.0, 0.0, 1.0f );

struct vin
{ 
    float4 position   : POSITION0;
    float3 normal	  : NORMAL;
    float2 tex0       : TEXCOORD0;
    float2 tex1       : TEXCOORD1;
};
 
//vertex to pixel shader structure
struct vout
{
    float4 position	   : POSITION0;
    float2 tex0        : TEXCOORD0;
	float4 viewDir     : TEXCOORD1;
	float4 worldPos    : TEXCOORD2;
	float3 normal      : TEXCOORD3;
};

//VERTEX SHADER
vout VS( in vin IN )
{
	vout OUT = (vout)0;
	OUT.position = mul( IN.position, matWorldViewProj );
	float4 worldPos = IN.position; //mul(IN.position, matWorld);

	OUT.tex0 = IN.tex0;
	OUT.worldPos = worldPos;
	OUT.normal = normalize( mul(IN.normal, matWorld) );
	OUT.viewDir = camera-worldPos;
	return OUT;
}
//PIXEL SHADER
float4 PS_WithTexel(in vout IN ) : COLOR
{	
	float len = length(lightCenter.xyz-IN.worldPos.xyz);
	float f=0;//maxRange*0.01;
	float intens = 1.0f-saturate((len-f)/maxRange);
	
	intens = pow(intens,2);
	float4 result = saturate((diffuseMaterial*intens)*intensity);	
	result.a = intens;	
	float4 texel = tex2D( texSampler0, IN.tex0 );
	result = Overlay( texel, result );
    return Screen( texel, result );
}

float4 PS_WithoutTexel(in vout IN ) : COLOR
{	
	float len = length(lightCenter.xyz-IN.worldPos.xyz);
	float f=0;//maxRange*0.01;
	float intens = 1.0f-saturate((len-f)/maxRange);
	
	intens *= intens;
	float4 result = saturate((diffuseMaterial*intens)*intensity);	
	result.a = intens;	
	float4 color=diffuseMaterial*0.2f;
    return Screen( color, result );
}

float4 PS_BulbLight( in vout IN ) : COLOR
{
	float len = length(lightCenter.xyz-IN.worldPos.xyz);
	float f=0;//maxRange*0.01;
	float intens = 1.0f-saturate((len-f)/maxRange);
	
	intens *= intens;
	float4 result = saturate((diffuseMaterial*intens)*intensity);	
	float4 texel = tex2D( texSampler0, IN.tex0 );
	result.a = intens;	
	return result;
}

technique BasicLightWithTexture
{
	pass p0 
	{		
		vertexshader = compile vs_3_0 VS();
		pixelshader  = compile ps_3_0 PS_WithTexel();
	}
}
technique BasicLightWithoutTexture
{
	pass p0 
	{		
		vertexshader = compile vs_3_0 VS();
		pixelshader  = compile ps_3_0 PS_WithoutTexel();

	}
}

technique BulbLight
{
	pass p0 
	{		
		vertexshader = compile vs_3_0 VS();
		pixelshader  = compile ps_3_0 PS_BulbLight();
		SrcBlend=ONE;
		DestBlend=ONE;
		AlphaTestEnable=true;
		AlphaBlendEnable=true;
		BlendOp=Add;
	}
}
