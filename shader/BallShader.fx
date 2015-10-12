//!! always adapt to changes from BasicShader.fx
//!! add playfield color, glossy, etc for more precise reflections of playfield

#define NUM_BALL_LIGHTS 4

#include "Helpers.fxh"

// transformation matrices
float4x4 matWorldViewProj : WORLDVIEWPROJ;
float4x4 matWorldView     : WORLDVIEW;
float4x4 matWorldViewInverse;
float4x4 matWorldViewInverseTranspose;
float4x4 matView;
float4x4 matViewInverseInverseTranspose;
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

sampler2D texSampler1 : TEXUNIT1 = sampler_state // playfield (should actually be environment if specular and glossy needed in lightloop!)
{
	Texture	  = (Texture1);
    MIPFILTER = LINEAR; //!! ?
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Wrap;
	ADDRESSV  = Wrap;
};

sampler2D texSampler2 : TEXUNIT2 = sampler_state // see below (should actually be diffuse environment contribution/radiance if diffuse needed in lightloop!)
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

bool     hdrEnvTextures = false;

#include "Material.fxh"

float4   position_radius;
float4   ballStretch_invTableRes;

//float    reflection_ball_playfield;

float3x3 orientation;

float    playfield_height;

bool     decalMode;
bool     cabMode;
bool     hdrTexture0;

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
    float3 tex0_alpha  : TEXCOORD0;
};

//------------------------------------
// VERTEX SHADER

vout vsBall( in vin IN )
{
    vout OUT;

	// apply spinning and move the ball to it's actual position
	float4 pos = IN.position;
	pos.xyz = mul(pos.xyz, orientation);
	pos.x = pos.x*position_radius.w*ballStretch_invTableRes.x + position_radius.x;
	pos.y = pos.y*position_radius.w*ballStretch_invTableRes.y + position_radius.y;
	pos.z = pos.z*position_radius.w + position_radius.z;
	
	const float3 p = mul(pos, matWorldView).xyz;
	
	// apply spinning to the normals too to get the sphere mapping effect
	const float3 nspin = mul(IN.normal, orientation);
    const float3 normal = normalize(mul(float4(nspin,0.), matWorldViewInverseTranspose).xyz);
    
	OUT.position = mul(pos, matWorldViewProj);
    OUT.tex0	 = IN.tex0;
	OUT.normal   = normal;
	OUT.worldPos = p;
	return OUT;
}

#if 0
voutReflection vsBallReflection( in vin IN )
{
    voutReflection OUT;
    
	// apply spinning and move the ball to it's actual position
	float4 pos = IN.position;
	pos.xyz = mul(pos.xyz, orientation);
	pos.x = pos.x*position_radius.w*ballStretch_invTableRes.x + position_radius.x;
	pos.y = pos.y*position_radius.w*ballStretch_invTableRes.y + position_radius.y;
	pos.z = pos.z*position_radius.w + position_radius.z;

	// this is not a 100% ball reflection on the table due to the quirky camera setup
	// the ball is moved a bit down and rendered again
	pos.y += position_radius.w*(2.0*0.35);
	pos.z = pos.z*0.5 - 10.0;
	
	const float3 p = mul(pos, matWorldView).xyz;
	
    const float3 nspin = mul(IN.normal, orientation);
    const float3 normal = normalize(mul(float4(nspin,0.), matWorldViewInverseTranspose).xyz);
    
    const float3 r = reflect(normalize(/*camera=0,0,0,1*/-p), normal);

    OUT.position = mul(pos, matWorldViewProj);
	OUT.tex0	 = pos.xy;
    OUT.r		 = r;
	return OUT;
}
#endif

voutTrail vsBallTrail( in vin IN )
{
    voutTrail OUT;
    
    OUT.position = mul(IN.position, matWorldViewProj);
	OUT.tex0_alpha = float3(IN.tex0, IN.normal.x); //!! abuses normal for now
	return OUT;
}

//------------------------------------

float3 ballLightLoop(float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, float3 specular, float edge)
{
   // normalize input vectors for BRDF evals
   N = normalize(N);
   V = normalize(V);
   
   // normalize BRDF layer inputs //!! use diffuse = (1-glossy)*diffuse instead?
   const float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));
   const float glossyMax = max(glossy.x,max(glossy.y,glossy.z));
   const float specularMax = max(specular.x,max(specular.y,specular.z)); //!! not needed as 2nd layer only so far
   const float sum = diffuseMax + glossyMax; //+ specularMax
   if(sum > 1.0)
   {
      const float invsum = 1.0/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      //specular *= invsum;
   }

   //if(dot(N,V) < 0.0) //!! flip normal in case of wrong orientation? (backside lighting)
   //   N = -N;

   float3 color = float3(0.0, 0.0, 0.0);
      
   if(((Roughness_WrapL_Edge_IsMetal.w == 0.0) && (diffuseMax > 0.0)) || (glossyMax > 0.0))
   {
      for(int i = 0; i < iLightPointBallsNum; i++)  
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_IsMetal.x, i); // no clearcoat needed as only pointlights so far
   }

   if(specularMax > 0.0)
      color += specular; //!! blend? //!! Fresnel with 1st layer?
  
   return /*Gamma(ToneMap(*/color/*))*/;
}


//------------------------------------
// PIXEL SHADER

float4 psBall( in vout IN ) : COLOR
{
    const float3 v = normalize(/*camera=0,0,0,1*/-IN.worldPos);
    const float3 r = reflect(v, normalize(IN.normal));
    // calculate the intermediate value for the final texture coords. found here http://www.ozone3d.net/tutorials/glsl_texturing_p04.php
	const float  m = (r.z + 1.0 > 0.) ? 0.3535533905932737622 / sqrt(r.z + 1.0) : 0.; // 0.353...=0.5/sqrt(2)
    const float edge = dot(v, r);
    const float lod = (edge > 0.6) ? // edge falloff to reduce aliasing on edges (picks smaller mipmap -> more blur)
		edge*(6.0*1.0/0.4)-(6.0*0.6/0.4) :
		0.0;

    const float2 uv0 = cabMode ? float2(r.y*-m + 0.5, r.x*-m + 0.5) : float2(r.x*-m + 0.5, r.y*m + 0.5);
    float3 ballImageColor = tex2Dlod(texSampler0, float4(uv0, 0., lod)).xyz;
    if (!hdrTexture0)
        ballImageColor = InvGamma(ballImageColor);

	const float4 decalColorT = tex2D( texSampler7, IN.tex0 );
	float3 decalColor = InvGamma(decalColorT.xyz);
	if ( !decalMode )
	{
	   // decal texture is an alpha scratch texture and must be added to the ball texture
	   // the strength of the scratches totally rely on the alpha values.
	   decalColor *= decalColorT.a;
	   ballImageColor = (ballImageColor+decalColor) * fenvEmissionScale_TexWidth.x;
	}
	else
	   ballImageColor = Screen( saturate(ballImageColor), decalColor ) * (0.5*fenvEmissionScale_TexWidth.x); //!! 0.5=magic
	
	const float3 playfield_normal = mul(float4(0.,0.,1.,0.), matWorldViewInverseTranspose).xyz;
	const float NdotR = dot(playfield_normal,r);
	
	float3 playfieldColor;
	if(/*(reflection_ball_playfield > 0.0) &&*/ (NdotR > 0.0))
	{
       const float3 playfield_p0 = mul(float4(/*playfield_pos=*/0.,0.,playfield_height,1.), matWorldView).xyz;
	   const float t = dot(playfield_normal, IN.worldPos - playfield_p0) / NdotR;
       const float3 playfield_hit = IN.worldPos - t*r;

       const float2 uv = mul(float4(playfield_hit,1.), matWorldViewInverse).xy * ballStretch_invTableRes.zw;
	   playfieldColor = (t < 0.) ? float3(0., 0., 0.) // happens for example when inside kicker
                                 : InvGamma(tex2Dlod(texSampler1, float4(uv, 0., 0.)).xyz); //!! rather use screen space sample from previous frame??

       //!! hack to get some lighting on sample, but only diffuse, the rest is not setup correctly anyhow
       playfieldColor = lightLoop(playfield_hit, playfield_normal, -r, playfieldColor, float3(0.,0.,0.), float3(0.,0.,0.), 1.0);

	   //!! magic falloff & weight the rest in from the ballImage
	   const float weight = /*reflection_ball_playfield*/NdotR; //!! sqrt(NdotR)?
	   playfieldColor *= weight;
	   playfieldColor += ballImageColor*(1.0-weight);
	}
	else
	   playfieldColor = ballImageColor;

	float3 diffuse = cBase_Alpha.xyz*0.075;
	if(!decalMode)
	    diffuse *= decalColor; // scratches make the material more rough
    const float3 glossy = max(diffuse*2.0, float3(0.1,0.1,0.1)); //!! meh
    float3 specular = playfieldColor*cBase_Alpha.xyz; //!! meh, too, as only added in ballLightLoop anyhow
	if(!decalMode)
	    specular *= float3(1.,1.,1.)-decalColor; // see above
   
    float4 result;
	result.xyz = ballLightLoop(IN.worldPos, IN.normal, /*camera=0,0,0,1*/-IN.worldPos, diffuse, glossy, specular, 1.0);
	result.a = cBase_Alpha.a;

    return result;
}

#if 0
float4 psBallReflection( in voutReflection IN ) : COLOR
{
   const float2 envTex = cabMode ? float2(IN.r.y*0.5f + 0.5f, -IN.r.x*0.5f + 0.5f) : float2(IN.r.x*0.5f + 0.5f, IN.r.y*0.5f + 0.5f);
   float3 ballImageColor = tex2D(texSampler0, envTex).xyz;
   if(!hdrTexture0)
      ballImageColor = InvGamma(ballImageColor);
   ballImageColor = (cBase_Alpha.xyz*(0.075*0.25) + ballImageColor)*fenvEmissionScale_TexWidth.x; //!! just add the ballcolor in, this is a whacky reflection anyhow
   float alpha = saturate((IN.tex0.y - position_radius.y) / position_radius.w);
   alpha = (alpha*alpha)*(alpha*alpha)*reflection_ball_playfield;
   return float4(ballImageColor,alpha);
}
#endif

float4 psBallTrail( in voutTrail IN ) : COLOR
{
   float3 ballImageColor = tex2D(texSampler0, IN.tex0_alpha.xy).xyz;
   if (!hdrTexture0)
      ballImageColor = InvGamma(ballImageColor);
   return float4((cBase_Alpha.xyz*(0.075*0.25) + ballImageColor)*fenvEmissionScale_TexWidth.x, IN.tex0_alpha.z); //!! just add the ballcolor in, this is a whacky trail anyhow
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

/*technique RenderBallReflection
{
	pass p0 
	{		
		vertexshader = compile vs_3_0 vsBallReflection();
		pixelshader  = compile ps_3_0 psBallReflection();
	}
}*/

technique RenderBallTrail
{
	pass p0 
	{		
		vertexshader = compile vs_3_0 vsBallTrail();
		pixelshader  = compile ps_3_0 psBallTrail();
	}
}
