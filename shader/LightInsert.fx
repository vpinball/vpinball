float4x4 matWorldViewProj   : WORLDVIEWPROJ;
float4x4 matWorld           : WORLD;
float4   diffuseMaterial    = float4(1,1,1,1);
float4   lightCenter;
float    maxRange;

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

//PIXEL SHADER
float4 PS( in vout IN ) : COLOR
{	
	float len = length(lightCenter.xyz-IN.worldPos.xyz);
	float f=0;//maxRange*0.01;
	float intens = 1-saturate((len-f)/maxRange);
	
	intens = pow(intens,2);
	float4 result = saturate(diffuseMaterial*intens);
	result.a = intens;
	float4 texel = tex2D( texSampler0, IN.tex0 );
	return result +texel*texel.a;
}

technique BasicLight
{
	pass p0 
	{		
		vertexshader = compile vs_2_0 VS();
		pixelshader  = compile ps_2_0 PS();

	}
}
