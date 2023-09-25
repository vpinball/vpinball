//!! always adapt to changes from BasicShader.fx
//!! add playfield color, glossy, etc for more precise reflections of playfield

#define NUM_BALL_LIGHTS 8

#include "Helpers.fxh"

// transformation matrices
const float4x4 matWorldViewProj : WORLDVIEWPROJ;
const float4x4 matWorldView     : WORLDVIEW;
const float4x3 matWorldViewInverse;
//const float4x4 matWorldViewInverseTranspose; // matWorldViewInverse used instead and multiplied from other side
const float4x3 matView;
//const float4x4 matViewInverseInverseTranspose; // matView used instead and multiplied from other side

texture  Texture0; // base texture
texture  Texture3; // decal
texture  Texture4; // playfield

sampler2D tex_ball_color : TEXUNIT0 = sampler_state // base texture
{
    Texture   = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
    //ADDRESSU  = Wrap; //!! ?
    //ADDRESSV  = Wrap;
    SRGBTexture = true;
};

sampler2D tex_ball_decal : TEXUNIT3 = sampler_state // ball decal
{
    Texture   = (Texture3);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    ADDRESSU  = Wrap;
    ADDRESSV  = Wrap;
    SRGBTexture = true;
};

sampler2D tex_ball_playfield : TEXUNIT4 = sampler_state // playfield
{
    Texture   = (Texture4);
    MIPFILTER = LINEAR; //!! ?
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    ADDRESSU  = Wrap;
    ADDRESSV  = Wrap;
    SRGBTexture = true;
};

#include "Material.fxh"

const float4x3 orientation;
const float4   invTableRes_playfield_height_reflection;
const bool     disableLighting;
//const float    reflection_ball_playfield;

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

vout vsBall( const in vin IN )
{
    // apply spinning and move the ball to it's actual position
    float4 pos = IN.position;
    pos.xyz = mul_w1(pos.xyz, orientation);

    // apply spinning to the normals too to get the sphere mapping effect
    const float3 nspin = mul_w0(IN.normal, orientation);
    // Needs to use a 'normal' matrix, and to normalize since we allow non uniform stretching, therefore matWorldView is not orthonormal
    const float3 normal = normalize(mul(matWorldViewInverse, nspin).xyz); // actually: mul(float4(nspin,0.), matWorldViewInverseTranspose), but optimized to save one matrix

    const float3 p = mul_w1(pos.xyz, matWorldView);

    vout OUT;
    OUT.position = mul(pos, matWorldViewProj);
    OUT.normal_t0x = float4(normal,IN.tex0.x);
    OUT.worldPos_t0y = float4(p,IN.tex0.y);
    return OUT;
}

#if 0
voutReflection vsBallReflection( const in vin IN )
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

voutTrail vsBallTrail( const in vin IN )
{
    voutTrail OUT;
    OUT.position = mul(IN.position, matWorldViewProj);
    OUT.tex0_alpha = float3(IN.tex0, IN.normal.x); //!! abuses normal for now

    return OUT;
}

//------------------------------------

float3 ballLightLoop(const float3 pos, const float3 N, const float3 V, float3 diffuse, float3 glossy, const float3 specular, const float edge, const bool is_metal)
{
   // N and V must be already normalized by the caller
   // N = normalize(N);
   // V = normalize(V);

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

   BRANCH if((!is_metal && (diffuseMax > 0.0)) || (glossyMax > 0.0))
   {
      BRANCH if (fDisableLighting_top_below.x == 1.0)
         color += float(iLightPointNum) * diffuse; // Old bug kept for backward compatibility: when lighting is disabled, it results to applying it twice
      else for(int i = 0; i < iLightPointBallsNum; i++)  
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal); // no clearcoat needed as only pointlights so far
   }

   BRANCH if(!is_metal && (diffuseMax > 0.0))
      color += DoEnvmapDiffuse(normalize(mul(matView, N).xyz), diffuse); // trafo back to world for lookup into world space envmap // actually: mul(float4(N, 0.0), matViewInverseInverseTranspose)

   if(specularMax > 0.0)
      color += specular; //!! blend? //!! Fresnel with 1st layer?

   return color;
}

float3 PFDoPointLight(const float3 pos, const float3 N, const float3 diffuse, const int i) 
{
   //!! do in vertex shader?! or completely before?!
   const float3 lightDir = mul_w1(lights[i].vPos, matView) - pos;
   const float3 L = normalize(lightDir);
   const float NdotL = dot(N, L);
   // compute diffuse color (lambert)
   const float3 Out = (NdotL > 0.0) ? diffuse * NdotL : float3(0.0,0.0,0.0);

   const float sqrl_lightDir = dot(lightDir,lightDir); // tweaked falloff to have ranged lightsources
   float fAtten = saturate(1.0 - sqrl_lightDir*sqrl_lightDir/(cAmbient_LightRange.w*cAmbient_LightRange.w*cAmbient_LightRange.w*cAmbient_LightRange.w)); //!! pre-mult/invert cAmbient_LightRange.w?
   fAtten = fAtten*fAtten/(sqrl_lightDir + 1.0);

   return Out * lights[i].vEmission * fAtten;
}

float3 PFlightLoop(const float3 pos, const float3 N, const float3 diffuse)
{
   const float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));

   float3 color = float3(0.0,0.0,0.0);

   BRANCH if (diffuseMax > 0.0)
   {
      for (int i = 0; i < iLightPointNum; i++)
         color += PFDoPointLight(pos, N, diffuse, i);

      color += DoEnvmapDiffuse(float3(0.,0.,1.), diffuse); // directly wire world space playfield normal
   }

   return color;
}



//------------------------------------
// PIXEL SHADER

float4 psBall( const in vout IN, uniform bool equirectangularMap, uniform bool decalMode ) : COLOR
{
    const float3 V = normalize( /*camera=0,0,0,1*/-IN.worldPos_t0y.xyz);
    const float3 N = normalize(IN.normal_t0x.xyz);
    const float3 r = reflect(V, N);

    float3 ballImageColor;
    const float edge = dot(V, r);
    // edge falloff to reduce aliasing on edges (picks smaller mipmap -> more blur)
    const float lod = (edge > 0.6) ? edge*(6.0*1.0/0.4)-(6.0*0.6/0.4) : 0.0;
    BRANCH if (equirectangularMap)
    { // Equirectangular Map Reflections
      // trafo back to world for lookup into world space envmap
      // matView is always an orthonormal matrix, so no need to normalize after transform
      const float3 rv = /*normalize*/(mul(matView, -r).xyz);
      const float2 uv = ray_to_equirectangular_uv(rv);
      ballImageColor = tex2Dlod(tex_ball_color, float4(uv, 0., lod)).rgb;
    }
    else
    { // Spherical Map Reflections
      // calculate the intermediate value for the final texture coords. found here http://www.ozone3d.net/tutorials/glsl_texturing_p04.php
      const float m = (1.0 - r.z > 0.) ? 0.3535533905932737622 * rsqrt(1.0 - r.z) : 0.; // 0.353...=0.5/sqrt(2)
      const float2 uv = float2(0.5 - m * r.x, 0.5 - m * r.y);
      ballImageColor = tex2Dlod(tex_ball_color, float4(uv, 0., lod)).rgb;
    }

    const float4 decalColorT = tex2D(tex_ball_decal, float2(IN.normal_t0x.w, IN.worldPos_t0y.w));
    float3 decalColor = decalColorT.rgb;

    if (!decalMode)
    {
       // decal texture is an alpha scratch texture and must be added to the ball texture
       // the strength of the scratches totally rely on the alpha values.
       decalColor *= decalColorT.a;
       ballImageColor += decalColor;
    }
    else
       ballImageColor = ScreenHDR(ballImageColor, decalColor);

    BRANCH if (disableLighting)
       return float4(ballImageColor,cBase_Alpha.a);

    if (!decalMode)
       ballImageColor *= fenvEmissionScale_TexWidth.x;
    else
       ballImageColor *= 0.5*fenvEmissionScale_TexWidth.x; //!! 0.5=magic

    // No need to use a dedicated 'normal' matrix and normalize here since the matWorldView matrix is normal (world is identity and view is always orthonormal)
    //const float3 playfield_normal = normalize(mul(matWorldViewInverse, float3(0.,0.,1.)).xyz); //!! normalize necessary? // actually: mul(float4(0.,0.,1.,0.), matWorldViewInverseTranspose), but optimized to save one matrix
    //const float3 playfield_normal = mul(float4(0.0, 0.0, 1.0, 0.0), matWorldView).xyz;
    const float3 playfield_normal = float3(matWorldView._31, matWorldView._32, matWorldView._33);
    const float NdotR = dot(playfield_normal,r);

    const float3 playfield_p0 = mul_w1(float3(/*playfield_pos=*/0.,0.,invTableRes_playfield_height_reflection.z), matWorldView);
    const float t = dot(playfield_normal, IN.worldPos_t0y.xyz - playfield_p0) / NdotR;
    const float3 playfield_hit = IN.worldPos_t0y.xyz - t * r;
    const float2 uv = mul_w1(playfield_hit, matWorldViewInverse).xy * invTableRes_playfield_height_reflection.xy;

    // This will break with custom playfield texture coordinates (like Flupper's TOTAN and many others)
    // => Rather use screen space sample from previous frame (it will include the lighting/shadowing but also the ball itself, and would need to account for viewer movement) ?
    float3 playfieldColor = tex2D(tex_ball_playfield, uv).rgb * invTableRes_playfield_height_reflection.w;
    BRANCH if (NdotR <= 0. || t < 0.)
    {
        // t < 0.0 may happen in some situation where ball intersects the playfield (like in kicker)
        playfieldColor = ballImageColor;
    }
    else
    {
        //!! hack to get some lighting on reflection sample, but only diffuse, the rest is not setup correctly anyhow
        playfieldColor = PFlightLoop(playfield_hit, playfield_normal, playfieldColor);

        //!! magic falloff & weight the rest in from the ballImage
        // Before 10.8, used to be: const float weight = NdotR*NdotR; 
        // const float weight = 1.0 / (1.0 + max(0.0, t - 12.5) * 0.01);
        const float weight = smoothstep(t, 0.0, 25.0);
        playfieldColor = lerp(playfieldColor, ballImageColor, weight);
    }

    float3 diffuse = cBase_Alpha.rgb*0.075;

    if(!decalMode)
       diffuse *= decalColor; // scratches make the material more rough
    const float3 glossy = max(diffuse*2.0, float3(0.1,0.1,0.1)); //!! meh
    float3 specular = playfieldColor*cBase_Alpha.rgb; //!! meh, too, as only added in ballLightLoop anyhow

    if(!decalMode)
       specular *= float3(1.,1.,1.)-decalColor; // see above

    float4 result;
    result.rgb = ballLightLoop(IN.worldPos_t0y.xyz, N, V, diffuse, glossy, specular, 1.0, false);
    result.a = cBase_Alpha.a;
    return result;
}

#if 0
float4 psBallReflection( const in voutReflection IN ) : COLOR
{
   const float2 envTex = float2(IN.r.x*0.5 + 0.5, IN.r.y*0.5 + 0.5);
   float3 ballImageColor = tex2D(tex_ball_color, envTex).rgb;
   ballImageColor = (cBase_Alpha.rgb*(0.075*0.25) + ballImageColor)*fenvEmissionScale_TexWidth.x; //!! just add the ballcolor in, this is a whacky reflection anyhow
   float alpha = saturate((IN.tex0.y - position_radius.y) / position_radius.w);
   alpha = (alpha*alpha)*(alpha*alpha)*reflection_ball_playfield;
   return float4(ballImageColor,alpha);
}
#endif

float4 psBallTrail( in voutTrail IN ) : COLOR
{
   const float3 ballImageColor = tex2D(tex_ball_color, IN.tex0_alpha.xy).rgb;
   if (disableLighting)
      return float4(ballImageColor, IN.tex0_alpha.z);
   else
      return float4((cBase_Alpha.rgb*(0.075*0.25) + ballImageColor)*fenvEmissionScale_TexWidth.x, IN.tex0_alpha.z); //!! just add the ballcolor in, this is a whacky trail anyhow
}

//------------------------------------
// Techniques

technique RenderBall
{
	pass p0 
	{
		vertexshader = compile vs_3_0 vsBall();
		pixelshader  = compile ps_3_0 psBall(true, false);
	}
}

technique RenderBall_DecalMode
{
	pass p0
	{
		vertexshader = compile vs_3_0 vsBall();
		pixelshader  = compile ps_3_0 psBall(true, true);
	}
}

technique RenderBall_SphericalMap
{
	pass p0
	{
		vertexshader = compile vs_3_0 vsBall();
		pixelshader = compile ps_3_0 psBall(false, false);
	}
}

technique RenderBall_SphericalMap_DecalMode
{
	pass p0
	{
		vertexshader = compile vs_3_0 vsBall();
		pixelshader = compile ps_3_0 psBall(false, true);
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
