//!! change to PB model, 2layer? (for clearcoat)
//!! change all to float3 pipeline

#define NUM_LIGHTS 2

float3 vSpecularColor = float3(1.0f, 1.0f, 1.0f); //!! pass from material & texture?
float  fMaterialPower = 16.f; //!! pass from material
bool   bSpecular = false; //!! remove, steer from specular
float  fWrap = 0.5f; //!! pass from material, w in [0..1] for wrap lighting

float3 vAmbient = float3(0.0f,0.0f,0.0f);
float  flightRange = 3000.0f;

struct CLight 
{ 
   float3 vPos; 
   float3 vEmission;
}; 
 
int iLightPointNum = NUM_LIGHTS;
CLight lights[NUM_LIGHTS] = {          //NUM_LIGHTS == 2
   { 
      float3(0.0f, 0.0f, 0.0f),        //position 
      float3(0.0f, 0.0f, 0.0f),        //emission
   }, 
   { 
      float3(0.0f, 0.0f, 0.0f),        //position 
      float3(0.0f, 0.0f, 0.0f),        //emission
   }
}; 

float4x4 matWorldViewProj   : WORLDVIEWPROJ;
float4x4 matWorld           : WORLD;
float4x4 matWorldView       : WORLD;
float    invTableHeight;
float    invTableWidth;
float4   position;
float    radius;
float    reflectionStrength;
float    ballStretchX;
float    ballStretchY;
float4   camera;

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
    float3 tex0        : TEXCOORD0;
	float4 normal      : TEXCOORD1;
	float4 eye         : TEXCOORD2;
	float4 tex1        : TEXCOORD3;
	float2 tex2        : TEXCOORD4;
	float4 worldPos    : TEXCOORD5;
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
	pos.x *= ballStretchX;
	pos.y *= ballStretchY;
	float4 v = mul(pos,matWorld);
	float4 c = mul(camera, matWorld);
	pos += position;
	
    //convert to world space and pass along to our output
    OUT.position = mul(pos, matWorldViewProj);
    
	// apply spinning to the normals too to get the sphere mapping effect
	float4 npos = float4(IN.normal,0.0f);
	npos = mul(npos,orientation);
    float4 normal = normalize(mul(npos, matWorld));
	float4 eye = normalize(v-c);
	float4 r = reflect(eye,normal);
    //pass along texture info
	OUT.tex0		= r.xyz;
	OUT.tex2		= IN.tex0;
	OUT.normal      = normal;
	OUT.eye         = eye;
	OUT.worldPos    = pos;
	return OUT;
}

vout vsBallReflection( in vin IN )
{
	//init our output data to all zeros
	vout OUT = (vout)0;
	float sizeY = radius*2.0f;

	// apply spinning and move the ball to it's actual position
	float4x4 orientation = {m1,m2,m3,float4(0,0,0,0)};
	float4 pos = IN.position;
	pos = mul(pos,orientation);
	pos *= radius;
	pos.x *= ballStretchX;
	pos.y *= ballStretchY;
	pos += position;

	// this is not a 100% ball reflection on the table due to the quirky camera setup
	// the ball is moved a bit down and rendered again
	pos.y += sizeY*0.35f;
	pos.z = pos.z*0.5f - 10.0f;
    //convert to world space and pass along to our output
    OUT.position = mul(pos, matWorldViewProj);
    
	float4 npos = float4(IN.normal,0.0f);
	npos = mul(npos,orientation);
    float4 normal = normalize(mul(npos, matWorld));
	float4 eye = normalize(pos);
	float4 r=normalize(reflect(eye,normal));

    //pass along texture info
	OUT.tex0		= r.xyz;
	OUT.tex1	    = pos;
	return OUT;
}

float3 FresnelSchlick(float3 spec, float3 E, float3 H)
{
    return spec + (1.0f - spec) * pow(1.0f - saturate(dot(E, H)), 5);
}

// assumes all light input premultiplied by PI
float3 DoPointLight(float4 vPosition, float3 N, float3 V, float3 diffuse, float3 specular, int i) 
{ 
   float3 pos = mul(matWorld,vPosition).xyz;
   float3 light = lights[i].vPos;
   float3 lightDir = light-pos;
   float3 L = normalize(lightDir); 
   float NdotL = dot(N, L); 
   float3 Out = float3(0.0f,0.0f,0.0f); 
   
   if(NdotL + fWrap > 0.0f)
   { 
      //compute diffuse color 
      Out = /*NdotL * diffuse; //*saturate(4.0f*NdotL); //!! /*/ diffuse * (NdotL + fWrap)/((1.0f+fWrap) * (1.0f+fWrap));
 
      //add specular component 
      if(bSpecular && (NdotL > 0.0f)) 
      { 
		 float3 H = normalize(L + V);   //half vector 
         Out += FresnelSchlick(specular, L, H) * ((fMaterialPower + 2.0f) / 8.0f ) * pow(saturate(dot(N, H)), fMaterialPower) * NdotL * specular;
         //Out += pow(max(0.f, dot(H,N)), fMaterialPower) * specular;
      } 
 
      float fAtten = saturate( 1.0f - dot(lightDir/flightRange, lightDir/flightRange) ); //!!
  	  //float fAtten = flightRange*flightRange/dot(lightDir,lightDir);
      Out *= lights[i].vEmission * fAtten; 
   }
   
   return Out;
} 

//PIXEL SHADER
float4 psBall( in vout IN ) : COLOR
{
	float2 uv = float2(0,0);
	uv.x = IN.tex0.x*0.5f+0.5f;
	uv.y = IN.tex0.y*0.5f+0.5f;
	float4 ballImageColor = tex2D( texSampler0, uv );
	float4 decalColor = tex2D( texSampler2, IN.tex2 );
    
    float4 r = reflect(IN.eye,IN.normal);
	uv.x = position.x * invTableWidth;
	uv.y = position.y * invTableHeight;
	uv.x = uv.x + (r.x/(1.0f+r.z))*0.02f;
	uv.y = uv.y + (r.y/(1.0f+r.z))*0.02f;
	float4 playfieldColor = tex2D( texSampler1, uv );
	
	// !! fix these hacks!!
	playfieldColor.a = /*sqrt(((position.z-IN.worldPos.z)/radius+1.0f)*0.5f) **/ saturate(r.y)*sqrt(saturate(1.0f-(1.0f+r.z)/0.8f));
	playfieldColor.a=0;
	/*if( r.z<-0.4f )
	{
		playfieldColor.a = saturate(r.y)*0.4f;
	}
	else
	{
		playfieldColor.a = 0;
	}*/

    float3 color = float3(0.0f, 0.0f, 0.0f);
    
	for(int i = 0; i < iLightPointNum; i++)  
    { 
      color += DoPointLight(IN.worldPos, IN.normal, IN.eye, ballImageColor + decalColor.a*decalColor, vSpecularColor * decalColor.a*decalColor, i); //!! decal vs spec vs playfield??
    } 
    
	return float4(saturate(color + playfieldColor*playfieldColor.a + vAmbient),1);
}


float4 psBallReflection( in vout IN ) : COLOR
{
	float4 ballImageColor = tex2D( texSampler0, IN.tex0.xy );
	float sizeY = radius*(2.0f*0.9f);
	float alpha = (IN.tex1.y-position.y)/sizeY;
	alpha = (alpha*alpha)*(alpha*alpha)/0.4f;
	ballImageColor.a = saturate( alpha )*reflectionStrength*2.0f;
	return ballImageColor;
}

//------------------------------------

technique RenderBall
{
	pass p0 
	{		
		vertexshader = compile vs_3_0 vsBall();
		pixelshader  = compile ps_3_0 psBall();
	}
}

technique RenderBallReflection
{
	pass p0 
	{		
		vertexshader = compile vs_3_0 vsBallReflection();
		pixelshader  = compile ps_3_0 psBallReflection();
	}
}
