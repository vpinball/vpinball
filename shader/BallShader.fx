//!! always adapt to changes from BasicShader.fx
//!! add playfield color, glossy, etc for more precise reflections of playfield

#include "Helpers.fxh"

// transformation matrices
float4x4 matWorldViewProj : WORLDVIEWPROJ;
float4x4 matWorldView     : WORLDVIEW;
//float4x4 matWorldViewInverseTranspose;
float4x4 matView;
float4x4 matViewInverse;

texture  Texture0; // base texture
texture  Texture1; // envmap
texture  Texture2; // envmap radiance

sampler2D texSampler0 : TEXUNIT0 = sampler_state // base texture
{
	Texture	  = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
	//ADDRESSU  = Wrap; //!! ?
	//ADDRESSV  = Wrap;
};

sampler2D texSampler1 : TEXUNIT1 = sampler_state // environment
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

sampler2D texSampler7 : TEXUNIT2 = sampler_state // ball decal
{
	Texture	  = (Texture2);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Wrap;
	ADDRESSV  = Wrap;
};

#include "Material.fxh"

float4   position_radius;
float4   ballStretch_invTableRes;

// this is used for the orientation matrix
float4   m1;
float4   m2;
float4   m3;

float2   reflection_ball_playfield;

bool     decalMode;

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
	pos.x = pos.x*position_radius.w*ballStretch_invTableRes.x + position_radius.x;
	pos.y = pos.y*position_radius.w*ballStretch_invTableRes.y + position_radius.y;
	pos.z = pos.z*position_radius.w + position_radius.z;
	
	float3 p = mul(pos, matWorldView).xyz;
	
	// apply spinning to the normals too to get the sphere mapping effect
	float4 nspin = mul(float4(IN.normal,0.0), orientation);
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
	pos.x = pos.x*position_radius.w*ballStretch_invTableRes.x + position_radius.x;
	pos.y = pos.y*position_radius.w*ballStretch_invTableRes.y + position_radius.y;
	pos.z = pos.z*position_radius.w + position_radius.z;

	// this is not a 100% ball reflection on the table due to the quirky camera setup
	// the ball is moved a bit down and rendered again
	pos.y += position_radius.w*(2.0*0.35);
	pos.z = pos.z*0.5 - 10.0;
	
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

float3 ballLightLoop(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, float3 specular, float edge)
{
   // normalize input vectors for BRDF evals
   N = normalize(N);
   V = normalize(V);
   
   // normalize BRDF layer inputs //!! use diffuse = (1-glossy)*diffuse instead?
   float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));
   float glossyMax = max(glossy.x,max(glossy.y,glossy.z));
   float specularMax = max(specular.x,max(specular.y,specular.z)); //!! not needed as 2nd layer only so far
   float sum = diffuseMax + glossyMax; //+ specularMax
   if(sum > 1.0)
   {
      float invsum = 1.0/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      //specular *= invsum;
   }

   //if(dot(N,V) < 0.0) //!! flip normal in case of wrong orientation? (backside lighting)
   //   N = -N;

   float3 color = float3(0.0, 0.0, 0.0);
      
   if((!bIsMetal && (diffuseMax > 0.0)) || (glossyMax > 0.0))
   {
      for(int i = 0; i < iLightPointBallsNum; i++)  
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge.x, i); // no clearcoat needed as only pointlights so far
   }

   if(specularMax > 0.0)
      color += specular; //!! blend? //!! Fresnel with 1st layer?
  
   return /*Gamma(ToneMap(*/cAmbient_LightRange.xyz + color/*))*/;
}


//------------------------------------
// PIXEL SHADER

float4 psBall( in vout IN ) : COLOR
{
    float3 v = normalize(/*camera=0,0,0,1*/-IN.worldPos);
    float3 r = reflect(v, normalize(IN.normal));

    float edge = dot(v, r);
    float lod = (edge > 0.6) ? // edge falloff to reduce aliasing on edges
	edge*(6.0*1.0/0.4)-(6.0*0.6/0.4) :
	0.0;

	float2 uv0;
	uv0.x = r.x*0.5 + 0.5;
	uv0.y = r.y*0.5 + 0.5;
    float3 ballImageColor = InvGamma(tex2Dlod( texSampler0, float4(uv0, 0.,lod) ).xyz);
   
	float4 decalColorT = tex2D( texSampler7, IN.tex0 );
	float3 decalColor = InvGamma(decalColorT.xyz);
	if ( !decalMode )
	{
	   // decal texture is an alpha scratch texture and must be added to the ball texture
	   // the strength of the scratches totally rely on the alpha values.
	   decalColor *= decalColorT.a;
	   ballImageColor = (ballImageColor+decalColor) * fenvEmissionScale;
	}
	else
	   ballImageColor = Screen( ballImageColor, decalColor ) * (0.5*fenvEmissionScale); //!! 0.5=magic
	
	/*float3 normal = float3(0,0,1);
	float NdotR = dot(normal,r);*/
	// opt.:
	float NdotR = r.z;
	
	float3 playfieldColor;
	if((reflection_ball_playfield.y > 0.0) && (NdotR < 0.0))
	{      
	   float3 mid = mul(float4(position_radius.xyz,1.0), matWorldView).xyz;
	   /*float3 p0 = float3(0,0,0);
	   float3 pos = IN.worldPos-mid;
	   pos.z += radius;	
	   float t = (normal.x*(p0.x - pos.x) + normal.y*(p0.y - pos.y) + normal.z*(p0.z - pos.z))/NdotR;
       float3 hit = pos + t*r;*/
       // opt.:
       float2 hit = IN.worldPos.xy - mid.xy - (IN.worldPos.z - mid.z + position_radius.w)*r.xy/NdotR;

       float2 uv;
	   uv.x = (position_radius.x + hit.x) * ballStretch_invTableRes.z;
	   uv.y = (position_radius.y + hit.y) * ballStretch_invTableRes.w;
	   playfieldColor = InvGamma(tex2Dlod( texSampler1, float4(uv, 0.,0.) ).xyz); //!! rather use screen space sample from previous frame??
	   
	   //!! hack to get some lighting on sample, but only diffuse, the rest is not setup correctly anyhow
	   playfieldColor = lightLoop(mid, mul(float4(/*normal=*/0,0,1,0), matWorldView).xyz, /*camera=0,0,0,1*/-mid, playfieldColor, float3(0,0,0), float3(0,0,0), 1.0);
	   
	   //!! magic falloff & weight the rest in from the ballImage
	   float weight = reflection_ball_playfield.y*-NdotR; //!! sqrt(-NdotR)?
	   playfieldColor *= weight;
	   playfieldColor += ballImageColor*(1.0-weight);
	}
	else
	   playfieldColor = ballImageColor;

	float3 diffuse  = cBase_Alpha.xyz*0.075;
	if(!decalMode)
	    diffuse *= decalColor; // scratches make the material more rough
    float3 glossy   = max(diffuse*2.0, float3(0.1,0.1,0.1)); //!! meh
    float3 specular = playfieldColor*cBase_Alpha.xyz;
	if(!decalMode)
	    specular *= float3(1.,1.,1.)-decalColor; // see above
   
    float4 result;
	result.xyz = ballLightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, 1.0);
	result.a = cBase_Alpha.a;

    return result;
}


float4 psBallReflection( in voutReflection IN ) : COLOR
{
	float3 ballImageColor = (cBase_Alpha.xyz*(0.075*0.25) + InvGamma(tex2D( texSampler0, IN.r.xy ).xyz))*fenvEmissionScale; //!! just add the ballcolor in, this is a whacky reflection anyhow
	float alpha = saturate((IN.tex0.y-position_radius.y)/position_radius.w);
	alpha = (alpha*alpha)*(alpha*alpha)*reflection_ball_playfield.x;
	return float4(ballImageColor,alpha);
}

float4 psBallTrail( in voutTrail IN ) : COLOR
{
	return float4((cBase_Alpha.xyz*(0.075*0.25) + InvGamma(tex2D( texSampler0, IN.tex0 ).xyz))*fenvEmissionScale, IN.alpha); //!! just add the ballcolor in, this is a whacky trail anyhow
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
