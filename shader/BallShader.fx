
float4x4 matWorldViewProj   : WORLDVIEWPROJ;
float4x4 matWorldI          : WORLDIT;
float4x4 matWorld           : WORLD;
float4   diffuseMaterial    = float4(1,1,1,0.1);
float    invTableHeight;
float    invTableWidth;
float    posX;
float    posY;
float    sizeY;
float    reflectionStrength;

texture Texture0;
texture Texture1;
texture Texture2;

sampler2D texSampler0 : TEXUNIT0 = sampler_state
{
	Texture	  = (Texture0);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
};

sampler2D texSampler1 : TEXUNIT1 = sampler_state
{
	Texture	  = (Texture1);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	AddressU = Mirror;
	AddressV = Mirror;
};

sampler2D texSampler2 : TEXUNIT2 = sampler_state
{
	Texture	  = (Texture2);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
};

//------------------------------------

//application to vertex structure
struct vin
{ 
    float4 position   : POSITION0;
    float3 normal	  : NORMAL;
    float2 tex0       : TEXCOORD0;
};
 
//vertex to pixel shader structure
struct vout
{
    float4 position	   : POSITION0;
    float2 tex0        : TEXCOORD0;
	float4 tex1        : TEXCOORD1;
	float2 tex2        : TEXCOORD2;
};

//VERTEX SHADER
vout vsBall( in vin IN )
{
	//init our output data to all zeros
	vout OUT = (vout)0;

    //convert to world space and pass along to our output
    OUT.position    = mul(IN.position, matWorldViewProj);
    
    float4 normal = normalize(mul(IN.normal, matWorld));
	float4 norm = mul(IN.normal, matWorld);

	float l = length(norm.xyz);
	norm = norm/l;
    float2 uv=normal.xy;
	uv.x = 0.5+uv.x*0.5;
	uv.y = 0.5+uv.y*0.5;
    //pass along texture info
	OUT.tex0		= uv;
	OUT.tex1        = norm;
	OUT.tex2        = IN.tex0;
	return OUT;
}

vout vsBallReflection( in vin IN )
{
	//init our output data to all zeros
	vout OUT = (vout)0;

	IN.position.y += sizeY*0.4;
	IN.position.z = (IN.position.z*0.5)-10;
    //convert to world space and pass along to our output
    OUT.position    = mul(IN.position, matWorldViewProj);
    
    float4 normal = normalize(mul(IN.normal, matWorld));

    float2 uv=normal.xy;
	uv.x = 0.5+uv.x*0.5;
	uv.y = 0.5+uv.y*0.5;
    //pass along texture info
	OUT.tex0		= uv;
	OUT.tex1	    = IN.position;
	return OUT;
}


//PIXEL SHADER
float4 psBallCabinet( in vout IN ) : COLOR
{
	float u=IN.tex0.x;
	IN.tex0.x=IN.tex0.y;
	IN.tex0.y=u;
	float4 color1 = tex2D( texSampler0, IN.tex0 );
	float4 color3 = tex2D( texSampler2, IN.tex2 );
	float2 uv = float2(0,0);
	uv.x = posX * invTableWidth;
	uv.y = posY * invTableHeight;
	uv.x = uv.x + (IN.tex1.y/(IN.tex1.z))*0.03;
	uv.y = uv.y + (IN.tex1.x/(IN.tex1.z))*0.03;
	float4 color2 = tex2D( texSampler1, uv );
	if( IN.tex1.x>0.33 )
	{
		color2.a = saturate(IN.tex1.x-0.33);
	}
	else
	{
		color2.a = 0;
	}
	
	color1.a=1.0;
	color2 *=2;
	return ((color3.a)*color3)+(color1 +  (color2 * (color2.a))/2.9);	
}

float4 psBallDesktop( in vout IN ) : COLOR
{
	float4 color1 = tex2D( texSampler0, IN.tex0 );
	float4 color3 = tex2D( texSampler2, IN.tex2 );
	float2 uv = float2(0,0);
	uv.x = posX * invTableWidth;
	uv.y = posY * invTableHeight;
	uv.x = uv.x + (IN.tex1.x/(IN.tex1.z))*0.03;
	uv.y = uv.y + (IN.tex1.y/(IN.tex1.z))*0.03;
	float4 color2 = tex2D( texSampler1, uv );
	if( IN.tex1.y>0.33 )
	{
		color2.a = saturate(IN.tex1.y-0.33);
	}
	else
	{
		color2.a = 0;
	}
	
	color1.a=1.0;
	color2 *=2;
	return ((color3.a)*color3)+(color1 +  (color2 * (color2.a))/2.9);	
}


float4 psBallReflection( in vout IN ) : COLOR
{
	float4 color1 = tex2D( texSampler0, IN.tex0 );
	float alpha = ((IN.tex1.y-posY)/(sizeY*0.4));
	color1.a = saturate( alpha )*reflectionStrength*2;
	return color1;
}

//------------------------------------
technique RenderBall_Desktop
{
	pass p0 
	{		
		vertexshader = compile vs_2_0 vsBall();
		pixelshader  = compile ps_2_0 psBallDesktop();

	}
}

technique RenderBall_Cabinet
{
	pass p0 
	{		
		vertexshader = compile vs_2_0 vsBall();
		pixelshader  = compile ps_2_0 psBallCabinet();

	}
}

technique RenderBallReflection
{
	pass p0 
	{		
		vertexshader = compile vs_2_0 vsBallReflection();
		pixelshader  = compile ps_2_0 psBallReflection();
	}
}


