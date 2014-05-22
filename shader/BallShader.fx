float4x4 matWorldViewProj   : WORLDVIEWPROJ;
float4x4 matWorld           : WORLD;
float4   diffuseMaterial    = float4(1,1,1,0.1);
float    invTableHeight;
float    invTableWidth;
float4   position;
float    radius;
float    inclination;
float    reflectionStrength;

// this is used for the orientation matrix
float4   m1;
float4   m2;
float4   m3;

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
	
	// apply spinning and move the ball to it's actual position
	float4x4 orientation = {m1,m2,m3,float4(0,0,0,0)};
	float4 pos = IN.position;
	pos = mul(pos,orientation);
	pos *= radius;
	pos.x *= sin(inclination);
	pos.y *= cos(inclination);
	pos.z *= sin(inclination);
	pos += position;
	
    //convert to world space and pass along to our output
    OUT.position    = mul(pos, matWorldViewProj);
    
	// apply spinning to the normals too to get the sphere mapping effect
	float4 npos = float4(IN.normal,0.0);
	npos = mul(npos,orientation);
    float4 normal = normalize(mul(npos, matWorld));
	float4 norm = mul(npos, matWorld);

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
	float sizeY = radius*2;

	// apply spinning and move the ball to it's actual position
	float4x4 orientation = {m1,m2,m3,float4(0,0,0,0)};
	float4 pos = IN.position;
	pos = mul(pos,orientation);
	pos *= radius;
	pos.x *= sin(inclination);
	pos.y *= cos(inclination);
	pos.z *= sin(inclination);
	pos += position;

	// this is no a 100% ball reflection on the table due to the quirky camera setup
	// the ball is moved a bit down and rendered again
	pos.y += sizeY*0.25;
	pos.z = (pos.z*0.5)-10;
    //convert to world space and pass along to our output
    OUT.position    = mul(pos, matWorldViewProj);
    
	float4 npos = float4(IN.normal,0.0);
	npos = mul(npos,orientation);
    float4 normal = normalize(mul(npos, matWorld));

    float2 uv=normal.xy;
	uv.x = 0.5+uv.x*0.5;
	uv.y = 0.5+uv.y*0.5;
    //pass along texture info
	OUT.tex0		= uv;
	OUT.tex1	    = pos;
	return OUT;
}


//PIXEL SHADER
float4 psBallCabinet( in vout IN ) : COLOR
{
	// the reflection of the playfield on the ball is another hack at the moment
	// for a correct reflection we need a dynamic cube map, but generating this is not feasible at the moment 
	// because of the render pipeline!
	
	// flip uv because cabinets are rotated about 270°
	float u=IN.tex0.x;
	IN.tex0.x=IN.tex0.y;
	IN.tex0.y=u;
	float4 ballImageColor = tex2D( texSampler0, IN.tex0 );
	float4 decalColor = tex2D( texSampler2, IN.tex2 );
	float2 uv = float2(0,0);
	// this isn't real reflection because creating a cube map for every movement would be too slow
	// instead the texture coordinates must be changed to map only a small area of the 
	// playfield image at the ball's current location
	uv.x = position.x * invTableWidth;
	uv.y = position.y * invTableHeight;
	uv.x = uv.x + (IN.tex1.y/(IN.tex1.z))*0.03;
	uv.y = uv.y + (IN.tex1.x/(IN.tex1.z))*0.03;
	float4 playfieldColor = tex2D( texSampler1, uv );
	
	// don't map the playfield area completely over the whole ball but only on the lower half
	// 0 is in the middle so start a bit later for a discreet reflection
	if( IN.tex1.x>0.23 )
	{
		playfieldColor.a = saturate(IN.tex1.x-0.23);
	}
	else
	{
		playfieldColor.a = 0;
	}
	
	ballImageColor.a=1.0;
	playfieldColor *=2;
	return ((decalColor.a)*decalColor)+(ballImageColor +  (playfieldColor * (playfieldColor.a))/2.9);	
}

float4 psBallDesktop( in vout IN ) : COLOR
{
	// the reflection of the playfield on the ball is another hack at the moment
	// for a correct reflection we need a dynamic cube map, but generating this is not feasible at the moment 
	// because of the render pipeline!

	float4 ballImageColor = tex2D( texSampler0, IN.tex0 );
	float4 decalColor = tex2D( texSampler2, IN.tex2 );
	float2 uv = float2(0,0);
	uv.x = position.x * invTableWidth;
	uv.y = position.y * invTableHeight;
	uv.x = uv.x + (IN.tex1.x/(IN.tex1.z))*0.03;
	uv.y = uv.y + (IN.tex1.y/(IN.tex1.z))*0.03;
	float4 playfieldColor = tex2D( texSampler1, uv );
	
	// don't map the playfield area completely over the whole ball but only on the lower half
	// 0 is in the middle so start a bit later for a discreet reflection
	if( IN.tex1.y>0.33 )
	{
		playfieldColor.a = saturate(IN.tex1.y-0.33);
	}
	else
	{
		playfieldColor.a = 0;
	}
	ballImageColor.a=1.0;
	playfieldColor *=2;
	return ((decalColor.a)*decalColor)+(ballImageColor +  (playfieldColor * (playfieldColor.a))/2.9);	
}


float4 psBallReflection( in vout IN ) : COLOR
{
	float4 ballImageColor = tex2D( texSampler0, IN.tex0 );
	float sizeY = radius*2*0.9;
	float alpha = ((IN.tex1.y-position.y)/(sizeY*0.4));
	ballImageColor.a = saturate( alpha )*reflectionStrength*2;
	return ballImageColor;
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


