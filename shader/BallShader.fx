#define NUM_LIGHTS 2

float3 vDiffuseColor = float3(0.05f,0.05f,0.05f); //!! pass from material!
float  fDiffuseWrap = 0.25f; //!! pass from material?, w in [0..1] for wrap lighting
float  fGlossyPower = 8.f; //!! pass from material?
float  fmaterialAlpha = 1.0f; //!! remove?

bool   bDiffuse  = true; //!! remove, steer from diffuse?  (performance?)
bool   bGlossy   = true; //!! remove, steer from glossy?   (performance?)
bool   bSpecular = true; //!! remove, steer from specular? (performance?)

float  freflectionStrength;

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
      float3(0.0f, 0.0f, 0.0f)         //emission
   }, 
   { 
      float3(0.0f, 0.0f, 0.0f),        //position 
      float3(0.0f, 0.0f, 0.0f)         //emission
   }
}; 

float4x4 matWorldViewProj   : WORLDVIEWPROJ;
float4x4 matWorldView       : WORLDVIEW;
//float4x4 matWorld           : WORLD;
float4x4 matWorldViewInverseTranspose;
float4x4 matView;

float    invTableHeight;
float    invTableWidth;

float4   position;
float    radius;
float    ballStretchX;
float    ballStretchY;

//float4   camera;

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
	float3 normal      : TEXCOORD1;
	float3 worldPos    : TEXCOORD2;
};

//vertex to pixel shader structure
struct voutReflection
{
    float4 position	   : POSITION0;
	float2 tex0        : TEXCOORD0;
	float3 r           : TEXCOORD1;
};


//VERTEX SHADER
vout vsBall( in vin IN )
{
    vout OUT;

	// apply spinning and move the ball to it's actual position
	float4x4 orientation = {m1,m2,m3,float4(0,0,0,1)};
	float4 pos = IN.position;
	pos = mul(pos, orientation);
	pos.xyz *= radius;
	pos.x *= ballStretchX;
	pos.y *= ballStretchY;
	pos.xyz += position.xyz;
	
	float3 p = mul(pos, matWorldView).xyz;
	
	// apply spinning to the normals too to get the sphere mapping effect
	float4 nspin = mul(float4(IN.normal,0.0f), orientation);
    float3 normal = normalize(mul(nspin, matWorldView/*InverseTranspose*/)).xyz; //!!?
    
	OUT.position = mul(pos, matWorldViewProj);
    OUT.tex0	 = IN.tex0;
	OUT.normal   = normal;
	OUT.worldPos = p;
	return OUT;
}

voutReflection vsBallReflection( in vin IN )
{
    voutReflection OUT;
    
	// apply spinning and move the ball to it's actual position
	float4x4 orientation = {m1,m2,m3,float4(0,0,0,1)};
	float4 pos = IN.position;
	pos = mul(pos, orientation);
	pos.xyz *= radius;
	pos.x *= ballStretchX;
	pos.y *= ballStretchY;
	pos.xyz += position.xyz;

	// this is not a 100% ball reflection on the table due to the quirky camera setup
	// the ball is moved a bit down and rendered again
	pos.y += radius*(2.0f*0.35f);
	pos.z = pos.z*0.5f - 10.0f;
	
	float3 p = mul(pos, matWorldView).xyz;
	
    float4 nspin = mul(float4(IN.normal,0.0f),orientation);
    float3 normal = normalize(mul(nspin, matWorldView/*InverseTranspose*/).xyz); //!!?
    
	float3 r = normalize(reflect(normalize(/*camera=0,0,0,1*/-p), normal));

    OUT.position = mul(pos, matWorldViewProj);
    OUT.r		 = r;
	OUT.tex0	 = pos.xy;
	return OUT;
}

float3 FresnelSchlick(float3 spec, float LdotH)
{
    return spec + (1.0f - spec) * pow(1.0f - LdotH, 5);
}

// assumes all light emission is premultiplied by PI
float3 DoPointLight(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, int i) 
{ 
   float3 lightDir = mul(float4(lights[i].vPos,1.0f), matView).xyz - pos; //!! do in vertex shader?! or completely before?!
   float3 L = normalize(lightDir);
   float NdotL = dot(N, L);
   float3 Out = float3(0.0f,0.0f,0.0f);
   
   // compute diffuse color (lambert with optional rim/wrap component)
   if(bDiffuse && (NdotL + fDiffuseWrap > 0.0f))
      Out = diffuse * ((NdotL + fDiffuseWrap) / ((1.0f+fDiffuseWrap) * (1.0f+fDiffuseWrap)));
 
   // add glossy component (modified ashikhmin/blinn bastard), not fully energy conserving, but good enough
   if(bGlossy && (NdotL > 0.0f))
   { 
	 float3 H = normalize(L + V); // half vector
	 float NdotH = dot(N, H);
	 float LdotH = dot(L, H);
	 float VdotH = dot(V ,H);
	 if((NdotH > 0.0f) && (LdotH > 0.0f) && (VdotH > 0.0f))
		Out += glossy * FresnelSchlick(glossy, LdotH) * (((fGlossyPower + 1.0f) / (8.0f*VdotH)) * pow(NdotH, fGlossyPower));
   }
 
   float fAtten = saturate( 1.0f - dot(lightDir/flightRange, lightDir/flightRange) ); //!!
   //float fAtten = flightRange/dot(lightDir,lightDir);

   Out *= lights[i].vEmission * fAtten;
   
   return Out; 
}

float4 lightLoop(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, float3 specular)
{
   // normalize input vectors for BRDF evals
   N = normalize(N);
   V = normalize(V);
   
   // normalize BRDF layer inputs
   float diffuseMax = bDiffuse ? max(diffuse.x,max(diffuse.y,diffuse.z)) : 0.0f;
   float glossyMax = bGlossy ? max(glossy.x,max(glossy.y,glossy.z)) : 0.0f;
   float specularMax = bSpecular ? max(specular.x,max(specular.y,specular.z)) : 0.0f;
   float sum = diffuseMax + glossyMax + specularMax;
   if(sum > 1.0f)
   {
      float invsum = 1.0f/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      specular *= invsum;
   }

   //if(dot(N,V) < 0.0f) //!! flip normal in case of wrong orientation? (backside lighting)
   //   N = -N;

   float3 color = float3(0.0f, 0.0f, 0.0f);
      
   if((bDiffuse && diffuseMax > 0.0f) || (bGlossy && glossyMax > 0.0f))
      for(int i = 0; i < iLightPointNum; i++)  
         color += DoPointLight(pos, N, V, diffuse, glossy, i); // no specular needed as only pointlights so far
         
   //if((bDiffuse && diffuseMax > 0.0f) || (bSpecular && specularMax > 0.0f))
   //   color += DoEnvmap(pos, N, V, diffuse, specular); // no glossy, as it's the most hacky one //!! -> use mipmap-hacks for glossy?
   
   if(bSpecular && specularMax > 0.0f)
      color += specular; //!! blend?
  
   return float4(saturate(vAmbient + color), fmaterialAlpha); //!! in case of HDR out later on, remove saturate
}

//PIXEL SHADER
float4 psBall( in vout IN ) : COLOR
{
    float3 r = reflect(normalize(/*camera=0,0,0,1*/-IN.worldPos), normalize(IN.normal));

	float2 uv0;
	uv0.x = r.x*0.5f+0.5f;
	uv0.y = r.y*0.5f+0.5f;
	float3 ballImageColor = tex2D( texSampler0, uv0 ).xyz;

	float4 decalColor = tex2D( texSampler2, IN.tex0 );
	decalColor.xyz *= decalColor.a;
	
	/*float3 normal = float3(0,0,1);
	float NdotR = dot(normal,r);*/
	// opt.:
	float NdotR = r.z;
	
	float3 playfieldColor;
	if((freflectionStrength > 0.0f) && (NdotR < 0.0f))
	{      
	   float3 mid = mul(position, matWorldView).xyz;
	   /*float3 p0 = float3(0,0,0);
	   float3 pos = IN.worldPos-mid;
	   pos.z += radius;	
	   float t = (normal.x*(p0.x - pos.x) + normal.y*(p0.y - pos.y) + normal.z*(p0.z - pos.z))/NdotR;
       float3 hit = pos + t*r;*/
       // opt.:
       float2 hit = IN.worldPos.xy - mid.xy - (IN.worldPos.z - mid.z + radius)*r.xy/NdotR;

       float2 uv;
	   uv.x = (position.x + hit.x) * invTableWidth;
	   uv.y = (position.y + hit.y) * invTableHeight;
	   playfieldColor = tex2D( texSampler1, uv ).xyz;
	   
	   //!! hack to get some lighting on sample
	   playfieldColor = lightLoop(mid, mul(float4(/*normal=*/0,0,1,0), matWorldView).xyz, /*camera=0,0,0,1*/-IN.worldPos, playfieldColor, float3(0,0,0), float3(0,0,0)).xyz;
	   
	   //!! magic falloff & weight the rest in from the ballImage
	   float weight = freflectionStrength*sqrt(-NdotR);
	   playfieldColor *= weight;
	   playfieldColor += ballImageColor*(1.0f-weight);
	}
	else
	   playfieldColor = ballImageColor;

	float3 diffuse  = vDiffuseColor + decalColor.xyz; // assume that decal is used for scratches and/or stickers/logos
    float3 glossy   = diffuse;
    float3 specular = playfieldColor;
   
    return lightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular);
}


float4 psBallReflection( in voutReflection IN ) : COLOR
{
	float3 ballImageColor = vDiffuseColor + tex2D( texSampler0, IN.r.xy ).xyz; //!! just add the ballcolor in, this is a whacky reflection anyhow
	float alpha = saturate((IN.tex0.y-position.y)/radius);
	alpha = (alpha*alpha)*(alpha*alpha)*freflectionStrength;
	return float4(saturate(ballImageColor),alpha);
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
