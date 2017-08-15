//!! always adapt to changes from BasicShader.fx
//!! add playfield color, glossy, etc for more precise reflections of playfield

#define NUM_BALL_LIGHTS 8

#include "Helpers.fxh"

// transformation matrices
float4x4 matWorldViewProj : WORLDVIEWPROJ;
float4x4 matWorldView     : WORLDVIEW;
float4x3 matWorldViewInverse;
//float4x4 matWorldViewInverseTranspose; // matWorldViewInverse used instead and multiplied from other side
float4x3 matView;
//float4x4 matViewInverseInverseTranspose; // matView used instead and multiplied from other side

texture  Texture0; // base texture
texture  Texture1; // playfield (should be envmap if specular or glossy needed/enabled, see below)
texture  Texture2; // envmap radiance
texture  Texture3; // decal

sampler2D texSampler0 : TEXUNIT0 = sampler_state // base texture
{
	Texture	  = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
	//ADDRESSU  = Wrap; //!! ?
	//ADDRESSV  = Wrap;
};

sampler2D texSampler1 : TEXUNIT1 = sampler_state // playfield (should actually be environment if specular and glossy needed/enabled in lightloop!)
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

sampler2D texSampler7 : TEXUNIT3 = sampler_state // ball decal
{
	Texture	  = (Texture3);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Wrap;
	ADDRESSV  = Wrap;
};

bool     hdrEnvTextures = false;

#include "Material.fxh"

float4   invTableRes__playfield_height_reflection;

//float    reflection_ball_playfield;

float4x3 orientation;

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
	float4 normal_t0x  : TEXCOORD0; // tex0 is stored in w of float4s
	float4 worldPos_t0y: TEXCOORD1;
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
	// apply spinning and move the ball to it's actual position
	float4 pos = IN.position;
	pos.xyz = mul_w1(pos.xyz, orientation);
	
	// apply spinning to the normals too to get the sphere mapping effect
	const float3 nspin = mul_w0(IN.normal, orientation);
	const float3 normal = normalize(mul(matWorldViewInverse, nspin).xyz); // actually: mul(float4(nspin,0.), matWorldViewInverseTranspose), but optimized to save one matrix

	const float3 p = mul_w1(pos.xyz, matWorldView);

	vout OUT;
	OUT.position = mul(pos, matWorldViewProj);
	OUT.normal_t0x = float4(normal,IN.tex0.x);
	OUT.worldPos_t0y = float4(p,IN.tex0.y);
	return OUT;
}

#if 0
voutReflection vsBallReflection( in vin IN )
{
	// apply spinning and move the ball to it's actual position
	float4 pos = IN.position;
	pos.xyz = mul_w1(pos.xyz, orientation);

	// this is not a 100% ball reflection on the table due to the quirky camera setup
	// the ball is moved a bit down and rendered again
	pos.y += position_radius.w*(2.0*0.35);
	pos.z = pos.z*0.5 - 10.0;

	const float3 p = mul_w1(pos.xyz, matWorldView);

    const float3 nspin = mul_w0(IN.normal, orientation);
	const float3 normal = normalize(mul(matWorldViewInverse, nspin).xyz); // actually: mul(float4(nspin,0.), matWorldViewInverseTranspose), but optimized to save one matrix

    const float3 r = reflect(normalize(/*camera=0,0,0,1*/-p), normal);

	voutReflection OUT;
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

float3 ballLightLoop(const float3 pos, float3 N, float3 V, float3 diffuse, float3 glossy, const float3 specular, const float edge, const bool is_metal)
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
      
   [branch] if((!is_metal && (diffuseMax > 0.0)) || (glossyMax > 0.0))
   {
      for(int i = 0; i < iLightPointBallsNum; i++)  
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal); // no clearcoat needed as only pointlights so far
   }

   [branch] if(!is_metal && (diffuseMax > 0.0))
      color += DoEnvmapDiffuse(normalize(mul(matView, N).xyz), diffuse); // trafo back to world for lookup into world space envmap // actually: mul(float4(N, 0.0), matViewInverseInverseTranspose)

   if(specularMax > 0.0)
      color += specular; //!! blend? //!! Fresnel with 1st layer?
  
   return color;
}


//------------------------------------
// PIXEL SHADER

float4 psBall( in vout IN, uniform bool cabMode, uniform bool decalMode ) : COLOR
{
    const float3 v = normalize(/*camera=0,0,0,1*/-IN.worldPos_t0y.xyz);
    const float3 r = reflect(v, normalize(IN.normal_t0x.xyz));
    // calculate the intermediate value for the final texture coords. found here http://www.ozone3d.net/tutorials/glsl_texturing_p04.php
	const float  m = (r.z + 1.0 > 0.) ? 0.3535533905932737622 * rsqrt(r.z + 1.0) : 0.; // 0.353...=0.5/sqrt(2)
    const float edge = dot(v, r);
    const float lod = (edge > 0.6) ? // edge falloff to reduce aliasing on edges (picks smaller mipmap -> more blur)
		edge*(6.0*1.0/0.4)-(6.0*0.6/0.4) :
		0.0;

    const float2 uv0 = cabMode ? float2(r.y*-m + 0.5, r.x*-m + 0.5) : float2(r.x*-m + 0.5, r.y*m + 0.5);
    float3 ballImageColor = tex2Dlod(texSampler0, float4(uv0, 0., lod)).xyz;
    if (!hdrTexture0)
        ballImageColor = InvGamma(ballImageColor);

	const float4 decalColorT = tex2D(texSampler7, float2(IN.normal_t0x.w,IN.worldPos_t0y.w));
	float3 decalColor = InvGamma(decalColorT.xyz);
	if ( !decalMode )
	{
	   // decal texture is an alpha scratch texture and must be added to the ball texture
	   // the strength of the scratches totally rely on the alpha values.
	   decalColor *= decalColorT.a;
	   ballImageColor = (ballImageColor+decalColor) * fenvEmissionScale_TexWidth.x;
	}
	else
	   ballImageColor = ScreenHDR( ballImageColor, decalColor ) * (0.5*fenvEmissionScale_TexWidth.x); //!! 0.5=magic
	
	const float3 playfield_normal = mul(matWorldViewInverse, float3(0.,0.,1.)).xyz; // actually: mul(float4(0.,0.,1.,0.), matWorldViewInverseTranspose), but optimized to save one matrix
	const float NdotR = dot(playfield_normal,r);
	
	float3 playfieldColor;
	[branch] if(/*(reflection_ball_playfield > 0.0) &&*/ (NdotR > 0.0))
	{
       const float3 playfield_p0 = mul_w1(float3(/*playfield_pos=*/0.,0.,invTableRes__playfield_height_reflection.z), matWorldView);
       const float t = dot(playfield_normal, IN.worldPos_t0y.xyz - playfield_p0) / NdotR;
       const float3 playfield_hit = IN.worldPos_t0y.xyz - t*r;

       const float2 uv = mul_w1(playfield_hit, matWorldViewInverse).xy * invTableRes__playfield_height_reflection.xy;
	   playfieldColor = (t < 0.) ? float3(0., 0., 0.) // happens for example when inside kicker
                                 : InvGamma(tex2Dlod(texSampler1, float4(uv, 0., 0.)).xyz)*invTableRes__playfield_height_reflection.w; //!! rather use screen space sample from previous frame??

       //!! hack to get some lighting on sample, but only diffuse, the rest is not setup correctly anyhow
       playfieldColor = lightLoop(playfield_hit, playfield_normal, -r, playfieldColor, float3(0.,0.,0.), float3(0.,0.,0.), 1.0, true, false);

	   //!! magic falloff & weight the rest in from the ballImage
	   const float weight = NdotR*NdotR;
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
	result.xyz = ballLightLoop(IN.worldPos_t0y.xyz, IN.normal_t0x.xyz, /*camera=0,0,0,1*/-IN.worldPos_t0y.xyz, diffuse, glossy, specular, 1.0, false);
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
		pixelshader  = compile ps_3_0 psBall(false,false);
	}
}

technique RenderBall_DecalMode
{
   pass p0
   {
      vertexshader = compile vs_3_0 vsBall();
      pixelshader = compile ps_3_0 psBall(false, true);
   }
}

technique RenderBall_CabMode
{
   pass p0
   {
      vertexshader = compile vs_3_0 vsBall();
      pixelshader = compile ps_3_0 psBall(true,false);
   }
}
technique RenderBall_CabMode_DecalMode
{
   pass p0
   {
      vertexshader = compile vs_3_0 vsBall();
      pixelshader = compile ps_3_0 psBall(true,true);
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
