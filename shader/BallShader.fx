#define NUM_LIGHTS               2 
 
float4 vMaterialColor = float4(192.f/255.f, 128.f/255.f, 96.f/255.f, 1.f); 
float fMaterialPower = 16.f; 
 
float4 vAmbientColor = float4(128.f/255.f, 128.f/255.f, 128.f/255.f, 1.f); 
float  materialAlpha = 1.0f;
bool   bSpecular = false; 
float  lightRange=3000.0f;

struct CLight 
{ 
   float3 vPos; 
   float4 vAmbient; 
   float4 vDiffuse; 
   float4 vSpecular; 
   float3 vAttenuation; //1, D, D^2; 
}; 
 
int iLightPointNum=2; 
CLight lights[NUM_LIGHTS] = {                         //NUM_LIGHTS == 2
   { 
      float3(0.0f, 0.0f, 0.0f),              //position 
      float4(0.0f, 0.0f, 0.0f, 1.0f),        //ambient 
      float4(0.0f, 0.0f, 0.0f, 1.0f),        //diffuse 
      float4(0.0f, 0.0f, 0.0f, 1.0f),        //specular 
      float3(0.6f, 0.0000005f, 0.0000009f),                 //attenuation 
   }, 
   { 
      float3(0.0f, 0.0f, 0.0f),              //position 
      float4(0.0f, 0.0f, 0.0f, 1.0f),        //ambient 
      float4(0.0f, 0.0f, 0.0f, 1.0f),        //diffuse 
      float4(0.0f, 0.0f, 0.0f, 1.0f),        //specular 
      float3(0.6f, 0.0000005f, 0.0000009f),                 //attenuation 
   } 
}; 

float4x4 matWorldViewProj   : WORLDVIEWPROJ;
float4x4 matWorld           : WORLD;
float4x4 matWorldView       : WORLD;
float4   diffuseMaterial    = float4(1.0f,1.0f,1.0f,0.1f);
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
	float4 npos = float4(IN.normal,0.0);
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

float4 DoPointLight(float4 vPosition, float3 N, float3 V, int i) 
{ 
   float3 pos = (float3)mul(matWorld,vPosition);
   float3 light = lights[i].vPos;
   float3 lightDir = light-pos;
   float3 L = normalize(lightDir); 
   float NdotL = dot(N, L); 
   float4 color = lights[i].vAmbient; 
   float4 colorSpec = float4(0.0f,0.0f,0.0f,1.0f); 
   float fAtten = 1.f; 
   if(NdotL >= 0.f) 
   { 
      //compute diffuse color 
      color += (NdotL * lights[i].vDiffuse)*saturate(4.0f*NdotL); 
 
      //add specular component 
      if(bSpecular) 
      { 
         float3 H = normalize(L + V);   //half vector 
         colorSpec = pow(max(0, dot(H,N)), fMaterialPower) * lights[i].vSpecular; 
      } 
 
      fAtten = saturate( 1.0f - dot(lightDir/lightRange, lightDir/lightRange) );
      color.rgb *= fAtten; 
      colorSpec.rgb *= fAtten; 
   } 
   return saturate(color+colorSpec); 
} 

//PIXEL SHADER
float4 psBall( in vout IN ) : COLOR
{
	float2 uv = float2(0,0);
	uv.x = IN.tex0.x*0.5f+0.5f;
	uv.y = IN.tex0.y*0.5f+0.5f;
	float4 ballImageColor = tex2D( texSampler0, uv );
	float4 decalColor = tex2D( texSampler2, IN.tex2 );
    
    float4 r=(reflect(IN.eye,IN.normal));
	uv.x = position.x * invTableWidth;
	uv.y = position.y * invTableHeight;
	uv.x = uv.x + (r.x/(1.0f+r.z))*0.02f;
	uv.y = uv.y + (r.y/(1.0f+r.z))*0.02f;
	float4 playfieldColor = tex2D( texSampler1, uv );
	
	playfieldColor.a = /*sqrt(((position.z-IN.worldPos.z)/radius+1.0f)*0.5f)**/saturate(r.y)*sqrt(saturate(1.0f-(1.0f+r.z)/0.8f));
	playfieldColor.a=0;
	/*if( r.z<-0.4f )
	{
		playfieldColor.a = saturate(r.y)*0.4f;
	}
	else
	{
		playfieldColor.a = 0;
	}*/

	float4 lightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    lightColor = DoPointLight(IN.worldPos, IN.normal, IN.eye, 0);
    lightColor += DoPointLight(IN.worldPos, IN.normal, IN.eye, 1);
    lightColor.a=1.0f;
	float4 result = decalColor.a*decalColor + ballImageColor;
	return result*lightColor + playfieldColor*playfieldColor.a;
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
