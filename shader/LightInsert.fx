float4x4 matWorldViewProj   : WORLDVIEWPROJ;
float4x4 matWorld           : WORLD;
float4   diffuseMaterial    = float4(1,1,1,1);
float4   lightCenter;
float    maxRange;
float    intensity=1;

texture OffTexture;
sampler2D texSampler0 : TEXUNIT0 = sampler_state
{
	Texture	  = (OffTexture);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
};

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
	float2 tex1        : TEXCOORD1;
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
	OUT.tex1 = IN.tex1;
	OUT.worldPos = worldPos;
	OUT.normal = normalize( mul(IN.normal, matWorld) );
	return OUT;
}
float4 Screen (float4 cBase, float4 cBlend)
{
	return (1 - (1 - cBase) * (1 - cBlend));
}
float4 Multiply (float4 cBase, float4 cBlend)
{
	return (cBase * cBlend);
}
float4 Overlay (float4 cBase, float4 cBlend)
{
	// Vectorized (easier for compiler)
	float4 cNew;
	
	// overlay has two output possbilities
	// which is taken is decided if pixel value
	// is below half or not

	cNew = step(0.5,cBase);
	
	// we pick either solution
	// depending on pixel
	
	// first is case of < 0.5
	// second is case for >= 0.5
	
	// interpolate between the two, 
	// using color as influence value
	cNew= lerp((cBase*cBlend*2),(1.0-(2.0*(1.0-cBase)*(1.0-cBlend))),cNew);

	cNew.a = 1.0;
	return cNew;
}
//PIXEL SHADER
float4 PS_WithTexel(in vout IN ) : COLOR
{	
	float len = length(lightCenter.xyz-IN.worldPos.xyz);
	float f=0;//maxRange*0.01;
	float intens = 1-saturate((len-f)/maxRange);
	
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
	float intens = 1-saturate((len-f)/maxRange);
	
	intens = pow(intens,2);
	float4 result = saturate((diffuseMaterial*intens)*intensity);	
	result.a = intens;	
	float4 color=diffuseMaterial*0.2;
    return Screen( color, result );
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
