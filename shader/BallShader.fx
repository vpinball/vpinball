//!! always adapt to changes from BasicShader.fx
//!! add playfield color, glossy, etc for more precise reflections of playfield
#include "Globals.fxh"

float    freflectionStrength;
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

//vertex to pixel shader structure
struct voutTrail
{
    float4 position	   : POSITION0;
	float2 tex0        : TEXCOORD0;
	float  alpha       : TEXCOORD1;
};

//------------------------------------
// VERTEX SHADER

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
	OUT.tex0	 = pos.xy;
    OUT.r		 = r;
	return OUT;
}

voutTrail vsBallTrail( in vin IN )
{
    voutTrail OUT;
    
    OUT.position = mul(IN.position, matWorldViewProj);
	OUT.tex0	 = IN.tex0;
    OUT.alpha	 = IN.normal.x; //!! abuses normal for now
	return OUT;
}

//------------------------------------

float4 ballLightLoop(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, float3 specular, float edge)
{
   // normalize input vectors for BRDF evals
   N = normalize(N);
   V = normalize(V);
   
   // normalize BRDF layer inputs //!! use diffuse = (1-glossy)*diffuse instead?
   float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));
   float glossyMax = max(glossy.x,max(glossy.y,glossy.z));
   float specularMax = max(specular.x,max(specular.y,specular.z)); //!! not needed as 2nd layer only so far
   float sum = diffuseMax + glossyMax; //+ specularMax
   if(sum > 1.0f)
   {
      float invsum = 1.0f/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      //specular *= invsum;
   }

   //if(dot(N,V) < 0.0f) //!! flip normal in case of wrong orientation? (backside lighting)
   //   N = -N;

   float3 color = float3(0.0f, 0.0f, 0.0f);
      
   if((!bIsMetal && (diffuseMax > 0.0f)) || (glossyMax > 0.0f))
   {
      for(int i = 0; i < iLightPointNum; i++)  
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, fRoughness, i); // no clearcoat needed as only pointlights so far
   }

   if(specularMax > 0.0f)
      color += specular; //!! blend? //!! Fresnel with 1st layer?
  
   return float4(/*Gamma(ToneMap(*/vAmbient + color/*))*/, fmaterialAlpha);
}


//------------------------------------
// PIXEL SHADER

float4 psBall( in vout IN ) : COLOR
{
    float3 r = reflect(normalize(/*camera=0,0,0,1*/-IN.worldPos), normalize(IN.normal));

	float2 uv0;
	uv0.x = r.x*0.5f+0.5f;
	uv0.y = r.y*0.5f+0.5f;
	float3 ballImageColor = InvGamma(tex2D( texSampler0, uv0 ).xyz)*fenvEmissionScale;

	float4 decalColor = tex2D( texSampler2, IN.tex0 );
	decalColor.xyz = InvGamma(decalColor.xyz)*decalColor.a;
	
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
	   playfieldColor = InvGamma(tex2D( texSampler1, uv ).xyz); //!! rather use screen space sample from previous frame??
	   
	   //!! hack to get some lighting on sample
	   playfieldColor = lightLoop(mid, mul(float4(/*normal=*/0,0,1,0), matWorldView).xyz, /*camera=0,0,0,1*/-mid, playfieldColor, float3(0,0,0), float3(0,0,0), 1.0f).xyz;
	   
	   //!! magic falloff & weight the rest in from the ballImage
	   float weight = freflectionStrength*sqrt(-NdotR);
	   playfieldColor *= weight;
	   playfieldColor += ballImageColor*(1.0f-weight);
	}
	else
	   playfieldColor = ballImageColor;

	float3 diffuse  = cBase + decalColor.xyz; // assume that decal is used for scratches and/or stickers/logos
    float3 glossy   = diffuse;
    float3 specular = playfieldColor;
   
    return ballLightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, 1.0f);
}


float4 psBallReflection( in voutReflection IN ) : COLOR
{
	float3 ballImageColor = cBase + tex2D( texSampler0, IN.r.xy ).xyz; //!! just add the ballcolor in, this is a whacky reflection anyhow
	float alpha = saturate((IN.tex0.y-position.y)/radius);
	alpha = (alpha*alpha)*(alpha*alpha)*freflectionStrength;
	return float4(saturate(ballImageColor),alpha);
}

float4 psBallTrail( in voutTrail IN ) : COLOR
{
	return saturate((float4(cBase,0.0f) + tex2D( texSampler0, IN.tex0 ))*IN.alpha); //!! just add the ballcolor in, this is a whacky reflection anyhow
}

//------------------------------------
// Techniques

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

technique RenderBallTrail
{
	pass p0 
	{		
		vertexshader = compile vs_3_0 vsBallTrail();
		pixelshader  = compile ps_3_0 psBallTrail();
	}
}
